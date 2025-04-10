#include <gtest/gtest.h>
#include <tuple>
#include "../include/Matrix.h"

using namespace std;


Matrix matrix_0;
Matrix matrix_1(5);
Matrix matrix_2(3, 4);
Matrix matrix_3(matrix_1);

TEST(Matrix, Constructors) {
  EXPECT_EQ(matrix_0.size(), make_tuple(0, 0));
  EXPECT_EQ(matrix_1.size(), make_tuple(1, 5));
  EXPECT_EQ(matrix_2.size(), make_tuple(3, 4));
  EXPECT_EQ(matrix_3.size(), make_tuple(1, 5));
}

TEST(Matrix, Setters_Gettters) {
  // Setter & Getter
  matrix_1(0, 1) = 1;
  EXPECT_EQ(matrix_1(0, 1), 1);
 
  // Fill
  matrix_2.fill(2);
  EXPECT_EQ(matrix_2(0, 0), 2);
  EXPECT_EQ(matrix_2(0, 1), 2);
  EXPECT_EQ(matrix_2(2, 3), 2);

  matrix_2(1, 3) = 9;
  EXPECT_EQ(matrix_2(1, 3), 9);
}

TEST(Matrix, Dimensions) {
  Matrix matrix;
  // Size
  EXPECT_EQ(matrix.size(), make_tuple(0, 0));
  
  // Length
  EXPECT_EQ(matrix_2.length(), 4);
  EXPECT_EQ(matrix_3.length(), 5);
}

TEST(Matrix, Values) {
  // Minimum
  EXPECT_EQ(matrix_1.min(), 0);
  EXPECT_EQ(matrix_2.min(), 2);
  
  // Maximum
  EXPECT_EQ(matrix_1.max(), 1);
  EXPECT_EQ(matrix_2.max(), 9);
}

TEST(Matrix, Booleans) {
  // Not equal
  EXPECT_NE(matrix_1, matrix_2);
  
  // Equal & Not equal
  Matrix matrix_4(matrix_2);
  EXPECT_EQ(matrix_2, matrix_4);
  EXPECT_NE(matrix_1, matrix_4);
}

TEST(Matrix, Mathematical_operations) {
  Matrix a(2, 2);
  a(0, 0) = 1; a(0, 1) = 2;
  a(1, 0) = 3; a(1, 1) = 4;

  // Copy assignment
  Matrix b = a;
  EXPECT_EQ(a, b);

  // Addition
  Matrix c = a;
  c += b;
  EXPECT_EQ(c(0, 0), 2);
  EXPECT_EQ(c(0, 1), 4);
  EXPECT_EQ(c(1, 0), 6);
  EXPECT_EQ(c(1, 1), 8);

  // Subtraction
  c -= b;
  EXPECT_EQ(c, a);

  // Scalar multiplication
  c *= 2;
  EXPECT_EQ(c(0, 0), 2);
  EXPECT_EQ(c(0, 1), 4);
  EXPECT_EQ(c(1, 0), 6);
  EXPECT_EQ(c(1, 1), 8);

  // Matrix multiplication
  Matrix d(2, 3);
  d(0, 0) = 1; d(0, 1) = 2; d(0, 2) = 3;
  d(1, 0) = 4; d(1, 1) = 5; d(1, 2) = 6;

  Matrix e(3, 2);
  e(0, 0) = 7;  e(0, 1) = 8;
  e(1, 0) = 9;  e(1, 1) = 10;
  e(2, 0) = 11; e(2, 1) = 12;

  d *= e; // (2x3) * (3x2) -> (2x2)
  EXPECT_EQ(d.size(), make_tuple(2, 2));
  EXPECT_EQ(d(0, 0), 58);  // 1*7 + 2*9 + 3*11
  EXPECT_EQ(d(0, 1), 64);  // 1*8 + 2*10 + 3*12
  EXPECT_EQ(d(1, 0), 139); // 4*7 + 5*9 + 6*11
  EXPECT_EQ(d(1, 1), 154); // 4*8 + 5*10 + 6*12

  // Transpose
  d.transpose();
  EXPECT_EQ(d.size(), make_tuple(2, 2));
  EXPECT_EQ(d(0, 0), 58);
  EXPECT_EQ(d(1, 0), 64);
  EXPECT_EQ(d(0, 1), 139);
  EXPECT_EQ(d(1, 1), 154);
}

TEST(Matrix, Exceptions) {
  // Constructors
  EXPECT_THROW(Matrix(-1), logic_error);
  EXPECT_THROW(Matrix(0), logic_error);
  EXPECT_THROW(Matrix(-1, 5), logic_error);
  EXPECT_THROW(Matrix(4, 0), logic_error);
  EXPECT_THROW(Matrix(0, 0), logic_error);

  // Values
  Matrix empty;
  EXPECT_THROW(empty.min(), logic_error);
  EXPECT_THROW(empty.max(), logic_error);

  // Mathematical operations
  EXPECT_THROW(matrix_1 *= matrix_2, logic_error);
  EXPECT_THROW(matrix_2 += matrix_3, logic_error);
  EXPECT_THROW(matrix_3 -= matrix_0, logic_error);
}

