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
		//double gatt, qatt; //$q_\att, \gamma_\att$ in doc

		double gsum; //filled by different modules
		double hsum; //filled by different modules
		double fric; //filled by different modules
		double Cp; // computed by S->CalcForces, need /=dt before print (its implemented in SaveProfile);
		double Fr; // computed by S->CalcForces, need /=dt before print (its implemented in SaveProfile);
		double Nu; // computed by S->CalcForces, need /=dt before print (its implemented in SaveProfile);

		double gdead;

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
		double RotationSpeed_slae_prev; //\omega_? from previous step
		TVec   MotionSpeed_slae_prev; //\vec V_? from previous step

		double kx, ky, ka;
		double density; //in doc \frac{\rho_b}{\rho_0}

		TObj Friction, Friction_prev, Force_export; //computed by S->CalcForces
		TObj Force_born, Force_dead; //computed in flowmove
		double Nusselt; //computed by S->CalcForces
		double g_dead;

	public:
		//functions \vec V(t), \omega(t)
		ShellScript *SpeedX;
		ShellScript *SpeedY;
		ShellScript *SpeedO;
		double getRotation() const;
		double getSpeedX() const;
		double getSpeedY() const;
		TVec getMotion() const;

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
		TVec   _com; //center of mass in body ref frame (Oxbyb)
		double _moi_com; //moment of inertia about com;
		double _moi_c; //moi about rotation axis

		vector<TVec> *HeatLayerList;
		template <class T>
		TAtt* isPointInContour(TVec p, vector<T> *list);

		void doRotation();
		void doMotion();
};

#endif /* BODY_H_ */
