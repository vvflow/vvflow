#pragma once

#include "TVec.hpp"
#include "TVec3D.hpp"
#include "TObj.hpp"
#include "TEval.hpp"

#include <string>
#include <vector>
#include <memory> // weak_ptr
#include <cmath>

//boundary condition
enum class bc_t { steady, kutta };
enum class hc_t { neglect, isolate, const_t, const_w };

class TAtt : public TObj
{
    public:
        //rx, ry: center coordinates ; $\vec r$ in doc
        //g: unkonown circulation $\gamma$ in doc
        TVec corner; //$\vec c$ in doc
        // v variable isnt used
        //_1_eps always stores 3.0/dl.abs()
        //double gatt, qatt; //$q_\att, \gamma_\att$ in doc
        int64_t  heat_layer_obj_no;
        float    heat_const;
        uint32_t slip;

        TVec dl; //$\Delta \vec l$ in doc
        double gsum; //filled by flowmove: MoveAndClean() & VortexShed()
        double hsum; //filled by flowmove: MoveAndClean() & HeatShed()
        double fric; //filled by diffusivefast: SegmentInfluence()
        double Cp; // computed by S->CalcForces;
        double Fr; // computed by S->CalcForces;
        double Nu; // computed by S->CalcForces;

        TAtt() = delete;
        TAtt(double x, double y, bool slip = false):
            TObj(),
            corner(x, y),
            heat_layer_obj_no(-1),
            heat_const(),
            slip(slip),
            dl(),
            gsum(), hsum(), fric(),
            Cp(), Fr(), Nu(),
            eq_no() {}
        TAtt(TVec corner, bool slip = false):
            TAtt(corner.x, corner.y, slip) {}

    public:
        // TBody* body;
        uint32_t eq_no;
};

class TBody
{
    public:
        TBody();
        ~TBody() = default;
        TBody(const TBody& copy);
        TBody& operator= (const TBody& copy);

        // delete std::move
        TBody(TBody&& move) = delete;
        TBody& operator= (TBody&& move) = delete;

        int load_txt(const char* filename);
        // int load_bin(const char* filename);

        std::string label;
        // FIXME rename to lower case
        std::vector<TAtt> alist;
        std::weak_ptr<TBody> root_body;

        TVec3D holder; //in doc = $R_bx$, $R_by$, $\alpha_b$
        TVec3D dpos; // same with delta
        TVec3D kspring; // spring const: kx, ky, ka;
        TVec3D damping; // spring damping: tau_x, tau_y, tau_a;
        TVec3D speed_slae;
        TVec3D speed_slae_prev;

        // coordinates of collision
        TVec3D collision_min;
        TVec3D collision_max;
        double bounce;

        //double kx, ky, ka;
        double density; //in doc rho_b/rho_0
        int32_t special_segment_no;
        bc_t boundary_condition;
        hc_t heat_condition;

        TVec3D friction, friction_prev; //computed by S->CalcForces
        TVec3D force_hydro, force_holder; //computed by convectivefast->CalcCirculationFast
        double nusselt; //computed by S->CalcForces
        TVec3D fdt_dead; //сила, индуцированная умирающими вихрями
        // перед использованием силу надо поделить на dt, а момент - на 2*dt
        double g_dead;

    public:
        //functions \vec V(t), \omega(t)
        TEval speed_x;
        TEval speed_y;
        TEval speed_o;
        TEval force_o;
        TVec3D speed(double t) const;

        //see \vec c_s \vert_Rotation
        void move(TVec3D deltaHolder, TVec3D deltaBody);
        //update TAtt-> rx, ry, dl after doRotationAndMotion()
        void doUpdateSegments();

        TAtt* isPointInvalid(TVec p);
        TAtt* isPointInHeatLayer(TVec p);
        bool isInsideValid() {return _area<=0;}

        void doFillProperties();
        bool   get_slip() const {return _slip;} // true if at least one segment states slip condition
        double get_slen() const {return _slen;} // surface arc length
        double get_area() const {return _area;} // body area
        TVec   get_cofm() const {return _cofm;} // center of mass (in global ref frame)
        TVec   get_axis() const {return holder.r + dpos.r;}
        double get_moi_cofm() const {
            // moment of inertia about center of mass
            return _moi_cofm;
        }
        double get_moi_axis() const {
            // moment of inertia about rotation axis
            return _moi_cofm + (get_axis()-_cofm).abs2()*_area;
        }
        size_t size() const    {return alist.size();}

        int eq_forces_no; // starting number of forces equations

        static bool isrigid(double k) {
            return !( k>=0 && std::isfinite(k) );
        }
        void validate(bool critical);
        //Heat layer
        //void CleanHeatLayer();
        //int *ObjectIsInHeatLayer(TObj &obj); //link to element of HeatLayer array

    private:
        bool   _slip;
        double _slen;
        double _area;
        TVec   _cofm; //center of mass (in global ref frame)
        double _moi_cofm; //moment of inertia about cofm;
        double _min_disc_r2; //min((corner-cofm).abs2())
        TVec   _min_rect_bl; //TVec(min(corner.x), min(corner.y))
        TVec   _min_rect_tr; //TVec(max(corner.x), max(corner.y))

        std::vector<TVec> heat_layer;
        template <class T>
            TAtt* isPointInContour(TVec p, std::vector<T> &list);
};
