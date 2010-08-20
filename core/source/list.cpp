#include "list.h"
#include <stdlib.h>
#include <iostream>

using namespace std;

template <class T>
TList<T>::TList()
{
	maxsize = 4;
	Elements = (T*)malloc(maxsize*sizeof(T));
	size = 0;
}

template <class T>
TList<T>::~TList()
{
	free(Elements);
}

template <class T>
void TList<T>::Add(T item)
{
	if ( size == maxsize ) 
	{
		maxsize = maxsize << 1;
		Elements = (T*)realloc(Elements, maxsize*sizeof(T));
	}
	Elements[size] = item;
	size++;
}

template <class T>
void TList<T>::Copy(T *item)
{
	if ( size == maxsize ) 
	{
		maxsize = maxsize << 1;
		Elements = (TVortex*)realloc(Elements, maxsize*sizeof(TVortex));
	}
	Elements[size] = *item;
	size++;
}

template <class T>
void TList<T>::Remove(long i)
{
	if ((i<0)||(i>=size)) return;
	size--;
	Elements[i] = Elements[size];
	return 0;
}

template <class T>
void TList<T>::Remove(T* item)
{
	size--;
	*item = Elements[size];
	return;
}

template <class T>
void TList<T>::Clear()
{
	size=0;
	return;
}

template <class T>
T TList<T>::item(long i)
{
	return Elements[i];
}
