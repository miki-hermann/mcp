// all solutions

#include <iostream>
#include <vector>
#include "gyt-common.hpp"

using namespace std;

const string header    = "Young Tableaux 2D All Solutions";
const string underline = "===============================";

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  bigint B;
  polynomial p;
  bigint nres = 0;

  read_input(p, B);
  val_tuple bound = get_bounds(p, B);
  bigint row = 0;
  bigint column = bound[1];
  while (row < bound[0] && column >= 0 && row <= column) {
    val_tuple val = {row, column};
    bigint result = eval(p, val);
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
