#include "XField.hpp"

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

    float result = *map;
    for (int i=0; i<xres*yres; i++) {
        result = std::min(result, map[i]);
    }
    return result;
}

float XField::max() const
{
    if (!evaluated) {
        throw std::invalid_argument("XField::max(): not evaluated");
    }

    float result = *map;
    for (int i=0; i<xres*yres; i++) {
        result = std::max(result, map[i]);
    }
    return result;
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
