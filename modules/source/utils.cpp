#include <iostream>
#include "libVVHD/core.h"
#include "utils.h"

using namespace std;

int PrintBody(std::ostream& os, Space *S)
{
	if (!S->BodyList) return -1;
	TObject *Obj = S->BodyList->First;
	TObject *&LastObj = S->BodyList->Last;
	for ( ; Obj<LastObj; Obj++ )
	{
		os << (*Obj) << endl;
	}
	return 0;
}

int PrintVorticity(std::ostream& os, Space *S, bool PrintSpeed)
{
	if (!S->VortexList) return -1;
	TObject *Obj = S->VortexList->First;
	TObject *&LastObj = S->VortexList->Last;
	for ( ; Obj<LastObj; Obj++ )
	{
		if (PrintSpeed)
			os << *Obj << "\t" << Obj->vx << "\t" << Obj->vy << endl;
		else
			os << *Obj << endl;
	}
	return 0;
}

int PrintHeat(std::ostream& os, Space *S, bool PrintSpeed)
{
	if (!S->HeatList) return -1;
	TObject *Obj = S->HeatList->First;
	TObject *&LastObj = S->HeatList->Last;
	for ( ; Obj<LastObj; Obj++ )
	{
		if (PrintSpeed)
			os << *Obj << "\t" << Obj->vx << "\t" << Obj->vy << endl;
		else
			os << *Obj << endl;
	}
	return 0;
}
