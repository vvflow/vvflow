#define H5_USE_18_API
#include <hdf5.h>

#include "TSpace.hpp"
#include "elementary.h"
#include "vvhdf.h"

#include <map>
typedef std::map<std::string, std::shared_ptr<TBody>> body_map_t;

static void dataset_read_list(hid_t fid, std::string name, std::vector<TObj>& list)
{
    if (!H5Lexists(fid, name.c_str(), H5P_DEFAULT))
        return;

    hid_t h5d = H5Dopen(fid, name.c_str(), H5P_DEFAULT);
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
}

static void dataset_read_body(hid_t hid, std::string name, body_map_t& body_map)
{
    TBody& body = *body_map[name];

    hid_t h5d = H5Dopen(hid, name.c_str(), H5P_DEFAULT);
    if (h5d < 0)
        h5_throw("H5Dopen", name);

    hid_t h5s_file = H5Dget_space(h5d);
    if (h5s_file < 0)
        h5_throw("H5Dget_space", name);
    hsize_t dims[2];
    H5Sget_simple_extent_dims(h5s_file, dims, dims);
    hsize_t N = dims[0];

    body. label = h5a_read<std::string> (h5d, "label");
    body.holder = h5a_read<TVec3D> (h5d, "holder_position");
    body.dpos = h5a_read<TVec3D> (h5d, "delta_position");
    body.speed_x = h5a_read<std::string> (h5d, "speed_x");
    body.speed_y = h5a_read<std::string> (h5d, "speed_y");
    body.speed_o = h5a_read<std::string> (h5d, "speed_o");
    body.force_o = h5a_read<std::string> (h5d, "force_o");
    body.speed_slae = h5a_read<TVec3D> (h5d, "speed_slae");
    body.speed_slae_prev = h5a_read<TVec3D> (h5d, "speed_slae_prev");
    body.kspring = h5a_read<TVec3D> (h5d, "spring_const");
    body.damping = h5a_read<TVec3D> (h5d, "spring_damping");
    body.density = h5a_read<double> (h5d, "density");
    body.force_hydro = h5a_read<TVec3D> (h5d, "force_hydro");
    body.force_holder = h5a_read<TVec3D> (h5d, "force_holder");
    body.friction_prev = h5a_read<TVec3D> (h5d, "friction_prev");

    body.fdt_dead = h5a_read<TVec3D> (h5d, "fdt_dead");
    body.g_dead = h5a_read<double> (h5d, "g_dead");

    body.collision_min = h5a_read<TVec3D> (h5d, "collision_min");
    body.collision_max = h5a_read<TVec3D> (h5d, "collision_max");
    body.bounce = h5a_read<double> (h5d, "bounce");

    std::string root_name = h5a_read<std::string> (h5d, "root_body");
    if (root_name.empty()) {
        body.root_body.reset();
    } else if (body_map.find(root_name) == body_map.end()) {
        fprintf(stderr, "The root '%s' of a body '%s' not found. Resetting.\n", root_name.c_str(), name.c_str());
        body.root_body.reset();
    } else {
        body.root_body = body_map[root_name];
    }

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

        body.special_segment_no = h5a_read<int32_t> (h5d, "special_bc_segment");
        switch (h5a_read<int32_t> (h5d, "special_bc"))
        {
            case 's':
            case 'i': body.boundary_condition = bc_t::steady; break;
            case 'z': body.boundary_condition = bc_t::kutta; break;
        }
        switch (h5a_read<int32_t>(h5d, "heat_condition"))
        {
            case 'n': body.heat_condition = hc_t::neglect; break;
            case 'i': body.heat_condition = hc_t::isolate; break;
            case 't': body.heat_condition = hc_t::const_t; break;
            case 'w': body.heat_condition = hc_t::const_w; break;
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
        body.special_segment_no = h5a_read<int32_t> (h5d, "special_segment_no");
        body.boundary_condition = h5a_read<bc_t> (h5d, "boundary_condition");
        body.heat_condition     = h5a_read<hc_t> (h5d, "heat_condition");
        heat_const = h5a_read<double> (h5d, "general_heat_const");
    }

    for(hsize_t i=0; i<N; i++)
    {
        body.alist.emplace_back(att_array[i][0], att_array[i][1]);
        TAtt& latt = body.alist.back();
        latt.g = att_array[i][2];
        latt.gsum = att_array[i][3];
        latt.slip = slip_array[i];
        latt.heat_const = heat_const;
    }

    body.doUpdateSegments();
    body.doFillProperties();

    err = H5Dclose(h5d);
    if (err < 0)
        h5_throw("H5Dclose", name);
}

static herr_t list_bodies(hid_t g_id, const char* name, const H5L_info_t*, void *op_data)
{
    body_map_t& body_map = *(body_map_t*)op_data;
    if (std::string(name, 4) == "body") {
        body_map[name] = std::make_shared<TBody>();
    }
    return 0;
}

void Space::load_hdf(int64_t fid, metainfo_t *info)
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
        info->git_rev = h5a_read<std::string>(fid, "git_rev");
        info->git_info = h5a_read<std::string>(fid, "git_info");
        info->git_diff = h5a_read<std::string>(fid, "git_diff");
        info->time_local = h5a_read<std::string>(fid, "time_local");
    }

    dataset_read_list(fid, "vort", VortexList);
    dataset_read_list(fid, "heat", HeatList);
    dataset_read_list(fid, "ink", StreakList);
    dataset_read_list(fid, "ink_source", StreakSourceList);
    dataset_read_list(fid, "source", SourceList);

    body_map_t body_map;
    H5Literate(fid, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, list_bodies, &body_map);

    for (const auto& pair: body_map) {
        const std::string& name = pair.first;
        const std::shared_ptr<TBody>& lbody = pair.second;

        dataset_read_body(fid, name, body_map);
        BodyList.push_back(lbody);
    }

    EnumerateBodies();
    for (std::shared_ptr<TBody>& lbody: BodyList) {
        lbody->validate(/*critical=*/false);
    }

    h5t_close_all();
}