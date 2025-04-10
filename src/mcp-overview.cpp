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
 *      File:    mcp-overview.cpp                                         *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Given the position of the concept, this software gives the overview    *
 * of the values and their percentual representation in the dataset.      *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include "mcp-matrix+formula.hpp"
#include "mcp-tally.hpp"

using namespace std;

bool debug = false;

#define STDIN    "STDIN"
#define STDOUT   "STDOUT"
#define SENTINEL -1

string input     = STDIN;
string output    = STDOUT;
string csvput;

ifstream infile;
ofstream outfile;
ofstream csvfile;
streambuf *backup;

size_t lineno = 0;
int concept_column = SENTINEL;
// map<string, size_t> accountant;

//------------------------------------------------------------------------------

inline size_t max (size_t a, size_t b) {
  return a >= b ? a : b;
}

// read the input parameters
void read_arg (int argc, char *argv[]) {
  size_t argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "-i"
	|| arg == "--input") {
      input = argv[++argument];
    } else if (arg == "-o"
	       || arg == "--output") {
      output = argv[++argument];
    } else if (arg == "-c"
	       || arg == "--concept") {
      try {
	concept_column = stoi(argv[++argument]);
      } catch (invalid_argument err) {
	cerr << "+++ '" << argv[argument]
	     << "' after --concept is not a valid column number"
	     << endl;
	exit(1);
      }
    } else {
      cerr << "+++ argument error: " << arg << endl;
      exit(1);
    }
    ++argument;
  }
}

// adjusts input parameters and open files
void adjust_and_open () {
  if (concept_column == SENTINEL) {
    cerr << "+++ No concept column specified" << endl;
    exit(2);
  }

  if (input != STDIN && output == STDOUT) {
    string::size_type pos = input.rfind('.');
    output = (pos == string::npos
	      ? input
	      : input.substr(0, pos)) + "-overview.txt";
  }

  if (input != STDIN) {
    infile.open(input);
    if (infile.is_open())
      cin.rdbuf(infile.rdbuf());
    else {
      cerr << "+++ Cannot open input file " << input << endl;
      exit(2);
    }
  }

  if (output != STDOUT) {
    outfile.open(output);
    if (outfile.is_open()) {
      backup = cout.rdbuf();
      cout.rdbuf(outfile.rdbuf());
    } else {
      cerr << "+++ Cannot open output file " << output << endl;
      exit(2);
    }
  }
}

template <typename T>
void defect_print (const account<T> &defect,
		   const size_t numlen,
		   const size_t percentlen) {
  cout << "... "
       << left << setw(10) << "coordinate"
       << right << setw(numlen) << "number"
       << right << setw(percentlen+1) << "percentage"
       << endl;

  for (const auto &val : defect.number) {
    double pc = defect.percent.at(val.first);
    cout << "    "
	 << right << setw(10) << val.first
	 << setw(numlen) << val.second
	 << setw(percentlen)
	 << setprecision(2) << fixed << showpoint << pc  << "%"
	 << endl;
  }
}

void write_output (const size_t &total) {
  cerr << "+++ read " << total << " nonempty lines" << endl;

  size_t maxval = 0;
  size_t maxnum = 0;
  for (const auto &val : concept_items.number) {
    maxval = max(maxval, val.first.length());
    maxnum += val.second;
  }
  maxval = max(maxval, 5)+1;
  const size_t numlen = max(to_string(maxnum).length(), 6)+1;
  const size_t percentlen = 11;


  cout << "+++ CONCEPT VALUES +++" << endl
       << "    ==============" << endl << endl;
  cout << "... "
       << left << setw(maxval) << "value"
       << right << setw(numlen) << "number"
       << right << setw(percentlen+1) << " percentage"
       << endl;

  for (const auto &val : concept_items.number) {
    double pc = concept_items.percent.at(val.first);
    cout << "    "
	 << left << setw(maxval) << val.first
	 << right << setw(numlen) << val.second
	 << setw(percentlen)
	 << setprecision(2) << fixed << showpoint << pc  << "%"
	 << endl;
  }
  cout << "... ";
  cout << left  << setw(maxval) << "Total"
       << right << setw(numlen) << total
       << endl;

  if (!empty_items.empty() || !qmark_items.empty()) {
    cout << endl << endl
	 << "+++ DEFECTS +++" << endl
	 << "    =======" << endl << endl;
    
    if (!qmark_items.empty()) {
      cout << "+++ question marks +++" << endl;
      defect_print(qmark_items, numlen, percentlen);
    }
    
    if (!empty_items.empty()) {
      cout << "+++ empty items +++" << endl;
      defect_print(empty_items, numlen, percentlen);
    }
  }

  cerr << "+++ overview written on " << output << endl;
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  version = NOARCH_VERSION "overview";
  cerr << "+++ version = " << version << endl;

  read_arg(argc, argv);
  adjust_and_open();
  size_t total = tally(concept_column);
  write_output(total);
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
