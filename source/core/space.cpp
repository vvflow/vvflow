#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <cstring>
#include <fstream>
#include <time.h>
#include <type_traits> // static_assert

#include "space.h"
#include "body.h"
#include "space_hdf.cpp"

#ifndef DEF_GITINFO
#define DEF_GITINFO "not available"
#endif
#ifndef DEF_GITDIFF
#define DEF_GITDIFF "not available"
#endif
static const char* gitInfo = DEF_GITINFO;
static const char* gitDiff = DEF_GITDIFF;
const char* Space::getGitInfo() {return gitInfo;}
const char* Space::getGitDiff() {return gitDiff;}

using std::cout;
using std::cerr;
using std::endl;
using std::fstream;
using std::ios;

// FIXME убрать везде using namespace std

Space::Space():
    caption(),
    InfSpeedX(),
    InfSpeedY(),
    Time(), dt(1, 1),
    dt_save(), dt_streak(), dt_profile()
{
    // static_assert(std::is_pod<TVec>::value, "TVec is not POD");
    // static_assert(std::is_pod<TObj>::value, "TObj is not POD");
    // static_assert(std::is_pod<TAtt>::value, "TAtt is not POD");
    InfCirculation = 0.;
    gravitation = TVec(0., 0.);
    Finish = std::numeric_limits<double>::max();
    Re = std::numeric_limits<double>::infinity();
    Pr = 0.;
    InfMarker = TVec(0., 0.);
}

    inline
void Space::FinishStep()
{
    Time= TTime::add(Time, dt);
}

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
        TAtt att = body.alist[i];
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
    attribute_write(file_dataset, "com", body.get_com());
    attribute_write(file_dataset, "moi_c", body.get_moi_c());

    attribute_write(file_dataset, "boundary_condition", body.boundary_condition);
    attribute_write(file_dataset, "special_segment_no", body.special_segment_no);
    attribute_write(file_dataset, "heat_condition", body.heat_condition);

    H5ASSERT(H5Dwrite(file_dataset, H5T_NATIVE_DOUBLE, H5S_ALL, file_dataspace, H5P_DEFAULT, att_array), "H5Dwrite");
    H5ASSERT(H5Dclose(file_dataset), "H5Dclose");
    free(att_array);
    free(slip_array);
    free(heat_array);
}

void Space::Save(const char* format)
{
    char fname[64]; sprintf(fname, format, int(Time/dt+0.5));
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
    attribute_write(fid, "time", Time);
    attribute_write(fid, "dt", dt);
    attribute_write(fid, "dt_save", dt_save);
    attribute_write(fid, "dt_streak", dt_streak);
    attribute_write(fid, "dt_profile", dt_profile);
    attribute_write(fid, "re", Re);
    attribute_write(fid, "pr", Pr);
    attribute_write(fid, "inf_marker", InfMarker);
    attribute_write<std::string>(fid, "inf_speed_x", InfSpeedX);
    attribute_write<std::string>(fid, "inf_speed_y", InfSpeedY);
    attribute_write(fid, "inf_circulation", InfCirculation);
    attribute_write(fid, "gravity", gravitation);
    attribute_write(fid, "time_to_finish", Finish);
    attribute_write(fid, "git_info", std::string(gitInfo));
    attribute_write(fid, "git_diff", std::string(gitDiff));

    time_t rt; time(&rt);
    char *timestr = ctime(&rt); timestr[strlen(timestr)-1] = 0;
    attribute_write(fid, "time_local", std::string(timestr));

    dataset_write_list("vort", VortexList);
    dataset_write_list("heat", HeatList);
    dataset_write_list("ink", StreakList);
    dataset_write_list("ink_source", StreakSourceList);
    dataset_write_list("source", SourceList);

    for (auto& lbody: BodyList)
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


    attribute_read(dataset, "label", body->label);
    attribute_read(dataset, "holder_position", body->holder);
    attribute_read(dataset, "delta_position", body->dpos);
    attribute_read(dataset, "speed_x", body->speed_x);
    attribute_read(dataset, "speed_y", body->speed_y);
    attribute_read(dataset, "speed_o", body->speed_o);
    attribute_read(dataset, "speed_slae", body->speed_slae);
    attribute_read(dataset, "speed_slae_prev", body->speed_slae_prev);
    attribute_read(dataset, "spring_const", body->kspring);
    attribute_read(dataset, "spring_damping", body->damping);
    attribute_read(dataset, "density", body->density);
    attribute_read(dataset, "force_hydro", body->force_hydro);
    attribute_read(dataset, "force_holder", body->force_holder);
    attribute_read(dataset, "friction_prev", body->friction_prev);

    attribute_write(dataset, "fdt_dead", body->fdt_dead);
    attribute_write(dataset, "g_dead", body->g_dead);

    attribute_read(dataset, "collision_min", body->collision_min);
    attribute_read(dataset, "collision_max", body->collision_max);
    attribute_read(dataset, "bounce", body->bounce);

    std::string root_body_name;
    int root_body_idx;
    attribute_read(dataset, "root_body", root_body_name);
    if (sscanf(root_body_name.c_str(), "body%d", &root_body_idx) == 1)
        body->root_body = S->BodyList[root_body_idx];
    else
        body->root_body.reset();

    struct ATT *att_array = (struct ATT*)malloc(dims[0] * sizeof(struct ATT));
    uint32_t *slip_array = NULL;
    H5ASSERT(H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, file_dataspace, H5P_DEFAULT, att_array), "H5Dread");

    uint32_t simplified_dataset;
    int32_t general_slip;
    double heat_const;
    attribute_read(dataset, "simplified_dataset", simplified_dataset);

    // simplified_dataset по хорошему должен называться dataset_version,
    // но исторически я теперь привязан именно к этому названию.
    // Предыдущая версия с HDF форматом использовала H5T_ENUM, который сейчас не читается
    // и в любом случае отвечает 0. К моему счастью я сохранял только один формат,
    // поэтому отныне 0 будет означать старую версию.
    // Новая версия упрощенного датасета имеет номер 2.
    // Новая версия полноценного датасета имеет номер 3.
    if (simplified_dataset < 2)
    {
        attribute_read(dataset, "general_bc", general_slip);
        attribute_read(dataset, "heat_const", heat_const);
        general_slip = general_slip == 'l';

        int32_t special_bc, special_bc_segment, heat_condition;
        attribute_read(dataset, "special_bc", special_bc);
        attribute_read(dataset, "special_bc_segment", special_bc_segment);
        attribute_read(dataset, "heat_condition", heat_condition);
        body->special_segment_no = special_bc_segment;
        switch (special_bc)
        {
            case 's': 
            case 'i': body->boundary_condition = bc_t::steady; break;
            case 'z': body->boundary_condition = bc_t::kutta; break;
        }
        switch (heat_condition)
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
            slip_array = (uint32_t*)malloc(dims[0] * sizeof(uint32_t));
            hid_t aid = H5Aopen(dataset, "slip", H5P_DEFAULT);
            H5Aread(aid, H5T_NATIVE_UINT32, slip_array);
            H5Aclose(aid);
        }
        else
        {
            attribute_read(dataset, "general_slip", general_slip);
        }
        attribute_read(dataset, "general_heat_const", heat_const);
        attribute_read(dataset, "boundary_condition", body->boundary_condition);
        attribute_read(dataset, "special_segment_no", body->special_segment_no);
        attribute_read(dataset, "heat_condition", body->heat_condition);
    }


    for(hsize_t i=0; i<dims[0]; i++)
    {
        TAtt latt; // latt.body = body;
        latt.corner.x = att_array[i].x;
        latt.corner.y = att_array[i].y;
        latt.g = att_array[i].g;
        latt.gsum = att_array[i].gsum;

        latt.slip = slip_array ? slip_array[i] : general_slip;
        latt.heat_const = heat_const;
        body->alist.push_back(latt);
    }

    body->doUpdateSegments();
    body->doFillProperties();

    H5ASSERT(H5Dclose(dataset), "H5Dclose");
    free(att_array);
    free(slip_array);
    S->BodyList.push_back(body);
    return 0;
}

void Space::Load(const char* fname, std::string *info)
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
    Load(fid, info);

    H5Fclose(fid);
}

void Space::Load(hid_t fid, std::string *info)
{
    datatypes_create_all();

    attribute_read(fid, "caption", caption);
    attribute_read(fid, "time", Time);
    attribute_read(fid, "dt", dt);
    attribute_read(fid, "dt_save", dt_save);
    attribute_read(fid, "dt_streak", dt_streak);
    attribute_read(fid, "dt_profile", dt_profile);
    attribute_read(fid, "re", Re);
    attribute_read(fid, "pr", Pr);
    attribute_read(fid, "inf_marker", InfMarker);
    attribute_read(fid, "inf_speed_x", InfSpeedX);
    attribute_read(fid, "inf_speed_y", InfSpeedY);
    attribute_read(fid, "inf_circulation", InfCirculation);
    attribute_read(fid, "gravity", gravitation);
    attribute_read(fid, "time_to_finish", Finish);

    if (info)
    {
        attribute_read(fid, "git_info", info[0]);
        attribute_read(fid, "git_diff", info[1]);
        attribute_read(fid, "time_local", info[2]);
    }

    dataset_read_list(fid, "vort", VortexList);
    dataset_read_list(fid, "heat", HeatList);
    dataset_read_list(fid, "ink", StreakList);
    dataset_read_list(fid, "ink_source", StreakSourceList);
    dataset_read_list(fid, "source", SourceList);

    H5Literate(fid, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, dataset_read_body, this);
    EnumerateBodies();

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

void read_shell_script(FILE* file, ShellScript &script)
{
    int32_t len;
    fread(&len, 4, 1, file);
    char *str = new char[len+1];
    fread(str, 1, len, file);
    str[len] = 0;
    if (!script.setEvaluator(str))
        fprintf(stderr, "Warning: bad math expression (%s), using 0\n", str);
    delete[] str;
}

void Space::Load_v1_3(const char* fname)
{
    FILE *fin = fopen(fname, "rb");
    if (!fin) { perror("Error loading the space"); return; }

    //loading different lists
    int64_t tmp;
    char comment[9]; comment[8]=0;
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
            fread(&Time, 8, 1, fin);
            fread(&dt, 8, 1, fin);
            fread(&dt_save, 8, 1, fin);
            fread(&dt_streak, 8, 1, fin);
            fread(&dt_profile, 8, 1, fin);
            fread(&Re, 8, 1, fin);
            fread(&Pr, 8, 1, fin);
            fread(&InfMarker, 16, 1, fin);
            read_shell_script(fin, InfSpeedX);
            read_shell_script(fin, InfSpeedY);
            fread(&InfCirculation, 8, 1, fin);
            fread(&gravitation, 16, 1, fin);
            fread(&Finish, 8, 1, fin);

            int64_t rawtime; fread(&rawtime, 8, 1, fin); realtime = rawtime;
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

            TAtt att; // att.body = body; // FIXME
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
        else fprintf(stderr, "S->Load(): ignoring field \"%s\"", comment);
    }

    EnumerateBodies();

    return;
}

FILE* Space::OpenFile(const char* format)
{
    char fname[64]; sprintf(fname, format, int(Time/dt+0.5));
    FILE *fout;
    fout = fopen(fname, "w");
    if (!fout) { perror("Error opening file"); return NULL; }
    return fout;
}

void Space::CalcForces()
{
    const double C_NyuDt_Pi = dt/(C_PI*Re);
    const double C_Nyu_Pi = 1./(C_PI*Re);
    for (auto& lbody: BodyList)
    {
        double tmp_gsum = 0;
        //TObj tmp_fric(0,0,0);
        lbody->friction_prev = lbody->friction;
        lbody->friction = TVec3D();

        for (auto& latt: lbody->alist)
        {
            static_assert(std::is_same<decltype(latt), TAtt&>::value, "latt is not a reference");
            tmp_gsum+= latt.gsum;
            latt.Cp += tmp_gsum;
            latt.Fr += latt.fric * C_NyuDt_Pi;
            latt.Nu += latt.hsum * (Re*Pr / latt.dl.abs());

            lbody->friction.r -= latt.dl * (latt.fric * C_Nyu_Pi / latt.dl.abs());
            lbody->friction.o -= (rotl(latt.r)* latt.dl) * (latt.fric  * C_Nyu_Pi / latt.dl.abs());
            lbody->nusselt += latt.hsum * (Re*Pr);
        }

        lbody->nusselt /= dt;
    }

    //FIXME calculate total pressure
}

void Space::ZeroForces()
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

// FIXME merge in one template
int Space::LoadVorticityFromFile(const char* filename)
{
    FILE *fin = fopen(filename, "r");
    if (!fin) { cerr << "No file called \'" << filename << "\'\n"; return -1; }

    TObj obj(0, 0, 0);
    while ( fscanf(fin, "%lf %lf %lf", &obj.r.x, &obj.r.y, &obj.g)==3 )
    {
        VortexList.push_back(obj);
    }

    fclose(fin);
    return 0;
}

int Space::LoadVorticity_bin(const char* filename)
{
    fstream fin;
    fin.open(filename, ios::in | ios::binary);
    if (!fin) { cerr << "No file called \'" << filename << "\'\n"; return -1; }

    fin.seekg (0, ios::end);
    // size_t N = (size_t(fin.tellg())-1024)/(sizeof(double)*3);
    fin.seekp(1024, ios::beg);

    TObj obj(0, 0, 0);

    while ( fin.good() )
    {
        fin.read(pchar(&obj), 3*sizeof(double));
        VortexList.push_back(obj);
    }

    fin.close();
    return 0;
}

int Space::LoadHeatFromFile(const char* filename)
{
    FILE *fin = fopen(filename, "r");
    if (!fin) { cerr << "No file called " << filename << endl; return -1; }

    TObj obj(0, 0, 0);
    while ( fscanf(fin, "%lf %lf %lf", &obj.r.x, &obj.r.y, &obj.g)==3 )
    {
        HeatList.push_back(obj);
    }

    fclose(fin);
    return 0;
}

int Space::LoadStreak(const char* filename)
{
    FILE *fin = fopen(filename, "r");
    if (!fin) { perror("Error opening streak file"); return -1; }

    TObj obj(0, 0, 0);
    while ( fscanf(fin, "%lf %lf %lf", &obj.r.x, &obj.r.y, &obj.g)==3 )
    {
        StreakList.push_back(obj);
    }

    fclose(fin);
    return 0;
}

int Space::LoadStreakSource(const char* filename)
{
    FILE *fin = fopen(filename, "r");
    if (!fin) { perror("Error opening streak source file"); return -1; }

    TObj obj(0, 0, 0);
    while ( fscanf(fin, "%lf %lf %lf", &obj.r.x, &obj.r.y, &obj.g)==3 )
    {
        StreakSourceList.push_back(obj);
    }

    fclose(fin);
    return 0;
}

int Space::LoadSource(const char* filename)
{
    FILE *fin = fopen(filename, "r");
    if (!fin) { perror("Error opening source file"); return -1; }

    TObj obj(0, 0, 0);
    while ( fscanf(fin, "%lf %lf %lf", &obj.r.x, &obj.r.y, &obj.g)==3 )
    {
        SourceList.push_back(obj);
    }

    fclose(fin);
    return 0;
}


int Space::LoadBody(const char* filename)
{
    int err = 0;
    auto body = std::make_shared<TBody>();
    TAtt att;

    FILE *fin = fopen(filename, "r");
    if (!fin) goto fail;

    char str[128];
    while (!err && !feof(fin) && !ferror(fin) && fgets(str, sizeof(str), fin))
    {
        err |= sscanf(str, "%lf %lf %u", &att.corner.x, &att.corner.y, &att.slip) < 2;
        body->alist.push_back(att);
    }

    err |= ferror(fin);
    fclose(fin);

    if (err)
    {
fail:
        errno = errno?:EINVAL;
        perror("Error parsing body from file");
        return -1;
    }

    BodyList.push_back(body);
    body->doUpdateSegments();
    body->doFillProperties();
    EnumerateBodies();

    return 0;
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

void Space::ZeroSpeed()
{
    for (auto& lobj: VortexList) lobj.v = TVec();
    for (auto& lobj: HeatList) lobj.v = TVec();
}

double Space::integral()
{
    double res = 0;
    for (const auto& obj: VortexList) res += obj.g * obj.r.abs2();
    return res;
}

double Space::gsum()
{
    double res = 0;
    for (const auto& obj: VortexList) res += obj.g;
    return res;
}

double Space::gmax()
{
    double res = 0;
    for (const auto& obj: VortexList) res = ( fabs(obj.g) > fabs(res) ) ? obj.g : res;
    return res;
}

TVec Space::HydroDynamicMomentum()
{
    TVec res(0., 0.);
    for (const auto& obj: VortexList) res += obj.g * obj.r;
    return res;
}

double Space::AverageSegmentLength()
{
    if (!BodyList.size()) return std::numeric_limits<double>::lowest();

    double SurfaceLength = BodyList.front()->get_surface();
    int N = BodyList.front()->size() - 1;
    if (N<=0) return std::numeric_limits<double>::lowest();
    return SurfaceLength / N;
}

int Space::TotalSegmentsCount()
{
    int res = 0;
    for (const auto& lbody: BodyList)
    {
        res += lbody->alist.size();
    }

    return res;
}

bool Space::PointIsInBody(TVec p)
{
    for (const auto& lbody: BodyList)
    {
        if (lbody->isPointInvalid(p)) return true;
    }
    return false;
}

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



