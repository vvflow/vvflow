#include "XIsoline.hpp"

#include "MFlowmove.hpp"
#include "MEpsFast.hpp"
#include "elementary.h"

#include <cmath>
#include <limits>

using std::min;
using std::max;
using std::exp;
using std::isfinite;
using std::deque;
using std::list;
using std::numeric_limits;

static const double inf = numeric_limits<double>::infinity();

XIsoline::XIsoline(
    const XField& field,
    double vmin, double vmax, double dv
):
    isolines(),
    xmin(field.xmin),
    ymin(field.ymin),
    dxdy(field.dxdy)
{
    if (!field.evaluated)
        throw std::invalid_argument("XIsoline(): field is not evaluated");
    if (!isfinite(vmax))
        throw std::invalid_argument("XIsoline(): vmax must be finite");
    if (!(vmin <= vmax))
        throw std::invalid_argument("XIsoline(): vmin must be <= vmax");
    if (!(dv > 0))
        throw std::invalid_argument("XIsoline(): dv must be positive");

    for (int i=0; i<field.xres-1; i++)
    {
        // float x = xmin + i*spacing;
        for (int j=0; j<field.yres-1; j++)
        {
            // float y = ymin + j*spacing;

            // 1 2
            // 0 3
            float corners[5] = {field.at(i+0, j+0), field.at(i+0, j+1),
                                field.at(i+1, j+1), field.at(i+1, j+0),
                                field.at(i+0, j+0)};

            for (const TGap& gap: field.gaps)
            {
                if (gap.yj != j)
                    continue;
                if (gap.xi >= i)
                    corners[0] += gap.gap;
                if (gap.xi > i) {
                    corners[3] += gap.gap;
                    corners[4] += gap.gap;
                }

            }

            for (double v=vmin; v<=vmax; v+=dv)
                process_rect(i, j, corners, v);
        }
    }
}

inline static
bool inrange(float z1, float z2, float c)
{
    return (z1 <= c && c < z2) || (z1 >= c && c > z2);
}

void XIsoline::merge_lines(TLine* dst, bool dst_side)
{
    TPoint p = dst_side ? dst->front() : dst->back();

    for (TLine& l: isolines)
    {
        if (&l == dst) {
            continue;
        } else if (l.front() == p) {
            if (dst_side) dst->insert(dst->begin(), l.rbegin(), l.rend());
            else          dst->insert(dst->end(),   l.begin(), l.end());
        } else if (l.back() == p) {
            if (dst_side) dst->insert(dst->begin(), l.begin(), l.end());
            else          dst->insert(dst->end(),   l.rbegin(), l.rend());
        }
        else {
            continue;
        }

        l.clear();
        break;
    }
    isolines.remove_if([](const TLine& l){ return l.empty(); });
}

void XIsoline::commit_segment(TPoint p1, TPoint p2)
{
    for (TLine& l: isolines)
    {
        /**/ if (l.front() == p1) { l.push_front(p2); merge_lines(&l, 1); return; }
        else if (l.front() == p2) { l.push_front(p1); merge_lines(&l, 1); return; }
        else if (l.back()  == p1) { l.push_back (p2); merge_lines(&l, 0); return; }
        else if (l.back()  == p2) { l.push_back (p1); merge_lines(&l, 0); return; }
    }

    list<TLine>::iterator l = isolines.emplace(isolines.begin());
    l->push_back(p1);
    l->push_back(p2);
    return;
}

void XIsoline::process_rect(float x, float y, float z[5], float C)
{
    TPoint p[4];
    int N = 0;
    if (inrange(z[0], z[1], C)) p[N++] = TPoint(x,                        y + (C-z[0])/(z[1]-z[0]));
    if (inrange(z[1], z[2], C)) p[N++] = TPoint(x + (C-z[1])/(z[2]-z[1]), y + 1.0);
    if (inrange(z[2], z[3], C)) p[N++] = TPoint(x + 1.0,                  y + (C-z[3])/(z[2]-z[3]));
    if (inrange(z[3], z[4], C)) p[N++] = TPoint(x + (C-z[4])/(z[3]-z[4]), y);

    if (N>=2) commit_segment(p[0], p[1]);
    if (N>=4) commit_segment(p[2], p[3]);
}

std::ostream& operator<< (std::ostream& os, const XIsoline& xiso)
{
    for (const XIsoline::TLine& line: xiso.isolines)
    {
        for (const XIsoline::TPoint& pt: line)
        {
            float xy[2] = {
                static_cast<float>(xiso.xmin + pt.x * xiso.dxdy),
                static_cast<float>(xiso.ymin + pt.y * xiso.dxdy)
            };
            os.write(
                reinterpret_cast<const char*>(&xy),
                2*sizeof(float)
            );
        }
        const static float NaN = numeric_limits<double>::quiet_NaN();
        float nans[2] = {NaN, NaN};
        os.write(reinterpret_cast<const char*>(&nans), 2*sizeof(float));
    }

    return os;
}
