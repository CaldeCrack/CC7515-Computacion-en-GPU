#include "../include/Matrix.h"
#include <iostream>

using namespace std;


// Constructors
Matrix::Matrix() {}
Matrix::Matrix(int n) : n(1), m(n), mat(make_unique<double[]>(n)) {
  for (size_t i = 0; i < n; i++)
    mat[i] = 0;
}
Matrix::Matrix(int n, int m) : n(n), m(m), mat(make_unique<double[]>(n * m)) {
  for (size_t i = 0; i < n * m; i++)
    mat[i] = 0;
}
Matrix::Matrix(const Matrix &matrix) : n(matrix.n), m(matrix.m), mat(make_unique<double[]>(matrix.n * matrix.m)) {
  // copy(matrix.mat.get(), matrix.mat.get() + n * m, mat.get());
  for (size_t i = 0; i < n * m; i++)
    mat[i] = matrix.mat[i];
}
Matrix::~Matrix() {}

// Setters & getters
double &Matrix::operator()(size_t x, size_t y) {
  return mat[x * m + y];
}
const double &Matrix::operator()(size_t x, size_t y) const {
  return mat[x * m + y];
}
void Matrix::fill(double value) {
  for (size_t i = 0; i < n * m; i++)
    mat[i] = value;
}

// Dimensions
tuple<int, int> Matrix::size() const {
  return {n, m};
}
int Matrix::length() const {
  return std::max(n, m);
}

// Values
double Matrix::max() const {
  double max_value = mat[0];
  for (size_t i = 0; i < n * m; i++)
    max_value = std::max(max_value, mat[i]);
  return max_value;
}
double Matrix::min() const {
  double min_value = mat[0];
  for (size_t i = 0; i < n * m; i++)
    min_value = std::min(min_value, mat[i]);
  return min_value;
}

// Utilitary functions
ostream &operator<<(ostream &os, const Matrix &matrix) {
  for (size_t i = 0; i < matrix.n; i++) {
        for (size_t j = 0; j < matrix.m; j++)
            os << matrix(i, j) << ' ';
        os << '\n';
    }
    return os;
}
istream &operator>>(istream &is, Matrix &matrix) {
  cout << "Ingrese el tamaño de la matriz: ";
  is >> matrix.n >> matrix.m;
  matrix.mat = std::make_unique<double[]>(matrix.n * matrix.m);
  cout << "Ingrese los valores de la matriz: ";
  for (size_t i = 0; i < matrix.n * matrix.m; i++)
    is >> matrix.mat[i];
  return is;
}

// Booleans
bool Matrix::operator==(const Matrix &matrix) const {
  if (this->size() != matrix.size()) return false;

  for (size_t i = 0; i < n * m; i++) {
    if (mat[i] != matrix.mat[i])
      return false;
  }
  return true;
}
bool Matrix::operator!=(const Matrix &matrix) const {
  return !(*this == matrix);
}

// Mathematical operation
Matrix &Matrix::operator=(const Matrix &matrix) {
  n = matrix.n;
  m = matrix.m;
  mat = make_unique<double[]>(n * m);
  // copy(matrix.mat.get(), matrix.mat.get() + n * m, mat.get());
  for (size_t i = 0; i < n * m; i++)
    mat[i] = matrix.mat[i];
  return *this;
}
Matrix &Matrix::operator*=(const Matrix &matrix) {
  Matrix result(n, matrix.m);
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < matrix.m; j++) {
      result(i, j) = 0;
      for (size_t k = 0; k < m; k++)
        result(i, j) += (*this)(i, k) * matrix(k, j);
    }
  }
  *this = result;
  return *this;
}
Matrix &Matrix::operator*=(double a) {
  for (size_t i = 0; i < n * m; i++)
    mat[i] *= a;
  return *this;
}
Matrix &Matrix::operator+=(const Matrix &matrix) {  
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < m; j++) {
      mat[i * n + j] += matrix.mat[i * n + j];
    }
  }
  return *this;
}
Matrix &Matrix::operator-=(const Matrix &matrix) {
  for (size_t i = 0; i < n * m; i++)
    mat[i] -= matrix.mat[i];
  return *this;
}
void Matrix::transpose() {
  for (size_t i = 0; i < n; i++) {
    for (size_t j = i + 1; j < m; j++)
      swap(mat[i * n + j], mat[j * n + i]);
  }
}

