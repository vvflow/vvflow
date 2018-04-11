#define H5_USE_18_API
#include <hdf5.h>

#include "TSpace.hpp"
#include "elementary.h"
#include "vvhdf.h"

#include <fstream>
using std::fstream;
using std::ios;
static const double d_inf = 1.0l/0.0l; 

Space::Space():
    caption(),
    re(d_inf),
    pr(0),
    inf_g(),
    inf_vx(),
    inf_vy(),
    inf_marker(),
    gravity(),
    time(),
    dt(1, 1),
    dt_save(),
    dt_streak(),
    dt_profile(),
    finish(d_inf),

    BodyList(),
    VortexList(),
    HeatList(),
    SourceList(),
    StreakSourceList(),
    StreakList()
{}

void Space::load(const char* fname, metainfo_t *info)
{
    BodyList.clear();
    VortexList.clear();
    HeatList.clear();
    StreakSourceList.clear();
    StreakList.clear();

    if (H5Fis_hdf5(fname) == 0)
    {
        load_v13(fname);
        return;
    }

    hid_t fid = H5Fopen(fname, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (fid < 0)
        h5_throw("H5Fopen", fname);

    load_hdf(fid, info);

    herr_t err = H5Fclose(fid);
    if (err < 0)
        h5_throw("H5Fclose", fname);
}

void Space::save(const char* format)
{
    for (std::shared_ptr<TBody>& lbody: BodyList)
    {
        lbody->validate(/*critical=*/true);
    }

    char fname[64];
    snprintf(fname, 64, format, int(time/dt+0.5));
    hid_t fid = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (fid < 0)
        h5_throw("H5Fcreate", fname);

    save_hdf(fid);

    herr_t err = H5Fclose(fid);
    if (err < 0)
        h5_throw("H5Fclose", fname);
}

FILE* Space::open_file(const char* format)
{
    char fname[64]; sprintf(fname, format, int(time/dt+0.5));
    FILE *fout;
    fout = fopen(fname, "w");
    if (!fout) { perror("Error opening file"); return NULL; }
    return fout;
}

void Space::calc_forces()
{
    const double C_NyuDt_Pi = dt/(M_PI*re);
    const double C_Nyu_Pi = 1./(M_PI*re);
    for (std::shared_ptr<TBody>& lbody: BodyList)
    {
        double tmp_gsum = 0;
        //TObj tmp_fric(0,0,0);
        lbody->friction_prev = lbody->friction;
        lbody->friction = TVec3D();

        for (TAtt& latt: lbody->alist)
        {
            tmp_gsum+= latt.gsum;
            latt.Cp += tmp_gsum;
            latt.Fr += latt.fric * C_NyuDt_Pi;
            latt.Nu += latt.hsum * (re*pr / latt.dl.abs());

            lbody->friction.r -= latt.dl * (latt.fric * C_Nyu_Pi / latt.dl.abs());
            lbody->friction.o -= (rotl(latt.r)* latt.dl) * (latt.fric  * C_Nyu_Pi / latt.dl.abs());
            lbody->nusselt += latt.hsum * (re*pr);
        }

        lbody->nusselt /= dt;
    }

    //FIXME calculate total pressure
}

void Space::zero_forces()
{
    for (auto& lbody: BodyList)
    {
        for (auto& latt: lbody->alist)
        {
            latt.gsum =
                latt.fric =
                latt.hsum = 0;
            latt.heat_layer_obj_no = -1;
        }

        lbody->force_hydro = TVec3D();
        lbody->force_holder = TVec3D();
        lbody->friction = TVec3D();
        lbody->fdt_dead = TVec3D();
        lbody->g_dead = 0;

        lbody->nusselt = 0.;
    }
}

/********************************** SAVE/LOAD *********************************/
int Space::load_list_txt(std::vector<TObj>& li, const char* filename)
{
    FILE *fin = fopen(filename, "r");
    if (!fin) { return errno; }

    TObj obj(0, 0, 0);
    while ( fscanf(fin, "%lf %lf %lf", &obj.r.x, &obj.r.y, &obj.g)==3 )
    {
        li.push_back(obj);
    }

    fclose(fin);
    return 0;
}

int Space::load_list_bin(std::vector<TObj>& li, const char* filename)
{
    fstream fin;
    fin.open(filename, ios::in | ios::binary);
    if (!fin) { return errno; }

    TObj obj(0, 0, 0);
    while ( fin.read(reinterpret_cast<char*>(&obj), 3*sizeof(double)) )
    {
        li.push_back(obj);
    }

    fin.close();
    return 0;
}

int Space::load_body_txt(const char* filename)
{
    std::shared_ptr<TBody> body = std::make_shared<TBody>();
    int err = body->load_txt(filename);
    if (!err) {
        BodyList.push_back(body);
        EnumerateBodies();
    }

    return err;
}

void Space::EnumerateBodies()
{
    int eq_no=0;

    for(auto &lbody: BodyList)
    {
        for (auto& latt: lbody->alist)
        {
            latt.eq_no = eq_no++;
        }

        lbody->eq_forces_no = eq_no;
        eq_no+= 9;
    }
}

/********************************* INTEGRALS **********************************/

int Space::get_body_index(const TBody* body) const
{
    // TODO: rewrite with uint
    int i = 0;
    for (auto lbody: BodyList)
    {
        if (lbody.get() == body) return i;
        i++;
    }
    return -1;
}

std::string Space::get_body_name(const TBody* body) const
{
    char name[8];
    sprintf(name, "body%02d", get_body_index(body));
    return std::string(name);
}
