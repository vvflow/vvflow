#include "list.h"
#include <stdlib.h>
#include <iostream>

using namespace std;

/**************** VortexList *********************/

TList::TList()
{
	maxsize = 4;
	Elements = (TVortex*)malloc(maxsize*sizeof(TVortex));
	size = 0;
}

TList::~TList()
{
	free(Elements);
}

int TList::Add(TVortex vort)
{
	if ( size == maxsize ) 
	{
		maxsize = maxsize << 1;
		Elements = (TVortex*)realloc(Elements, maxsize*sizeof(TVortex));
	}
	Elements[size] = vort;
	size++;
	return 0;
}

int TList::Copy(TVortex *vort)
{
	if ( size == maxsize ) 
	{
		maxsize = maxsize << 1;
		Elements = (TVortex*)realloc(Elements, maxsize*sizeof(TVortex));
	}
	Elements[size] = *vort;
	size++;
	return 0;
}

int TList::Remove(long i)
{
	if ((i<0)||(i>=size)) return -1;
	size--;
	Elements[i] = Elements[size];
	return 0;
}

int TList::Remove(TVortex* vort)
{
	size--;
	*vort = Elements[size];
	return 0; 
}

int TList::Clear()
{
	size=0;
	return 0;
}

TVortex TList::Item(long i)
{
	return Elements[i];
}

std::ostream& operator<< (std::ostream& os, const TList &l)
{
	TVortex *Vort = l.Elements;
	int lsize = l.size;
	for ( int i=0; i<lsize; i++)
	{
		os << *Vort << endl;
		Vort++;
	}
	return os;
}

/********************* Link List ***********************************************/

TlList::TlList()
{
	maxsize = 4;
	Elements = (void**)malloc(maxsize*sizeof(Pointer));
	size = 0;
}

TlList::~TlList()
{
	free(Elements);
}

int TlList::Add(Pointer link)
{
	if ( size == maxsize ) 
	{
		maxsize = maxsize << 1;
		Elements = (void**)realloc(Elements, maxsize*sizeof(Pointer));
	}
	Elements[size] = link;
	size++;
	return 0;
}

int TlList::Remove(long i)
{
	if ((i<0)||(i>=size)) return -1;
	size--;
	Elements[i] = Elements[size];
	return 0;	
}

int TlList::Remove(Pointer *p)
{
	size--;
	*p = Elements[size];
	return 0; 
}

int TlList::Clear()
{
	size=0;
	return 0;
}

Pointer TlList::Item(long i)
{
	return Elements[i];
}
