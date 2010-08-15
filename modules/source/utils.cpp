#include <iostream>
#include "libVVHD/core.h"
#include "utils.h"

using namespace std;

int PrintBody(std::ostream& os, Space *S)
{
	int i, lsize;
	TList *list = S->BodyList;
	if (!list) return -1;
	lsize = list->size;
	TVortex *Vort = list->Elements;
	for ( i=0; i<lsize; i++ )
	{
		os << (*Vort) << endl;
		Vort++;
	}
	return 0;
}

int PrintVorticity(std::ostream& os, Space *S, bool PrintSpeed)
{
	int i, lsize;
	TList *list = S->VortexList;
	if (!list) return -1;
	lsize = list->size;
	TVortex *Vort = list->Elements;
	if ( !PrintSpeed )
		for ( i=0; i<lsize; i++ )
		{
			os << *Vort << endl;
			Vort++;
		}
	else
		for ( i=0; i<lsize; i++ )
		{
			os << *Vort << "\t" << Vort->vx << "\t" << Vort->vy << endl;
			Vort++;
		}
	return 0;
}

int PrintHeat(std::ostream& os, Space *S, bool PrintSpeed)
{
	int i, lsize;
	TList *list = S->HeatList;
	if (!list) return -1;
	lsize = list->size;
	TVortex *Vort = list->Elements;
	if ( !PrintSpeed )
		for ( i=0; i<lsize; i++ )
		{
			os << *Vort << endl;
			Vort++;
		}
	else
		for ( i=0; i<lsize; i++ )
		{
			os << *Vort << "\t" << Vort->vx << "\t" << Vort->vy << endl;
			Vort++;
		}
	return 0;
}
