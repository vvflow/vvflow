#ifndef BODY_H_
#define BODY_H_

#include "elementary.h"

class TAtt : public TVec
{
	public:
		double g, q;
		double gsum;
		double pres, fric;

		TAtt() {}
		void zero() { rx = ry = g = q = pres = fric = gsum = 0; }
		TAtt& operator= (const TVec& p) { rx=p.rx; ry=p.ry; return *this; }
};

class TBody
{
	public:
		TBody(double (*sRotationV)(double Time) = NULL,
				double sRotationAxisX=0, double sRotationAxisY=0);
		~TBody();

		int LoadFromFile(const char* filename);
		void Rotate(double angle);
		TAtt* PointIsInvalid(TVec p);
		double SurfaceLength();
		double AverageSegmentLength() { return SurfaceLength() / List->size(); }

		vector<TObj> *List;
		vector<TAtt> *AttachList;
		TAtt* att(const TObj* obj); //returns attach next to the obj
		bool InsideIsValid;
		bool isInsideValid();
		TVec RotationAxis;
		double (*RotationV)(double Time); double RotationVVar;
		double Angle;
		void UpdateAttach();

		void calc_pressure(); // need /dt
		//void calc_friction(); // need /Pi/Eps_min^2

		TVec Force; //dont forget to zero it when u want

		TObj* next(TObj* obj) { return List->next(obj); }
		TObj* prev(TObj* obj) { return List->prev(obj); }

		//Heat layer
		void CleanHeatLayer();
		int *ObjectIsInHeatLayer(TObj &obj); //link to element of HeatLayer array

		int *HeatLayer;
		double HeatLayerHeight;
};

#endif /* BODY_H_ */

