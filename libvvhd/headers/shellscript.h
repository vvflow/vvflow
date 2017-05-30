#ifndef __shellscript_h__
#define __shellscript_h__

#include "stdio.h"
#include <iostream>

class ShellScript
{
    private:
        std::string script;
    public:
        ShellScript();
        ~ShellScript();
        bool setEvaluator(const std::string &s);
        double getValue(double t) const;

        operator const std::string() const { return script; }

        ShellScript(const ShellScript&) = delete;
        ShellScript(ShellScript&&) = delete;
        ShellScript& operator= (const ShellScript&) = delete;
        ShellScript& operator= (ShellScript&&) = delete;

    private:
        void* evaluator;
        mutable double cacheTime1;
        mutable double cacheValue1;
        mutable double cacheTime2;
        mutable double cacheValue2;
};

#endif //__shellscript_h__
