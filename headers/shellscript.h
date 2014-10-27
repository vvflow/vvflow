#ifndef __shellscript_h__
#define __shellscript_h__

#include "stdio.h"
#include <iostream>

class ShellScript
{
	public:
		std::string script;
	public:
		ShellScript();
		ShellScript(const std::string &s);
		~ShellScript();
		double getValue(double t) const;
		ShellScript& operator=(const std::string &s);

	private:
		void* evaluator;
		mutable double cacheTime1;
		mutable double cacheValue1;
		mutable double cacheTime2;
		mutable double cacheValue2;
};

#endif //__shellscript_h__
