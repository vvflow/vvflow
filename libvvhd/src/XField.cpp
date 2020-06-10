#include "XField.hpp"

#include <cmath>
#include <stdexcept>
#include <algorithm>

XField::XField(
    float xmin, float ymin, float dxdy,
    int xres, int yres
):
    xmin(xmin), ymin(ymin), dxdy(dxdy),
    xres(xres), yres(yres),
    map(new float[xres*yres]),
    gaps(),
    evaluated(false)
{
    if (xres <= 1)
        throw std::invalid_argument("XField(): xres must be > 1");
    if (yres <= 1)
        throw std::invalid_argument("XField(): yres must be > 1");
    if (!std::isfinite(dxdy))
        throw std::invalid_argument("XField(): dxdy must be finite");
    if (dxdy <= 0)
        throw std::invalid_argument("XField(): dxdy must be positive");
}

XField::XField(const std::string& str):
    xmin(0), ymin(0), dxdy(0),
    xres(0), yres(0),
    map(new float[xres*yres]),
    gaps(),
    evaluated(false)
{
    if (str.size() % sizeof(float) != 0)
        throw std::invalid_argument("XField(str): odd string");
    if (str.size() < sizeof(float) * 9) // 2x2 matrix minimum
        throw std::invalid_argument("XField(str): insufficient length");

    const float* data = reinterpret_cast<const float*>(str.c_str());
    float N = data[0];
    if (!std::isfinite(N) || N <= 1 || fmod(N, 1) != 0) {
        fprintf(stderr, "N = %lg\n", N);
        throw std::invalid_argument("XField(str): invalid data");
    }

    xres = N;
    yres = str.size()/sizeof(float)/(xres+1) - 1;
    if (str.size() != sizeof(float)*(xres+1)*(yres+1))
        throw std::invalid_argument("XField(str): can't factorize matrix");

    float dx = data[2] - data[1];
    float dy = data[2*(xres+1)] - data[1*(xres+1)];
    if (fabs(dx-dy) > std::min(fabs(dx), fabs(dy))/1000 ) {
        fprintf(stderr, "Bad spacing: %lg != %lg (diff %lg)\n",
            dx, dy, fabs(dx-dy)
        );
        throw std::invalid_argument("XField(str): non-uniform grid");
    }

    xmin = data[1];
    ymin = data[1*(xres+1)];
    dxdy = data[2]-data[1];
    map = new float[xres*yres];
    for (int yj=0; yj<yres; yj++)
    {
        for (int xi=0; xi<xres; xi++)
        {
            map[yj*xres+xi] = data[(yj+1)*(xres+1) + (xi+1)];
        }
    }
    evaluated = true;
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

    size_t N = std::floor(v.size()*p);
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
