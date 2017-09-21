#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits>
#include <matheval.h>
#include <stdexcept>

#include "TEval.hpp"
static const char* evaluator_names[] = {(char*)"t"};

TEval::TEval(const TEval& copy):
    TEval()
{
    if (copy.evaluator) {
        evaluator = evaluator_create(evaluator_get_string(copy.evaluator));
        if (!evaluator) {
            throw std::runtime_error("evaluator_create(): failed");
        }
        if (!validate()) {
            evaluator_destroy(evaluator);
            throw std::runtime_error("evaluator_create(): invalid expression");
        }
    }
}

TEval::TEval(const std::string& str):
    TEval()
{
    evaluator = evaluator_create((char*)str.c_str());
    if (!evaluator) {
        throw std::runtime_error("evaluator_create(): failed");
    }
    if (!validate()) {
        evaluator_destroy(evaluator);
        throw std::runtime_error("evaluator_create(): invalid expression");
    }
}


TEval::~TEval()
{
    if (evaluator) {
        evaluator_destroy(evaluator);
    }
}

bool TEval::validate() {
    char **var_names;
    int var_count;
    evaluator_get_variables(evaluator, &var_names, &var_count);
    if (var_count > 1)
        return false;
    if (var_count && strcmp(var_names[0], evaluator_names[0]))
        return false;
    return true;
}

// bool TEval::setEvaluator(const std::string &s)
// {
//     cacheTime1 = cacheTime2 = std::numeric_limits<double>::lowest();

//     if (evaluator)
//     {
//         evaluator_destroy(evaluator);
//         evaluator = NULL;
//     }
//     if (script.empty())
//         goto pass;
//     evaluator = evaluator_create((char*)script.c_str());
//     if (!evaluator)
//         goto fail;

//     char **var_names;
//     int var_count;
//     evaluator_get_variables(evaluator, &var_names, &var_count);
//     if (var_count > 1)
//         goto fail;
//     if (var_count && strcmp(var_names[0], evaluator_names[0]))
//         goto fail;

// pass:
//     return true;

// fail:
//     script = "";
//     return false;
// }

double TEval::getValue(double t) const
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


