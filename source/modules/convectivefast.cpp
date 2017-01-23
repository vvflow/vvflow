#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <assert.h>
#include <complex>

using namespace std;

#include "convectivefast.h"
//#define dbg(func) cout << "Doing " << #func << "... " << flush; func; cout << "done\n";
#define dbg(func) func;

#define VAR_U_x 0
#define VAR_U_y 1
#define VAR_U_o 2
#define VAR_FH_x 3
#define VAR_FH_y 4
#define VAR_FH_o 5
#define VAR_FP_x 6
#define VAR_FP_y 7
#define VAR_FP_o 8

/****************************** MAIN FUNCTIONS ********************************/

convectivefast::convectivefast(Space *sS)
{
    S = sS;
}

TVec convectivefast::SpeedSumFast(TVec p)
{
    TVec res(0, 0);
    TSortedNode* Node = S->Tree->findNode(p);
    if (!Node) return res;

    for (TSortedNode* lfnode: *Node->FarNodes)
    {
        res+= BioSavar(lfnode->CMp, p) + BioSavar(lfnode->CMm, p);
    }

    for (auto& lbody: S->BodyList)
    {
        res += BoundaryConvective(*lbody.get(), p);
    }
    res *= C_1_2PI;
    res += SpeedSum(*Node, p);
    res += SrcSpeed(p);
    res += S->InfSpeed();

    return res;
}

    inline
TVec convectivefast::BioSavar(const TObj &obj, const TVec &p)
{
    TVec dr = p - obj.r;
    return rotl(dr)*(obj.g / (dr.abs2() + sqr(1./obj._1_eps)) );
}

TVec convectivefast::SpeedSum(const TSortedNode &Node, const TVec &p)
{
    TVec res(0, 0);

    for (TSortedNode* lnnode: *Node.NearNodes)
    {
        for (TObj *lobj = lnnode->vRange.first; lobj < lnnode->vRange.last; lobj++)
        {
            if (!lobj->g) continue;
            res+= BioSavar(*lobj, p); 
        }
    }

    res *= C_1_2PI;
    return res;
}

TVec convectivefast::SrcSpeed(const TVec &p)
{
    TVec res(0, 0);
    // что бы избежать неустойчивости при использовании стока
    // эпсилон динамически определяется из шага по времени
    // условием устойчивости для точечного стока является V(r)*dt < r
    // отсюда sqr(eps) > g*dt/2pi
    // мы используем sqr(eps) = 2*g*dt/2pi = g*dt/pi
    const double eps2_div_srcg = S->dt * C_1_PI;
    for (const auto& src: S->SourceList)
    {
        TVec dr = p - src.r;
        res += dr*(src.g / (dr.abs2() + eps2_div_srcg*abs(src.g)) );
    }

    res *= C_1_2PI;
    return res;
}

void convectivefast::CalcConvectiveFast()
{
    auto& bnodes = S->Tree->getBottomNodes();

    //FIXME omp here
    for (auto llbnode = bnodes.begin(); llbnode < bnodes.end(); llbnode++)
    {
        const TSortedNode *lbnode = *llbnode;

        double Teilor1, Teilor2, Teilor3, Teilor4;
        Teilor1 = Teilor2 = Teilor3 = Teilor4 = 0;

        for (const TSortedNode* lfnode: *lbnode->FarNodes)
        {
            TVec DistP = TVec(lbnode->x, lbnode->y) - lfnode->CMp.r;
            TVec DistM = TVec(lbnode->x, lbnode->y) - lfnode->CMm.r;

            //			double _1_DistPabs = 1./DistP.abs2();
            //			double _1_DistMabs = 1./DistM.abs2();
            double FuncP1 = lfnode->CMp.g / DistP.abs2(); //Extremely complicated useless variables
            double FuncM1 = lfnode->CMm.g / DistM.abs2();
            double FuncP2 = lfnode->CMp.g / sqr(DistP.abs2());
            double FuncM2 = lfnode->CMm.g / sqr(DistM.abs2());

            Teilor1 -= (FuncP1*DistP.y + FuncM1*DistM.y);
            Teilor2 += (FuncP1*DistP.x + FuncM1*DistM.x);
            Teilor3 += (FuncP2*DistP.y*DistP.x + FuncM2*DistM.y*DistM.x);
            Teilor4 += (FuncP2 * (sqr(DistP.y) - sqr(DistP.x)) + FuncM2 * (sqr(DistM.y) - sqr(DistM.x)));
        }

        Teilor1 *= C_1_2PI;
        Teilor2 *= C_1_2PI;
        Teilor3 *= C_1_PI;
        Teilor4 *= C_1_2PI;

        TVec dr_local, nodeCenter = TVec(lbnode->x, lbnode->y);

    	#pragma omp parallel for schedule(dynamic, 10)
        for (TObj *lobj = lbnode->vRange.first; lobj < lbnode->vRange.last; lobj++)
        {
            if (!lobj->g) {continue;}
            dr_local = lobj->r - nodeCenter;
            lobj->v += S->InfSpeed() + SrcSpeed(lobj->r) + SpeedSum(*lbnode, lobj->r) +
                TVec(Teilor1, Teilor2) +
                TVec(TVec(Teilor3,  Teilor4)*dr_local, TVec(Teilor4, -Teilor3)*dr_local);
        }

    	#pragma omp parallel for schedule(dynamic, 10)
        for (TObj *lobj = lbnode->hRange.first; lobj < lbnode->hRange.last; lobj++)
        {
            if (!lobj->g) {continue;}
            dr_local = lobj->r - nodeCenter;
            lobj->v += S->InfSpeed() + SrcSpeed(lobj->r) + SpeedSum(*lbnode, lobj->r) +
                TVec(Teilor1, Teilor2) +
                TVec(TVec(Teilor3,  Teilor4)*dr_local, TVec(Teilor4, -Teilor3)*dr_local);
        }

    	#pragma omp parallel for schedule(dynamic, 10)
        for (TObj *lobj = lbnode->sRange.first; lobj < lbnode->sRange.last; lobj++)
        {
            dr_local = lobj->r - nodeCenter;
            lobj->v += S->InfSpeed() + SrcSpeed(lobj->r) + SpeedSum(*lbnode, lobj->r) +
                TVec(Teilor1, Teilor2) +
                TVec(TVec(Teilor3,  Teilor4)*dr_local, TVec(Teilor4, -Teilor3)*dr_local);
        }
    }
}

void convectivefast::CalcBoundaryConvective()
{
    for (auto& lbody: S->BodyList)
    {
        bool calcBC = !lbody->speed_slae.iszero();
        bool calcBCS = false;
        for (auto& latt: lbody->alist) { if (latt.slip) {calcBCS = true; break;} }

        if (!calcBC && !calcBCS) continue;

#pragma omp parallel for
        for (auto lobj = S->VortexList.begin(); lobj < S->VortexList.end(); lobj++)
        {
            if (calcBC) lobj->v += BoundaryConvective(*lbody, lobj->r)*C_1_2PI;
            if (calcBCS) lobj->v += BoundaryConvectiveSlip(*lbody, lobj->r)*C_1_2PI;
        }

#pragma omp parallel for
        for (auto lobj = S->HeatList.begin(); lobj < S->HeatList.end(); lobj++)
        {
            if (calcBC) lobj->v += BoundaryConvective(*lbody, lobj->r)*C_1_2PI;
            if (calcBCS) lobj->v += BoundaryConvectiveSlip(*lbody, lobj->r)*C_1_2PI;
        }

#pragma omp parallel for
        for (auto lobj = S->StreakList.begin(); lobj < S->StreakList.end(); lobj++)
        {
            if (calcBC) lobj->v += BoundaryConvective(*lbody, lobj->r)*C_1_2PI;
            if (calcBCS) lobj->v += BoundaryConvectiveSlip(*lbody, lobj->r)*C_1_2PI;
        }
    }
}

TVec convectivefast::BoundaryConvective(const TBody &b, const TVec &p)
{
    TVec res = TVec(0, 0);

    for (auto& latt: b.alist)
    {
        double drabs2 = (p-latt.r).abs2();
        if (drabs2 < latt.dl.abs2())
        {
            TVec Vs1 = b.speed_slae.r + b.speed_slae.o * rotl(latt.corner - b.get_axis());
            double g1 = -Vs1 * latt.dl;
            double q1 = -rotl(Vs1) * latt.dl; 

            TVec Vs2 = b.speed_slae.r + b.speed_slae.o * rotl(latt.corner + latt.dl - b.get_axis());
            double g2 = -Vs2 * latt.dl;
            double q2 = -rotl(Vs2) * latt.dl;

            res+= (rotl(SegmentInfluence_linear_source(p, latt, g1, g2)) + SegmentInfluence_linear_source(p, latt, q1, q2));
        } else
        {
            TVec Vs = b.speed_slae.r + b.speed_slae.o * rotl(latt.r - b.get_axis());
            double g = -Vs * latt.dl;
            double q = -rotl(Vs) * latt.dl; 
            TVec dr = p - latt.r;
            res += (dr*q + rotl(dr)*g) / drabs2;
        }
        //res+= SegmentInfluence(p, *latt, latt->g, latt->q, 1E-6);
    }

    return res;
}


TVec convectivefast::BoundaryConvectiveSlip(const TBody &b, const TVec &p)
{
    TVec dr, res = TVec(0, 0);

    for (auto& latt: b.alist)
    {
        if (latt.slip) res += BioSavar(latt, p);
    }

    return res;
}


bool convectivefast::canUseInverse()
{
    //Algorithm:

    for (auto& lbody1: S->BodyList)
    {
        if (!lbody1->speed(S->Time).iszero()) return false;
        if (lbody1->kspring.r.x >= 0) return false;
        if (lbody1->kspring.r.y >= 0) return false;
        if (lbody1->kspring.o >= 0) return false;
        if (lbody1->collision_state) return false;
    }

    return true;

    /*
    // if Nb <= 1 -> use_inverse=TRUE
    // else check bodies
    //     if any kx, ky, ka >= 0 -> use_inverse=FALSE
    //     else if for any pair of bodies (Vo!=0) and (Vo or R*) differ -> use_inverse=FALSE
    //     else if for any pair of bodies Vx or Vy differ -> use_inverse=FALSE
    //     else use_inverse=TRUE
    if (S->BodyList->size() <= 1) return true;

    const_for(S->BodyList, llbody1)
    {
    TBody *lbody1 = *llbody1;

    if (lbody1->k.r.x >= 0) return false;
    if (lbody1->k.r.y >= 0) return false;
    if (lbody1->k.o >= 0) return false;

    const_for(S->BodyList, llbody2)
    {
    TBody *lbody2 = *llbody2;
    if ((lbody1->getSpeed().o!=0) || (lbody2->getSpeed().o!=0))
    {
    if (lbody1->getSpeed().o != lbody2->getSpeed().o) return false;
    if ((lbody1->pos.r+lbody1->dPos.r) != (lbody2->pos.r+lbody2->dPos.r)) return false;
    }

    if (lbody1->getSpeed().r != lbody2->getSpeed().r) return false;
    }
    }*/

    return true;
}

void convectivefast::CalcCirculationFast()
{
    bool use_inverse = canUseInverse() && S->Time>0;

    matrix.resize(S->TotalSegmentsCount()+S->BodyList.size()*9);
    if (matrix.bodyMatrixIsOk())
        FillMatrix(true);
    else
        FillMatrix(false);

    matrix.solveUsingInverseMatrix(use_inverse);
    for (auto& libody: S->BodyList)
    {
        if (libody->collision_state)
            matrix.spoilMatrix();
        libody->collision_state = 0;
    }

}

static inline double _2PI_Xi_g_near(TVec p, TVec pc, TVec dl, double rd)
{
    // pc - center of segment (seg.r)
    return (pc-p)*dl/sqr(rd);
}

static inline double _2PI_Xi_g_dist(TVec p, TVec p1, TVec p2)
{
    return 0.5*log( (p-p2).abs2() / (p-p1).abs2() );
}

double convectivefast::_2PI_Xi_g(TVec p, const TAtt &seg, double rd) // in doc 2\pi\Xi_\gamma (1.7)
{
    double rd_sqr = sqr(rd);
    double dr1_sqr = (p-seg.corner).abs2();
    double dr2_sqr = (p-seg.corner-seg.dl).abs2();
    if ( dr1_sqr>=rd_sqr && dr2_sqr>=rd_sqr )
        return 0.5*log(dr2_sqr/dr1_sqr);
    else if ( dr1_sqr<=rd_sqr && dr2_sqr<=rd_sqr )
        return _2PI_Xi_g_near(p, seg.r, seg.dl, rd);
    else
    {
        double a0 = seg.dl.abs2();
        double b0 = (p-seg.r)*seg.dl;
        double d  = sqrt(b0*b0-a0*((p-seg.r).abs2()-rd*rd));
        double k  = (b0+d)/a0; if ((k<=-0.5)||(k>=0.5)) k = (b0-d)/a0;
        TVec p3 = seg.r + k*seg.dl;

        if ( dr1_sqr<rd_sqr )
            return _2PI_Xi_g_near(p, 0.5*(p3+seg.corner), (p3-seg.corner), rd) +
                _2PI_Xi_g_dist(p, p3, seg.corner+seg.dl);
        else
            return _2PI_Xi_g_dist(p, seg.corner, p3) +
                _2PI_Xi_g_near(p, 0.5*(seg.corner+seg.dl+p3), (seg.corner+seg.dl-p3), rd);
    }
}

static inline double _2PI_Xi_q_near(TVec p, TVec pc, TVec dl, double rd)
{
    return (pc-p)*rotl(dl)/sqr(rd);
}

static inline double _2PI_Xi_q_dist(TVec p, TVec p1, TVec p2)
{
    TVec dp1 = p-p1;
    TVec dp2 = p-p2;
    return atan2(dp1*rotl(dp2), dp1*dp2);
}

TVec convectivefast::_2PI_Xi(const TVec &p, const TAtt &seg, double rd)
{
    if (&p == &seg.r) { TVec pnew = p+rotl(seg.dl)*0.001; return _2PI_Xi(pnew, seg, rd); }

    double rd_sqr = sqr(rd);
    TVec p1 = seg.corner;
    TVec p2 = seg.corner+seg.dl;
    double dr1_sqr = (p-p1).abs2();
    double dr2_sqr = (p-p2).abs2();
    if ( dr1_sqr>=rd_sqr && dr2_sqr>=rd_sqr )
    {
        TVec dp1 = p-p1;
        TVec dp2 = p-p2;
        return TVec(
                0.5*log(dr2_sqr/dr1_sqr),
                atan2(dp1*rotl(dp2), dp1*dp2)
                );
    }
    else if ( dr1_sqr<=rd_sqr && dr2_sqr<=rd_sqr )
    {
        TVec dp = p-seg.r;
        return TVec(dp*seg.dl, dp*rotl(seg.dl))/sqr(rd);
    }
    else
    {
        double a0 = seg.dl.abs2();
        double b0 = (p-seg.r)*seg.dl;
        double d  = sqrt(b0*b0-a0*((p-seg.r).abs2()-rd*rd));
        double k  = (b0+d)/a0; if ((k<=-0.5)||(k>=0.5)) k = (b0-d)/a0;
        TVec p3 = seg.r + k*seg.dl;

        if ( dr1_sqr<rd_sqr )
            return
                TVec(
                        _2PI_Xi_g_near(p, 0.5*(p1+p3), p3-p1, rd),
                        _2PI_Xi_q_near(p, 0.5*(p1+p3), p3-p1, rd)) +
                TVec(
                        _2PI_Xi_g_dist(p, p3, p2),
                        _2PI_Xi_q_dist(p, p3, p2));
        else
            return
                TVec(
                        _2PI_Xi_g_dist(p, p1, p3),
                        _2PI_Xi_q_dist(p, p1, p3)) +
                TVec(
                        _2PI_Xi_g_near(p, 0.5*(p3+p2), p2-p3, rd),
                        _2PI_Xi_q_near(p, 0.5*(p3+p2), p2-p3, rd));

    }
}

void convectivefast::_2PI_A123(const TAtt &seg, const TBody* ibody, const TBody &b, double *_2PI_A1, double *_2PI_A2, double *_2PI_A3)
{
    *_2PI_A1 = (ibody == &b)?  C_2PI * seg.dl.y : 0;
    *_2PI_A2 = (ibody == &b)? -C_2PI * seg.dl.x : 0;
    *_2PI_A3 = 0;
    if ((b.kspring.o<0) && (!b.speed_o.getValue(S->Time))) 
    {
        //FIXME econome time. uncomment return
        //fprintf(stderr, "ret:\t%lf\t%lf\n", seg.corner.rx, seg.corner.ry);
        return;
    }
    for (auto& latt: b.alist)
    {
        TVec Xi = _2PI_Xi(latt.r, seg, latt.dl.abs()*0.25);
        TVec r0 = latt.r - b.get_axis();
        //		*A1 -= Xi*latt.dl;
        //		*A2 -= rotl(Xi)*latt.dl;
        *_2PI_A3 -= Xi * TVec(rotl(r0) * latt.dl, -latt.dl*r0);
    }
}

double convectivefast::NodeInfluence(const TSortedNode &Node, const TAtt &seg)
{
    double res = 0;

    for (TSortedNode* lnnode: *Node.NearNodes)
    {
        for (TObj *lobj = lnnode->vRange.first; lobj < lnnode->vRange.last; lobj++)
        {
            if (!lobj->g) {continue;}
            res+= _2PI_Xi_g(lobj->r, seg, 1./lobj->_1_eps) * lobj->g;
        }
    }

    for (TSortedNode* lfnode: *Node.FarNodes)
    {
        res+= _2PI_Xi_g_dist(lfnode->CMp.r, seg.corner, seg.corner+seg.dl) * lfnode->CMp.g;
        res+= _2PI_Xi_g_dist(lfnode->CMm.r, seg.corner, seg.corner+seg.dl) * lfnode->CMm.g;
    }

    return res*C_1_2PI;
}

double convectivefast::AttachInfluence(const TAtt &seg, double rd)
{
    double res = 0;

    for (auto& lbody: S->BodyList)
    {
        double res_tmp = 0;
        for (auto& latt: lbody->alist)
        {
            TVec Vs = lbody->speed_slae.r + lbody->speed_slae.o * rotl(latt.r - lbody->get_axis());
            if (&latt == &seg) { res_tmp+= -(-rotl(Vs) * latt.dl)*0.5*C_2PI; continue; }
            TVec Xi = _2PI_Xi(latt.r, seg, rd);
            res+= Xi.x * (-Vs * latt.dl);
            res+= Xi.y * (-rotl(Vs) * latt.dl);
        }
    }

    return res * C_1_2PI;
}

TVec convectivefast::SegmentInfluence_linear_source(TVec p, const TAtt &seg, double q1, double q2)
{
    //	cerr << "orly?" << endl;
    complex<double> z(p.x, p.y);
    complex<double> zc(seg.r.x, seg.r.y);
    complex<double> dz(seg.dl.x, seg.dl.y);
    complex<double> zs1 = zc-dz*0.5;
    complex<double> zs2 = zc+dz*0.5;
    complex<double> z1 = z - zs1;
    complex<double> z2 = z - zs2;
    complex<double> zV;
    //complex<double> i(0,1);

    //cerr << z2 << "\t" << z1 << endl;
    zV = ( (q2-q1) - conj(q2*z1 - q1*z2)/conj(dz)*log(conj(z2)/conj(z1))) / conj(dz);
    //cerr << zV << endl;
    return TVec(zV.real(), zV.imag());
}

void convectivefast::fillSlipEquationForSegment(TAtt* seg, TBody* ibody, bool rightColOnly)
{
    const int seg_eq_no = seg->eq_no; //equation number for current segment

    //right column
    //influence of infinite speed
    *matrix.rightColAtIndex(seg_eq_no) = rotl(S->InfSpeed())*seg->dl;
    *matrix.rightColAtIndex(seg_eq_no) += rotl(SrcSpeed(seg->r))*seg->dl;
    //influence of all free vortices
    TSortedNode* Node = S->Tree->findNode(seg->r);
    *matrix.rightColAtIndex(seg_eq_no) -= NodeInfluence(*Node, *seg);
    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(seg_eq_no) = &seg->g;

    for (auto& ljbody: S->BodyList)
    {
        for (auto& latt: ljbody->alist)
        {
            *matrix.objectAtIndex(seg_eq_no, latt.eq_no) = _2PI_Xi_g(latt.corner, *seg, latt.dl.abs()*0.25)*C_1_2PI;
        }

        double _2PI_A1, _2PI_A2, _2PI_A3;
        _2PI_A123(*seg, ibody, *ljbody, &_2PI_A1, &_2PI_A2, &_2PI_A3);
        *matrix.objectAtIndex(seg_eq_no, ljbody->eq_forces_no+VAR_U_x) = _2PI_A1*C_1_2PI;
        *matrix.objectAtIndex(seg_eq_no, ljbody->eq_forces_no+VAR_U_y) = _2PI_A2*C_1_2PI;
        *matrix.objectAtIndex(seg_eq_no, ljbody->eq_forces_no+VAR_U_o) = _2PI_A3*C_1_2PI;

#undef jbody
    }
}

void convectivefast::fillZeroEquationForSegment(TAtt* seg, TBody* ibody, bool rightColOnly)
{
    const int seg_eq_no = seg->eq_no; //equation number for current segment

    //right column
    *matrix.rightColAtIndex(seg_eq_no) = 0;
    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(seg_eq_no) = &seg->g;

    //the only non-zero value
    *matrix.objectAtIndex(seg_eq_no, seg_eq_no) = 1;
}

void convectivefast::fillSteadyEquationForSegment(TAtt* seg, TBody* ibody, bool rightColOnly)
{
    const int seg_eq_no = seg->eq_no; //equation number for current segment

    //right column
    *matrix.rightColAtIndex(seg_eq_no) = ibody->g_dead + 2*ibody->get_area()*ibody->speed_slae_prev.o;
    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(seg_eq_no) = &seg->g;

    // self
    {
        for (auto& latt: ibody->alist)
            *matrix.objectAtIndex(seg_eq_no, latt.eq_no) = 1;
        *matrix.objectAtIndex(seg_eq_no, ibody->eq_forces_no+VAR_U_o) = 2*ibody->get_area();
    }
}

void convectivefast::fillInfSteadyEquationForSegment(TAtt* seg, TBody* ibody, bool rightColOnly)
{
    const int seg_eq_no = seg->eq_no; //equation number for current segment

    //right column
    *matrix.rightColAtIndex(seg_eq_no) = -S->gsum() - S->InfCirculation;
    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(seg_eq_no) = &seg->g;

    // all
    // FIXME везде, где используется auto напихать статик асертов
    for (auto& ljbody: S->BodyList)
    {
        for (auto& latt: ljbody->alist)
            *matrix.objectAtIndex(seg_eq_no, latt.eq_no) = 1;
        *matrix.objectAtIndex(seg_eq_no, ljbody->eq_forces_no+VAR_U_o) = 2*ljbody->get_area();
    }

}

//   888    888 Y88b   d88P 8888888b.  8888888b.   .d88888b.
//   888    888  Y88b d88P  888  "Y88b 888   Y88b d88P" "Y88b
//   888    888   Y88o88P   888    888 888    888 888     888
//   8888888888    Y888P    888    888 888   d88P 888     888
//   888    888     888     888    888 8888888P"  888     888
//   888    888     888     888    888 888 T88b   888     888
//   888    888     888     888  .d88P 888  T88b  Y88b. .d88P
//   888    888     888     8888888P"  888   T88b  "Y88888P"

void convectivefast::fillHydroXEquation(TBody* ibody, bool rightColOnly)
{
    const int eq_no = ibody->eq_forces_no+VAR_FH_x;
    const double _1_dt = 1/S->dt;
    const TVec r_c_com = ibody->get_com() - ibody->get_axis();

    //right column
    *matrix.rightColAtIndex(eq_no) =
        + ibody->get_area()*_1_dt*ibody->speed_slae_prev.r.x
        + ibody->get_area()*_1_dt*rotl(2*ibody->get_com()+r_c_com).x * ibody->speed_slae_prev.o
        // - (-ibody->speed_slae_prev.r.y) * ibody->speed_slae_prev.o * ibody->get_area()
        + sqr(ibody->speed_slae_prev.o) * ibody->get_area() * r_c_com.x
        + ibody->fdt_dead.r.x*_1_dt;

    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(eq_no) = &ibody->force_hydro.r.x;

    // self
    {
        for (auto& latt: ibody->alist)
            *matrix.objectAtIndex(eq_no, latt.eq_no) = _1_dt * (-latt.corner.y);

        // speed_slae
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_x) = ibody->get_area()*_1_dt;
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_y) = 0;
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_o) = ibody->get_area()*_1_dt * (-2*ibody->get_com().y - r_c_com.y);

        // force_hydro
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_FH_x) = -1;
    }
}

void convectivefast::fillHydroYEquation(TBody* ibody, bool rightColOnly)
{
    const int eq_no = ibody->eq_forces_no+VAR_FH_y;
    const double _1_dt = 1/S->dt;
    const TVec r_c_com = ibody->get_com() - ibody->get_axis();

    //right column
    *matrix.rightColAtIndex(eq_no) =
        + ibody->get_area()*_1_dt*ibody->speed_slae_prev.r.y
        + ibody->get_area()*_1_dt*rotl(2*ibody->get_com()+r_c_com).y * ibody->speed_slae_prev.o
        // - (ibody->speed_slae_prev.r.x) * ibody->speed_slae_prev.o * ibody->get_area()
        + sqr(ibody->speed_slae_prev.o) * ibody->get_area() * r_c_com.y
        + ibody->fdt_dead.r.y*_1_dt;
    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(eq_no) = &ibody->force_hydro.r.y;

    // self
    {
        for (auto& latt: ibody->alist)
            *matrix.objectAtIndex(eq_no, latt.eq_no) = _1_dt * (latt.corner.x);

        // Speed_slae
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_x) = 0;
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_y) = ibody->get_area()*_1_dt;
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_o) = ibody->get_area()*_1_dt * (2*ibody->get_com().x + r_c_com.x);

        // force_hydro
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_FH_y) = -1;
    }
}

void convectivefast::fillHydroOEquation(TBody* ibody, bool rightColOnly)
{
    const int eq_no = ibody->eq_forces_no+VAR_FH_o;
    const double _1_dt = 1/S->dt;
    const double _1_2dt = 0.5/S->dt;
    const TVec r_c_com = ibody->get_com() - ibody->get_axis();

    //right column
    *matrix.rightColAtIndex(eq_no) =
        + ibody->get_area()*_1_dt * rotl(r_c_com) * ibody->speed_slae_prev.r
        + 2*ibody->get_moi_c()*_1_dt * ibody->speed_slae_prev.o
        // - (r_c_com * ibody->speed_slae_prev.r) * ibody->speed_slae_prev.o * ibody->get_area()
        + ibody->fdt_dead.o*_1_2dt;
    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(eq_no) = &ibody->force_hydro.o;

    // self
    {
        for (auto& latt: ibody->alist)
            *matrix.objectAtIndex(eq_no, latt.eq_no) = _1_2dt * (latt.corner - ibody->get_axis()).abs2();

        // Speed_slae
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_x) = ibody->get_area()*_1_dt * (-r_c_com.y);
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_y) = ibody->get_area()*_1_dt * (r_c_com.x);
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_o) = 2*ibody->get_moi_c()*_1_dt;

        // force_hydro
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_FH_o) = -1;
    }
}

//   888b    888 8888888888 888       888 88888888888  .d88888b.  888b    888
//   8888b   888 888        888   o   888     888     d88P" "Y88b 8888b   888
//   88888b  888 888        888  d8b  888     888     888     888 88888b  888
//   888Y88b 888 8888888    888 d888b 888     888     888     888 888Y88b 888
//   888 Y88b888 888        888d88888b888     888     888     888 888 Y88b888
//   888  Y88888 888        88888P Y88888     888     888     888 888  Y88888
//   888   Y8888 888        8888P   Y8888     888     Y88b. .d88P 888   Y8888
//   888    Y888 8888888888 888P     Y888     888      "Y88888P"  888    Y888

void convectivefast::fillNewtonXEquation(TBody* ibody, bool rightColOnly)
{
    const int eq_no = ibody->eq_forces_no+VAR_FP_x;
    const double _1_dt = 1/S->dt;
    const TVec r_c_com = ibody->get_com() - ibody->get_axis();

    //right column
    *matrix.rightColAtIndex(eq_no) =
        - ibody->get_area()*_1_dt*ibody->density * ibody->speed_slae_prev.r.x
        + ibody->get_area()*_1_dt*ibody->density * r_c_com.y * ibody->speed_slae_prev.o
        + ibody->get_area()*ibody->density * r_c_com.x * sqr(ibody->speed_slae_prev.o)
        - (ibody->density-1.0) * S->gravitation.x * ibody->get_area()
        - ibody->friction_prev.r.x;
    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(eq_no) = &ibody->force_holder.r.x;

    // self
    {
        // Speed_slae
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_x) = -ibody->get_area()*_1_dt*ibody->density;
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_y) = 0;
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_o) = ibody->get_area()*_1_dt*ibody->density*r_c_com.y;
        // force_hydro
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_FH_x) = 1;
        // force_holder
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_FP_x) = -1;
    }

    // children
    for (auto& ljbody: S->BodyList)
    {
        if (ljbody->root_body.lock().get() != ibody) continue;

        // force_holder
        *matrix.objectAtIndex(eq_no, ljbody->eq_forces_no+VAR_FP_x) = 1;
    }
}

void convectivefast::fillNewtonYEquation(TBody* ibody, bool rightColOnly)
{
    const int eq_no = ibody->eq_forces_no+VAR_FP_y;
    const double _1_dt = 1/S->dt;
    const TVec r_c_com = ibody->get_com() - ibody->get_axis();

    //right column
    *matrix.rightColAtIndex(eq_no) =
        - ibody->get_area()*_1_dt*ibody->density * ibody->speed_slae_prev.r.y
        - ibody->get_area()*_1_dt*ibody->density * r_c_com.x * ibody->speed_slae_prev.o
        + ibody->get_area()*ibody->density * r_c_com.y * sqr(ibody->speed_slae_prev.o)
        - (ibody->density-1.0) * S->gravitation.y * ibody->get_area()
        - ibody->friction_prev.r.y;
    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(eq_no) = &ibody->force_holder.r.y;

    // self
    {
        // Speed_slae
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_x) = 0;
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_y) = -ibody->get_area()*_1_dt*ibody->density;
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_o) = -ibody->get_area()*_1_dt*ibody->density*r_c_com.x;
        // force_hydro
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_FH_y) = 1;
        // force_holder
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_FP_y) = -1;
    }

    // children
    for (auto& ljbody: S->BodyList)
    {
        if (ljbody->root_body.lock().get() != ibody) continue;

        // force_holder
        *matrix.objectAtIndex(eq_no, ljbody->eq_forces_no+VAR_FP_y) = 1;
    }
}

void convectivefast::fillNewtonOEquation(TBody* ibody, bool rightColOnly)
{
    const int eq_no = ibody->eq_forces_no+VAR_FP_o;
    const double _1_dt = 1/S->dt;
    const TVec r_c_com = ibody->get_com() - ibody->get_axis();

    //right column
    *matrix.rightColAtIndex(eq_no) =
        - ibody->get_area()*_1_dt*ibody->density * (rotl(r_c_com)*ibody->speed_slae_prev.r)
        - ibody->get_moi_c()*_1_dt*ibody->density * ibody->speed_slae_prev.o
        - (ibody->density-1.0) * (rotl(r_c_com)*S->gravitation) * ibody->get_area()
        - ibody->friction_prev.o;
    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(eq_no) = &ibody->force_holder.o;

    // self
    {
        // Speed_slae
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_x) = ibody->get_area()*_1_dt*ibody->density * r_c_com.y;
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_y) = -ibody->get_area()*_1_dt*ibody->density * r_c_com.x;
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_o) = -ibody->get_moi_c()*_1_dt*ibody->density;
        // force_hydro
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_FH_o) = 1;
        // force_holder
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_FP_o) = -1;
    }

    // children
    for (auto& ljbody: S->BodyList)
    {
        if (ljbody->root_body.lock().get() != ibody) continue;

        // force_holder
        TVec r_child_c = ljbody->get_axis() - ibody->get_axis();
        *matrix.objectAtIndex(eq_no, ljbody->eq_forces_no+VAR_FP_x) = -r_child_c.y;
        *matrix.objectAtIndex(eq_no, ljbody->eq_forces_no+VAR_FP_y) = r_child_c.x;
        *matrix.objectAtIndex(eq_no, ljbody->eq_forces_no+VAR_FP_o) = 1;
    }
}

void convectivefast::fillCollisionOEquation(TBody* ibody)
{
    const int eq_no = ibody->eq_forces_no+VAR_FP_o;

    if (!ibody->collision_state)
    {
        fprintf(stderr, "Trying to fill collision equetion without the need\n");
        exit(-2);
    }
    else if (ibody->collision_state>0)
    {
        *matrix.rightColAtIndex(eq_no) =
            ibody->collision_max.o
            -ibody->holder.o
            -ibody->dpos.o;
        *matrix.rightColAtIndex(eq_no) /= S->dt;
    }
    else if (ibody->collision_state<0)
    {
        *matrix.rightColAtIndex(eq_no) =
            ibody->holder.o+
            ibody->dpos.o
            -ibody->collision_min.o;
        *matrix.rightColAtIndex(eq_no) /= S->dt;
    }

    //place solution pointer
    *matrix.solutionAtIndex(eq_no) = &ibody->force_holder.o;

    // self
    {
        // speed_slae
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_o) = 1;
    }
}

//   888    888  .d88888b.   .d88888b.  888    d8P  8888888888
//   888    888 d88P" "Y88b d88P" "Y88b 888   d8P   888
//   888    888 888     888 888     888 888  d8P    888
//   8888888888 888     888 888     888 888d88K     8888888
//   888    888 888     888 888     888 8888888b    888
//   888    888 888     888 888     888 888  Y88b   888
//   888    888 Y88b. .d88P Y88b. .d88P 888   Y88b  888
//   888    888  "Y88888P"   "Y88888P"  888    Y88b 8888888888

void convectivefast::fillHookeXEquation(TBody* ibody, bool rightColOnly)
{
    const int eq_no = ibody->eq_forces_no+VAR_U_x;

    *matrix.rightColAtIndex(eq_no) = ibody->kspring.r.x * ibody->dpos.r.x;
    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(eq_no) = &ibody->speed_slae.r.x;

    // self
    {
        // speed_slae
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_x) = ibody->damping.r.x;
        // force_holder
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_FP_x) = 1;
    }
}

void convectivefast::fillHookeYEquation(TBody* ibody, bool rightColOnly)
{
    const int eq_no = ibody->eq_forces_no+VAR_U_y;

    *matrix.rightColAtIndex(eq_no) = ibody->kspring.r.y * ibody->dpos.r.y;
    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(eq_no) = &ibody->speed_slae.r.y;

    // self
    {
        // speed_slae
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_y) = ibody->damping.r.y;
        // force_holder
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_FP_y) = 1;
    }
}

void convectivefast::fillHookeOEquation(TBody* ibody, bool rightColOnly)
{
    const int eq_no = ibody->eq_forces_no+VAR_U_o;

    *matrix.rightColAtIndex(eq_no) = ibody->kspring.o * ibody->dpos.o;
    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(eq_no) = &ibody->speed_slae.o;

    // self
    {
        // speed_slae
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_o) = ibody->damping.o;
        // force_holder
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_FP_o) = 1;
    }
}

//    .d8888b.  8888888b.  8888888888 8888888888 8888888b.
//   d88P  Y88b 888   Y88b 888        888        888  "Y88b
//   Y88b.      888    888 888        888        888    888
//    "Y888b.   888   d88P 8888888    8888888    888    888
//       "Y88b. 8888888P"  888        888        888    888
//         "888 888        888        888        888    888
//   Y88b  d88P 888        888        888        888  .d88P
//    "Y8888P"  888        8888888888 8888888888 8888888P"

void convectivefast::fillSpeedXEquation(TBody* ibody, bool rightColOnly)
{
    const int eq_no = ibody->eq_forces_no+VAR_U_x;

    *matrix.rightColAtIndex(eq_no) = ibody->speed_x.getValue(S->Time);
    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(eq_no) = &ibody->speed_slae.r.x;

    // self
    {
        // speed_slae
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_x) = 1;
    }

    // root
    auto root_body = ibody->root_body.lock();
    if (root_body)
    {
        *matrix.objectAtIndex(eq_no, root_body->eq_forces_no+VAR_U_x) = -1;
        *matrix.objectAtIndex(eq_no, root_body->eq_forces_no+VAR_U_y) = 0;
        *matrix.objectAtIndex(eq_no, root_body->eq_forces_no+VAR_U_o) = (ibody->holder.r - root_body->get_axis()).y;
    }
}

void convectivefast::fillSpeedYEquation(TBody* ibody, bool rightColOnly)
{
    const int eq_no = ibody->eq_forces_no+VAR_U_y;

    *matrix.rightColAtIndex(eq_no) = ibody->speed_y.getValue(S->Time);
    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(eq_no) = &ibody->speed_slae.r.y;

    // self
    {
        // speed_slae
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_y) = 1;
    }

    // root
    auto root_body = ibody->root_body.lock();
    if (root_body)
    {
        *matrix.objectAtIndex(eq_no, root_body->eq_forces_no+VAR_U_x) = 0;
        *matrix.objectAtIndex(eq_no, root_body->eq_forces_no+VAR_U_y) = -1;
        *matrix.objectAtIndex(eq_no, root_body->eq_forces_no+VAR_U_o) = -(ibody->get_axis() - root_body->get_axis()).x;
    }
}

void convectivefast::fillSpeedOEquation(TBody* ibody, bool rightColOnly)
{
    const int eq_no = ibody->eq_forces_no+VAR_U_o;

    *matrix.rightColAtIndex(eq_no) = ibody->speed_o.getValue(S->Time);
    if (rightColOnly) return;

    //place solution pointer
    *matrix.solutionAtIndex(eq_no) = &ibody->speed_slae.o;

    // self
    {
        // speed_slae
        *matrix.objectAtIndex(eq_no, ibody->eq_forces_no+VAR_U_o) = 1;
    }

    // root
    auto root_body = ibody->root_body.lock();
    if (root_body)
    {
        *matrix.objectAtIndex(eq_no, root_body->eq_forces_no+VAR_U_o) = -1;
    }
}

//   888b     d888        d8888 88888888888 8888888b.  8888888 Y88b   d88P
//   8888b   d8888       d88888     888     888   Y88b   888    Y88b d88P
//   88888b.d88888      d88P888     888     888    888   888     Y88o88P
//   888Y88888P888     d88P 888     888     888   d88P   888      Y888P
//   888 Y888P 888    d88P  888     888     8888888P"    888      d888b
//   888  Y8P  888   d88P   888     888     888 T88b     888     d88888b
//   888   "   888  d8888888888     888     888  T88b    888    d88P Y88b
//   888       888 d88P     888     888     888   T88b 8888888 d88P   Y88b

void convectivefast::FillMatrix(bool rightColOnly)
{
    if (!rightColOnly) matrix.fillWithZeros();

    bool special_body = true;
    for (auto& libody: S->BodyList)
    {
        TAtt *special_segment = &libody->alist[libody->special_segment_no];
#pragma omp parallel for
        for (auto latt = libody->alist.begin(); latt < libody->alist.end(); latt++)
        {
            if (&*latt != special_segment)
                fillSlipEquationForSegment(&*latt, libody.get(), rightColOnly);
            else
            {
                if (libody->boundary_condition == bc_t::kutta)
                    fillZeroEquationForSegment(&*latt, libody.get(), rightColOnly);
                else if (!special_body)
                    fillSteadyEquationForSegment(&*latt, libody.get(), rightColOnly);
                else
                {
                    fillInfSteadyEquationForSegment(&*latt, libody.get(), rightColOnly);
                    special_body = false;
                }
            }
        }

        fillHydroXEquation(libody.get(), rightColOnly);
        fillHydroYEquation(libody.get(), rightColOnly);
        fillHydroOEquation(libody.get(), rightColOnly);

        fillNewtonXEquation(libody.get(), rightColOnly);
        fillNewtonYEquation(libody.get(), rightColOnly);

        if (!libody->collision_state)
            fillNewtonOEquation(libody.get(), rightColOnly);
        else if (abs(libody->collision_state) == 1)
        {
            fillNewtonOEquation(libody.get(), rightColOnly);
            const int eq_no = libody->eq_forces_no+VAR_FP_o;
            *matrix.rightColAtIndex(eq_no) +=
                (1+libody->bounce)
                *libody->get_moi_c()
                *libody->density
                *libody->speed_slae.o/S->dt;
        }
        else if (abs(libody->collision_state) == 2)
        {
            fillCollisionOEquation(libody.get());
            matrix.spoilInverseMatrix();
        }

        if (libody->kspring.r.x >= 0 && S->Time>0)
            fillHookeXEquation(libody.get(), rightColOnly);
        else
            fillSpeedXEquation(libody.get(), rightColOnly);

        if (libody->kspring.r.y >= 0 && S->Time>0)
            fillHookeYEquation(libody.get(), rightColOnly);
        else
            fillSpeedYEquation(libody.get(), rightColOnly);

        if (libody->kspring.o >= 0 && S->Time>0)
            fillHookeOEquation(libody.get(), rightColOnly);
        else
            fillSpeedOEquation(libody.get(), rightColOnly);
    }

    if (!rightColOnly)
        matrix.markBodyMatrixAsFilled();

    // matrix->save("matrix");
}

