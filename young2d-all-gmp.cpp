// Compile with 'g++ -O4 -o young2d-all-gmp young2d-all-gmp.cpp -lgmpxx -lgmp'
// all solutions

#include <iostream>
#include <vector>
#include <gmpxx.h>
#include "gyt-common-gmp.hpp"

using namespace std;

const string header    = "Young Tableaux 2D Multiprecision All Solutions";
const string underline = "==============================================";

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  mpz_class B;
  polynomial p;
  mpz_class nres = 0;

  read_input(p, B);
  vector<mpz_class> bound = get_bounds(p, B);
  mpz_class row = 0;
  mpz_class column = bound[1];
  while (row < bound[0] && column >= 0 && row <= column) {
    val_tuple val = {row, column};
    mpz_class result = eval(p, val);
    if (result == B) {
      nres++;
      cout << endl << "*** solution for values:" << endl;
      cout << "    x_1 = " << val[0] << endl;
      cout << "    x_2 = " << val[1] << endl;
      column--;
    } else if (result < B)
      row++;
    else if (result > B)
      column--;
  }

  cout << endl;
  cout << "+++ number of solutions = " << nres << endl;
}
//////////////////////////////////////////////////////////////////////////////
