// Compile with 'g++ -O4 -o young-pq-gmp young-pq-gmp.cpp -lgmpxx -lgmp'

#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <utility>
#include <gmpxx.h>
#include "gyt-common-gmp.hpp"
#include "gyt-pq-common-gmp.hpp"

using namespace std;

const string header    = "Priority-Driven Multidimensional Multiprecision Generalized Young Tableaux";
const string underline = "==========================================================================";

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  mpz_class B;
  polynomial p;
  priority_queue<val_res, vector<val_res>, cmp_vr> pq;
  set<val_tuple> memo;
  mpz_class maxstack = 1;
  bool flip = true;
  mpz_class nback = 0;
  mpz_class split = 0;

  read_input(p, B);
  val_tuple bound = get_bounds(p, B);

  val_tuple val(p.k, 0);
  val[p.k-1] = bound[p.k-1];
  pq.push(make_pair(val, 0));
  memo.insert(val);
  bool solution = false;
  mpz_class dbl = 0;
  while (!solution && !pq.empty()) {
    val = pq.top().first;
    pq.pop();
    nback += !flip;
    flip = false;

    while (test_bound(val, bound, p.k) && val[p.k-1] >= 0) {
      mpz_class result = eval(p, val);
      if (result == B) {
	solution = true;
	break;
      } else if (result > B)
	val[p.k-1]--;
      else if (result < B) {
	mpz_class put = 0;
	for (unsigned int i = 0; i < p.k-1; ++i) {
	  val_tuple valx = val;
	  valx[i]++;
	  mpz_class resx = eval(p, valx);
	  if (valx[i] <= bound[i] && memo.find(valx) == memo.cend()) {
	    pq.push(make_pair(valx, abs(resx-B)));
	    if (p.k > 2)
	      memo.insert(valx);
	    put++;
	    flip = true;
	  } else if (valx[i] <= bound[i])
	    dbl++;
	}
	split += put > 1;
	maxstack = max(maxstack, mpz_class(pq.size()));
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

  statistics(memo.size(), "queue", maxstack, split, nback, dbl);
}
//////////////////////////////////////////////////////////////////////////////
