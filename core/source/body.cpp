#include "body.h"
#include "stdio.h"
#include "iostream"
#include "math.h"
using namespace std;

TBody::TBody()
{
	List = new TList<TObject>();
}

int TBody::LoadFromFile(const char* filename)
{
	if (!this) return -1;
	if ( !List ) return -1;
	FILE *fin;

	fin = fopen(filename, "r");
	if (!fin) { cerr << "No file called " << filename << endl; return -1; } 

	TObject Obj; ZeroObject(Obj);
	char line[255];
	while ( fgets(line, 254, fin) )
	{
		if (sscanf(line, "%lf\t%lf\n", &Obj.rx, &Obj.ry) == 2)
			List->Copy(&Obj);
	}
	fclose(fin);

	return 0;
}

bool TBody::ObjectIsValid(TObject &obj)
{
	if (!this) return true;
	return ( (obj.rx*obj.rx + obj.ry*obj.ry)>1 );
	//FIXME wrong formula
}

double TBody::SurfaceLenght()
{
	if (!this) return 0;
	double res=0;

	TObject *Obj = List->First;
	TObject *&LastObj = List->Last;
	for (; Obj<LastObj; Obj++)
	{
		double dx = (Obj->rx - (Obj+1)->rx);
		double dy = (Obj->ry - (Obj+1)->ry);
		res += sqrt(dx*dx+dy*dy);
	}

	return res;
}

/************************** HEAT LAYER ****************************************/

void TBody::CleanHeatLayer()
{
	if (!this) return;
	if (!HeatLayer) return;

	for (int i=0; i<List->size; i++)
	{
		HeatLayer[i]=0;
	}
}

int* TBody::ObjectIsInHeatLayer(TObject &obj)
{
	return false;
}
