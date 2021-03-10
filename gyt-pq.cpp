#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include "gyt-common.hpp"
#include "gyt-pq-common.hpp"

using namespace std;

const string header    = "Priority-Driven Multidimensional Generalized Young Tableaux";
const string underline = "===========================================================";

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  bigint B;
  polynomial p;
  priority_queue<val_res, vector<val_res>, cmp_vr> pq;
  set<val_tuple> memo;
  bigint maxstack = 1;
  bool flip = true;
  bigint nback = 0;
  bigint split = 0;

  read_input(p, B);
  val_tuple bound = get_bounds(p, B);

  val_tuple val(p.k, 0);
  val[p.k-1] = bound[p.k-1];
  pq.push(make_pair(val, 0));
  memo.insert(val);
  bool solution = false;
  bigint dbl = 0;
  while (!solution && !pq.empty()) {
    val = pq.top().first;
    pq.pop();
    nback += !flip;
    flip = false;

    while (test_bound(val, bound, p.k) && val[p.k-1] >= 0) {
      bigint result = eval(p, val);
      if (result == B) {
	solution = true;
	break;
      } else if (result > B)
	val[p.k-1]--;
      else if (result < B) {
	bigint put = 0;
	for (bigint i = 0; i < p.k-1; ++i) {
	  val_tuple valx = val;
	  valx[i]++;
	  bigint resx = eval(p, valx);
	  if (valx[i] <= bound[i] && memo.find(valx) == memo.cend()) {
	    long long rxB = resx - B;
	    pq.push(make_pair(valx, abs(rxB)));
	    if (p.k > 2)
	      memo.insert(valx);
	    put++;
	    flip = true;
	  } else if (valx[i] <= bound[i])
	    dbl++;
	}
	split += put > 1;
	maxstack = max(maxstack, bigint(pq.size()));
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

  statistics(memo.size(), "queue", maxstack, split, nback, dbl);
}
//////////////////////////////////////////////////////////////////////////////
