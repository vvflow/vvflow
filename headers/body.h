#ifndef BODY_H_
#define BODY_H_

#include "elementary.h"

class TAtt : public TVec
{
	public:
		double g, q;

		TAtt() {}
		void zero() { rx = ry = g = q = 0; }
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
		bool PointIsValid(TVec p);
		double SurfaceLength();
		double AverageSegmentLength() { return SurfaceLength() / List->size(); }

		vector<TObj> *List;
		vector<TAtt> *AttachList;
		bool InsideIsValid;
		bool isInsideValid();
		TVec RotationAxis;
		double (*RotationV)(double Time); double RotationVVar;
		double Angle;
		void UpdateAttach();

		TVec Force; //dont forget to zero it when u want

		vector<TObj>::const_iterator next(vector<TObj>::const_iterator obj) {
			return (obj == List->end()-1) ? List->begin() : (obj+1);
		}

		vector<TObj>::const_iterator prev(vector<TObj>::const_iterator obj) {
			return (obj == List->begin()) ? (List->end()-1) : (obj-1);
		}

		//Heat layer
		void CleanHeatLayer();
		int *ObjectIsInHeatLayer(TObj &obj); //link to element of HeatLayer array

		int *HeatLayer;
		double HeatLayerHeight;
};

#endif /* BODY_H_ */

