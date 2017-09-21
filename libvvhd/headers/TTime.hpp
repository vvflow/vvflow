#pragma once

#include <stdint.h>
#include "elementary.h"

class TTime
{
    public:
        // time = value/timescale
        int32_t value;
        uint32_t timescale;

    public:
        TTime():value(0),timescale(1) {}
        TTime(int32_t v, uint32_t ts):value(v),timescale(ts) {}

        static TTime makeWithSeconds(double seconds, uint32_t preferredTimescale)
        {
            return TTime(int32_t(seconds*preferredTimescale), preferredTimescale);
        }

        static TTime makeWithSecondsDecimal(double seconds)
        {
            uint32_t newTS = 1;
            while ((int32_t(seconds*newTS)/double(newTS) != seconds) && (newTS<0x19999999)/*0xffffffff/10*/)
            { newTS*= 10; }
            while (!(int32_t(seconds*newTS)%10) && (newTS > 1))
            { newTS/= 10; }
            //fprintf(stderr, "%lf sec -> %d / %u\n", seconds, int32_t(seconds*newTS), newTS);
            return TTime(int32_t(seconds*newTS), newTS);
        }

        static TTime add(TTime time1, TTime time2)
        {
            uint32_t newTS = lcm(time1.timescale, time2.timescale);
            return TTime(
                    time1.value*(newTS/time1.timescale)
                    + time2.value*(newTS/time2.timescale)
                    , newTS);
        }

        bool divisibleBy(TTime divisor)
        {
            // divisible = кратно
            // dividend = делимое
            // divisor = делитель
            if (!divisor.value || !divisor.timescale)
                return false;
            uint32_t lcm_ts = lcm(timescale, divisor.timescale);
            return !((int64_t(value)*lcm_ts/timescale) % (int64_t(divisor.value)*lcm_ts/divisor.timescale));
        }

    public:
        operator double() const {return double(value)/double(timescale);}
        operator std::string() const
        {
            char buf[64];
            sprintf(buf, "%d/%u", value, timescale);
            return std::string(buf);
        }

    private:
        static uint32_t lcm(uint32_t x, uint32_t y)
        {
            if (x == y) return x;
            if (!x || !y) return 1;
            uint32_t result = 1;
            uint32_t k = 2;
            while ((x!=1) || (y!=1))
            {
                bool kIsPrime = true;
                for (uint32_t d = 2; d<k/2; d++) { if (!k%d) {kIsPrime = false; break;}}
                if (!kIsPrime) {k++; continue;}
                int xk(x%k), yk(y%k);
                if (!xk || !yk)
                {
                    if (result > (1<<31)/k) { /*overflow*/ return result; }
                    result*=k;
                    if (!xk) x/=k;
                    if (!yk) y/=k;
                    k = 2; continue;
                }
                k++;
            }
            return result;
        }
};
