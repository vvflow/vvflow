#include "body.h"
#include "stdio.h"
#include "iostream"
#include "math.h"
using namespace std;

TBody::TBody(double (*sRotationV)(double Time),
				double sRotationAxisX, double sRotationAxisY)
{
	List = new TList<TObject>();
	AttachList = new TList<TAttach>();
	RotationV = sRotationV;
	RotationAxis = Vector(sRotationAxisX, sRotationAxisY);
	InsideIsValid = true;
	Angle = 0;
}

int TBody::LoadFromFile(const char* filename)
{
	if (!this) return -1;
	if ( !List ) return -1;
	FILE *fin;

	fin = fopen(filename, "r");
	if (!fin) { cerr << "No file called " << filename << endl; return -1; } 

	TObject obj(0, 0, 0);
	char line[255];
	while ( fgets(line, 254, fin) )
	{
		if (sscanf(line, "%lf\t%lf\n", &obj.rx, &obj.ry) == 2)
			List->Copy(&obj);
	}
	fclose(fin);

	InsideIsValid = isInsideValid();

	return 0;
}

void TBody::Rotate(double angle)
{
	if (!this) return;

	Vector dr;
	TObject *obj = List->First;
	TObject *&LastObj = List->Last;
	for (; obj<LastObj; obj++)
	{
		dr = *obj - RotationAxis;
		*obj = RotationAxis + dr*cos(angle) + rotl(dr)*sin(angle);
	}
	Angle += angle;
	UpdateAttach();
}

bool TBody::PointIsValid(Vector p)
{
	if (!this) return true;

	bool res = !InsideIsValid;

	TObject *i = List->First;
	TObject *j = List->Last-1;
	TObject *&LastVort = List->Last;
	for ( ; i<LastVort; j=i++)
	{
		if ((
			(i->ry < j->ry) && (i->ry < p.ry) && (p.ry <= j->ry) &&
			((j->ry - i->ry) * (p.rx - i->rx) > (j->rx - i->rx) * (p.ry - i->ry))
			) || (
			(i->ry > j->ry) && (i->ry > p.ry) && (p.ry >= j->ry) &&
			((j->ry - i->ry) * (p.rx - i->rx) < (j->rx - i->rx) * (p.ry - i->ry))
		)) res = !res;
	}

	return res;
}

double TBody::SurfaceLength()
{
	if (!this) return 0;
	double res=0;

	TObject *Obj = List->First;
	TObject *&LastObj = List->Last;
	for (; Obj<LastObj; Obj++)
	{
		res += abs(*Obj - *(Obj+1));
	}

	return res;
}

bool TBody::isInsideValid()
{
	if (!this) return true;

	TObject *minObj, *Obj = minObj = List->First;
	TObject *&LastObj = List->Last;
	for (; Obj<LastObj; Obj++)
	{
		minObj = (Obj->rx < minObj->rx)?Obj:minObj;
	}

	TObject *prev = (minObj==List->First)?(List->Last-1):(minObj-1);
	TObject *next = (minObj==(List->Last-1))?List->First:(minObj+1);

	return ((atan2(prev->ry-minObj->ry, prev->rx-minObj->rx) - 
			atan2(next->ry-minObj->ry, next->rx-minObj->rx)) > 0);
}

void TBody::UpdateAttach()
{
	if (!this) return;
	AttachList->Clear();
	TAttach att;

	TObject *Obj = List->First;
	TObject *&LastObj = List->Last;
	for (; Obj<LastObj; Obj++)
	{
		Vector dr = *Next(Obj) - *Obj;
		att = 0.5*(*Next(Obj) + *Obj);

		att.g = rotl(att-RotationAxis)*dr;
		att.q =     (att-RotationAxis)*dr;

		AttachList->Copy(&att);
	}
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
