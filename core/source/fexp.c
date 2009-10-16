/* --------------------------------------------------------------
    Fastest exponent function with 11-bit precision.

    Home page: www.imach.uran.ru/exp

    Copyright 2001-2002 by Dr. Raul N.Shakirov, IMach of RAS(UB),
    Phillip S. Pang, Ph.D. Biochemistry and Molecular Biophysics.
    Columbia University. NYC. All Rights Reserved.

    Permission has been granted to copy, distribute and modify
    software in any context without fee, including a commercial
    application, provided that the aforesaid copyright statement
    is present here as well as exhaustive description of changes.

    THE SOFTWARE IS DISTRIBUTED "AS IS". NO WARRANTY OF ANY KIND
    IS EXPRESSED OR IMPLIED. YOU USE AT YOUR OWN RISK. THE AUTHOR
    WILL NOT BE LIABLE FOR DATA LOSS, DAMAGES, LOSS OF PROFITS OR
    ANY OTHER KIND OF LOSS WHILE USING OR MISUSING THIS SOFTWARE.

    Method:     A Fast, Compact Approximation
                of the Exponential Function
                Technical Report IDSIA-07-98
                to appear in Neural Computation 11(4)
                Nicol N. Schraudolph
                ftp://ftp.idsia.ch/pub/nic/exp.ps.gz
-------------------------------------------------------------- */

#include "fexp.h"       /* Header file for fexp() function */

#ifdef  __cplusplus
extern "C" {
#endif/*__cplusplus*/

/* --------------------------------------------------------------
    Structure and constants for fexp() function and macros.
-------------------------------------------------------------- */

union eco _eco;
const double _eco_m = (1048576L/0.693147180559945309417232121458177);
const double _eco_a = (1072693248L - 60801L);

/* --------------------------------------------------------------
    Name:       fexp

    Purpose:    11-bit precision exponent for Intel x86.

    Usage:      fexp (arg)

    Domain:     Same as for standard exp() function
                (approximately -709 <= arg <= 709).

    Result:     Approximate exp of arg, if within domain,
                otherwise undefined.
-------------------------------------------------------------- */

double fexp (double arg)
{
#ifdef  _MSC_VER

    #pragma warning(push)
    #pragma warning(disable: 4410)

    __asm fld   _eco_m
    __asm fmul  arg
    __asm fadd  _eco_a
    __asm fistp _eco.n.j
    return _eco.d;

    #pragma warning(pop)

#else /*_MSC_VER*/

    return RFEXP (arg);

#endif/*_MSC_VER*/
}

#ifdef  __cplusplus
}
#endif/*__cplusplus*/
