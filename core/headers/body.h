#ifndef BODY_H_
#define BODY_H_

class TBody
{
	public:
		TBody();
		~TBody();

		//load
		//InsideCheck
		//Matrix
		TList<TObject> *List;

		int *BodyControlLayer; //its filled by flowmove
		double ForceX, ForceY; //dont forget to zero it when u want
};

#endif /* BODY_H_ */
