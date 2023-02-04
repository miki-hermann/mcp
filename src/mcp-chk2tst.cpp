/**************************************************************************
 *                                                                        *
 *                                                                        *
 *	       Multiple Characterization Problem (MCP)                    *
 *                                                                        *
 *	Author:   Miki Hermann                                            *
 *	e-mail:   hermann@lix.polytechnique.fr                            *
 *	Address:  LIX (CNRS UMR 7161), Ecole Polytechnique, France        *
 *                                                                        *
 *	Author: Gernot Salzer                                             *
 *	e-mail: gernot.salzer@tuwien.ac.at                                *
 *	Address: Technische Universitaet Wien, Vienna, Austria            *
 *                                                                        *
 *	Version: all                                                      *
 *      File:    mcp-chk2tst                                              *
 *                                                                        *
 *      Copyright (c) 2019 - 2023                                         *
 *                                                                        *
 * Take a check file *.chk and transforms it into a test file             *
 * *.tst. The test file contains the check file without the leading       *
 * group identifiers. This module is a technical preparation support      *
 * for mcp-predict.                                                       *
 *                                                                        *
 * This software has been created within the ACCA Project.                *
 *                                                                        *
 *                                                                        *
 **************************************************************************/


#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "mcp-matrix+formula.hpp"

using namespace std;

const string STDIN  = "STDIN";
const string STDOUT = "STDOUT";

string input  = STDIN;
string output = STDOUT;
ifstream infile;
ofstream outfile;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void read_arg (int argc, char *argv[]) {	// reads the input parameters
  int argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "--input"
	|| arg == "-i") {
      input = argv[++argument];
    } else if (arg == "--output"
	       || arg == "-o"
	       || arg == "--test"
	       || arg == "--tst") {
      output = argv[++argument];
    } else
      cerr << "+++ unknown option " << arg << endl;
    ++argument;
  }
}

void adjust_and_open () {
  if (input != STDIN) {
    infile.open(input);
    if (infile.is_open())
      cin.rdbuf(infile.rdbuf());
    else {
      cerr << "+++ Cannot open check input file " << input << endl;
      exit(2);
    }
  }
  
  if (input != STDIN && output == STDOUT) {
    string::size_type pos = input.rfind('.');
    output = (pos == string::npos ? input : input.substr(0, pos)) + ".tst";
  }
  
  if (output != STDOUT) {
    outfile.open(output);
    if (outfile.is_open())
      cout.rdbuf(outfile.rdbuf());
    else {
      cerr << "+++ Cannot open output file " << output << endl;
      exit(2);
    }
  }
}

void matrix2tst () {
  int ind_a, ind_b;
  string line;

  getline(cin, line);
  istringstream inds(line);
  inds >> ind_a >> ind_b;
  cout << ind_a << " " << ind_b << endl;

  if (ind_a == 1) {
    getline(cin, line);
    cout << line << endl;
  }
  if (ind_b == 1) {
    getline(cin, line);
    cout << line << endl;
  }

  long nrows = 0;
  string group;
  while (getline(cin, line)) {
    nrows++;
    istringstream nums(line);
    nums >> group;
    int number;
    while (nums >> number)
      cout << " " << number;
    cout << endl;
  }
  cerr << "+++ " << nrows << " rows written on test file " << output << endl;
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  read_arg(argc, argv);
  adjust_and_open();
  matrix2tst();
}

//////////////////////////////////////////////////////////////////////////////
