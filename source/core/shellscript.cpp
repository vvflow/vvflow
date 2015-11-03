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
    cacheValue2()
{
}

ShellScript::~ShellScript()
{
    if (evaluator)
        evaluator_destroy(evaluator);
}

bool ShellScript::setEvaluator(const std::string &s)
{
    cacheTime1 = cacheTime2 = std::numeric_limits<double>::lowest();
    script = s.empty() ? "0" : s;

    if (evaluator)
        evaluator_destroy(evaluator);
    evaluator = evaluator_create((char*)script.c_str());
    if (!evaluator)
        goto fail;

    char **var_names;
    int var_count;
    evaluator_get_variables(evaluator, &var_names, &var_count);
    if (var_count > 1)
        goto fail;
    if (var_count && strcmp(var_names[0], evaluator_names[0]))
        goto fail;

    return true;

fail:
    script = "";
    return false;
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


