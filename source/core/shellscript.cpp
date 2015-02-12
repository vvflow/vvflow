#include "shellscript.h"
#include "string.h"
#include <stdlib.h>
#include "stdio.h"
#include <limits>
#include "matheval.h"

static const char* evaluator_names[] = {(char*)"t"};

ShellScript::ShellScript():
			script(),
			evaluator(NULL),
			cacheTime1(std::numeric_limits<double>::lowest()),
			cacheValue1(),
			cacheTime2(std::numeric_limits<double>::lowest()),
			cacheValue2() {}

ShellScript::ShellScript(const std::string &s):
			script(s),
			cacheTime1(std::numeric_limits<double>::lowest()),
			cacheValue1(),
			cacheTime2(std::numeric_limits<double>::lowest()),
			cacheValue2()
{
	if (script.empty()) return;
	evaluator = evaluator_create((char*)script.c_str());
	if (!evaluator)
	{
		fprintf(stderr, "Can not create evaluator for %s\n", script.c_str());
		script = "";
	}
}

ShellScript::~ShellScript()
{
	if (evaluator) evaluator_destroy(evaluator);
	evaluator = NULL;
}

ShellScript& ShellScript::operator=(const std::string &s)
{
	script = s;
	if (evaluator)
	{
		evaluator_destroy(evaluator);
		evaluator = NULL;
	}

	if (script.empty()) return *this;
	evaluator = evaluator_create((char*)script.c_str());
	if (!evaluator)
	{
		fprintf(stderr, "Can not assign evaluator for %s\n", script.c_str());
		script = "";
	}
	return *this;
}

double ShellScript::getValue(double t) const
{
	if (!evaluator)
		return 0;

	double resultValue = 0;
	#pragma omp critical
	{
		/**/ if (t == cacheTime2) resultValue = cacheValue2;
		else if (t == cacheTime1) resultValue = cacheValue1;
		else
		{
			resultValue = evaluator_evaluate(evaluator, 1, (char**)evaluator_names, &t);

			cacheTime2 = cacheTime1;
			cacheValue2 = cacheValue1;
			cacheTime1 = t;
			cacheValue1 = resultValue;
		}
	}

	return resultValue;
}


