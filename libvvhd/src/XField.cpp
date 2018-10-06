#include "XField.hpp"

#include <stdexcept>
#include <algorithm>

XField::XField(
    double xmin, double ymin, double dxdy,
    int xres, int yres
):
    xmin(xmin), ymin(ymin), dxdy(dxdy),
    xres(xres), yres(yres),
    map(new float[xres*yres]),
    gaps(),
    evaluated(false)
{
    if (xres <= 0)
        throw std::invalid_argument("XField(): xres must be positive");
    if (yres <= 0)
        throw std::invalid_argument("XField(): yres must be positive");
    if (dxdy <= 0)
        throw std::invalid_argument("XField(): dxdy must be positive");
}

XField::~XField()
{
    delete[] map;
}

float XField::min() const
{
    if (!evaluated) {
        throw std::invalid_argument("XField::min(): not evaluated");
    }
    return *std::min_element(&map[0], &map[xres*yres]);
}

float XField::max() const
{
    if (!evaluated) {
        throw std::invalid_argument("XField::max(): not evaluated");
    }
    return *std::max_element(&map[0], &map[xres*yres]);
}

static bool gt(float x, float y) { return x>y; }
static bool lt(float x, float y) { return x<y; }
float XField::percentile(float p) const
{
    if (!evaluated) {
        throw std::invalid_argument("XField::percentile(): not evaluated");
    }

    if (!(p>0 && p<1)) {
        throw std::invalid_argument("XField::percentile(p): p must be in range (0, 1)");
    }

    bool (*cmp)(float, float);
    if (p < 0.5) {
        cmp = lt;
    } else {
        p = 1 - p;
        cmp = gt;
    }

    std::vector<float> v(xres*yres);
    auto it = std::copy_if(&map[0], &map[xres*yres], v.begin(), [](float x){return x!=0;});
    v.resize(std::distance(v.begin(),it));

    size_t N = floor(v.size()*p);
    std::partial_sort(v.begin(), v.begin()+N, v.end(), cmp);
    return v[N-1];
}

std::ostream& operator<< (std::ostream& os, const XField& field)
{
    if (!field.evaluated) {
        throw std::invalid_argument("XField::operator<<: not evaluated");
    }

    float N = field.xres;
    os.write(reinterpret_cast<const char*>(&N), sizeof(float));
    for (int xi=0; xi<field.xres; xi++) {
        float x = field.xmin + field.dxdy*xi;
        os.write(
            reinterpret_cast<const char*>(&x),
            sizeof(float)
        );
    }

    for (int yj=0; yj<field.yres; yj++) {
        float y = field.ymin + field.dxdy*yj;
        os.write(
            reinterpret_cast<const char*>(&y),
            sizeof(float)
        );
        os.write(
            reinterpret_cast<const char*>(field.map+yj*field.xres),
            sizeof(float)*field.xres
        );
    }

    return os;
}
