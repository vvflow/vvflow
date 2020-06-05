#pragma once

#include <sstream>
#include <vector>

class XIsoline;
struct TGap {
    TGap(uint16_t xi, uint16_t yj, float gap):
        xi(xi),
        yj(yj),
        gap(gap) {}
    uint16_t xi;
    uint16_t yj;
    float gap;
};

class XField {
public:
    XField() = delete;
    XField(const XField&) = delete;
    XField(XField&&) = delete;
    XField& operator=(const XField&) = delete;
    XField& operator=(XField&&) = delete;
    ~XField();

    XField(
        float xmin, float ymin, float dxdy,
        int xres, int yres
    );
    XField(
        const std::string& str
    );

    void evaluate() {};

    const float& at(int xi, int yj) const {
        return map[yj*xres + xi];
    }

    float min() const;
    float max() const;
    float percentile(float p) const;

    //  N   y1  y2  yN
    // x1  z11 z12 z1N
    // x2  z21 z22 z2N
    // xM  zM1 zM2 zMN
    friend std::ostream& operator<< (std::ostream& os, const XField& field);

protected:
    float xmin, ymin, dxdy;
    int xres, yres;

    float* map;
    std::vector<TGap> gaps;

    bool evaluated;

    friend class XIsoline;
};
