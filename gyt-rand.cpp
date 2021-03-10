#include <iostream>
#include <vector>
#include <stack>
#include <set>
#include <algorithm>
#include <random>
#include "gyt-common.hpp"

using namespace std;

const string header    = "Randomized Multidimensional Generalized Young Tableaux";
const string underline = "======================================================";

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  bigint B;
  polynomial p;
  stack<val_tuple> stck;
  set<val_tuple> memo;
  bigint maxstack = 1;
  bool flip = true;
  bigint nback = 0;
  bigint split = 0;

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
  bigint dbl = 0;
  while (!solution && !stck.empty()) {
    val = stck.top();
    stck.pop();
    nback += !flip;
    flip = false;

    while (test_bound(val, bound, p.k) &&
	   val[p.k-1] >= 0) {
      bigint result = eval(p, val);
      if (result == B) {
	solution = true;
	break;
      } else if (result > B)
	val[p.k-1]--;
      else if (result < B) {
	bigint put = 0;
	vector<val_tuple> newstck;
	for (bigint i = 0; i < p.k-1; ++i) {
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
	maxstack = max(maxstack, bigint(stck.size()));
	break;
      }
    }
  }

  if (solution) {
    cout << endl << "+++ YES +++" << endl;
    cout << "*** solution for values:" << endl;
    for (bigint i = 0; i < p.k; ++i)
      cout << "    x_" << i+1 << " = " << val[i] << endl;
  } else
    cout << endl << "+++ NO solution +++" << endl;

  statistics(memo.size(), "stack", maxstack, split, nback, dbl);
}
//////////////////////////////////////////////////////////////////////////////
