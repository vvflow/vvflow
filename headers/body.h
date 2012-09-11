#ifndef BODY_H_
#define BODY_H_

class TBody;
class TAtt;

#include "elementary.h"
#include "space.h"

namespace bc{enum BoundaryCondition {slip, noslip, kutta, steady, inf_steady};}
namespace hc{enum HeatCondition {neglect, isolate, const_t, const_W};}

class TAtt : public TObj
{
	public:
		//rx, ry: center coordinates ; $\vec r$ in doc
		//g: unkonown circulation $\gamma$ in doc
		TVec corner; //$\vec c$ in doc
		//double gatt, qatt; //$q_\att, \gamma_\att$ in doc

		double gsum; //filled by different modules
		double hsum; //filled by different modules
		double fric; //filled by different modules
		double Cp; // computed by S->CalcForces, need /=dt before print (its implemented in SaveProfile);
		double Fr; // computed by S->CalcForces, need /=dt before print (its implemented in SaveProfile);
		double Nu; // computed by S->CalcForces, need /=dt before print (its implemented in SaveProfile);

		double heat_const;
		TVec dl; //$\Delta \vec l$ in doc
		bc::BoundaryCondition bc;
		hc::HeatCondition hc;
		long ParticleInHeatLayer;

		TAtt(){}
		//TAtt(TBody *body, int eq_no);
		void zero() { rx = ry = g = gsum = hsum = /*FIXME fric?*/ Cp = Fr = Nu = 0; ParticleInHeatLayer = -1; }
		TAtt& operator= (const TVec& p) { rx=p.rx; ry=p.ry; return *this; }

	public:
		int eq_no;
		TBody* body;
};

class TBody
{
	public:
		TBody(Space *sS);
		~TBody();

		vector<TAtt> *List;

		double Angle; // in documentation = $\alpha$
		TVec   Position; // = $\vec R$
		double deltaAngle; // in doc $\Delta \alpha$
		TVec   deltaPosition; //in doc $\Delta \vec R$
		double RotationSpeed_slae; //in doc \omega_?
		TVec   MotionSpeed_slae; // in doc \vec V_?

		double kx, ky, ka;

		TObj Force, Friction; //computed by S->CalcForces
		double Nusselt; //computed by S->CalcForces
		double g_dead;

	public:
		//set \omega, \vec V
		void setRotation(double (*sRotSpeed)(double time), double sRotSpeed_const = 0);
		void setMotion(TVec (*sMotSpeed)(double time), TVec sMotSpeed_const = TVec(0,0));

		//get \omega, \vec V 
		double getRotation(double time) const
			{ return RotSpeed_link ? RotSpeed_link(time):RotSpeed_const; }
		TVec   getMotion(double time) const
			{ return MotSpeed_link ? MotSpeed_link(time):MotSpeed_const; }

		//see \vec c_s \vert_Rotation
		void doRotationAndMotion();
		//update TAtt-> rx, ry, dl after doRotationAndMotion()
		void doUpdateSegments();

		TAtt* isPointInvalid(TVec p);
		TAtt* isPointInHeatLayer(TVec p);
		bool isInsideValid() {return _area<=0;}

		void doFillProperties();
		double getSurface() {return _surface;}
		double getArea()    {return _area;}
		TVec   getCom()     {return _com;} // center of mass
		double getMoi_c()   {return _moi_c;} // moment of inertia about rotation axis
		double size()       {return List->size_safe();}

		TAtt* next(TAtt* att) const { return List->next(att); }
		TAtt* prev(TAtt* att) const { return List->prev(att); }

		//Heat layer
		//void CleanHeatLayer();
		//int *ObjectIsInHeatLayer(TObj &obj); //link to element of HeatLayer array

	private:
		Space *S;

		double _surface;
		double _area;
		TVec   _com; //center of mass in body ref frame (Oxbyb)
		double _moi_com; //moment of inertia about com;
		double _moi_c; //moi about rotation axis

		vector<TVec> *HeatLayerList;
		template <class T>
		TAtt* isPointInContour(TVec p, vector<T> *list);
		double (*RotSpeed_link)(double time);
		double RotSpeed_const;
		TVec   (*MotSpeed_link)(double time);
		TVec   MotSpeed_const;

		void doRotation();
		void doMotion();
};

#endif /* BODY_H_ */
