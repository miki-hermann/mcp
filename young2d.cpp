#include <iostream>
#include <vector>
#include "gyt-common.hpp"

using namespace std;

const string header    = "Young Tableaux 2D";
const string underline = "=================";

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  bigint B;
  polynomial p;

  read_input(p, B);
  vector<bigint> bound = get_bounds(p, B);
  bigint row = 0;
  bigint column = bound[1];
  while (row <= bound[0] && column >= 0) {
    val_tuple val = {row, column};
    bigint result = eval(p, val);
    if (result == B) {
      cout << endl << "+++ YES +++" << endl;
      cout << "*** for values:" << endl;
      cout << "    x_1 = " << val[0] << endl;
      cout << "    x_2 = " << val[1] << endl;
      exit(0);
    } else if (result < B)
      row++;
    else if (result > B)
      column--;
  }

  cout << endl << "+++ NO +++" << endl;
}
//////////////////////////////////////////////////////////////////////////////
