#include <cstddef>
#include <memory>
#include <iostream>
#include <string>
#include <tuple>

class Matrix
{
private:
    std::unique_ptr<double[]> mat; // Store the matrix
    int n = 0;                     // Number of rows
    int m = 0;                     // Number of columns

public:
    Matrix();             // Empty constructor
    Matrix(int n);        // Constructor, vector like [1xn]
    Matrix(int n, int m); // Constructor [nxm], n:rows, m: columns
    Matrix(const Matrix &
               matrix); // Copy constructor,
                        // https://www.geeksforgeeks.org/copy-constructor-in-cpp/
    ~Matrix();          // Destructor

    // Setters and getters
    double &operator()(std::size_t x,
                       std::size_t y); // Set value to (i,j) <row,column>
    const double &operator()(
        std::size_t x,
        std::size_t y) const; // Get value from (i,j) <row,column>
    void fill(double value);  // Fill all the matrix with a value

    // Dimensions
    std::tuple<int, int> size() const; // Returns a list of the size of the
                                       // matrix, e.g. [2,4], 2 rows, 4 columns
    int length()
        const; // Return max dimension, usefull for vectors, e.g. [2,4] -> 4

    // Values
    double max() const; // Maximum value of the matrix
    double min() const; // Minimum value of the matrix

    // Utilitary functions
    friend std::ostream &operator<<(
        std::ostream &os, const Matrix &mat); // Display matrix to console
    friend std::istream &operator>>(
        std::istream &is, Matrix &mat); // Interactive create matrix from
                                        // console

    // Booleans
    bool operator==(const Matrix &matrix) const; // Equal operator
    bool operator!=(const Matrix &matrix) const; // Not equal operator

    // Mathematical operation
    Matrix &operator=(const Matrix &matrix);  // Assignment operator (copy)
    Matrix &operator*=(const Matrix &matrix); // Multiplication
    Matrix &operator*=(double a);             // Multiply by a constant
    Matrix &operator+=(const Matrix &matrix); // Add
    Matrix &operator-=(const Matrix &matrix); // Substract
    void transpose();                         // Transpose the matrix
};
