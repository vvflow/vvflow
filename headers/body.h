#ifndef BODY_H_
#define BODY_H_

#include "elementary.h"

namespace bc{
enum BoundaryCondition {slip, noslip, kutta, noperturbations};}
namespace sc{
enum SourceCondition {none, source, sink};}
namespace hc{
enum HeatCondition {none, temperature};}

class TAtt : public TVec
{
	public:
		double g, q;
		double gsum;
		double pres, fric;
		int i;
		bc::BoundaryCondition bc;

		TAtt() {}
		void zero() { rx = ry = g = q = pres = fric = gsum = 0; }
		TAtt& operator= (const TVec& p) { rx=p.rx; ry=p.ry; return *this; }
};

class TBody
{
	public:
		TBody();
		~TBody();

		int LoadFromFile(const char* filename);
		void Rotate(double angle);
		TAtt* PointIsInvalid(TVec p);
		double SurfaceLength();
		double AverageSegmentLength() { return SurfaceLength() / List->size(); }
		void SetRotation(double (*sRotSpeed)(double time), TVec sRotAxis);

		vector<TObj> *List;
		vector<TAtt> *AttachList;
		bool InsideIsValid;
		bool isInsideValid();
		double RotSpeed(double time) const { return RotSpeed_link?RotSpeed_link(time):0; }
		double Angle;
		TVec Position;
		void UpdateAttach();

		void calc_pressure(); // need /dt
		//void calc_friction(); // need /Pi/Eps_min^2
		void zero_variables();

		TVec Force; //dont forget to zero it when u want

		TObj* next(TObj* obj) const { return List->next(obj); }
		TObj* prev(TObj* obj) const { return List->prev(obj); }
		TAtt* att(const TObj* obj) const { return &AttachList->at(obj - List->begin());}
		TObj* obj(const TAtt* att) const { return &List->at(att - AttachList->begin());}

		//Heat layer
		void CleanHeatLayer();
		int *ObjectIsInHeatLayer(TObj &obj); //link to element of HeatLayer array

		int *HeatLayer;
		double HeatLayerHeight;

	private:
		double (*RotSpeed_link)(double time);
		TVec RotAxis;
};

#endif /* BODY_H_ */
