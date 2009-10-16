#ifndef LIST_H_
#define LIST_H_
#include "elementary.h"

class TList
{
	public:
		TList();
		~TList();
		long size;
		long maxsize;
		int Add(TVortex vort);
		int Copy(TVortex *vort);
		int Remove(long i);
		int Remove(TVortex* vort);
		int Clear();
		TVortex Item(long i);

		TVortex *Elements;
};

std::ostream& operator<< (std::ostream& os, const TList &l);

typedef void* Pointer;
class TlList
{
	public:
		TlList();
		~TlList();
		long size;
		long maxsize;
		int Add(Pointer link);
		int Remove(long i);
		int Clear();
		Pointer Item(long i);

		Pointer *Elements;


};

#endif /*LIST_H_*/
