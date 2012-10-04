#include "shellscript.h"
#include "string.h"
#include <stdlib.h>
#include "stdio.h"
#include "float.h"


ShellScript::ShellScript()
{
	script = new char[1024];
	cacheTime1 = -DBL_MAX; cacheValue1 = 0.;
	cacheTime2 = -DBL_MAX; cacheValue2 = 0.;
}

void ShellScript::initWithString(const char *str)
{
	strcpy(script, str);
}

double ShellScript::getValue(double t)
{
	if (!strlen(script))
		return 0;

	if (t == cacheTime2)
		return cacheValue2;

	if (t == cacheTime1)
		return cacheValue1;

	char *exec = new char[strlen(script)+64];
	sprintf(exec, "t=%lf; T=%lf; %s", t, t, script);
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

void ShellScript::write(FILE* file)
{
	int32_t len = strlen(script)+1;
	fwrite(&len, 4, 1, file);
	fwrite(script, 1, len, file);
}

void ShellScript::read(FILE* file)
{
	int32_t len;
	fread(&len, 4, 1, file);
	fread(script, 1, len, file);
}

const char* ShellScript::getScript()
{
	return script;
}


