#include <iostream>
#include "Matrix.h"

int main(int argc, char *argv[])
{
  Matrix mat;
  std::cin >> mat;
  /**
   * Ingrese el tamaño de la matriz: 3 2
   * Ingrese los valores de la matriz: 0 3 0 2 1 0
   */
  mat(0, 0) = 3; /* set(fila, columna, valor) */
  std::cout << mat;
  /**
   * 3 3
   * 0 2
   * 1 0
   */

  std::cout << mat(2, 0) << std::endl; /* get(fila, columna) */
  /**
   * 1
   */
  return 0;
}
