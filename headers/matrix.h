#ifndef matrix_h
#define matrix_h

#include "stdint.h"

class Matrix
{
    public:
        Matrix();
        ~Matrix();
        Matrix(const Matrix&) = delete;
        Matrix(Matrix&&) = delete;
        Matrix& operator= (const Matrix&) = delete;
        Matrix& operator= (Matrix&&) = delete;

        void resize(unsigned size);
        unsigned size() { return N; }

        double *objectAtIndex(unsigned eq, unsigned j);
        double **solutionAtIndex(unsigned i);
        double *rightColAtIndex(unsigned i);
        void markBodyMatrixAsFilled() {bodyMatrixIsOk_ = true;}
        void spoilMatrix() {bodyMatrixIsOk_ = inverseMatrixIsOk_ = false;}
        void solveUsingInverseMatrix(bool useInverseMatrix);

    private:
        void transpose(double* A, unsigned N);
        void FillInverseMatrix();

    private:
        size_t N; //size
        double* BodyMatrix;
        double* InverseMatrix;
        double* RightCol;
        double** solution;
        int *ipvt; //technical variable for lapack

        bool bodyMatrixIsOk_;
        bool inverseMatrixIsOk_;

    public:
        bool bodyMatrixIsOk() {return bodyMatrixIsOk_;}
        bool inverseMatrixIsOk() {return inverseMatrixIsOk_;}
        void save(const char* filename);
        void fillWithZeros();
        bool testNan();
        //Source for saving and loading matrix can be found
        //in git baecb9308569b6f52a863697908c33ced7ed811b
};

#endif
