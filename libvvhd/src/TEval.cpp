#include "TEval.hpp"

#include <matheval.h>
#include <cstring> // strcmp
#include <cmath>
#include <stdexcept>

static const char* evaluator_names[] = {(char*)"t"};
static const double NaN = std::nan("");

TEval::TEval():
    evaluator(nullptr),
    cacheTime1(NaN),
    cacheTime2(NaN),
    cacheValue1(),
    cacheValue2()
{
}

TEval::TEval(const TEval& copy):
    TEval(std::string(copy))
{
}


TEval::TEval(const std::string& str):
    evaluator(nullptr),
    cacheTime1(NaN),
    cacheTime2(NaN),
    cacheValue1(),
    cacheValue2()
{
    if (str.empty())
        return;

    evaluator = evaluator_create((char*)str.c_str());
    if (!evaluator) {
        throw std::invalid_argument("evaluator_create(): malformed expression");
    }
    if (!validate(evaluator)) {
        evaluator_destroy(evaluator);
        throw std::invalid_argument("evaluator_create(): invalid expression");
    }
}

TEval& TEval::operator=(const TEval& copy)
{
    return *this = std::string(copy);
}

TEval& TEval::operator=(const std::string& str)
{
    if (str.empty())
        return *this;

    void* evaluator_new = evaluator_create((char*)str.c_str());
    if (!evaluator_new) {
        throw std::invalid_argument("evaluator_create(): malformed expression");
    }
    if (!validate(evaluator_new)) {
        evaluator_destroy(evaluator_new);
        throw std::invalid_argument("evaluator_create(): invalid expression");
    }

    cacheTime1 = cacheTime2 = NaN;
    if (evaluator) {
        evaluator_destroy(evaluator);
    }
    evaluator = evaluator_new;
    return *this;
}


TEval::~TEval()
{
    if (evaluator) {
        evaluator_destroy(evaluator);
    }
}

TEval::operator std::string() const
{
    if (evaluator) {
        return evaluator_get_string(evaluator);
    } else {
        return std::string();
    }
}

bool TEval::validate(void *evaluator) {
    char **var_names;
    int var_count;
    evaluator_get_variables(evaluator, &var_names, &var_count);
    if (var_count > 1)
        return false;
    if (var_count && strcmp(var_names[0], evaluator_names[0]))
        return false;
    return true;
}

double TEval::eval(double t) const
{
    if (!evaluator)
        return 0;

    double ret = 0;
#pragma omp critical
    {
        if (t == cacheTime2) {
            ret = cacheValue2;
        } else if (t == cacheTime1) {
            ret = cacheValue1;
        } else {
            ret = evaluator_evaluate(evaluator, 1, (char**)evaluator_names, &t);

            cacheTime2 = cacheTime1;
            cacheValue2 = cacheValue1;
            cacheTime1 = t;
            cacheValue1 = ret;
        }
    }

    return ret;
}
