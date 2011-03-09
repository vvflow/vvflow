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

		int *BodyControlLayer; //its filled by flowmove
		double ForceX, ForceY; //dont forget to zero it when u want
};

#endif /* BODY_H_ */
