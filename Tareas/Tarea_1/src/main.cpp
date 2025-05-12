#include "../include/Matrix.h"
#include <iostream>
#include <limits>

using namespace std;

int main() {
  string exit;
  while (true) {
    cout << "Presiona Enter para ingresar una matriz o 'q' para salir: ";
    getline(cin, exit);
    if (exit == "q" || exit == "Q")
      break;

    Matrix mat;
    cin >> mat;
    cout << "Matriz ingresada:\n" << mat << endl;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
  }
  return 0;
}
