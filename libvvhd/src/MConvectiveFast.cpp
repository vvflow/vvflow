#include "MConvectiveFast.hpp"
#include "elementary.h"

#include <complex>

using std::complex;
using std::shared_ptr;

/****************************** MAIN FUNCTIONS ********************************/

MConvectiveFast::MConvectiveFast(Space *S, const TSortedTree *tree):
    S(S),
    tree(tree),
    matrix()
{
}

TVec MConvectiveFast::velocity(TVec p) const
{
    TVec res = {0, 0};
    const TSortedNode* node = tree->findNode(p);
    if (!node)
        return res;

    res += near_nodes_influence(*node, p);
    res += far_nodes_influence(*node, p);
    res += sink_list_influence(p);
    res += body_list_influence(p);
    res += S->inf_speed();

    return res;
}

void MConvectiveFast::process_all_lists()
{
    auto& bnodes = tree->getBottomNodes();

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

        TVec nodeCenter = TVec(lbnode->x, lbnode->y);

        #pragma omp parallel
        {
            #pragma omp for schedule(dynamic, 256)
            for (TObj *lobj = lbnode->vRange.first; lobj < lbnode->vRange.last; lobj++)
            {
                if (!lobj->g) {continue;}
                TVec dr_local = lobj->r - nodeCenter;
                lobj->v += S->inf_speed();
                lobj->v += near_nodes_influence(*lbnode, lobj->r);
                lobj->v += sink_list_influence(lobj->r);
                lobj->v += body_list_influence(lobj->r);
                lobj->v += TVec(Teilor1, Teilor2);
                lobj->v += TVec(TVec(Teilor3,  Teilor4)*dr_local, TVec(Teilor4, -Teilor3)*dr_local);
            }

        	#pragma omp for schedule(dynamic, 256)
            for (TObj *lobj = lbnode->hRange.first; lobj < lbnode->hRange.last; lobj++)
            {
                if (!lobj->g) {continue;}
                TVec dr_local = lobj->r - nodeCenter;
                lobj->v += S->inf_speed();
                lobj->v += near_nodes_influence(*lbnode, lobj->r);
                lobj->v += sink_list_influence(lobj->r);
                lobj->v += body_list_influence(lobj->r);
                lobj->v += TVec(Teilor1, Teilor2);
                lobj->v += TVec(TVec(Teilor3,  Teilor4)*dr_local, TVec(Teilor4, -Teilor3)*dr_local);
            }

        	#pragma omp for schedule(dynamic, 256)
            for (TObj *lobj = lbnode->sRange.first; lobj < lbnode->sRange.last; lobj++)
            {
                TVec dr_local = lobj->r - nodeCenter;
                lobj->v += S->inf_speed();
                lobj->v += near_nodes_influence(*lbnode, lobj->r);
                lobj->v += sink_list_influence(lobj->r);
                lobj->v += body_list_influence(lobj->r);
                lobj->v += TVec(Teilor1, Teilor2);
                lobj->v += TVec(TVec(Teilor3,  Teilor4)*dr_local, TVec(Teilor4, -Teilor3)*dr_local);
            }
        }
    }
}

TVec MConvectiveFast::biot_savart(const TObj &obj, const TVec &p) const
{
    TVec dr = p - obj.r;
    return rotl(dr)*(obj.g / (dr.abs2() + sqr(1./obj._1_eps)) );
}

TVec MConvectiveFast::near_nodes_influence(const TSortedNode &Node, const TVec &p) const
{
    TVec res = {0, 0};

    for (TSortedNode* lnnode: *Node.NearNodes)
    {
        for (TObj *lobj = lnnode->vRange.first; lobj < lnnode->vRange.last; lobj++)
        {
            if (!lobj->g) continue;
            res+= biot_savart(*lobj, p);
        }
    }

    res *= C_1_2PI;
    return res;
}

TVec MConvectiveFast::far_nodes_influence(const TSortedNode &node, const TVec &p) const
{
    TVec res = {0, 0};

    for (TSortedNode* lfnode: *node.FarNodes)
    {
        res += biot_savart(lfnode->CMp, p);
        res += biot_savart(lfnode->CMm, p);
    }

    res *= C_1_2PI;
    return res;
}

TVec MConvectiveFast::sink_list_influence(const TVec &p) const
{
    TVec res = {0, 0};
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

TVec MConvectiveFast::body_list_influence(const TVec &p) const
{
    TVec res = {0, 0};

    for (shared_ptr<TBody>& lbody: S->BodyList) {
        if (lbody->get_slip()) {
            for (TAtt& latt: lbody->alist) {
                if (latt.slip) {
                    res += biot_savart(latt, p);
                }
            }
        }

        if (!lbody->speed_slae.iszero()) {
            for (TAtt& latt: lbody->alist) {
                double drabs2 = (p-latt.r).abs2();
                if (drabs2 < latt.dl.abs2())
                {
                    TVec Vs1 = lbody->speed_slae.r + lbody->speed_slae.o * rotl(latt.corner - lbody->get_axis());
                    double g1 = -Vs1 * latt.dl;
                    double q1 = -rotl(Vs1) * latt.dl;

                    TVec Vs2 = lbody->speed_slae.r + lbody->speed_slae.o * rotl(latt.corner + latt.dl - lbody->get_axis());
                    double g2 = -Vs2 * latt.dl;
                    double q2 = -rotl(Vs2) * latt.dl;

                    res += rotl(SegmentInfluence_linear_source(p, latt, g1, g2));
                    res += SegmentInfluence_linear_source(p, latt, q1, q2);
                } else
                {
                    TVec Vs = lbody->speed_slae.r + lbody->speed_slae.o * rotl(latt.r - lbody->get_axis());
                    double g = -Vs * latt.dl;
                    double q = -rotl(Vs) * latt.dl;
                    TVec dr = p - latt.r;
                    res += (dr*q + rotl(dr)*g) / drabs2;
                }
                //res+= SegmentInfluence(p, *latt, latt->g, latt->q, 1E-6);
            }
        }

    }

    return res * C_1_2PI;
}


bool MConvectiveFast::can_use_inverse()
{
    if (S->BodyList.size() <= 1) {
        return true;
    }

    for (auto& lbody1: S->BodyList)
    {
        if (!lbody1->speed(S->time).iszero()) return false;
        if (!TBody::isrigid(lbody1->kspring.r.x)) return false;
        if (!TBody::isrigid(lbody1->kspring.r.y)) return false;
        if (!TBody::isrigid(lbody1->kspring.o  )) return false;
    }

    return true;
}

void MConvectiveFast::calc_circulation(const void** collision)
{
    if (collision == nullptr) {
        throw std::invalid_argument("MConvectiveFast::calc_circulation(): invalid collision pointer");
    }

    bool use_inverse = (*collision == nullptr) && can_use_inverse() && S->time>0;

    matrix.resize(S->total_segment_count()+S->BodyList.size()*9);

    if (matrix.bodyMatrixIsOk() && use_inverse)
        fill_matrix(/*rightColOnly=*/true, collision);
    else
    {
        unsigned eq_no = 0;
        for (auto& libody: S->BodyList)
        {
            for (auto& latt: libody->alist)
            {
                matrix.setSolutionForCol(eq_no++, &latt.g);
            }

            matrix.setSolutionForCol(eq_no++, &libody->speed_slae.r.x);
            matrix.setSolutionForCol(eq_no++, &libody->speed_slae.r.y);
            matrix.setSolutionForCol(eq_no++, &libody->speed_slae.o);

            matrix.setSolutionForCol(eq_no++, &libody->force_hydro.r.x);
            matrix.setSolutionForCol(eq_no++, &libody->force_hydro.r.y);
            matrix.setSolutionForCol(eq_no++, &libody->force_hydro.o);

            matrix.setSolutionForCol(eq_no++, &libody->force_holder.r.x);
            matrix.setSolutionForCol(eq_no++, &libody->force_holder.r.y);
            matrix.setSolutionForCol(eq_no++, &libody->force_holder.o);
        }
        fill_matrix(/*rightColOnly=*/false, collision);
    }

    matrix.solveUsingInverseMatrix(use_inverse);
}

double MConvectiveFast::_2PI_Xi_g_near(TVec p, TVec pc, TVec dl, double rd)
{
    // pc - center of segment (seg.r)
    return (pc-p)*dl/sqr(rd);
}

double MConvectiveFast::_2PI_Xi_g_dist(TVec p, TVec p1, TVec p2)
{
    return 0.5*log( (p-p2).abs2() / (p-p1).abs2() );
}

double MConvectiveFast::_2PI_Xi_g(TVec p, const TAtt &seg, double rd) // in doc 2\pi\Xi_\gamma (1.7)
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

double MConvectiveFast::_2PI_Xi_q_near(TVec p, TVec pc, TVec dl, double rd)
{
    return (pc-p)*rotl(dl)/sqr(rd);
}

double MConvectiveFast::_2PI_Xi_q_dist(TVec p, TVec p1, TVec p2)
{
    TVec dp1 = p-p1;
    TVec dp2 = p-p2;
    return atan2(dp1*rotl(dp2), dp1*dp2);
}

TVec MConvectiveFast::_2PI_Xi(const TVec &p, const TAtt &seg, double rd)
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

void MConvectiveFast::_2PI_A123(const TAtt &seg, const TBody* ibody, const TBody &b, double *_2PI_A1, double *_2PI_A2, double *_2PI_A3)
{
    *_2PI_A1 = (ibody == &b)?  C_2PI * seg.dl.y : 0;
    *_2PI_A2 = (ibody == &b)? -C_2PI * seg.dl.x : 0;
    *_2PI_A3 = 0;
    if (TBody::isrigid(b.kspring.o) && (b.speed_o.eval(S->time) == 0) && b.root_body.expired())
    {
        // в этом случае угловая скорость и так обратится в ноль
        // поэтому ради экономии времени коэффициент при ней не вычисляем
        return;
    }
    for (auto& latt: b.alist)
    {
        TVec Xi = _2PI_Xi(latt.r, seg, latt.dl.abs()*0.25);
        TVec r0 = latt.r - b.get_axis();
        // A1 и A2 в сумме тождественно (аналитически) равны нулю
        // не знаю почему, но если вкопаться в документацию - можно проверить
        //		*A1 -= Xi*latt.dl;
        //		*A2 -= rotl(Xi)*latt.dl;
        *_2PI_A3 -= Xi * TVec(rotl(r0) * latt.dl, -latt.dl*r0);
    }
}

double MConvectiveFast::NodeInfluence(const TSortedNode &Node, const TAtt &seg) const
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

double MConvectiveFast::AttachInfluence(const TAtt &seg, double rd) const
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

TVec MConvectiveFast::SegmentInfluence_linear_source(TVec p, const TAtt &seg, double q1, double q2) const
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

void MConvectiveFast::fillSlipEquationForSegment(unsigned eq_no, TAtt* seg, TBody* ibody, bool rightColOnly)
{
    //influence of infinite speed
    *matrix.getRightCol(eq_no) = rotl(S->inf_speed())*seg->dl;
    *matrix.getRightCol(eq_no) += rotl(sink_list_influence(seg->r))*seg->dl;
    //influence of all free vortices
    const TSortedNode* Node = tree->findNode(seg->r);
    *matrix.getRightCol(eq_no) -= NodeInfluence(*Node, *seg);
    if (rightColOnly) return;

    for (auto& ljbody: S->BodyList)
    {
        for (auto& latt: ljbody->alist)
        {
            *matrix.getCell(eq_no, &latt.g) = _2PI_Xi_g(latt.corner, *seg, latt.dl.abs()*0.25)*C_1_2PI;
        }

        double _2PI_A1, _2PI_A2, _2PI_A3;
        _2PI_A123(*seg, ibody, *ljbody, &_2PI_A1, &_2PI_A2, &_2PI_A3);
        *matrix.getCell(eq_no, &ljbody->speed_slae.r.x) = _2PI_A1*C_1_2PI;
        *matrix.getCell(eq_no, &ljbody->speed_slae.r.y) = _2PI_A2*C_1_2PI;
        *matrix.getCell(eq_no, &ljbody->speed_slae.o) = _2PI_A3*C_1_2PI;
    }
}

void MConvectiveFast::fillZeroEquationForSegment(unsigned eq_no, TAtt* seg, TBody* ibody, bool rightColOnly)
{
    //right column
    *matrix.getRightCol(eq_no) = 0;
    if (rightColOnly) return;

    //the only non-zero value
    *matrix.getCell(eq_no, &seg->g) = 1;
}

void MConvectiveFast::fillSteadyEquationForSegment(unsigned eq_no, TAtt* seg, TBody* ibody, bool rightColOnly)
{
    //right column
    *matrix.getRightCol(eq_no) = ibody->g_dead + 2*ibody->get_area()*ibody->speed_slae_prev.o;
    if (rightColOnly) return;

    // self
    {
        for (auto& latt: ibody->alist)
            *matrix.getCell(eq_no, &latt.g) = 1;
        *matrix.getCell(eq_no, &ibody->speed_slae.o) = 2*ibody->get_area();
    }
}

void MConvectiveFast::fillInfSteadyEquationForSegment(unsigned eq_no, TAtt* seg, TBody* ibody, bool rightColOnly)
{
    //right column
    *matrix.getRightCol(eq_no) = -S->gsum() - S->inf_g;
    if (rightColOnly) return;

    // FIXME везде, где используется auto напихать статик асертов
    for (auto& ljbody: S->BodyList)
    {
        for (auto& latt: ljbody->alist)
            *matrix.getCell(eq_no, &latt.g) = 1;
        *matrix.getCell(eq_no, &ljbody->speed_slae.o) = 2*ljbody->get_area();
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

void MConvectiveFast::fillHydroXEquation(unsigned eq_no, TBody* ibody, bool rightColOnly)
{
    const double _1_dt = 1/S->dt;
    const TVec r_c_com = ibody->get_cofm() - ibody->get_axis();

    //right column
    *matrix.getRightCol(eq_no) =
        + ibody->get_area()*_1_dt*ibody->speed_slae_prev.r.x
        + ibody->get_area()*_1_dt*rotl(2*ibody->get_cofm()+r_c_com).x * ibody->speed_slae_prev.o
        // - (-ibody->speed_slae_prev.r.y) * ibody->speed_slae_prev.o * ibody->get_area()
        + sqr(ibody->speed_slae_prev.o) * ibody->get_area() * r_c_com.x
        + ibody->fdt_dead.r.x*_1_dt;

    if (rightColOnly) return;

    // self
    {
        for (auto& latt: ibody->alist)
            *matrix.getCell(eq_no, &latt.g) = _1_dt * (-latt.corner.y);

        // speed_slae
        *matrix.getCell(eq_no, &ibody->speed_slae.r.x) = ibody->get_area()*_1_dt;
        *matrix.getCell(eq_no, &ibody->speed_slae.r.y) = 0;
        *matrix.getCell(eq_no, &ibody->speed_slae.o)   = ibody->get_area()*_1_dt * (-2*ibody->get_cofm().y - r_c_com.y);

        // force_hydro
        *matrix.getCell(eq_no, &ibody->force_hydro.r.x) = -1;
    }
}

void MConvectiveFast::fillHydroYEquation(unsigned eq_no, TBody* ibody, bool rightColOnly)
{
    const double _1_dt = 1/S->dt;
    const TVec r_c_com = ibody->get_cofm() - ibody->get_axis();

    //right column
    *matrix.getRightCol(eq_no) =
        + ibody->get_area()*_1_dt*ibody->speed_slae_prev.r.y
        + ibody->get_area()*_1_dt*rotl(2*ibody->get_cofm()+r_c_com).y * ibody->speed_slae_prev.o
        // - (ibody->speed_slae_prev.r.x) * ibody->speed_slae_prev.o * ibody->get_area()
        + sqr(ibody->speed_slae_prev.o) * ibody->get_area() * r_c_com.y
        + ibody->fdt_dead.r.y*_1_dt;
    if (rightColOnly) return;

    // self
    {
        for (auto& latt: ibody->alist)
            *matrix.getCell(eq_no, &latt.g) = _1_dt * (latt.corner.x);

        // Speed_slae
        *matrix.getCell(eq_no, &ibody->speed_slae.r.x) = 0;
        *matrix.getCell(eq_no, &ibody->speed_slae.r.y) = ibody->get_area()*_1_dt;
        *matrix.getCell(eq_no, &ibody->speed_slae.o)   = ibody->get_area()*_1_dt * (2*ibody->get_cofm().x + r_c_com.x);

        // force_hydro
        *matrix.getCell(eq_no, &ibody->force_hydro.r.y) = -1;
    }
}

void MConvectiveFast::fillHydroOEquation(unsigned eq_no, TBody* ibody, bool rightColOnly)
{
    const double _1_dt = 1/S->dt;
    const double _1_2dt = 0.5/S->dt;
    const TVec r_c_com = ibody->get_cofm() - ibody->get_axis();

    //right column
    *matrix.getRightCol(eq_no) =
        + ibody->get_area()*_1_dt * rotl(r_c_com) * ibody->speed_slae_prev.r
        + 2*ibody->get_moi_axis()*_1_dt * ibody->speed_slae_prev.o
        // - (r_c_com * ibody->speed_slae_prev.r) * ibody->speed_slae_prev.o * ibody->get_area()
        + ibody->fdt_dead.o*_1_2dt;
    if (rightColOnly) return;

    // self
    {
        for (auto& latt: ibody->alist)
            *matrix.getCell(eq_no, &latt.g) = _1_2dt * (latt.corner - ibody->get_axis()).abs2();

        // Speed_slae
        *matrix.getCell(eq_no, &ibody->speed_slae.r.x) = ibody->get_area()*_1_dt * (-r_c_com.y);
        *matrix.getCell(eq_no, &ibody->speed_slae.r.y) = ibody->get_area()*_1_dt * (r_c_com.x);
        *matrix.getCell(eq_no, &ibody->speed_slae.o)   = 2*ibody->get_moi_axis()*_1_dt;

        // force_hydro
        *matrix.getCell(eq_no, &ibody->force_hydro.o) = -1;
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

void MConvectiveFast::fillNewtonXEquation(unsigned eq_no, TBody* ibody, bool rightColOnly)
{
    const double _1_dt = 1/S->dt;
    const TVec r_c_com = ibody->get_cofm() - ibody->get_axis();

    //right column
    *matrix.getRightCol(eq_no) =
        - ibody->get_area()*_1_dt*ibody->density * ibody->speed_slae_prev.r.x
        + ibody->get_area()*_1_dt*ibody->density * r_c_com.y * ibody->speed_slae_prev.o
        + ibody->get_area()*ibody->density * r_c_com.x * sqr(ibody->speed_slae_prev.o)
        - (ibody->density-1.0) * S->gravity.x * ibody->get_area()
        - ibody->friction_prev.r.x;
    if (rightColOnly) return;

    // self
    {
        // Speed_slae
        *matrix.getCell(eq_no, &ibody->speed_slae.r.x) = -ibody->get_area()*_1_dt*ibody->density;
        *matrix.getCell(eq_no, &ibody->speed_slae.r.y) = 0;
        *matrix.getCell(eq_no, &ibody->speed_slae.o)   = ibody->get_area()*_1_dt*ibody->density*r_c_com.y;
        // force_hydro
        *matrix.getCell(eq_no, &ibody->force_hydro.r.x) = 1;
        // force_holder
        *matrix.getCell(eq_no, &ibody->force_holder.r.x) = -1;
    }

    // children
    for (auto& ljbody: S->BodyList)
    {
        if (ljbody->root_body.lock().get() != ibody) continue;

        // force_holder
        *matrix.getCell(eq_no, &ljbody->force_holder.r.x) = 1;
    }
}

void MConvectiveFast::fillNewtonYEquation(unsigned eq_no, TBody* ibody, bool rightColOnly)
{
    const double _1_dt = 1/S->dt;
    const TVec r_c_com = ibody->get_cofm() - ibody->get_axis();

    //right column
    *matrix.getRightCol(eq_no) =
        - ibody->get_area()*_1_dt*ibody->density * ibody->speed_slae_prev.r.y
        - ibody->get_area()*_1_dt*ibody->density * r_c_com.x * ibody->speed_slae_prev.o
        + ibody->get_area()*ibody->density * r_c_com.y * sqr(ibody->speed_slae_prev.o)
        - (ibody->density-1.0) * S->gravity.y * ibody->get_area()
        - ibody->friction_prev.r.y;
    if (rightColOnly) return;

    // self
    {
        // Speed_slae
        *matrix.getCell(eq_no, &ibody->speed_slae.r.x) = 0;
        *matrix.getCell(eq_no, &ibody->speed_slae.r.y) = -ibody->get_area()*_1_dt*ibody->density;
        *matrix.getCell(eq_no, &ibody->speed_slae.o)   = -ibody->get_area()*_1_dt*ibody->density*r_c_com.x;
        // force_hydro
        *matrix.getCell(eq_no, &ibody->force_hydro.r.y) = 1;
        // force_holder
        *matrix.getCell(eq_no, &ibody->force_holder.r.y) = -1;
    }

    // children
    for (auto& ljbody: S->BodyList)
    {
        if (ljbody->root_body.lock().get() != ibody) continue;

        // force_holder
        *matrix.getCell(eq_no, &ljbody->force_holder.r.y) = 1;
    }
}

void MConvectiveFast::fillNewtonOEquation(unsigned eq_no, TBody* ibody, bool rightColOnly)
{
    const double _1_dt = 1/S->dt;
    const TVec r_c_com = ibody->get_cofm() - ibody->get_axis();

    //right column
    *matrix.getRightCol(eq_no) =
        - ibody->get_area()*_1_dt*ibody->density * (rotl(r_c_com)*ibody->speed_slae_prev.r)
        - ibody->get_moi_axis()*_1_dt*ibody->density * ibody->speed_slae_prev.o
        - (ibody->density-1.0) * (rotl(r_c_com)*S->gravity) * ibody->get_area()
        - ibody->friction_prev.o;
    if (rightColOnly) return;

    // self
    {
        // Speed_slae
        *matrix.getCell(eq_no, &ibody->speed_slae.r.x) = +ibody->get_area()*_1_dt*ibody->density * r_c_com.y;
        *matrix.getCell(eq_no, &ibody->speed_slae.r.y) = -ibody->get_area()*_1_dt*ibody->density * r_c_com.x;
        *matrix.getCell(eq_no, &ibody->speed_slae.o)   = -ibody->get_moi_axis()*_1_dt*ibody->density;
        // force_hydro
        *matrix.getCell(eq_no, &ibody->force_hydro.o) = 1;
        // force_holder
        *matrix.getCell(eq_no, &ibody->force_holder.o) = -1;
    }

    // children
    for (auto& ljbody: S->BodyList)
    {
        if (ljbody->root_body.lock().get() != ibody) continue;

        // force_holder
        TVec r_child_c = ljbody->get_axis() - ibody->get_axis();
        *matrix.getCell(eq_no, &ljbody->force_holder.r.x) = -r_child_c.y;
        *matrix.getCell(eq_no, &ljbody->force_holder.r.y) = r_child_c.x;
        *matrix.getCell(eq_no, &ljbody->force_holder.o) = 1;
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

void MConvectiveFast::fillHookeXEquation(unsigned eq_no, TBody* ibody, bool rightColOnly)
{
    *matrix.getRightCol(eq_no) = ibody->kspring.r.x * ibody->dpos.r.x;
    if (rightColOnly) return;

    // self
    {
        // speed_slae
        *matrix.getCell(eq_no, &ibody->speed_slae.r.x) = -ibody->damping.r.x;
        // force_holder
        *matrix.getCell(eq_no, &ibody->force_holder.r.x) = 1;
    }
}

void MConvectiveFast::fillHookeYEquation(unsigned eq_no, TBody* ibody, bool rightColOnly)
{
    *matrix.getRightCol(eq_no) = ibody->kspring.r.y * ibody->dpos.r.y;
    if (rightColOnly) return;

    // self
    {
        // speed_slae
        *matrix.getCell(eq_no, &ibody->speed_slae.r.y) = -ibody->damping.r.y;
        // force_holder
        *matrix.getCell(eq_no, &ibody->force_holder.r.y) = 1;
    }
}

void MConvectiveFast::fillHookeOEquation(unsigned eq_no, TBody* ibody, bool rightColOnly)
{
    *matrix.getRightCol(eq_no) = ibody->kspring.o * ibody->dpos.o;
    if (rightColOnly) return;

    // self
    {
        // speed_slae
        *matrix.getCell(eq_no, &ibody->speed_slae.o)   = -ibody->damping.o;
        // force_holder
        *matrix.getCell(eq_no, &ibody->force_holder.o) = 1;
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

void MConvectiveFast::fillSpeedXEquation(unsigned eq_no, TBody* ibody, bool rightColOnly)
{
    *matrix.getRightCol(eq_no) = ibody->speed_x.eval(S->time);
    if (rightColOnly) return;

    // self
    {
        // speed_slae
        *matrix.getCell(eq_no, &ibody->speed_slae.r.x) = 1;
    }

    // root
    auto root_body = ibody->root_body.lock();
    if (root_body)
    {
        *matrix.getCell(eq_no, &root_body->speed_slae.r.x) = -1;
        *matrix.getCell(eq_no, &root_body->speed_slae.r.y) = 0;
        *matrix.getCell(eq_no, &root_body->speed_slae.o)   = (ibody->holder.r - root_body->get_axis()).y;
    }
}

void MConvectiveFast::fillSpeedYEquation(unsigned eq_no, TBody* ibody, bool rightColOnly)
{
    *matrix.getRightCol(eq_no) = ibody->speed_y.eval(S->time);
    if (rightColOnly) return;

    // self
    {
        // speed_slae
        *matrix.getCell(eq_no, &ibody->speed_slae.r.y) = 1;
    }

    // root
    auto root_body = ibody->root_body.lock();
    if (root_body)
    {
        *matrix.getCell(eq_no, &root_body->speed_slae.r.x) = 0;
        *matrix.getCell(eq_no, &root_body->speed_slae.r.y) = -1;
        *matrix.getCell(eq_no, &root_body->speed_slae.o)   = -(ibody->get_axis() - root_body->get_axis()).x;
    }
}

void MConvectiveFast::fillSpeedOEquation(unsigned eq_no, TBody* ibody, bool rightColOnly)
{
    *matrix.getRightCol(eq_no) = ibody->speed_o.eval(S->time);
    if (rightColOnly) return;

    // self
    {
        // speed_slae
        *matrix.getCell(eq_no, &ibody->speed_slae.o)   = 1;
    }

    // root
    auto root_body = ibody->root_body.lock();
    if (root_body)
    {
        *matrix.getCell(eq_no, &root_body->speed_slae.o) = -1;
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

typedef void (MConvectiveFast::*fptr)();
typedef struct {
    fptr eq_type;
    int eq_no;
    TAtt* seg;
    TBody* ibody;
} equationJob;


void MConvectiveFast::fill_matrix(bool rightColOnly, const void** collision)
{
    if (collision == nullptr) {
        throw std::invalid_argument("MMConvectiveFast::fill_matrix(): invalid collision pointer");
    }

    if (!rightColOnly) matrix.fillWithZeros();

    std::vector<equationJob> jobs;
#define addJob(TYPE, EQNO, SEG, IBODY) \
    jobs.push_back( \
        (equationJob){(fptr)&MConvectiveFast::fill##TYPE, EQNO, SEG, IBODY} \
    )

    int eq_no = 0;
    int once = 0;
    for (auto& libody: S->BodyList)
    {
        TAtt *special_segment = &libody->alist[libody->special_segment_no];
        for (auto& latt: libody->alist)
        {
            if (&latt != special_segment)
                addJob(SlipEquationForSegment, eq_no++, &latt, libody.get());
            else
            {
                if (libody->boundary_condition == bc_t::kutta)
                    addJob(ZeroEquationForSegment, eq_no++, &latt, libody.get());
                else if (!once++)
                    addJob(InfSteadyEquationForSegment, eq_no++, &latt, libody.get());
                else
                    addJob(SteadyEquationForSegment, eq_no++, &latt, libody.get());
            }
        }

        if (TBody::isrigid(libody->kspring.r.x) || !(S->time > 0) )
            fillSpeedXEquation(eq_no, libody.get(), rightColOnly);
        else if (*collision == &libody->kspring.r.x) {
            *matrix.getCell(eq_no, &libody->speed_slae.r.x) = 1;
            *matrix.getRightCol(eq_no) = 0;
            *collision = &libody->force_holder.r.x;
        } else {
            fillHookeXEquation(eq_no, libody.get(), rightColOnly);
            if (*collision == &libody->force_holder.r.x) {
                *matrix.getRightCol(eq_no) +=
                    (1+libody->bounce)
                    * libody->force_holder.r.x;
            }
            *collision = nullptr;
        }
        eq_no++;

        if (TBody::isrigid(libody->kspring.r.y) || !(S->time > 0) )
            fillSpeedYEquation(eq_no, libody.get(), rightColOnly);
        else if (*collision == &libody->kspring.r.y) {
            *matrix.getCell(eq_no, &libody->speed_slae.r.y) = 1;
            *matrix.getRightCol(eq_no) = 0;
            *collision = &libody->force_holder.r.y;
        } else {
            fillHookeYEquation(eq_no, libody.get(), rightColOnly);
            if (*collision == &libody->force_holder.r.y) {
                *matrix.getRightCol(eq_no) +=
                    (1+libody->bounce)
                    * libody->force_holder.r.y;
            }
            *collision = nullptr;
        }
        eq_no++;

        if (TBody::isrigid(libody->kspring.o) || !(S->time > 0) )
            fillSpeedOEquation(eq_no, libody.get(), rightColOnly);
        else if (*collision == &libody->kspring.o) {
            *matrix.getCell(eq_no, &libody->speed_slae.o) = 1;
            *matrix.getRightCol(eq_no) = 0;
            *collision = &libody->force_holder.o;
        }
        else {
            fillHookeOEquation(eq_no, libody.get(), rightColOnly);
            if (*collision == &libody->force_holder.o) {
                *matrix.getRightCol(eq_no) +=
                    (1+libody->bounce)
                    * libody->force_holder.o;
            }
            *collision = nullptr;
        }
        eq_no++;

        addJob(HydroXEquation, eq_no++, nullptr, libody.get());
        addJob(HydroYEquation, eq_no++, nullptr, libody.get());
        addJob(HydroOEquation, eq_no++, nullptr, libody.get());

        addJob(NewtonXEquation, eq_no++, nullptr, libody.get());
        addJob(NewtonYEquation, eq_no++, nullptr, libody.get());
        addJob(NewtonOEquation, eq_no++, nullptr, libody.get());
    }
#undef addJob

    #pragma omp parallel for
    for (auto job = jobs.begin(); job < jobs.end(); job++)
    {
        if (0) {}
#define CASE(TYPE) \
        else if (job->eq_type == (fptr)&MConvectiveFast::fill##TYPE)
#define FILL_EQ_FOR_SEG(TYPE)  fill##TYPE(job->eq_no, job->seg, job->ibody, rightColOnly)
#define FILL_EQ_FOR_BODY(TYPE) fill##TYPE(job->eq_no, job->ibody, rightColOnly)

        CASE(SlipEquationForSegment) FILL_EQ_FOR_SEG(SlipEquationForSegment);
        CASE(ZeroEquationForSegment) FILL_EQ_FOR_SEG(ZeroEquationForSegment);
        CASE(InfSteadyEquationForSegment) FILL_EQ_FOR_SEG(InfSteadyEquationForSegment);
        CASE(SteadyEquationForSegment) FILL_EQ_FOR_SEG(SteadyEquationForSegment);

        CASE(HydroXEquation) FILL_EQ_FOR_BODY(HydroXEquation);
        CASE(HydroYEquation) FILL_EQ_FOR_BODY(HydroYEquation);
        CASE(HydroOEquation) FILL_EQ_FOR_BODY(HydroOEquation);
        CASE(NewtonXEquation) FILL_EQ_FOR_BODY(NewtonXEquation);
        CASE(NewtonYEquation) FILL_EQ_FOR_BODY(NewtonYEquation);
        CASE(NewtonOEquation) FILL_EQ_FOR_BODY(NewtonOEquation);
#undef CASE
#undef FILL_EQ_FOR_SEG
#undef FILL_EQ_FOR_BODY
    }

    if (!rightColOnly)
        matrix.markBodyMatrixAsFilled();

    // matrix.save("matrix");
    // exit(0);
}

