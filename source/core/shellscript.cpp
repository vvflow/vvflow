#include "shellscript.h"
#include "string.h"
#include <stdlib.h>
#include "stdio.h"
#include <limits>

ShellScript::ShellScript():
			script(),
			cacheTime1(std::numeric_limits<double>::lowest()),
			cacheValue1(),
			cacheTime2(std::numeric_limits<double>::lowest()),
			cacheValue2() {}

ShellScript::ShellScript(const std::string &s):
			script(s),
			cacheTime1(std::numeric_limits<double>::lowest()),
			cacheValue1(),
			cacheTime2(std::numeric_limits<double>::lowest()),
			cacheValue2() {}

double ShellScript::getValue(double t) const
{
	if (!script.size())
		return 0;

	if (t == cacheTime2)
		return cacheValue2;

	if (t == cacheTime1)
		return cacheValue1;

	char *exec = new char[script.size()+64];
	sprintf(exec, "t=%lf; T=%lf; %s", t, t, script.c_str());
	double resultValue;
	FILE *pipe = popen(exec,"r");
	if (pipe)
	{
		if (!fscanf(pipe, "%lf", &resultValue)) resultValue = 0.;
		pclose(pipe);
	}
	delete exec;

	#pragma omp master
	{
		cacheTime2 = cacheTime1;
		cacheValue2 = cacheValue1;
		cacheTime1 = t;
		cacheValue1 = resultValue;
	}

	return resultValue;
}


