#define H5_USE_18_API
#include <hdf5.h>

#include "TSpace.hpp"
#include "elementary.h"
#include "space_hdf.cpp"
#include "vvhdf.h"

#include <cmath>
// #include <float.h>
#include <cstdio>
#include <cstdlib>
#include <malloc.h>
#include <cstring>
#include <fstream>
#include <ctime>
#include <type_traits> // static_assert

using std::cout;
using std::cerr;
using std::endl;
using std::fstream;
using std::ios;
using std::vector;
using std::shared_ptr;
using std::numeric_limits;
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

// 888       .d88888b.         d8888 8888888b.      888    888 8888888b.  8888888888
// 888      d88P" "Y88b       d88888 888  "Y88b     888    888 888  "Y88b 888
// 888      888     888      d88P888 888    888     888    888 888    888 888
// 888      888     888     d88P 888 888    888     8888888888 888    888 8888888
// 888      888     888    d88P  888 888    888     888    888 888    888 888
// 888      888     888   d88P   888 888    888     888    888 888    888 888
// 888      Y88b. .d88P  d8888888888 888  .d88P     888    888 888  .d88P 888
// 88888888  "Y88888P"  d88P     888 8888888P"      888    888 8888888P"  888

herr_t Space::dataset_read_list(hid_t fid, const char *name, vector<TObj>& list)
{
    if (!H5Lexists(fid, name, H5P_DEFAULT))
        return 0;

    hid_t h5d = H5Dopen(fid, name, H5P_DEFAULT);
    if (h5d < 0)
        h5_throw("H5Dopen", name);

    hid_t h5s_file = H5Dget_space(h5d);
    if (h5s_file < 0)
        h5_throw("H5Dget_space", name);

    hsize_t dims1[2];
    H5Sget_simple_extent_dims(h5s_file, dims1, dims1);    

    hsize_t dims2[2] = {dims1[0]*2, dims1[1]};
    hid_t h5s_mem = H5Screate_simple(2, dims2, dims2);
    if (h5s_mem < 0)
        h5_throw("H5Screate_simple", name);

    list.resize(dims1[0], TObj());
    hsize_t offset[2] = {0, 0};
    hsize_t stride[2] = {2, 1};
    herr_t err = H5Sselect_hyperslab(h5s_mem, H5S_SELECT_SET, offset, stride, dims1, NULL);
    if (err < 0)
        h5_throw("H5Sselect_hyperslab", name);

    err = H5Dread(h5d, H5T_NATIVE_DOUBLE, h5s_mem, h5s_file, H5P_DEFAULT, list.data());
    if (err < 0)
        h5_throw("H5Dread", name);
    err = H5Dclose(h5d);
    if (err < 0)
        h5_throw("H5Dclose", name);
    return 0;
}

herr_t dataset_read_body(hid_t g_id, const char* name, const H5L_info_t*, void *op_data)
{
    if (strncmp(name, "body", 4) != 0)
        return 0;

    Space *S = (Space*)op_data;
    std::shared_ptr<TBody> body(new TBody());

    hid_t h5d = H5Dopen(g_id, name, H5P_DEFAULT);
    if (h5d < 0)
        h5_throw("H5Dopen", name);

    hid_t h5s_file = H5Dget_space(h5d);
    if (h5s_file < 0)
        h5_throw("H5Dget_space", name);
    hsize_t dims[2];
    H5Sget_simple_extent_dims(h5s_file, dims, dims);
    hsize_t N = dims[0];

    body-> label = h5a_read<std::string> (h5d, "label");
    body->holder = h5a_read<TVec3D> (h5d, "holder_position");
    body->dpos = h5a_read<TVec3D> (h5d, "delta_position");
    body->speed_x = h5a_read<std::string> (h5d, "speed_x");
    body->speed_y = h5a_read<std::string> (h5d, "speed_y");
    body->speed_o = h5a_read<std::string> (h5d, "speed_o");
    body->speed_slae = h5a_read<TVec3D> (h5d, "speed_slae");
    body->speed_slae_prev = h5a_read<TVec3D> (h5d, "speed_slae_prev");
    body->kspring = h5a_read<TVec3D> (h5d, "spring_const");
    body->damping = h5a_read<TVec3D> (h5d, "spring_damping");
    body->density = h5a_read<double> (h5d, "density");
    body->force_hydro = h5a_read<TVec3D> (h5d, "force_hydro");
    body->force_holder = h5a_read<TVec3D> (h5d, "force_holder");
    body->friction_prev = h5a_read<TVec3D> (h5d, "friction_prev");

    body->fdt_dead = h5a_read<TVec3D> (h5d, "fdt_dead");
    body->g_dead = h5a_read<double> (h5d, "g_dead");

    body->collision_min = h5a_read<TVec3D> (h5d, "collision_min");
    body->collision_max = h5a_read<TVec3D> (h5d, "collision_max");
    body->bounce = h5a_read<double> (h5d, "bounce");

    std::string root_body_name = h5a_read<std::string> (h5d, "root_body");
    int root_body_idx;
    if (sscanf(root_body_name.c_str(), "body%d", &root_body_idx) == 1)
        body->root_body = S->BodyList[root_body_idx];
    else
        body->root_body.reset();

    double att_array[N][4] = {0};
    uint32_t slip_array[N] = {0};
    herr_t err = H5Dread(h5d, H5T_NATIVE_DOUBLE, H5S_ALL, h5s_file, H5P_DEFAULT, att_array);
    if (err < 0)
        h5_throw("H5Dread", name);

    double heat_const = 0;
    uint32_t simplified_dataset = h5a_read<uint32_t> (h5d, "simplified_dataset");

    // simplified_dataset по хорошему должен называться dataset_version,
    // но исторически я теперь привязан именно к этому названию.
    // Предыдущая версия с HDF форматом использовала H5T_ENUM, который сейчас не читается
    // и в любом случае отвечает 0. К моему счастью я сохранял только один формат,
    // поэтому отныне 0 будет означать старую версию.
    // Новая версия упрощенного датасета имеет номер 2.
    // Новая версия полноценного датасета имеет номер 3.
    if (simplified_dataset < 2)
    {
        uint32_t general_slip = h5a_read<uint32_t> (h5d, "general_bc") == 'l';
        for(hsize_t i=0; i<N; i++)
            slip_array[i] = general_slip;
        heat_const = h5a_read<double> (h5d, "heat_const");

        body->special_segment_no = h5a_read<int32_t> (h5d, "special_bc_segment");
        switch (h5a_read<int32_t> (h5d, "special_bc"))
        {
            case 's': 
            case 'i': body->boundary_condition = bc_t::steady; break;
            case 'z': body->boundary_condition = bc_t::kutta; break;
        }
        switch (h5a_read<int32_t>(h5d, "heat_condition"))
        {
            case 'n': body->heat_condition = hc_t::neglect; break;
            case 'i': body->heat_condition = hc_t::isolate; break;
            case 't': body->heat_condition = hc_t::const_t; break;
            case 'w': body->heat_condition = hc_t::const_w; break;
        }
    }
    else if (simplified_dataset == 2)
    {
        if (H5Aexists(h5d, "slip"))
        {
            // slip_array = (uint32_t*)malloc(dims[0] * sizeof(uint32_t));
            hid_t aid = H5Aopen(h5d, "slip", H5P_DEFAULT);
            H5Aread(aid, H5T_NATIVE_UINT32, slip_array);
            H5Aclose(aid);
        }
        else
        {
            uint32_t general_slip = h5a_read<uint32_t> (h5d, "general_slip");
            for(hsize_t i=0; i<N; i++)
                slip_array[i] = general_slip;
        }
        body->special_segment_no = h5a_read<int32_t> (h5d, "special_segment_no");
        body->boundary_condition = h5a_read<bc_t> (h5d, "boundary_condition");
        body->heat_condition     = h5a_read<hc_t> (h5d, "heat_condition");
        heat_const = h5a_read<double> (h5d, "general_heat_const");
    }

    for(hsize_t i=0; i<N; i++)
    {
        body->alist.emplace_back(att_array[i][0], att_array[i][1]);
        TAtt& latt = body->alist.back();
        latt.g = att_array[i][2];
        latt.gsum = att_array[i][3];
        latt.slip = slip_array[i];
        latt.heat_const = heat_const;        
    }

    body->doUpdateSegments();
    body->doFillProperties();

    err = H5Dclose(h5d);
    if (err < 0)
        h5_throw("H5Dclose", name);
    S->BodyList.push_back(body);
    return 0;
}

void Space::load(const char* fname, std::string *info)
{
    BodyList.clear();
    VortexList.clear();
    HeatList.clear();
    StreakSourceList.clear();
    StreakList.clear();

    // FIXME <= 0 here
    if (!H5Fis_hdf5(fname))
    {
        load_v13(fname);
        return;
    }

    H5Eset_auto2(H5E_DEFAULT, NULL, NULL);
    hid_t fid = H5Fopen(fname, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (fid < 0)
        h5_throw("H5Fopen", fname);
    load(fid, info);

    h5t_close_all();
    herr_t err = H5Fclose(fid);
    if (err < 0)
        h5_throw("H5Fclose", fname);
}

void Space::load(hid_t fid, std::string *info)
{
    caption = h5a_read<std::string> (fid, "caption");
    time = h5a_read<TTime> (fid, "time");
    dt = h5a_read<TTime> (fid, "dt");
    dt_save = h5a_read<TTime> (fid, "dt_save");
    dt_streak = h5a_read<TTime> (fid, "dt_streak");
    dt_profile = h5a_read<TTime> (fid, "dt_profile");
    re = h5a_read<double> (fid, "re");
    pr = h5a_read<double> (fid, "pr");
    inf_marker = h5a_read<TVec> (fid, "inf_marker");
    inf_vx = h5a_read<std::string> (fid, "inf_speed_x");
    inf_vy = h5a_read<std::string> (fid, "inf_speed_y");
    inf_g = h5a_read<double> (fid, "inf_circulation");
    gravity = h5a_read<TVec> (fid, "gravity");
    finish = h5a_read<double> (fid, "time_to_finish");

    if (info)
    {
        info[0] = h5a_read<std::string>(fid, "git_rev");
        info[1] = h5a_read<std::string>(fid, "git_info");
        info[2] = h5a_read<std::string>(fid, "git_diff");
        info[3] = h5a_read<std::string>(fid, "time_local");
    }

    dataset_read_list(fid, "vort", VortexList);
    dataset_read_list(fid, "heat", HeatList);
    dataset_read_list(fid, "ink", StreakList);
    dataset_read_list(fid, "ink_source", StreakSourceList);
    dataset_read_list(fid, "source", SourceList);

    H5Literate(fid, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, dataset_read_body, this);
    EnumerateBodies();
    for (shared_ptr<TBody>& lbody: BodyList) {
        lbody->validate(/*critical=*/false);
    }
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
    for (shared_ptr<TBody>& lbody: BodyList)
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
int Space::load_list_txt(vector<TObj>& li, const char* filename)
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

int Space::load_list_bin(vector<TObj>& li, const char* filename)
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
