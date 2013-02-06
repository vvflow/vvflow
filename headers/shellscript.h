#ifndef __shellscript_h__
#define __shellscript_h__

#include "stdio.h"

class ShellScript
{
	public:
		ShellScript();
		void initWithString(const char *str);
		double getValue(double t);
		void write(FILE* file);
		void read(FILE* file);
		const char* getScript();

	private:
		char *script;
		double cacheTime1;
		double cacheValue1;
		double cacheTime2;
		double cacheValue2;
};

#endif //__shellscript_h__
