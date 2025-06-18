/**************************************************************************
 *                                                                        *
 *                                                                        *
 *	         Multiple Classification Project (MCP)                    *
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
 *      File:    mcp-chk2tst.cpp                                          *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Take a check file *.chk and transforms it into a test file             *
 * *.tst. The test file contains the check file without the leading       *
 * group identifiers. This module is a technical preparation support      *
 * for mcp-predict.                                                       *
 *                                                                        *
 **************************************************************************/


#include <iostream>
#include <fstream>
#include <vector>
#include "mcp-defs.hpp"
#include "mcp-basics.hpp"

using namespace std;

string version = NOARCH_VERSION;

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
      if (argument < argc-1) {
	input = argv[++argument];
      } else
	cerr << "+++ no input file selected" << endl;
    } else if (arg == "--output"
	       || arg == "-o"
	       || arg == "--test"
	       || arg == "--tst") {
      if (argument < argc-1) {
	output = argv[++argument];
      } else
	cerr << "+++ no output file selected, revert to default" << endl;
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

  long nrows = 0;
  string group;
  while (getline(cin, line)) {
    nrows++;
    vector<string> nums = split(line, SPACE);
    // nums[0] is the group: ignore it;
    bool space = false;
    for (size_t i = 1; i < nums.size(); ++i) {
      if (space)
	cout << " ";
      else
	space = true;
      cout << nums[i];
    }
    cout << endl;
  }
  cerr << "+++ " << nrows << " rows written on test file " << output << endl;
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  version += "chk2tst";
  cerr << "+++ version = " << version << endl;

  read_arg(argc, argv);
  adjust_and_open();
  matrix2tst();
}

//////////////////////////////////////////////////////////////////////////////
