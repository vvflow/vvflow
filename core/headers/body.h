#ifndef BODY_H_
#define BODY_H_

#include "elementary.h"
#include "list.h"

class TBody
{
	public:
		TBody();
		~TBody();

		int LoadFromFile(const char* filename);
		bool ObjectIsValid(TObject &obj);
		TList<TObject> *List;
		double SurfaceLenght();

		double ForceX, ForceY; //dont forget to zero it when u want

		//Heat layer
		void CleanHeatLayer();
		int *ObjectIsInHeatLayer(TObject &obj); //link to element of HeatLayer array

		int *HeatLayer;
		double HeatLayerHeight;
};

#endif /* BODY_H_ */
