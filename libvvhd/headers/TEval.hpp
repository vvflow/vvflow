#pragma once

#include <string>

class TEval
{
    public:
        TEval();
        TEval(const std::string& str);
        TEval(const TEval& copy);
        TEval& operator= (const std::string& str);
        TEval& operator= (const TEval& copy);

        ~TEval();

        double eval(double t) const;

        operator std::string() const {
            return expr;
        }

    private:
        void* lua_state;
        std::string expr;
        mutable double cacheTime1;
        mutable double cacheTime2;
        mutable double cacheValue1;
        mutable double cacheValue2;
};
