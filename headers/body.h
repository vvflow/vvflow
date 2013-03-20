#ifndef BODY_H_
#define BODY_H_

class TBody;
class TAtt;

#include "elementary.h"
#include "space.h"

namespace bc{
	enum BoundaryCondition
	{
		slip = 'l',
		noslip = 'n',
		zero = 'z',
		steady = 's',
		inf_steady = 'i',
	};

	BoundaryCondition bc(int i);
}

namespace hc{
	enum HeatCondition {
		neglect = 'n',
		isolate = 'i',
		const_t = 't',
		const_W = 'w'
	};

	HeatCondition hc(int i);
}

class TAtt : public TObj
{
	public:
		//rx, ry: center coordinates ; $\vec r$ in doc
		//g: unkonown circulation $\gamma$ in doc
		TVec corner; //$\vec c$ in doc
		// v variable isnt used
		//_1_eps always stores 3.0/dl.abs()
		//double gatt, qatt; //$q_\att, \gamma_\att$ in doc

		double gsum; //filled by flowmove: MoveAndClean() & VortexShed()
		double hsum; //filled by flowmove: MoveAndClean() & HeatShed()
		double fric; //filled by diffusivefast: SegmentInfluence()
		double Cp; // computed by S->CalcForces;
		double Fr; // computed by S->CalcForces;
		double Nu; // computed by S->CalcForces;

		double heat_const;
		TVec dl; //$\Delta \vec l$ in doc
		bc::BoundaryCondition bc;
		hc::HeatCondition hc;
		long ParticleInHeatLayer;

		TAtt()
			:TObj()
			{gsum = hsum = fric = Cp = Fr = Nu = 0.; ParticleInHeatLayer = -1;}
		//TAtt(TBody *body, int eq_no);
		//void zero() { r.x = r.y = g = gsum = hsum = /*FIXME fric?*/ Cp = Fr = Nu = 0; ParticleInHeatLayer = -1; }

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
		//double kx, ky, ka;
		double density; //in doc \frac{\rho_b}{\rho_0}

		TVec3D Friction, Friction_prev, Force_export; //computed by S->CalcForces
		//TObj Friction, Friction_prev, Force_export; //computed by S->CalcForces
		TVec3D Force_born, Force_dead; //computed in flowmove
		//TObj Force_born, Force_dead; //computed in flowmove
		double Nusselt; //computed by S->CalcForces
		double g_dead;

	public:
		//functions \vec V(t), \omega(t)
		ShellScript *SpeedX;
		ShellScript *SpeedY;
		ShellScript *SpeedO;
		TVec3D getSpeed() const;

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
		int size()       {return List->size_safe();}
		void overrideMoi_c(double newMoi_c) {_moi_c = newMoi_c;}

		int eq_forces_no; // starting number of forces equations

		//Heat layer
		//void CleanHeatLayer();
		//int *ObjectIsInHeatLayer(TObj &obj); //link to element of HeatLayer array

	private:
		Space *S;

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
