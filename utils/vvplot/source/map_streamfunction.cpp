#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <ctime>
#include <hdf5.h>

#include "elementary.h"
#include "libvvplot_api.h"
#include "TSpace.hpp"
#include "TSortedTree.hpp"

static double dl;
static double rd2_global;
static double EPS_MULT;
static TVec RefFrame_Speed;

typedef struct {
    uint16_t xi;
    uint16_t yj;
    float psi_gap;
} gap_t;

double eps2h(const TSortedNode &Node, TVec p);
// eps2h is implemented in map_vorticity.cpp

inline static
double _4pi_psi_g(TVec dr, double rd2, double g)
{
    return -g * log(dr.abs2() + rd2);
}

inline static
double _2pi_psi_q(TVec dr, double q)
{
    return q * atan2(dr.y, dr.x);
}

inline static
double _2pi_psi_qatt(TVec p, TVec att, TVec cofm, double q)
{
    TVec v1 = cofm-p;
    TVec v2 = att-p;
    return q * atan2(rotl(v2)*v1, v2*v1);
}

double Psi(Space* S, const TSortedTree* tree, TVec p, double spacing, double& psi_gap)
{
    double tmp_4pi_psi_g = 0.0;
    double tmp_2pi_psi_q = 0.0;
    psi_gap = 0.0;

    for (auto& lbody: S->BodyList)
    {
        for (auto& latt: lbody->alist)
        {
            tmp_4pi_psi_g += _4pi_psi_g(p-latt.r, rd2_global, latt.g);
        }

        if (lbody->speed_slae.iszero()) continue;

        for (TAtt& latt: lbody->alist)
        {
            TVec Vs = lbody->speed_slae.r + lbody->speed_slae.o * rotl(latt.r - lbody->get_axis());
            double g = -Vs * latt.dl;
            double q = -rotl(Vs) * latt.dl;
            tmp_4pi_psi_g += _4pi_psi_g(p-latt.r, rd2_global, g);
            tmp_2pi_psi_q += _2pi_psi_qatt(p, latt.r, lbody->get_cofm(), q);
        }
    }

    for (const auto& src: S->SourceList)
    {
        tmp_2pi_psi_q += _2pi_psi_q(p-src.r, src.g);

        if (p.x < src.r.x && src.r.x <= p.x+spacing &&
            p.y < src.r.y && src.r.y <= p.y+spacing)
        {
            psi_gap += src.g;
        }
    }

    const TSortedNode* Node = tree->findNode(p);
    if (!Node) return 0;
    for (TSortedNode* lfnode: *Node->FarNodes)
    {
        tmp_4pi_psi_g += _4pi_psi_g(p-lfnode->CMp.r, rd2_global, lfnode->CMp.g);
        tmp_4pi_psi_g += _4pi_psi_g(p-lfnode->CMm.r, rd2_global, lfnode->CMm.g);
    }
    for (TSortedNode* lnnode: *Node->NearNodes)
    {
        for (TObj *lobj = lnnode->vRange.first; lobj < lnnode->vRange.last; lobj++)
        {
            tmp_4pi_psi_g += _4pi_psi_g(p-lobj->r, lobj->v.x, lobj->g); // v.x stores eps^2
        }
    }

    // printf("GAP %lf\n", psi_gap);
    return tmp_4pi_psi_g*C_1_4PI + tmp_2pi_psi_q*C_1_2PI + p*rotl(S->inf_speed() - RefFrame_Speed);
}

extern "C" {
int map_streamfunction(hid_t fid, char RefFrame, double xmin, double xmax, double ymin, double ymax, double spacing)
{
    char *mult_env = getenv("VV_EPS_MULT");
    EPS_MULT = mult_env ? atof(mult_env) : 2;

    /**************************** LOAD ARGUMENTS ******************************/
    Space *S = new Space();
    switch (RefFrame)
    {
        case 'o': RefFrame_Speed = TVec(0, 0); break;
        case 'f': RefFrame_Speed = S->inf_speed(); break;
        case 'b': RefFrame_Speed = S->BodyList[0]->speed_slae.r; break;
        default:
        fprintf(stderr, "Bad reference frame\n");
        fprintf(stderr, "Available options are:\n");
        fprintf(stderr, " 'o' : original reference frame\n" );
        fprintf(stderr, " 'f' : fluid reference frame\n" );
        fprintf(stderr, " 'b' : body reference frame\n" );
    }

    S->load(fid);
    // что б не мешались
    S->HeatList.clear();
    S->StreakList.clear();

    dl = S->average_segment_length();
    rd2_global = sqr(0.2*dl);
    TSortedTree tree(S, 8, dl*20, std::numeric_limits<double>::max());
    tree.build();

    auto& bnodes = tree.getBottomNodes();
    #pragma omp parallel for
    for (auto llbnode = bnodes.begin(); llbnode < bnodes.end(); llbnode++)
    {
        TSortedNode* lbnode = *llbnode;
        for (TObj *lobj = lbnode->vRange.first; lobj < lbnode->vRange.last; lobj++)
        {
            lobj->v.x = sqr(EPS_MULT)*std::max(eps2h(*lbnode, lobj->r)*0.25, rd2_global);
        }
    }

    // Calculate field ********************************************************
    hsize_t dims[2] = {
        static_cast<size_t>((xmax-xmin)/spacing) + 1,
        static_cast<size_t>((ymax-ymin)/spacing) + 1
    };
    float *mem = (float*)malloc(sizeof(float)*dims[0]*dims[1]);
    std::vector<gap_t> gap_list;

    for (size_t xi=0; xi<dims[0]; xi++)
    {
        double x = xmin + double(xi)*spacing;
        #pragma omp parallel for ordered schedule(dynamic, 10)
        for (size_t yj=0; yj<dims[1]; yj++)
        {
            double y = ymin + double(yj)*spacing;
            double psi_gap = 0;
            mem[xi*dims[1]+yj] = Psi(S, &tree, TVec(x, y), spacing, psi_gap);
            if (psi_gap)
            {
                gap_t gap = {0};
                gap.xi = static_cast<uint16_t>(xi);
                gap.yj = static_cast<uint16_t>(yj);
                gap.psi_gap = static_cast<float>(psi_gap);
                #pragma omp critical
                gap_list.push_back(gap);
            }
        }
    }

    // мы переходим на атан2. разрыв будет уходить влево.
    // перед сохранением мы будем считать разрывы
    // для каждого источника находим ближний узел слева снизу
    // сохранить индексы по Х и У этого узла
    // и третьей колонкой сохраняем поправку в функцию тока (по факту это будет половина циркуляции)

    // на построении линий тока сделать process rect приниающим 5 аргументов
    // порядок такой: lb(0)->lt(1)->rt(2)->rb(3)->lb(4)
    // (левый нижний угол будет иметь 2 варианта значения ф тока)
    // для каждого квадрата пробегаем список разрывов
    // если индекс разрыва соответствует левому нижнему узлу - инкрементим значение ф. тока в нем (в 0м узле)
    // если разрыв правее - инкрементим lb(0), lb(4), rb(3)

    // раньше мы искали нужные константы на отрезках 01, 12, 23, 30
    // теперь будет 01, 12, 23, 34

    char map_name[] = "map_streamfunction.?";
    map_name[19] = RefFrame;
    map_save(fid, map_name, mem, dims, xmin, xmax, ymin, ymax, spacing);
    free(mem);
    
    if (!gap_list.size())
        return 0;

    hid_t map_h5d = H5Dopen2(fid, map_name, H5P_DEFAULT);
    hid_t gap_h5t = H5Tcreate(H5T_COMPOUND, 8);
    H5Tinsert(gap_h5t, "xi", 0, H5T_NATIVE_UINT16);
    H5Tinsert(gap_h5t, "yj", 2, H5T_NATIVE_UINT16);
    H5Tinsert(gap_h5t, "gap", 4, H5T_NATIVE_FLOAT);
    H5Tcommit2(map_h5d, "gap_t", gap_h5t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    dims[0] = gap_list.size();
    hid_t gaps_h5s = H5Screate_simple(1, dims, dims);
    hid_t gaps_h5a = H5Acreate(map_h5d, "gaps", gap_h5t, gaps_h5s, H5P_DEFAULT, H5P_DEFAULT);
    H5Awrite(gaps_h5a, gap_h5t, gap_list.data());

    H5Aclose(gaps_h5a);
    H5Sclose(gaps_h5s);
    H5Tclose(gap_h5t);
    H5Dclose(map_h5d);
    return 0;
}}
