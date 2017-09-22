#pragma once

#include <string>

class TEval
{
    private:
        std::string script;
    public:
        TEval();
        ~TEval();
        bool setEvaluator(const std::string &s);
        double getValue(double t) const;

        operator const std::string() const { return script; }

        TEval(const TEval&) = delete;
        TEval(TEval&&) = delete;
        TEval& operator= (const TEval&) = delete;
        TEval& operator= (TEval&&) = delete;

    private:
        void* evaluator;
        mutable double cacheTime1;
        mutable double cacheValue1;
        mutable double cacheTime2;
        mutable double cacheValue2;
};
