#include "MEpsFast.hpp"

#include <cmath>
// #include <limits>
// #include <iostream>

using std::vector;
using std::isfinite;
using std::numeric_limits;

void epsfast::CalcEpsilonFast(bool merge)
{
    merged_ = 0;

    auto& bottom_nodes = Tree->getBottomNodes();
    static_assert(
        std::is_same<
            decltype(bottom_nodes),
            const vector<TSortedNode*>&
        >::value,
        "unexpected bottom_nodes must be reference"
    );

    for (auto lbnode: bottom_nodes)
    {
        TVec bnode_center = TVec(lbnode->x, lbnode->y);
        TAtt *nearestAtt = nearestBodySegment(*lbnode, bnode_center);

        double merge_criteria_sq;
        if (merge)
        {
            merge_criteria_sq = nearestAtt ?
            // объединять слишком близкие вихри
            // или вихри, у которого ближайшие два соседа имеют другой знак
                0.16 * nearestAtt->dl.abs2() * (1. + (bnode_center - nearestAtt->r).abs())
            // объединять только "свой среди чужих"
                : 0;
        }
        else
        {
            // nan - не объединять вообще никогда
            merge_criteria_sq = std::numeric_limits<double>::quiet_NaN();
        }

        double eps_restriction = nearestAtt ?
            nearestAtt->dl.abs()*(1.0/3.0)
            : 0;

        for (TObj *lobj = lbnode->vRange.first; lobj < lbnode->vRange.last; lobj++)
        {
            if (!lobj->g) continue;
            lobj->_1_eps = 1.0/std::max(epsv(*lbnode, lobj, merge_criteria_sq), eps_restriction);
            //eps is bounded below (cant be less than restriction)
        }

        for (TObj *lobj = lbnode->hRange.first; lobj < lbnode->hRange.last; lobj++)
        {
            if (!lobj->g) continue;
            lobj->_1_eps = 1.0/std::max(epsh(*lbnode, lobj, merge_criteria_sq), eps_restriction);
            //eps is bounded below (cant be less than restriction)
        }
    }
}


double epsfast::eps2h(const TSortedNode &node, TVec p)
{
    double res1 = numeric_limits<double>::infinity();
    double res2 = numeric_limits<double>::infinity();

    for (const TSortedNode* lnnode: *node.NearNodes)
    {
        for (const TObj *lobj = lnnode->vRange.first; lobj < lnnode->vRange.last; lobj++)
        {
            double drabs2 = (p-lobj->r).abs2();
            if ( !drabs2 ) {
                continue;
            } else if ( drabs2 < res1) {
                res2 = res1;
                res1 = drabs2;
            } else if ( drabs2 < res2 ) {
                res2 = drabs2;
            }
        }
    }

    if ( isfinite(res2) )
        return res2;
    if ( isfinite(res1) )
        return res1;
    return numeric_limits<double>::lowest();
}

double epsfast::h2(const TSortedNode &node, TVec p)
{
    double res = numeric_limits<double>::infinity();

    for (const TSortedNode* lnnode: *node.NearNodes)
    {
        for (TObj* latt: lnnode->bllist)
        {
            res = std::min(res, (p-latt->r).abs2());
        }
    }

    return res;
}

/******************************** NAMESPACE ***********************************/

void epsfast::MergeVortexes(TObj *lv1, TObj *lv2)
{
    merged_++;

    if ( sign(*lv1) == sign(*lv2) )
    {
        lv1->r = (lv1->r*lv1->g + lv2->r*lv2->g)/(lv1->g + lv2->g);
    }
    else if ( fabs(lv1->g) < fabs(lv2->g) )
    {
        lv1->r = lv2->r;
    }
    //lv1->v = (lv1->v*lv1->g + lv2->v*lv2->g)/(lv1->g + lv2->g);
    lv1->g+= lv2->g;
    lv2->g = 0;
}

double epsfast::epsv(const TSortedNode &Node, TObj *lv, double merge_criteria_sq)
{
    double res1, res2;
    res2 = res1 = std::numeric_limits<double>::max();

    TObj *lv1, *lv2;
    lv1 = lv2 = NULL;

    for (auto lnnode: *Node.NearNodes)
    {
        for (TObj *lobj = lnnode->vRange.first; lobj < lnnode->vRange.last; lobj++)
        {
            if (!lobj->g || (lv == lobj)) { continue; }
            double drabs2 = (lv->r - lobj->r).abs2();

            if ( res1 > drabs2 )
            {
                res2 = res1; lv2 = lv1;
                res1 = drabs2; lv1 = lobj;
            }
            else if ( res2 > drabs2 )
            {
                res2 = drabs2; lv2 = lobj;
            }
        }
    }

    if ( !lv1 )
        return std::numeric_limits<double>::min();
    if ( !lv2 )
        return sqrt(res1);
    if ( std::isnan(merge_criteria_sq) )
        return sqrt(res2);

    if (
            (res1 < merge_criteria_sq)
            ||
            ( (lv1->sign() == lv2->sign()) && (lv1->sign() != lv->sign()) )
       )
    {
        MergeVortexes(lv, lv1);
        return epsv(Node, lv, std::numeric_limits<double>::quiet_NaN());
    }

    return sqrt(res2);
}

double epsfast::epsh(const TSortedNode &Node, TObj *lv, double merge_criteria_sq)
{
    double res1, res2;
    res2 = res1 = std::numeric_limits<double>::max();

    TObj *lv1 = NULL;

    for (auto lnnode: *Node.NearNodes)
    {
        for (TObj *lobj = lnnode->hRange.first; lobj < lnnode->hRange.last; lobj++)
        {
            if (!lobj->g || (lv == lobj)) { continue; }
            double drabs2 = (lv->r - lobj->r).abs2();

            if ( res1 > drabs2 )
            {
                res2 = res1;
                res1 = drabs2;
                lv1 = lobj;
            }
            else if ( res2 > drabs2 )
            {
                res2 = drabs2;
            }
        }
    }

    if ( res1 == std::numeric_limits<double>::max() ) return std::numeric_limits<double>::min();
    if ( res2 == std::numeric_limits<double>::max() ) return sqrt(res1);

    if (res1 < merge_criteria_sq)
    {
        MergeVortexes(lv, lv1);
        return epsh(Node, lv, std::numeric_limits<double>::quiet_NaN());
    }

    return sqrt(res2);
}

TAtt* epsfast::nearestBodySegment(TSortedNode &Node, TVec p)
{
    TObj *att = NULL;
    double res = std::numeric_limits<double>::max();

    for (TSortedNode* lnnode: *Node.NearNodes)
    {
        // FIXME использовать algorithm
        for (TObj* latt: lnnode->bllist)
        {
            if (!latt) { fprintf(stderr, "epsfast.cpp:%d llatt = NULL. Is it possible?\n", __LINE__ ); continue; }

            double drabs2 = (p - latt->r).abs2();
            if ( drabs2 < res )
            {
                res = drabs2;
                att = latt;
            }
        }
    }

    if (att) return static_cast<TAtt*>(att);

    for (auto& lbody: S->BodyList)
        for (auto lobj = lbody->alist.begin(); lobj < lbody->alist.end(); lobj++)
        {
            double drabs2 = (p - lobj->r).abs2();
            if ( drabs2 < res )
            {
                res = drabs2;
                att = &*lobj;
            }
            else
            {
                lobj+= 9; //speed up
            }
        }

    return att ? static_cast<TAtt*>(att) : NULL;
}

