#ifndef LIST_H_
#define LIST_H_
#include "elementary.h"
#include <stdlib.h>
#include <iostream>

template <class T>
class TList
{
	public:
		TList()
		{
			maxsize = 4;
			First = (T*)malloc(maxsize*sizeof(T));
			size = 0;
			Last = First;
		}

		~TList() { free(First); }

		long size;
		long maxsize;

		void Add(T item)
		{
			if ( size == maxsize ) 
			{
				maxsize = maxsize << 1;
				First = (T*)realloc(First, maxsize*sizeof(T));
				Last = First + size;
			}
			*Last = item;
			size++;
			Last++;
		}

		void Copy(T *item)
		{
			if ( size == maxsize ) 
			{
				maxsize = maxsize << 1;
				First = (T*)realloc(First, maxsize*sizeof(TVortex));
				Last = First + size;
			}
			*Last = *item;
			size++;
			Last++;
		}

		void Remove(long i)
		{
			if ((i<0)||(i>=size)) return;
			size--;
			Last--;
			First[i] = *Last;
			return;
		}

		void Remove(T* item)
		{
			size--;
			Last--;
			*item = *Last;
			return;
		}

		void Clear() { size=0; Last = First; }
		T item(long i) { return First[i]; }

		T *First;
		T *Last;
};

#endif /*LIST_H_*/
