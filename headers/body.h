#ifndef BODY_H_
#define BODY_H_

#include "elementary.h"
#include "list.h"

class Attach : public Vector
{
	public:
		double g, q;

		Attach() {}
		void zero() { rx = ry = g = q = 0; }
		Attach& operator= (const Vector& p) { rx=p.rx; ry=p.ry; return *this; }
};
typedef Attach TAttach;

class TBody
{
	public:
		TBody(double (*sRotationV)(double Time) = NULL,
				double sRotationAxisX=0, double sRotationAxisY=0);
		~TBody();

		int LoadFromFile(const char* filename);
		void Rotate(double angle);
		bool PointIsValid(Vector p);
		double SurfaceLength();

		TList<TObject> *List;
		TList<TAttach> *AttachList;
		bool InsideIsValid;
		bool isInsideValid();
		Vector RotationAxis;
		double (*RotationV)(double Time); double RotationVVar;
		void UpdateAttach();

		Vector Force; //dont forget to zero it when u want

		TObject* Next(TObject* obj) {
			return (obj == (List->Last-1)) ? List->First : (obj+1);
		}

		TObject* Prev(TObject* obj) {
			return (obj == List->First) ? (List->Last-1) : (obj-1);
		}

		//Heat layer
		void CleanHeatLayer();
		int *ObjectIsInHeatLayer(TObject &obj); //link to element of HeatLayer array

		int *HeatLayer;
		double HeatLayerHeight;
};

#endif /* BODY_H_ */
