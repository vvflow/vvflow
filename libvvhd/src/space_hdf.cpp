#include <limits>
#include <hdf5.h>

static hid_t fid;

struct ATT {
    double x, y, g, gsum;
};
