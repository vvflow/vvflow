#include "stdio.h"
#include "string.h"
#include "malloc.h"
#include "stdlib.h"
#include "matrix.h"

#ifdef __INTEL_COMPILER
#include "mkl.h"
#else
#include "cblas.h"
extern "C" {
    void dgesv_(int* n, const int* nrhs, double* a, int* lda, int* ipiv, double *x, int *incx, int *info);
    void dgetrf_(int* M, int *N, double* A, int* lda, int* IPIV, int* INFO);
    void dgetri_(int* N, double* A, int* lda, int* IPIV, double* WORK, int* lwork, int* INFO);
}
#endif

#define NDEBUG
#include "assert.h"

#define sqr(x) (x*x)

Matrix::Matrix()
{
    N = 0;

    BodyMatrix = InverseMatrix = RightCol = NULL;
    ipvt = NULL;
    bodyMatrixIsOk_ = false;
}

Matrix::~Matrix()
{
    free(BodyMatrix);
    free(InverseMatrix);
    free(RightCol);
    free(ipvt);
}

void Matrix::resize(unsigned newsize)
{
    if (newsize == N) return;
    N = newsize;

    BodyMatrix =    (double*)realloc(BodyMatrix,    sqr(N)*sizeof(double));
    InverseMatrix = (double*)realloc(InverseMatrix, sqr(N)*sizeof(double));
    RightCol =      (double*)realloc(RightCol,      N*sizeof(double));
    ipvt =             (int*)realloc(ipvt,          (N+1)*sizeof(int));

    solution_ptr.resize(N);

    bodyMatrixIsOk_ = false;
}

double *Matrix::getCell(unsigned row, const double *solution)
{
    unsigned col = getColForSolution(solution);
    if ( (row>=N) || (col>=N) )
    {
        fprintf(stderr, "Error: Matrix::getCell(%d, %d) beyond limit\n", row, col);
        exit(3);
    }
    return BodyMatrix + row*N + col;
}

double *Matrix::getRightCol(unsigned eq)
{
    if ( eq>=N )
    {
        fprintf(stderr, "Error: Matrix::getRightCol(%d) beyond limit\n", eq);
        exit(3);
    }
    return RightCol + eq;
}

void Matrix::setSolutionForCol(unsigned col, double *ptr)
{
    solution_idx.erase(solution_ptr[col]);
    solution_ptr[col] = ptr;
    solution_idx[ptr] = col;
}

double *Matrix::getSolutionForCol(unsigned col)
{
    return solution_ptr.at(col);
}

unsigned Matrix::getColForSolution(const double *ptr)
{
    auto res = solution_idx.find(ptr);
    return res!=solution_idx.end()?res->second:-1;
}

void Matrix::selfTest()
{
    for (auto& ii: solution_idx)
    {
        const double *ptr = ii.first;
        unsigned idx = ii.second;
        if ( idx>=N || solution_ptr[idx]!=ptr )
        {
            fprintf(stderr, "Error: Matrix::selftest() failed\n");
            exit(4);
        }
    }
}


uint32_t Matrix::SuperFastHash (const char * data, int len) {
// http://www.azillionmonkeys.com/qed/hash.html

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

    uint32_t hash = len, tmp;
    int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

#undef get16bits

    return hash;
}

void Matrix::solveUsingInverseMatrix(bool useInverseMatrix)
{
    selfTest();

    if (useInverseMatrix)
    {
        FillInverseMatrix();

        double *y = new double[N];
        cblas_dgemv(
            CblasRowMajor, // layout; Specifies whether two-dimensional array storage is row-major or column-major.
            CblasNoTrans, // operation; y = alpha*A*x + beta*y,
            N, // m; Specifies the number of rows of the matrix A.
            N, // m; Specifies the number of columns of the matrix A.
            1, // alpha; Specifies the scalar alpha.
            InverseMatrix, // A; size lda*m.
            N, // lda; Specifies the leading dimension of A.
            RightCol, // x; size at least (1+(n-1)*abs(incx))
            1, // incx; Specifies the increment for the elements of x.
            0, // beta; Specifies the scalar beta. When beta is set to zero, then y need not be set on input.
            y, // y; size at least (1 +(m - 1)*abs(incy))
            1 // incy; Specifies the increment for the elements of y.
        );

#pragma omp parallel for
        for (unsigned i=0; i<N; i++)
        {
            if (!solution_ptr[i])
            {
                fprintf(stderr, "Warning: solution[%u] = NULL\n", i);
                continue;
            }
            *solution_ptr[i] = y[i];
        }
        delete[] y;
    } else
    {
        if (!bodyMatrixIsOk_)
        {
            fprintf(stderr, "Cannot solve SLAE without bodyMatrix\n");
            exit(-2);
        }

        int info, one=1, N_int=N;
        transpose(BodyMatrix, N);
        dgesv_(&N_int,&one,BodyMatrix,&N_int,ipvt,RightCol,&N_int,&info);

        if (info)
        {
            fprintf(stderr, "dgesv_() failed with info = %d\n", info);
            exit(-2);
        }

#pragma omp parallel for
        for (unsigned i=0; i<N; i++)
        {
            if (!solution_ptr[i])
            {
                fprintf(stderr, "Warning: solution[%u] = NULL\n", i);
                continue;
            }

            *solution_ptr[i] = RightCol[i];
        }

        bodyMatrixIsOk_ = false;
    }
}

void Matrix::FillInverseMatrix()
{
    if (!bodyMatrixIsOk_)
    {
        fprintf(stderr, "Cannot fill inverseMatrix without bodyMatrix\n");
        exit(-2);
    }

    uint32_t hash = SuperFastHash((char*)BodyMatrix, sqr(N)*sizeof(double));
    if (hash == bodyMatrixHash_)
        return;

    fprintf(stderr, "Filling inverse\n");
    memcpy(InverseMatrix, BodyMatrix, sqr(N)*sizeof(double));

    int size_int = N;
    int LWORK = sqr(N);
    int info;

    dgetrf_(&size_int,&size_int,InverseMatrix,&size_int,ipvt,&info);
    if (info)
    {
        fprintf(stderr, "dgetrf_() failed with info = %d\n", info);
        exit(-2);
    }

    dgetri_(&size_int,InverseMatrix,&size_int,ipvt,BodyMatrix,&LWORK,&info);
    if (info)
    {
        fprintf(stderr, "dgetri_() failed with info = %d\n", info);
        exit(-2);
    }

    bodyMatrixHash_ = hash;
    bodyMatrixIsOk_ = false;
}

void Matrix::transpose(double* A, unsigned N)
{
#pragma omp parallel for
    for (unsigned i=0; i<N; i++)
    {
        for (unsigned j=i+1; j<N; j++)
        {
            double tmp = A[i*N+j];
            A[i*N+j] = A[j*N+i];
            A[j*N+i] = tmp;
        }
    }
}

void Matrix::save(const char* filename)
{
    if (!bodyMatrixIsOk_) { perror("Matrix is not filled"); return; }
    FILE *fout = fopen(filename, "w");
    if (!fout) { perror("Error saving the matrix"); return; }

    for (unsigned i=0; i<N; i++)
    {
        for (unsigned j=0; j<N; j++)
        {
            fprintf(fout, "%lf\t", BodyMatrix[i*N+j]);
        }
        fprintf(fout, "\t%lf\n", RightCol[i]);
    }
    fclose(fout);
}

void Matrix::fillWithZeros()
{
    bodyMatrixIsOk_ = 0;
    memset(BodyMatrix, 0, sizeof(double)*sqr(N));
    memset(RightCol, 0, sizeof(double)*N);
}

bool Matrix::testNan()
{
    if (!bodyMatrixIsOk_) { perror("Test Nan: matrix is not filled"); return false; }
    bool foundNan = false;
    for (unsigned i=0; i<N; i++)
    {
        for (unsigned j=0; j<N; j++)
        {
            double el = BodyMatrix[i*N+j];
            if (el != el)
            {
                fprintf(stderr, "Nan element in matrix: row %d, col %d\n", i, j);
                foundNan = true;
            }
        }

        double el = RightCol[i];
        if (el != el)
        {
            fprintf(stderr, "Nan element in rightcol: row %d\n", i);
            foundNan = true;
        }
    }

    return foundNan;
}
