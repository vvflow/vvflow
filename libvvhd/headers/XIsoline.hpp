#pragma once

#include "XField.hpp"

#include <list>
#include <sstream>

class XIsoline {
public:
    XIsoline() = delete;
    XIsoline(const XIsoline&) = delete;
    XIsoline(XIsoline&&) = delete;
    XIsoline& operator=(const XIsoline&) = delete;
    XIsoline& operator=(XIsoline&&) = delete;

    XIsoline(
        const XField &field,
        double vmin,
        double vmax,
        double dv
    );
    // ~XIsoline();

    friend std::ostream& operator<< (std::ostream& os, const XIsoline&);

private:
    struct TPoint {
        float x, y;
        TPoint(): x(0),y(0) {}
        TPoint(float x, float y): x(x), y(y) {}
        inline bool operator==(const TPoint &v) const {
            return (v.x == x) && (v.y == y);
        }
    };
    typedef std::vector<TPoint> TLine;
    std::list<TLine> isolines;

private:
    static void merge_lines(TLine* dst, bool dst_side);
    static void commit_segment(TPoint p1, TPoint p2);
    static void process_rect(float x, float y, float z[5], float C);

};
