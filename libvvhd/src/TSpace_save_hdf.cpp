#define H5_USE_18_API
#include <hdf5.h>

#include "TSpace.hpp"
#include "elementary.h"
#include "vvhdf.h"

// #include <cmath>
// #include <float.h>
// #include <cstdio>
// #include <cstdlib>
#include <cstring>
#include <map>
// #include <fstream>
// #include <ctime>

using std::vector;

static void dataset_write_list(hid_t fid, std::string name, const vector<TObj>& list)
{
    if (list.empty())
        return;
    
    // 1D dataspace
    hsize_t N = list.size();
    hsize_t dims1[2] = {N, 3};
    hsize_t dims2[2] = {N*2, 3};
    hsize_t chunkdims[2] = {std::min<hsize_t>(512, N), 3};
    hid_t prop = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(prop, 2, chunkdims);
    H5Pset_deflate(prop, 9);

    hid_t h5s_file = H5Screate_simple(2, dims1, dims1);
    if (h5s_file < 0)
        h5_throw("H5Screate_simple", name);

    hid_t h5s_mem = H5Screate_simple(2, dims2, dims2);
    if (h5s_file < 0)
        h5_throw("H5Screate_simple", name);
    
    hsize_t start[2] = {0, 0};
    hsize_t stride[2] = {2, 1};
    herr_t err = H5Sselect_hyperslab(h5s_mem, H5S_SELECT_SET, start, stride, dims1, NULL);
    if (err < 0)
        h5_throw("H5Sselect_hyperslab", name);

    hid_t h5d = H5Dcreate(fid, name.c_str(), H5T_NATIVE_DOUBLE, h5s_file, H5P_DEFAULT, prop, H5P_DEFAULT);
    if (h5d < 0)
        h5_throw("H5Dcreate", name);

    err = H5Dwrite(h5d, H5T_NATIVE_DOUBLE, h5s_mem, h5s_file, H5P_DEFAULT, list.data());
    if (err < 0)
        h5_throw("H5Dwrite", name);
    err = H5Dclose(h5d);
    if (err < 0)
        h5_throw("H5Dclose", name);
}

static void dataset_write_body(hid_t fid, std::string name, std::string root_name, const TBody& body)
{
    bool can_simplify_slip = true;
    bool can_simplify_heat = true;

    hsize_t N = body.size();
    hsize_t dims[2] = {N, 4};

    double att_array[N][4] = {0};
    
    uint32_t general_slip = body.alist.front().slip;
    uint32_t slip_array[N] = {0};
    
    float heat_const = body.alist.front().heat_const;
    float heat_array[N] = {0};
    
    for (hsize_t i=0; i<N; i++) {
        const TAtt& att = body.alist[i];
        att_array[i][0] = att.corner.x;
        att_array[i][1] = att.corner.y;
        att_array[i][2] = att.g;
        att_array[i][3] = att.gsum;
        slip_array[i] = att.slip;
        heat_array[i] = att.heat_const;

        can_simplify_slip &= (att.slip == general_slip);
        can_simplify_heat &= (att.heat_const == heat_const);
    }

    hsize_t chunkdims[2] = {std::min<hsize_t>(512, N), 4};
    hid_t prop = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(prop, 2, chunkdims);
    H5Pset_deflate(prop, 9);

    hid_t h5s_file = H5Screate_simple(2, dims, dims);
    if (h5s_file < 0)
        h5_throw("H5Screate_simple", name);

    hid_t h5d = H5Dcreate(fid, name.c_str(), H5T_NATIVE_DOUBLE, h5s_file, H5P_DEFAULT, prop, H5P_DEFAULT);
    if (h5d < 0)
        h5_throw("H5Dcreate", name);

    if (!can_simplify_slip) {
        h5t<uint32_t>::init();
        
        hid_t h5s_slip = H5Screate_simple(1, dims, dims);
        if (h5s_slip < 0)
            h5_throw("H5Screate_simple", name);

        hid_t aid = H5Acreate(h5d, "slip", h5t<uint32_t>::id, h5s_slip, H5P_DEFAULT, H5P_DEFAULT);
        if (aid < 0)
            h5_throw("H5Acreate", name);

        herr_t err = H5Awrite(aid, h5t<uint32_t>::id, slip_array);
        if (err < 0)
            h5_throw("H5Awrite", name);

        err = H5Aclose(aid);
        if (err < 0)
            h5_throw("H5Aclose", name);
    } else {
        h5a_write<uint32_t> (h5d, "general_slip", general_slip);
    }

    if (!can_simplify_heat) {
        throw std::runtime_error("Non-simplified heat not implemented");
    } else {
        h5a_write<double> (h5d, "general_heat_const", heat_const);
    }

    h5a_write<uint32_t> (h5d, "simplified_dataset", 2);
        

    h5a_write<std::string const&> (h5d, "label", body.label);
    h5a_write<std::string const&> (h5d, "root_body", root_name);
    h5a_write<TVec3D> (h5d, "holder_position", body.holder);
    h5a_write<TVec3D> (h5d, "delta_position", body.dpos);
    h5a_write<std::string const&> (h5d, "speed_x", body.speed_x);
    h5a_write<std::string const&> (h5d, "speed_y", body.speed_y);
    h5a_write<std::string const&> (h5d, "speed_o", body.speed_o);
    h5a_write<TVec3D> (h5d, "speed_slae", body.speed_slae);
    h5a_write<TVec3D> (h5d, "speed_slae_prev", body.speed_slae_prev);
    h5a_write<TVec3D> (h5d, "spring_const", body.kspring);
    h5a_write<TVec3D> (h5d, "spring_damping", body.damping);
    h5a_write<double> (h5d, "density", body.density);
    h5a_write<TVec3D> (h5d, "force_hydro", body.force_hydro);
    h5a_write<TVec3D> (h5d, "force_holder", body.force_holder);
    h5a_write<TVec3D> (h5d, "friction_prev", body.friction_prev);

    h5a_write<TVec3D> (h5d, "fdt_dead", body.fdt_dead);
    h5a_write<double> (h5d, "g_dead", body.g_dead);

    h5a_write<TVec3D> (h5d, "collision_min", body.collision_min);
    h5a_write<TVec3D> (h5d, "collision_max", body.collision_max);
    h5a_write<double> (h5d, "bounce", body.bounce);

    h5a_write<double> (h5d, "area", body.get_area());
    h5a_write<TVec> (h5d, "cofm", body.get_cofm());
    h5a_write<double> (h5d, "moi_cofm", body.get_moi_cofm());
    h5a_write<double> (h5d, "moi_axis", body.get_moi_axis());

    h5a_write<int32_t> (h5d, "special_segment_no", body.special_segment_no);
    h5a_write<bc_t> (h5d, "boundary_condition", body.boundary_condition);
    h5a_write<hc_t> (h5d, "heat_condition", body.heat_condition);

    herr_t err = H5Dwrite(h5d, H5T_NATIVE_DOUBLE, H5S_ALL, h5s_file, H5P_DEFAULT, att_array);
    if (err < 0)
        h5_throw("H5Dwrite", name);
    err = H5Dclose(h5d);
    if (err < 0)
        h5_throw("H5Dclose", name);
}

void Space::save(const char* format)
{
    for (std::shared_ptr<TBody>& lbody: BodyList)
    {
        lbody->validate(/*critical=*/true);
    }

    char fname[64]; sprintf(fname, format, int(time/dt+0.5));
    hid_t fid = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (fid < 0) h5_throw("H5Fcreate", fname);

    h5a_write<std::string const&> (fid, "caption", caption);
    h5a_write<TTime> (fid, "time", time);
    h5a_write<TTime> (fid, "dt", dt);
    h5a_write<TTime> (fid, "dt_save", dt_save);
    h5a_write<TTime> (fid, "dt_streak", dt_streak);
    h5a_write<TTime> (fid, "dt_profile", dt_profile);
    h5a_write<double> (fid, "re", re);
    h5a_write<double> (fid, "pr", pr);
    h5a_write<TVec> (fid, "inf_marker", inf_marker);
    h5a_write<std::string const&> (fid, "inf_speed_x", inf_vx);
    h5a_write<std::string const&> (fid, "inf_speed_y", inf_vy);
    h5a_write<double> (fid, "inf_circulation", inf_g);
    h5a_write<TVec> (fid, "gravity", gravity);
    h5a_write<double> (fid, "time_to_finish", finish);
    h5a_write<const char*> (fid, "git_rev",  libvvhd_gitrev);
    h5a_write<const char*> (fid, "git_info", libvvhd_gitinfo);
    h5a_write<const char*> (fid, "git_diff", libvvhd_gitdiff);

    time_t rt;
    ::time(&rt);
    char *timestr = ctime(&rt); timestr[strlen(timestr)-1] = 0;
    h5a_write<const char*> (fid, "time_local", timestr);

    dataset_write_list(fid, "vort", VortexList);
    dataset_write_list(fid, "heat", HeatList);
    dataset_write_list(fid, "ink", StreakList);
    dataset_write_list(fid, "ink_source", StreakSourceList);
    dataset_write_list(fid, "source", SourceList);

    std::map<TBody*,std::string> body_names;
    for (size_t i = 0; i < BodyList.size(); i++) {
        std::shared_ptr<TBody>& lbody = BodyList[i];
        char body_name[32] = {};
        snprintf(body_name, 32, "body%02zd", i);
        body_names[lbody.get()] = body_name;
    }

    for (std::shared_ptr<TBody>& lbody: BodyList) {
        std::string body_name = body_names[lbody.get()];
        std::string root_name;
        if (!lbody->root_body.expired())
        {
            auto root_body = lbody->root_body.lock();
            root_name = body_names[root_body.get()];
        }

        dataset_write_body(fid, body_name.c_str(), root_name, *lbody);
    }

    h5t_commit_all(fid);
    h5t_close_all();
    herr_t err = H5Fclose(fid);
    if (err < 0)
        h5_throw("H5Fclose", fname);
}
