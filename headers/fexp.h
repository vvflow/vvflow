/* --------------------------------------------------------------
    Header file for exponent function with 11-bit precision.

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

#ifndef FEXP_H
#define FEXP_H

#ifdef  __cplusplus
extern "C" {
#endif/*__cplusplus*/

/* --------------------------------------------------------------
    Structure and constants for fexp() function and macros.
-------------------------------------------------------------- */

union eco
{
    double d;
    struct {long i, j;} n;
};

extern union eco _eco;
extern const double _eco_m;
extern const double _eco_a;

/* --------------------------------------------------------------
    Name:       fexp

    Purpose:    11-bit precision exponent for Intel x86.

    Usage:      fexp (arg)

    Domain:     Same as for standard exp() function
                (approximately -709 <= arg <= 709).

    Result:     Approximate exp of arg, if within domain,
                otherwise undefined.
-------------------------------------------------------------- */

extern double fexp (double arg);

/* --------------------------------------------------------------
    Name:       RFEXP

    Purpose:    ANSI compatible 11-bit precision exponent for
                machines, which support IEEE-754 and store
                least significant bit of integers first.

    Usage:      RFEXP (arg)

    Domain:     Same as for standard exp() function
                (approximately -709 <= arg <= 709).

    Result:     Approximate exp of arg if within domain,
                otherwise undefined.
-------------------------------------------------------------- */

#define RFEXP(v) (_eco.n.j = (long)(_eco_m * (v) + _eco_a), _eco.d)

/* --------------------------------------------------------------
    Name:       LFEXP

    Purpose:    ANSI compatible 11-bit precision exponent for
                machines, which support IEEE-754 and store
                the highest significant bit of integers first.

    Usage:      LFEXP (arg)

    Domain:     Same as for standard exp() function
                (approximately -709 <= arg <= 709).

    Result:     Approximate exp of arg if within domain,
                otherwise undefined.
-------------------------------------------------------------- */

#define LFEXP(v) (_eco.n.i = (long)(_eco_m * (v) + _eco_a), _eco.d)

/* --------------------------------------------------------------
    Name:       USE_RFEXP

    Purpose:    Check if machine supports IEEE-754 and stores
                the least significant bit of integers first.

    Usage:      USE_RFEXP()

    Result:     Non-zero value if yes, 0 if no.
-------------------------------------------------------------- */

#define USE_RFEXP (eco.d = 1.0, (eco.n.j - 1072693248L || eco.n.i == 0))

/* --------------------------------------------------------------
    Name:       USE_LFEXP

    Purpose:    Check if machine supports IEEE-754 and stores
                the highest significant bit of integers first.

    Usage:      USE_LFEXP()

    Result:     Non-zero value if yes, 0 if no.
-------------------------------------------------------------- */

#define USE_LFEXP (eco.d = 1.0, (eco.n.i == 1072693248L || eco.n.j == 0))

#ifdef  __cplusplus
}
#endif/*__cplusplus*/

#endif/*FEXP_H*/
