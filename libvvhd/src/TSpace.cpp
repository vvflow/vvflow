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

//  .d8888b.         d8888 888     888 8888888888     888    888 8888888b.  8888888888
// d88P  Y88b       d88888 888     888 888            888    888 888  "Y88b 888
// Y88b.           d88P888 888     888 888            888    888 888    888 888
//  "Y888b.       d88P 888 Y88b   d88P 8888888        8888888888 888    888 8888888
//     "Y88b.    d88P  888  Y88b d88P  888            888    888 888    888 888
//       "888   d88P   888   Y88o88P   888            888    888 888    888 888
// Y88b  d88P  d8888888888    Y888P    888            888    888 888  .d88P 888
//  "Y8888P"  d88P     888     Y8P     8888888888     888    888 8888888P"  888

#define H5ASSERT(expr, msg) if (expr<0) { \
	fprintf(stderr, "%s failed (%s:%d). Aborting.", msg, __FILE__, __LINE__); \
	std::exit(5); \
}

void Space::dataset_write_list(const char *name, const vector<TObj>& list)
{
    if (list.empty()) return;
    // 1D dataspace
    hsize_t dims[2] = {list.size(), 3};
    hsize_t dims2[2] = {dims[0]*2, dims[1]};
    hsize_t chunkdims[2] = {std::min<hsize_t>(512, dims[0]), 3};
    hid_t prop = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(prop, 2, chunkdims);
    H5Pset_deflate(prop, 9);

    hid_t file_dataspace = H5Screate_simple(2, dims, dims);
    H5ASSERT(file_dataspace, "H5Screate_simple");
    hid_t mem_dataspace = H5Screate_simple(2, dims2, dims2);
    hsize_t start[2] = {0, 0};
    hsize_t stride[2] = {2, 1};
    H5ASSERT(H5Sselect_hyperslab(mem_dataspace, H5S_SELECT_SET, start, stride, dims, NULL), "H5Sselect_hyperslab");
    hid_t file_dataset = H5Dcreate2(fid, name, H5T_NATIVE_DOUBLE, file_dataspace, H5P_DEFAULT, prop, H5P_DEFAULT);
    H5ASSERT(file_dataset, "H5Dcreate");
    H5ASSERT(H5Dwrite(file_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, list.data()), "H5Dwrite");
    H5ASSERT(H5Dclose(file_dataset), "H5Dclose");
}

void Space::dataset_write_body(const char* name, const TBody& body)
{
    float heat_const = body.alist.front().heat_const;
    uint32_t general_slip = body.alist.front().slip;
    bool can_simplify_slip = true;
    bool can_simplify_heat = true;

    hsize_t dims[2] = {body.size(), 4};
    struct ATT *att_array = (struct ATT*)malloc(dims[0] * sizeof(struct ATT));
    uint32_t *slip_array = (uint32_t*)malloc(dims[0] * sizeof(uint32_t));
    float *heat_array = (float*)malloc(dims[0] * sizeof(float));
    for(hsize_t i=0; i<dims[0]; i++)
    {
        const TAtt& att = body.alist[i];
        att_array[i].x = att.corner.x;
        att_array[i].y = att.corner.y;
        att_array[i].g = att.g;
        att_array[i].gsum = att.gsum;
        slip_array[i] = att.slip;
        heat_array[i] = att.heat_const;

        can_simplify_slip &= (att.slip == general_slip);
        can_simplify_heat &= (att.heat_const == heat_const);
    }

    hsize_t chunkdims[2] = {std::min<hsize_t>(512, dims[0]), 4};
    hid_t prop = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(prop, 2, chunkdims);
    H5Pset_deflate(prop, 9);

    hid_t file_dataspace = H5Screate_simple(2, dims, dims);
    H5ASSERT(file_dataspace, "H5Screate_simple");

    hid_t file_dataset = H5Dcreate2(fid, name, H5T_NATIVE_DOUBLE, file_dataspace, H5P_DEFAULT, prop, H5P_DEFAULT);
    H5ASSERT(file_dataset, "H5Dcreate");

    if (!can_simplify_slip)
    {
        hid_t slip_dataspace = H5Screate_simple(1, dims, dims);
        H5ASSERT(slip_dataspace, "H5Screate_simple");

        hid_t aid = H5Acreate(file_dataset, "slip", H5T_NATIVE_UINT32, slip_dataspace, H5P_DEFAULT, H5P_DEFAULT);
        if (H5Awrite(aid, H5T_NATIVE_UINT32, slip_array)<0) return;
        if (H5Aclose(aid)<0) return;
    }
    else
    {
        attribute_write(file_dataset, "general_slip", general_slip);
    }

    if (!can_simplify_heat)
    {
        fprintf(stderr, "Can not save simplified heat in %s\n", get_body_name(&body).c_str());
        exit(1);
    }
    else
    {
        attribute_write(file_dataset, "general_heat_const", double(heat_const));
    }

    attribute_write(file_dataset, "simplified_dataset", uint32_t(2));
        
    if (!body.root_body.expired())
    {
        auto root_body = body.root_body.lock();
        attribute_write(file_dataset, "root_body", get_body_name(root_body.get()));
    }

    attribute_write(file_dataset, "label", body.label);
    attribute_write(file_dataset, "holder_position", body.holder);
    attribute_write(file_dataset, "delta_position", body.dpos);
    attribute_write<std::string>(file_dataset, "speed_x", body.speed_x);
    attribute_write<std::string>(file_dataset, "speed_y", body.speed_y);
    attribute_write<std::string>(file_dataset, "speed_o", body.speed_o);
    attribute_write(file_dataset, "speed_slae", body.speed_slae);
    attribute_write(file_dataset, "speed_slae_prev", body.speed_slae_prev);
    attribute_write(file_dataset, "spring_const", body.kspring);
    attribute_write(file_dataset, "spring_damping", body.damping);
    attribute_write(file_dataset, "density", body.density);
    attribute_write(file_dataset, "force_hydro", body.force_hydro);
    attribute_write(file_dataset, "force_holder", body.force_holder);
    attribute_write(file_dataset, "friction_prev", body.friction_prev);

    attribute_write(file_dataset, "fdt_dead", body.fdt_dead);
    attribute_write(file_dataset, "g_dead", body.g_dead);

    attribute_write(file_dataset, "collision_min", body.collision_min);
    attribute_write(file_dataset, "collision_max", body.collision_max);
    attribute_write(file_dataset, "bounce", body.bounce);

    attribute_write(file_dataset, "area", body.get_area());
    attribute_write(file_dataset, "cofm", body.get_cofm());
    attribute_write(file_dataset, "moi_cofm", body.get_moi_cofm());
    attribute_write(file_dataset, "moi_axis", body.get_moi_axis());

    attribute_write(file_dataset, "boundary_condition", body.boundary_condition);
    attribute_write(file_dataset, "special_segment_no", body.special_segment_no);
    attribute_write(file_dataset, "heat_condition", body.heat_condition);

    H5ASSERT(H5Dwrite(file_dataset, H5T_NATIVE_DOUBLE, H5S_ALL, file_dataspace, H5P_DEFAULT, att_array), "H5Dwrite");
    H5ASSERT(H5Dclose(file_dataset), "H5Dclose");
    free(att_array);
    free(slip_array);
    free(heat_array);
}

void Space::save(const char* format)
{
    for (shared_ptr<TBody>& lbody: BodyList)
    {
        lbody->validate(/*critical=*/true);
    }

    char fname[64]; sprintf(fname, format, int(time/dt+0.5));
    fid = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (fid < 0)
    {
        H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
        H5Eprint2(H5E_DEFAULT, stderr);
        fprintf(stderr, "error: Space::Save: can't open file '%s'\n", fname);
        return;
    }
    datatypes_create_all();

    attribute_write(fid, "caption", caption);
    attribute_write(fid, "time", time);
    attribute_write(fid, "dt", dt);
    attribute_write(fid, "dt_save", dt_save);
    attribute_write(fid, "dt_streak", dt_streak);
    attribute_write(fid, "dt_profile", dt_profile);
    attribute_write(fid, "re", re);
    attribute_write(fid, "pr", pr);
    attribute_write(fid, "inf_marker", inf_marker);
    attribute_write<std::string>(fid, "inf_speed_x", inf_vx);
    attribute_write<std::string>(fid, "inf_speed_y", inf_vy);
    attribute_write(fid, "inf_circulation", inf_g);
    attribute_write(fid, "gravity", gravity);
    attribute_write(fid, "time_to_finish", finish);
    attribute_write(fid, "git_rev",  std::string(libvvhd_gitrev));
    attribute_write(fid, "git_info", std::string(libvvhd_gitinfo));
    attribute_write(fid, "git_diff", std::string(libvvhd_gitdiff));

    time_t rt;
    ::time(&rt);
    char *timestr = ctime(&rt); timestr[strlen(timestr)-1] = 0;
    attribute_write(fid, "time_local", std::string(timestr));

    dataset_write_list("vort", VortexList);
    dataset_write_list("heat", HeatList);
    dataset_write_list("ink", StreakList);
    dataset_write_list("ink_source", StreakSourceList);
    dataset_write_list("source", SourceList);

    for (shared_ptr<TBody>& lbody: BodyList)
    {
        dataset_write_body(get_body_name(lbody.get()).c_str(), *lbody);
    }

    datatypes_close_all();
    H5ASSERT(H5Fclose(fid), "H5Fclose");
}

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
    if (!H5Lexists(fid, name, H5P_DEFAULT)) return 0;

    hid_t dataset = H5Dopen2(fid, name, H5P_DEFAULT);
    if (dataset < 0)
    {
        H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
        H5Eprint2(H5E_DEFAULT, stderr);
        fprintf(stderr, "error: dataset_read_list: can't open dataset '%s'\n", name);
        return -1;
    }

    hid_t file_dataspace = H5Dget_space(dataset);
    H5ASSERT(file_dataspace, "H5Dget_space");
    hsize_t dims[2]; H5Sget_simple_extent_dims(file_dataspace, dims, dims);
    hsize_t dims2[2] = {dims[0]*2, dims[1]};
    hid_t mem_dataspace = H5Screate_simple(2, dims2, dims2);
    H5ASSERT(mem_dataspace, "H5Screate_simple");
    list.resize(dims[0], TObj());
    hsize_t offset[2] = {0, 0};
    hsize_t stride[2] = {2, 1};
    H5ASSERT(H5Sselect_hyperslab(mem_dataspace, H5S_SELECT_SET, offset, stride, dims, NULL), "H5Sselect_hyperslab");

    herr_t err = H5Dread(dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, list.data());
    if (err < 0)
    {
        H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
        H5Eprint2(H5E_DEFAULT, stderr);
        fprintf(stderr, "error: dataset_read_list: can't read dataset '%s'\n", name);
        return -1;
    }
    H5ASSERT(H5Dclose(dataset), "H5Dclose");
    return 0;
}

herr_t dataset_read_body(hid_t g_id, const char* name, const H5L_info_t*, void *op_data)
{
    if (strncmp(name, "body", 4) != 0)
        return 0;

    Space *S = (Space*)op_data;
    std::shared_ptr<TBody> body(new TBody());

    hid_t dataset = H5Dopen2(g_id, name, H5P_DEFAULT);
    if (dataset < 0)
    {
        H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
        H5Eprint2(H5E_DEFAULT, stderr);
        fprintf(stderr, "error: dataset_read_body: can't open dataset '%s'\n", name);
        return -1;
    }
    hid_t file_dataspace = H5Dget_space(dataset);
    H5ASSERT(file_dataspace, "H5Dget_space");
    hsize_t dims[2]; H5Sget_simple_extent_dims(file_dataspace, dims, dims);
    hsize_t N = dims[0];

    body-> label = h5a_read<std::string> (dataset, "label");
    body->holder = h5a_read<TVec3D> (dataset, "holder_position");
    body->dpos = h5a_read<TVec3D> (dataset, "delta_position");
    body->speed_x = h5a_read<std::string> (dataset, "speed_x");
    body->speed_y = h5a_read<std::string> (dataset, "speed_y");
    body->speed_o = h5a_read<std::string> (dataset, "speed_o");
    body->speed_slae = h5a_read<TVec3D> (dataset, "speed_slae");
    body->speed_slae_prev = h5a_read<TVec3D> (dataset, "speed_slae_prev");
    body->kspring = h5a_read<TVec3D> (dataset, "spring_const");
    body->damping = h5a_read<TVec3D> (dataset, "spring_damping");
    body->density = h5a_read<double> (dataset, "density");
    body->force_hydro = h5a_read<TVec3D> (dataset, "force_hydro");
    body->force_holder = h5a_read<TVec3D> (dataset, "force_holder");
    body->friction_prev = h5a_read<TVec3D> (dataset, "friction_prev");

    body->fdt_dead = h5a_read<TVec3D> (dataset, "fdt_dead");
    body->g_dead = h5a_read<double> (dataset, "g_dead");

    body->collision_min = h5a_read<TVec3D> (dataset, "collision_min");
    body->collision_max = h5a_read<TVec3D> (dataset, "collision_max");
    body->bounce = h5a_read<double> (dataset, "bounce");

    std::string root_body_name = h5a_read<std::string> (dataset, "root_body");
    int root_body_idx;
    if (sscanf(root_body_name.c_str(), "body%d", &root_body_idx) == 1)
        body->root_body = S->BodyList[root_body_idx];
    else
        body->root_body.reset();

    struct ATT *att_array = (struct ATT*)malloc(dims[0] * sizeof(struct ATT));
    uint32_t slip_array[N] = {0};
    H5ASSERT(H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, file_dataspace, H5P_DEFAULT, att_array), "H5Dread");

    double heat_const;
    uint32_t simplified_dataset = h5a_read<uint32_t> (dataset, "simplified_dataset");

    // simplified_dataset по хорошему должен называться dataset_version,
    // но исторически я теперь привязан именно к этому названию.
    // Предыдущая версия с HDF форматом использовала H5T_ENUM, который сейчас не читается
    // и в любом случае отвечает 0. К моему счастью я сохранял только один формат,
    // поэтому отныне 0 будет означать старую версию.
    // Новая версия упрощенного датасета имеет номер 2.
    // Новая версия полноценного датасета имеет номер 3.
    if (simplified_dataset < 2)
    {
        uint32_t general_slip = h5a_read<uint32_t> (dataset, "general_bc") == 'l';
        for(hsize_t i=0; i<N; i++)
            slip_array[i] = general_slip;
        heat_const = h5a_read<double> (dataset, "heat_const");

        body->special_segment_no = h5a_read<int32_t> (dataset, "special_bc_segment");
        switch (h5a_read<int32_t> (dataset, "special_bc"))
        {
            case 's': 
            case 'i': body->boundary_condition = bc_t::steady; break;
            case 'z': body->boundary_condition = bc_t::kutta; break;
        }
        switch (h5a_read<int32_t>(dataset, "heat_condition"))
        {
            case 'n': body->heat_condition = hc_t::neglect; break;
            case 'i': body->heat_condition = hc_t::isolate; break;
            case 't': body->heat_condition = hc_t::const_t; break;
            case 'w': body->heat_condition = hc_t::const_w; break;
        }
    }
    else if (simplified_dataset == 2)
    {
        if (H5Aexists(dataset, "slip"))
        {
            // slip_array = (uint32_t*)malloc(dims[0] * sizeof(uint32_t));
            hid_t aid = H5Aopen(dataset, "slip", H5P_DEFAULT);
            H5Aread(aid, H5T_NATIVE_UINT32, slip_array);
            H5Aclose(aid);
        }
        else
        {
            uint32_t general_slip = h5a_read<uint32_t> (dataset, "general_slip");
            for(hsize_t i=0; i<N; i++)
                slip_array[i] = general_slip;
        }
        body->special_segment_no = h5a_read<int32_t> (dataset, "special_segment_no");
        body->boundary_condition = h5a_read<bc_t> (dataset, "boundary_condition");
        body->heat_condition     = h5a_read<hc_t> (dataset, "heat_condition");
        heat_const = h5a_read<double> (dataset, "general_heat_const");
    }


    for(hsize_t i=0; i<N; i++)
    {
        body->alist.emplace_back(att_array[i].x, att_array[i].y);
        TAtt& latt = body->alist.back();
        latt.g = att_array[i].g;
        latt.gsum = att_array[i].gsum;
        latt.slip = slip_array[i];
        latt.heat_const = heat_const;        
    }

    body->doUpdateSegments();
    body->doFillProperties();

    herr_t err = H5Dclose(dataset);
    if (err < 0)
        throw std::runtime_error("H5Dclose failed");
    free(att_array);
    free(slip_array);
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
        Load_v1_3(fname);
        return;
    }

    H5Eset_auto2(H5E_DEFAULT, NULL, NULL);
    hid_t fid = H5Fopen(fname, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (fid < 0)
    {
        H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
        H5Eprint2(H5E_DEFAULT, stderr);
        fprintf(stderr, "error: Space::Load: can't open file '%s'\n", fname);
        return;
    }
    load(fid, info);

    H5Fclose(fid);
}

void Space::load(hid_t fid, std::string *info)
{
    datatypes_create_all();

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

    datatypes_close_all();
}

//   888       .d88888b.         d8888 8888888b.       .d88888b.  888      8888888b.
//   888      d88P" "Y88b       d88888 888  "Y88b     d88P" "Y88b 888      888  "Y88b
//   888      888     888      d88P888 888    888     888     888 888      888    888
//   888      888     888     d88P 888 888    888     888     888 888      888    888
//   888      888     888    d88P  888 888    888     888     888 888      888    888
//   888      888     888   d88P   888 888    888     888     888 888      888    888
//   888      Y88b. .d88P  d8888888888 888  .d88P     Y88b. .d88P 888      888  .d88P
//   88888888  "Y88888P"  d88P     888 8888888P"       "Y88888P"  88888888 8888888P"

int eq(const char *str1, const char *str2)
{
    for (int i=0; i<8; i++)
    {
        if (str1[i] != str2[i]) return i+1;
    }
    return 9;
}

void LoadList(vector<TObj> &list, FILE* fin)
{
    int64_t size;
    (void)fread(&size, 8, 1, fin);
    TObj obj;
    for (int64_t i=0; i<size; i++)
    {
        fread(&obj, 24, 1, fin);
        list.push_back(obj);
    }
}

void read_shell_script(FILE* file, TEval &script)
{
    int32_t len;
    fread(&len, 4, 1, file);
    char str[len+1];
    fread(&str, 1, len, file);
    str[len] = 0;
    script = str;
}

void Space::Load_v1_3(const char* fname)
{
    FILE *fin = fopen(fname, "rb");
    if (!fin) { perror("Error loading the space"); return; }

    //loading different lists
    int64_t tmp = 0;
    char comment[9] = {};
    for (int i=0; i<64; i++)
    {
        fseek(fin, i*16, SEEK_SET);
        fread(&tmp, 8, 1, fin);
        fread(comment, 8, 1, fin);
        if (!tmp) continue;
        fseek(fin, tmp, SEEK_SET);

        if (eq(comment, "Header  ")>8)
        {
            char version[8]; fread(&version, 8, 1, fin);
            if (eq(version, "v: 1.3  ") <= 8)
            {
                fprintf(stderr, "Cant read binary file version \"%s\".\n", version);
                exit(1);
            }

            char name[64];
            fread(name, 1, 64, fin);
            caption = name;
            fread(&time, 8, 1, fin);
            fread(&dt, 8, 1, fin);
            fread(&dt_save, 8, 1, fin);
            fread(&dt_streak, 8, 1, fin);
            fread(&dt_profile, 8, 1, fin);
            fread(&re, 8, 1, fin);
            fread(&pr, 8, 1, fin);
            fread(&inf_marker, 16, 1, fin);
            read_shell_script(fin, inf_vx);
            read_shell_script(fin, inf_vy);
            fread(&inf_g, 8, 1, fin);
            fread(&gravity, 16, 1, fin);
            fread(&finish, 8, 1, fin);

            int64_t rawtime; fread(&rawtime, 8, 1, fin);
        }
        else if (eq(comment, "Vortexes")>8) LoadList(VortexList, fin);
        else if (eq(comment, "Heat    ")>8) LoadList(HeatList, fin);
        else if (eq(comment, "StrkSrc ")>8) LoadList(StreakSourceList, fin);
        else if (eq(comment, "Streak  ")>8) LoadList(StreakList, fin);
        else if (eq(comment, "BData   ")>8)
        {
            std::shared_ptr<TBody> body(new TBody());

            fread(&body->holder, 24, 1, fin);
            fread(&body->dpos, 24, 1, fin);
            read_shell_script(fin, body->speed_x);
            read_shell_script(fin, body->speed_y);
            read_shell_script(fin, body->speed_o);
            fread(&body->speed_slae, 24, 1, fin);
            fread(&body->speed_slae_prev, 24, 1, fin);

            fread(&body->kspring, 24, 1, fin);
            fread(&body->density, 8, 1, fin);

            fread(&body->force_hydro, 24, 1, fin);
            fread(&body->fdt_dead, 24, 1, fin);
            fread(&body->friction_prev, 24, 1, fin);

            BodyList.push_back(body);
        }
        else if (eq(comment, "Body    ")>8)
        {
            auto body = BodyList.back();

            TAtt att(0.0, 0.0);
            int64_t size; fread(&size, 8, 1, fin);
            for (int64_t i=0; i<size; i++)
            {
                int64_t bc, hc;
                double heat_const;
                fread(&att.corner, 16, 1, fin);
                fread(&att.g, 8, 1, fin);
                fread(&bc, 8, 1, fin);
                fread(&hc, 8, 1, fin);
                fread(&heat_const, 8, 1, fin);
                fread(&att.gsum, 8, 1, fin);
                att.heat_const = heat_const;
                if (bc == 'l' || bc == 'n')
                    att.slip = bc == 'l';
                else
                    switch (bc)
                    {
                        case 's': 
                        case 'i': body->boundary_condition = bc_t::steady; break;
                        case 'z': body->boundary_condition = bc_t::kutta; break;
                    }
                switch (hc)
                {
                    case 'n': body->heat_condition = hc_t::neglect; break;
                    case 'i': body->heat_condition = hc_t::isolate; break;
                    case 't': body->heat_condition = hc_t::const_t; break;
                    case 'w': body->heat_condition = hc_t::const_w; break;
                }

                body->alist.push_back(att);
            }
            body->doUpdateSegments();
            body->doFillProperties();
        }
        else {
            fprintf(stderr, "S->Load(): ignoring field \"%s\"\n", comment);
            exit(-1);
        }
    }

    EnumerateBodies();

    return;
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
