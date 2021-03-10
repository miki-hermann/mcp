// Compile with 'g++ -O4 -o young-proba-gmp young-proba-gmp.cpp -lgmpxx -lgmp'

#include <iostream>
#include <vector>
#include <random>
#include <gmpxx.h>
#include "gyt-common-gmp.hpp"

using namespace std;

const string header    = "Probabilistic Multidimensional Multiprecision Generalized Young Tableaux";
const string underline = "========================================================================";

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  mpz_class B;
  polynomial p;

  read_input(p, B);
  val_tuple bound = get_bounds(p, B);

  random_device rd;
  static uniform_int_distribution<int> uni_dist(0,p.k-2);
  static default_random_engine dre(rd());

  val_tuple val(p.k, 0);
  val[p.k-1] = bound[p.k-1];
  bool solution = false;
  mpz_class choice = 0;

  while (test_bound(val, bound, p.k) &&
	 val[p.k-1] >= 0) {
    mpz_class result = eval(p, val);
    if (result == B) {
      solution = true;
      break;
    } else if (result > B)
      val[p.k-1]--;
    else if (result < B) {
      val[uni_dist(dre)]++;
      choice++;
    }
  }

  if (solution) {
    cout << endl << "+++ YES +++" << endl;
    cout << "*** solution for values:" << endl;
    for (unsigned int i = 0; i < p.k; ++i)
      cout << "    x_" << i+1 << " = " << val[i] << endl;
  } else
    cout << endl << "+++ NO solution +++" << endl;
  cout << "*** # of choices = " << choice << endl;
}
//////////////////////////////////////////////////////////////////////////////
