#include "list.h"
#include <stdlib.h>
#include <iostream>

using namespace std;

template <class T>
TList<T>::TList()
{
	maxsize = 4;
	First = (T*)malloc(maxsize*sizeof(T));
	size = 0;
	Last = First;
}

template <class T>
TList<T>::~TList()
{
	free(First);
}

template <class T>
void TList<T>::Add(T item)
{
	if ( size == maxsize ) 
	{
		maxsize = maxsize << 1;
		First = (T*)realloc(First, maxsize*sizeof(T));
		Last = First + size;
	}
	First[size] = item;
	size++;
	Last++;
}

template <class T>
void TList<T>::Copy(T *item)
{
	if ( size == maxsize ) 
	{
		maxsize = maxsize << 1;
		First = (T*)realloc(First, maxsize*sizeof(TVortex));
		Last = First + size;
	}
	First[size] = *item;
	size++;
	Last++;
}

template <class T>
void TList<T>::Remove(long i)
{
	if ((i<0)||(i>=size)) return;
	size--;
	Last--;
	First[i] = First[size];
	return;
}

template <class T>
void TList<T>::Remove(T* item)
{
	size--;
	Last--;
	*item = First[size];
	return;
}

template <class T>
void TList<T>::Clear()
{
	size=0;
	Last = First;
	return;
}

template <class T>
T TList<T>::item(long i)
{
	return First[i];
}

template class TList<TObject>;
template class TList<TObject*>;
#include "tree.h"
template class TList<TNode*>;
