#include <iostream>
#include <vector>
#include <stack>
#include <set>
#include "gyt-common.hpp"

using namespace std;

const string header    = "Sequential All-Solution Multidimensional Generalized Young Tableaux";
const string underline = "===================================================================";

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  bigint B;
  polynomial p;
  stack<val_tuple> stck;
  set<val_tuple> memo;
  set<val_tuple> sols;
  bigint nres = 0;
  bigint maxstack = 1;
  bool flip = true;
  bigint nback = 0;
  bigint split = 0;

  read_input(p, B);
  val_tuple bound = get_bounds(p, B);

  val_tuple val(p.k, 0);
  val[p.k-1] = bound[p.k-1];
  stck.push(val);
  memo.insert(val);
  bigint dbl = 0;
  while (!stck.empty()) {
    val = stck.top();
    stck.pop();
    nback += !flip;
    flip = false;

    while (test_bound(val, bound, p.k) && val[p.k-1] >= 0) {
      bigint result = eval(p, val);
      if (result == B) {
	if (sols.find(val) == sols.cend()) {
	  nres++;
	  cout << endl << "*** solution for values:" << endl;
	  for (bigint i = 0; i < p.k; ++i)
	    cout << "    x_" << i+1 << " = " << val[i] << endl;
	  sols.insert(val);
	  val[p.k-1]--;
	} else
	  break;
      } else if (result > B)
	val[p.k-1]--;
      else if (result < B) {
	bigint put = 0;
	for (bigint i = 0; i < p.k-1; ++i) {
	  val_tuple valx = val;
	  valx[i]++;
	  if (valx[i] <= bound[i] && memo.find(valx) == memo.cend()) {
	    stck.push(valx);
	    if (p.k > 2)
	      memo.insert(valx);
	    put++;
	    flip = true;
	  } else if (valx[i] <= bound[i])
	    dbl++;
	}
	split += put > 1;
	maxstack = max(maxstack, bigint(stck.size()));
	break;
      }
    }
  }

  cout << endl;
  cout << "+++ number of solutions = " << nres << endl;
  statistics(memo.size(), "stack", maxstack, split, nback, dbl);
}
//////////////////////////////////////////////////////////////////////////////
