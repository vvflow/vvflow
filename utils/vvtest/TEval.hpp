#pragma once

// #include <stdio.h>
#include <string>
#include <limits>

class TEval
{
    public:
        TEval():
            // script(),
            evaluator(nullptr),
            cacheTime1(std::numeric_limits<double>::quiet_NaN()),
            cacheTime2(std::numeric_limits<double>::quiet_NaN()),
            cacheValue1(),
            cacheValue2() {}

        TEval(const TEval&);
        // TEval& operator= (const TEval&) noexcept;

        TEval(const std::string& str);
        // TEval& operator= (const std::string& s);

        // TEval(TEval&&) noexcept;
        // TEval& operator= (TEval&&) noexcept;

        ~TEval();

        bool setEvaluator(const std::string &s);
        double getValue(double t) const;

        // operator const std::string() const { return script; }

    private:
        // std::string script;
        bool validate();
        void* evaluator;
        mutable double cacheTime1;
        mutable double cacheTime2;
        mutable double cacheValue1;
        mutable double cacheValue2;
};

// inline TEval::TEval(TEval) noexcept = default;