#include "stepdata.h"
#include "stddef.h"
#include "hdf5.h"

#define H5ASSERT(expr, msg) if (expr<0) { \
    fprintf(stderr, "%s failed (%s:%d). Aborting.\n", msg, __FILE__, __LINE__); \
    std::exit(5); \
}

Stepdata::Stepdata(Space* s_, const char *fname, bool b_save_profile)
{
    S = s_;
    blsize = S->BodyList.size();
    this->b_save_profile = b_save_profile;
    last_flush_time = 0;

    scalar_h5s = H5Screate(H5S_SCALAR);
    string_h5t = H5Tcopy(H5T_C_S1);
    H5Tset_size(string_h5t, H5T_VARIABLE);

    time_h5d = -1;
    force_born_h5d.resize(blsize, -1);
    force_hydro_h5d.resize(blsize, -1);
    force_holder_h5d.resize(blsize, -1);
    force_friction_h5d.resize(blsize, -1);
    nusselt_h5d.resize(blsize, -1);
    position_h5d.resize(blsize, -1);
    spring_h5d.resize(blsize, -1);
    speed_h5d.resize(blsize, -1);
    pressure_h5d.resize(blsize, -1);
    friction_h5d.resize(blsize, -1);

    size_t rows = 0;
    if (H5Fis_hdf5(fname)<=0)
    {
        h5f = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        H5ASSERT(h5f, "H5Hcreate");
        H5Tcommit(h5f, "string_t", string_h5t, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        attribute_write("caption", S->caption.c_str());
        attribute_write("git_info", S->getGitInfo());
        attribute_write("git_diff", S->getGitDiff());
    } else {
        h5f = H5Fopen(fname, H5F_ACC_RDWR, H5P_DEFAULT);
        H5ASSERT(h5f, "H5Fopen");

        // TODO update git_info, git_diff
        if (H5Lexists(h5f, "time", H5P_DEFAULT))
        {
            hid_t h5d = H5Dopen(h5f, "time", H5P_DEFAULT);
            hid_t h5s = H5Dget_space(h5d);
            hsize_t dims[2];
            H5Sget_simple_extent_dims(h5s, dims, NULL);
            float* tbuf = new float[dims[0]*dims[1]];
            H5Dread(h5d, H5T_NATIVE_FLOAT, h5s, h5s, H5P_DEFAULT, tbuf);
            for (rows=0; rows<dims[0] && tbuf[rows*dims[1]] < float(S->Time); rows++)
            {
                /* DO NOTHING */;
            }
            H5Sclose(h5s);
            H5Dclose(h5d);
            delete [] tbuf;
        }
    }

    time_h5d = h5d_init(h5f, "time", rows, 1);
    for (auto& lbody: S->BodyList)
    {
        int body_n = S->get_body_index(lbody.get());
        if (body_n > blsize) continue;
        const char *bname = S->get_body_name(lbody.get()).c_str();
        hid_t body_h5g;
        if (!H5Lexists(h5f, bname, H5P_DEFAULT))
        {
            body_h5g = H5Gcreate(h5f, bname, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            H5ASSERT(body_h5g, "H5Gcreate");
        } else {
            body_h5g = H5Gopen(h5f, bname, H5P_DEFAULT);
            H5ASSERT(body_h5g, "H5Gopen");
        }

        force_born_h5d[body_n]     = h5d_init(body_h5g, "force_born", rows, 3);
        force_hydro_h5d[body_n]    = h5d_init(body_h5g, "force_hydro", rows, 3);
        force_holder_h5d[body_n]   = h5d_init(body_h5g, "force_holder", rows, 3);
        force_friction_h5d[body_n] = h5d_init(body_h5g, "force_friction", rows, 3);
        // nusselt_h5d[body_n]        = h5d_init(body_h5g, "nusselt", rows, 1);
        position_h5d[body_n]       = h5d_init(body_h5g, "holder_position", rows, 3);
        spring_h5d[body_n]         = h5d_init(body_h5g, "delta_position", rows, 3);
        speed_h5d[body_n]          = h5d_init(body_h5g, "speed_slae", rows, 3);
        if (b_save_profile)
        {
            pressure_h5d[body_n]   = h5d_init(body_h5g, "pressure", rows, lbody->size());
            friction_h5d[body_n]   = h5d_init(body_h5g, "friction", rows, lbody->size());
        } else {
            if (H5Lexists(body_h5g, "pressure", H5P_DEFAULT) > 0)
            {
                // shrink dataset
                hid_t h5d = h5d_init(body_h5g, "pressure", rows, lbody->size());
                H5Dclose(h5d);
            }
            if (H5Lexists(body_h5g, "friction", H5P_DEFAULT) > 0)
            {
                // shrink dataset
                hid_t h5d = h5d_init(body_h5g, "friction", rows, lbody->size());
                H5Dclose(h5d);
            }
        }

        H5Gclose(body_h5g);
    }
}

Stepdata::~Stepdata()
{
    H5Dclose(time_h5d);
    for (int i=0; i<blsize; i++)
    {
        if (force_born_h5d[i]     >= 0) H5Dclose(force_born_h5d[i]);
        if (force_hydro_h5d[i]    >= 0) H5Dclose(force_hydro_h5d[i]);
        if (force_holder_h5d[i]   >= 0) H5Dclose(force_holder_h5d[i]);
        if (force_friction_h5d[i] >= 0) H5Dclose(force_friction_h5d[i]);
        if (nusselt_h5d[i]        >= 0) H5Dclose(nusselt_h5d[i]);
        if (position_h5d[i]       >= 0) H5Dclose(position_h5d[i]);
        if (spring_h5d[i]         >= 0) H5Dclose(spring_h5d[i]);
        if (speed_h5d[i]          >= 0) H5Dclose(speed_h5d[i]);
        if (pressure_h5d[i]       >= 0) H5Dclose(pressure_h5d[i]);
        if (friction_h5d[i]       >= 0) H5Dclose(friction_h5d[i]);
    }

    H5Tclose(string_h5t);
    H5Sclose(scalar_h5s);
    H5Fclose(h5f);
}

void Stepdata::flush()
{
    last_flush_time = 0;
}

void Stepdata::write()
{
    append(time_h5d, S->Time);

    for (auto& lbody: S->BodyList)
    {
        int body_n = S->get_body_index(lbody.get());
        if (body_n > blsize) continue;
        append(force_born_h5d[body_n], lbody->force_born - lbody->force_dead);
        append(force_hydro_h5d[body_n], lbody->force_hydro);
        append(force_holder_h5d[body_n], lbody->force_holder);
        append(force_friction_h5d[body_n], lbody->friction);
        append(nusselt_h5d[body_n], lbody->nusselt);
        append(position_h5d[body_n], lbody->holder);
        append(spring_h5d[body_n], lbody->dpos);
        append(speed_h5d[body_n], lbody->speed_slae);

        if (b_save_profile)
        {
            double pressure_buf[lbody->size()];
            double friction_buf[lbody->size()];
            for (size_t s=0; s<lbody->size(); s++)
            {
                TAtt &latt = lbody->alist[s];
                pressure_buf[s] = latt.Cp/S->dt;
                friction_buf[s] = latt.Fr/S->dt;
                latt.Cp = latt.Fr = latt.Nu = 0;
            }
            append(pressure_h5d[body_n], pressure_buf);
            append(friction_h5d[body_n], friction_buf);
        }
    }

    struct timespec clk;
    clock_gettime(CLOCK_MONOTONIC, &clk);
    if (clk.tv_sec - last_flush_time > 20)
    {
        H5Fflush(h5f, H5F_SCOPE_GLOBAL);
        last_flush_time = clk.tv_sec;
    }
}

/*****************************************************************************/
/** PRIVATE ******************************************************************/
/*****************************************************************************/

void Stepdata::attribute_write(const char *name, const char *str)
{
    hid_t aid = H5Acreate2(h5f, name, string_h5t, scalar_h5s, H5P_DEFAULT, H5P_DEFAULT);
    H5ASSERT(aid, "H5Acreate");
    H5ASSERT(H5Awrite(aid, string_h5t, &str), "H5Awrite");
    H5ASSERT(H5Aclose(aid), "H5Aclose");
}

void Stepdata::append(int dataspace_hid, const void *buf)
{
    if (dataspace_hid < 0) return;
    hsize_t dims[2];
    hsize_t ext[2];
    hsize_t offset[2];
    hid_t filespace = H5Dget_space(dataspace_hid);
    H5Sget_simple_extent_dims(filespace, dims, NULL);
    offset[0] = dims[0];
    offset[1] = 0;
    ext[0] = 1;
    ext[1] = dims[1];
    dims[0]++;
    H5Dset_extent(dataspace_hid, dims);
    filespace = H5Dget_space(dataspace_hid);
    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, ext, NULL);

    hid_t memspace = H5Screate_simple(2, ext, NULL);
    H5Dwrite(dataspace_hid, H5T_NATIVE_DOUBLE, memspace, filespace, H5P_DEFAULT, buf);

    H5Sclose(filespace);
    H5Sclose(memspace);
}

void Stepdata::append(int dataspace_hid, double value)
{
    append(dataspace_hid, &value);
}

void Stepdata::append(int dataspace_hid, TVec3D value)
{
    double arr[3] = {value.r.x, value.r.y, value.o};
    append(dataspace_hid, arr);
}

int Stepdata::h5d_init(int loc_id, const char *name, size_t rows, size_t cols)
{
    hid_t h5d;
    if (H5Lexists(loc_id, name, H5P_DEFAULT))
    {
        h5d = H5Dopen(loc_id, name, H5P_DEFAULT);
        hid_t h5s = H5Dget_space(h5d);

        hsize_t cur_dims[2];
        hsize_t new_dims[2] = {rows, cols};
        H5Sget_simple_extent_dims(h5s, cur_dims, NULL);
        H5ASSERT(H5Dset_extent(h5d, new_dims), "H5Dset_extent");
        H5Sclose(h5s);
    } else {
        hid_t h5p = H5Pcreate(H5P_DATASET_CREATE);
        hsize_t chunk_dims[2] = {0x80, cols};
        H5Pset_chunk(h5p, 2, chunk_dims);
        H5Pset_deflate(h5p, 9);
        float NaN = std::numeric_limits<float>::quiet_NaN();
        H5Pset_fill_value(h5p, H5T_NATIVE_FLOAT, &NaN);
        H5Pset_fill_time(h5p, H5D_FILL_TIME_ALLOC);

        hsize_t cur_dims[2] = {rows, cols};
        hsize_t max_dims[2] = {H5S_UNLIMITED, cols};
        hid_t h5s = H5Screate_simple(2, cur_dims, max_dims);
        h5d = H5Dcreate(loc_id, name, H5T_NATIVE_FLOAT, h5s, H5P_DEFAULT, h5p, H5P_DEFAULT);
        H5Sclose(h5s);
        H5Pclose(h5p);
    }

    return h5d;
}
