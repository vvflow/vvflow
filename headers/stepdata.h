#ifndef STEPDATA_H_
#define STEPDATA_H_

#include "core.h"

class Stepdata
{
	public:
		Stepdata(Space* s_);
		void open(const char *filename);
		void write();
		void close();

	private:
		void attribute_write(const char *name, const char *str);
		Space *S;
		int fid;
		int string_hid;
		int DATASPACE_SCALAR;
};

#endif