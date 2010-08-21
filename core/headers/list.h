#ifndef LIST_H_
#define LIST_H_
#include "elementary.h"

template <class T>
class TList
{
	public:
		TList();
		~TList();
		long size;
		long maxsize;
		void Add(T item);
		void Copy(T *item);
		void Remove(long i);
		void Remove(T* item);
		void Clear();
		T item(long i);

		T *First;
		T *Last;
};

#endif /*LIST_H_*/
