#ifndef LIST_H_
#define LIST_H_
#include "elementary.h"

class TList
{
	public:
		TList(long MaxSize);
		~TList();
		long size;
		long msize();
		int Add(TVortex vort);
		int Copy(TVortex *vort);
		int Remove(long i);
		int Remove(TVortex* vort);
		int Clear();
		TVortex Item(long i);

		TVortex *Elements;

	private:
		long maxsize;
};

std::ostream& operator<< (std::ostream& os, const TList &l);

typedef void* Pointer;
class TlList
{
	public:
		TlList(long MaxSize);
		~TlList();
		long size;
		long msize();
		int Add(Pointer link);
		int Remove(long i);
		int Clear();
		Pointer Item(long i);

		Pointer *Elements;

	private:
		long maxsize;
};

#endif /*LIST_H_*/
