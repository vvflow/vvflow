#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <map>

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

        double *getCell(unsigned eq, const double *solution);
        double *getRightCol(unsigned eq);
        void setSolutionForCol(unsigned col, double *ptr);
        double *getSolutionForCol(unsigned col);
        unsigned getColForSolution(const double *ptr);

        void solveUsingInverseMatrix(bool useInverseMatrix);

    private:
        void selfTest();
        void transpose(double* A, unsigned N);
        void FillInverseMatrix();

    private:
        size_t N; //size
        double* BodyMatrix;
        double* InverseMatrix;
        double* RightCol;
        int *ipvt; //technical variable for lapack

        std::map<const double*, unsigned> solution_idx;
        std::vector<double*> solution_ptr;

        uint32_t bodyMatrixHash_;
        bool bodyMatrixIsOk_;

    public:
        uint32_t SuperFastHash (const char * data, int len);
        void markBodyMatrixAsFilled() {bodyMatrixIsOk_ = true;}
        bool bodyMatrixIsOk() {return bodyMatrixIsOk_;}
        void save(const char* filename);
        void fillWithZeros();
        bool testNan();
        //Source for saving and loading matrix can be found
        //in git baecb9308569b6f52a863697908c33ced7ed811b
};
