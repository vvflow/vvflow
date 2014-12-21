#ifndef BODY_H_
#define BODY_H_

class Space;
class TBody;
class TAtt;

#include "elementary.h"
#include <vector>
#include <memory>

using std::vector;
using std::weak_ptr;

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

		TAtt():TObj(), heat_layer_obj_no(-1), heat_const(0.0), slip(0)
		{
			gsum = hsum = fric = 0.0;
			Cp = Fr = Nu = 0.0;
		}
		//TAtt(TBody *body, int eq_no);
		//void zero() { r.x = r.y = g = gsum = hsum = /*FIXME fric?*/ Cp = Fr = Nu = 0; heat_layer_obj_no = -1; }

	public:
		TBody* body;
		uint32_t eq_no;
};

class TBody
{
	public:
		TBody(Space *space);
		~TBody();
		TBody() = delete;
		TBody(const TBody&) = delete;
		TBody(TBody&&) = delete;
		TBody& operator= (const TBody&) = delete;
		TBody& operator= (TBody&&) = delete;
		int get_index() const; // index of body in Space list

		// FIXME rename to lower case
		std::vector<TAtt> List;
		weak_ptr<TBody> root_body;

		TVec3D pos; //in doc = $R_bx$, $R_by$, $\alpha_b$
		TVec3D dPos; // same with delta
		//double Angle; // in documentation = $\alpha$
		//TVec   Position; // = $\vec R$
		//double deltaAngle; // in doc $\Delta \alpha$
		//TVec   deltaPosition; //in doc $\Delta \vec R$
		TVec3D Speed_slae;
		TVec3D Speed_slae_prev;
		//double RotationSpeed_slae; //in doc \omega_?
		//TVec   MotionSpeed_slae; // in doc \vec V_?
		//double RotationSpeed_slae_prev; //\omega_? from previous step
		//TVec   MotionSpeed_slae_prev; //\vec V_? from previous step

		TVec3D k; //kx, ky, ka;
		TVec3D damping;
		//double kx, ky, ka;
		double density; //in doc \frac{\rho_b}{\rho_0}

		int64_t special_segment_no;
		bc_t boundary_condition;
		hc_t heat_condition;

		TVec3D Friction, Friction_prev; //computed by S->CalcForces
		TVec3D Force_born, Force_dead; //computed by flowmove->MoveAndClean
		TVec3D Force_hydro, Force_holder; //computed by convectivefast->CalcCirculationFast
		double Nusselt; //computed by S->CalcForces
		double g_dead;

	public:
		//functions \vec V(t), \omega(t)
		ShellScript SpeedX;
		ShellScript SpeedY;
		ShellScript SpeedO;
		TVec3D getSpeed() const;

		//see \vec c_s \vert_Rotation
		void doRotationAndMotion();
		//update TAtt-> rx, ry, dl after doRotationAndMotion()
		void doUpdateSegments();

		TAtt* isPointInvalid(TVec p);
		TAtt* isPointInHeatLayer(TVec p);
		bool isInsideValid() {return _area<=0;}

		void doFillProperties();
		double getSurface() const {return _surface;}
		double getArea() const {return _area;}
		TVec   getCom() const {return _com;} // center of mass
		TVec   getAxis() const {return pos.r + dPos.r;}
		double getMoi_c() const {return _moi_c;} // moment of inertia about rotation axis
		int size() const    {return List.size();}
		void overrideMoi_c(double newMoi_c) {_moi_c = newMoi_c;}

		int eq_forces_no; // starting number of forces equations

		//Heat layer
		//void CleanHeatLayer();
		//int *ObjectIsInHeatLayer(TObj &obj); //link to element of HeatLayer array

	private:
		Space* _space;

		double _surface;
		double _area;
		TVec   _com; //center of mass (in global ref frame)
		double _moi_com; //moment of inertia about com;
		double _moi_c; //moi about rotation axis

		vector<TVec> *HeatLayerList;
		template <class T>
		TAtt* isPointInContour(TVec p, vector<T> *list);

		void doRotation();
		void doMotion();
};

#endif /* BODY_H_ */
