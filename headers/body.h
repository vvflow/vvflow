#ifndef BODY_H_
#define BODY_H_

#include "elementary.h"
#include "list.h"

struct attach
{
	double rx, ry;
	double g, q;
};
typedef struct attach TAttach;

class TBody
{
	public:
		TBody(double (*sRotationV)(double Time) = NULL,
				double sRotationAxisX=0, double sRotationAxisY=0);
		~TBody();

		int LoadFromFile(const char* filename);
		void Rotate(double angle);
		bool PointIsValid(double x, double y);
		double SurfaceLength();

		TList<TObject> *List;
		TList<TAttach> *AttachList;
		bool InsideIsValid;
		bool isInsideValid();
		double RotationAxisX, RotationAxisY;
		double (*RotationV)(double Time); double RotationVVar;
		void UpdateAttach();

		double ForceX, ForceY; //dont forget to zero it when u want

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
