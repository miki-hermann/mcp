// Compile with 'g++ -O4 -o young-rand-gmp young-rand-gmp.cpp -lgmpxx -lgmp'

#include <iostream>
#include <vector>
#include <stack>
#include <set>
#include <algorithm>
#include <random>
#include <gmpxx.h>
#include "gyt-common-gmp.hpp"

using namespace std;

const string header    = "Randomized Multidimensional Multiprecision Generalized Young Tableaux";
const string underline = "=====================================================================";

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  mpz_class B;
  polynomial p;
  stack<val_tuple> stck;
  set<val_tuple> memo;
  mpz_class maxstack = 1;
  bool flip = true;
  mpz_class nback = 0;
  mpz_class split = 0;

  read_input(p, B);
  val_tuple bound = get_bounds(p, B);

  random_device rd;
  static uniform_int_distribution<int> uni_dist(0,p.k-2);
  static default_random_engine dre(rd());

  val_tuple val(p.k, 0);
  val[p.k-1] = bound[p.k-1];
  stck.push(val);
  memo.insert(val);
  bool solution = false;
  mpz_class dbl = 0;
  while (!solution && !stck.empty()) {
    val = stck.top();
    stck.pop();
    nback += !flip;
    flip = false;

    while (test_bound(val, bound, p.k) &&
	   val[p.k-1] >= 0) {
      mpz_class result = eval(p, val);
      if (result == B) {
	solution = true;
	break;
      } else if (result > B)
	val[p.k-1]--;
      else if (result < B) {
	mpz_class put = 0;
	vector<val_tuple> newstck;
	for (unsigned int i = 0; i < p.k-1; ++i) {
	  val_tuple valx = val;
	  valx[i]++;
	  if (valx[i] <= bound[i] && memo.find(valx) == memo.cend()) {
	    newstck.push_back(valx);
	    if (p.k > 2)
	      memo.insert(valx);
	    put++;
	    flip = true;
	  } else if (valx[i] <= bound[i])
	    dbl++;
	}
	split += put > 1;
	shuffle(newstck.begin(), newstck.end(), dre);
	for (val_tuple vt : newstck)
	  stck.push(vt);
	maxstack = max(maxstack, mpz_class(stck.size()));
	break;
      }
    }
  }

  if (solution) {
    cout << endl << "+++ YES +++" << endl;
    cout << "*** solution for values:" << endl;
    for (unsigned int i = 0; i < p.k; ++i)
      cout << "    x_" << i+1 << " = " << val[i] << endl;
  } else
    cout << endl << "+++ NO solution +++" << endl;

  statistics(memo.size(), "stack", maxstack, split, nback, dbl);
}
//////////////////////////////////////////////////////////////////////////////
