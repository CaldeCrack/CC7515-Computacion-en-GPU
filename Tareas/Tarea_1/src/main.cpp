#include <iostream>
#include "../include/Matrix.h"

using namespace std;


int main() {
  Matrix mat;
  cin >> mat;
  mat(0, 0) = 3;
  cout << "Se asignó el elemento (0, 0) al valor 3.\n";
  cout << mat;
  cout << "Elemento (2, 0): " << mat(2, 0) << endl;
  return 0;
}

