#include "shellscript.h"
#include "string.h"
#include <stdlib.h>
#include "stdio.h"
#include "float.h"
#include "matheval.h"

static const char* evaluator_names[] = {(char*)"t"};

ShellScript::ShellScript():
			script(),
			evaluator(NULL),
			cacheTime1(-DBL_MAX),
			cacheTime2(-DBL_MAX),
			cacheValue1(),
			cacheValue2() {}

ShellScript::ShellScript(const std::string &s):
			script(s),
			cacheTime1(-DBL_MAX),
			cacheTime2(-DBL_MAX),
			cacheValue1(),
			cacheValue2()
{
	evaluator = evaluator_create((char*)script.c_str());
}

ShellScript::~ShellScript()
{
	if (evaluator) evaluator_destroy(evaluator);	
}

ShellScript& ShellScript::operator=(const std::string &s)
{
	script = s;
	if (evaluator) evaluator_destroy(evaluator);
	evaluator = evaluator_create((char*)script.c_str());
	return *this;
}

double ShellScript::getValue(double t) const
{
	if (!script.size())
		return 0;

	if (t == cacheTime2)
		return cacheValue2;

	if (t == cacheTime1)
		return cacheValue1;

	double resultValue = evaluator_evaluate(evaluator, 1, (char**)evaluator_names, &t);

	#pragma omp master
	{
		cacheTime2 = cacheTime1;
		cacheValue2 = cacheValue1;
		cacheTime1 = t;
		cacheValue1 = resultValue;
	}

	return resultValue;
}


