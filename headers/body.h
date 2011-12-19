#ifndef BODY_H_
#define BODY_H_

class TBody;
class TAtt;

#include "elementary.h"
#include "space.h"

namespace bc{enum BoundaryCondition {slip, noslip, kutta, steady, inf_steady};}
namespace hc{enum HeatCondition {neglect, isolate, const_t, const_W};}

class TAtt : public TVec
{
	public:
		double g, q;
		double gsum; //filled by different modules
		double hsum; //filled by different modules
		double fric; //filled by different modules
		double Cp; // computed by S->CalcForces, need /=dt before print (its implemented in SaveProfile);
		double Fr; // computed by S->CalcForces, need /=dt before print (its implemented in SaveProfile);
		double Nu; // computed by S->CalcForces, need /=dt before print (its implemented in SaveProfile);

		double heat_const;
		TVec dl;
		bc::BoundaryCondition bc;
		hc::HeatCondition hc;
		long ParticleInHeatLayer;

		TAtt(){}
		//TAtt(TBody *body, int eq_no);
		void zero() { rx = ry = g = q = gsum = hsum = Cp = Fr = Nu = 0; }
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

		//int LoadFromFile(const char* filename, int start_eq_no);
		void Rotate(double angle);
		TAtt* PointIsInvalid(TVec p);
		TAtt* PointIsInHeatLayer(TVec p);
		double SurfaceLength();
		double size(){ return List->size_safe(); }
		void SetRotation(TVec sRotAxis, double (*sRotSpeed)(double time), double sRotSpeed_const = 0);

		vector<TObj> *List;
		vector<TAtt> *AttachList;
		bool InsideIsValid;
		bool isInsideValid();
		double RotSpeed(double time) const
			{ return RotSpeed_link?RotSpeed_link(time):RotSpeed_const; }
		double Angle;
		TVec Position;
		TVec RotAxis;
		void UpdateAttach();

		TObj Force, Friction; //computed by S->CalcForces
		double Nusselt; //computed by S->CalcForces
		double g_dead;

		TObj* next(TObj* obj) const { return List->next(obj); }
		TObj* prev(TObj* obj) const { return List->prev(obj); }
		TAtt* next(TAtt* att) const { return AttachList->next(att); }
		TAtt* prev(TAtt* att) const { return AttachList->prev(att); }
		TAtt* att(const TObj* obj) const { return &AttachList->at(obj - List->begin());}
		TObj* obj(const TAtt* att) const { return &List->at(att - AttachList->begin());}

		//Heat layer
		//void CleanHeatLayer();
		//int *ObjectIsInHeatLayer(TObj &obj); //link to element of HeatLayer array

	private:
		Space *S;
		vector<TObj> *HeatLayerList;
		TAtt* PointIsInContour(TVec p, vector<TObj> *list);
		double (*RotSpeed_link)(double time);
		double RotSpeed_const;
};

#endif /* BODY_H_ */
