#include "list.h"
#include <iostream>

using namespace std;

/**************** VortexList *********************/

TList::TList(long MaxSize)
{
	maxsize = MaxSize;
	Elements = new TVortex[maxsize];
	size = 0;
}

/*TList::TList()
{
	maxsize = DEFSIZE;
	Elements = new Pointer[maxsize];
	size = 0;
}*/

TList::~TList()
{
	delete [] Elements;
}

long TList::msize() { return maxsize; }

int TList::Add(TVortex vort)
{
	if (size==maxsize) return -1;
	Elements[size] = vort;
	size++;
	return 0;
}

int TList::Copy(TVortex *vort)
{
	if (size==maxsize) return -1;
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

TlList::TlList(long MaxSize)
{
	maxsize = MaxSize;
	Elements = new Pointer[maxsize];
	size = 0;
}

TlList::~TlList()
{
	delete [] Elements;
}

long TlList::msize() { return maxsize; }

int TlList::Add(Pointer link)
{
	if (size==maxsize) return -1;
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

int TlList::Clear()
{
	size=0;
	return 0;
}

Pointer TlList::Item(long i)
{
	return Elements[i];
}
