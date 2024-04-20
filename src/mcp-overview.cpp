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
 *      File:    mcp-overview.cpp                                         *
 *                                                                        *
 *      Copyright (c) 2019 - 2024                                         *
 *                                                                        *
 * Given the position of the concept, this software gives the overview    *
 * of the values and their percentual representation in the dataset.      *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "mcp-matrix+formula.hpp"

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
unordered_map<string, size_t> accountant;

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
    } else if (arg == "--csv") {
      csvput = argv[++argument];
    } else if (arg == "-c"
	       || arg == "--concept") {
      concept_column = stoi(argv[++argument]);
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

  if (output != STDOUT && csvput.empty()) {
    auto pos = output.rfind('.');
    csvput = (pos == string::npos
	      ? output + "-overview.csv"
	      : output.substr(0, pos)) + ".csv";
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

void read_input () {
  string line;
  while (getline(cin, line)) {
    if (line.empty())
      continue;

    // erase leading and trailing whitespace
    auto nospace = line.find_first_not_of(" \t");
    line.erase(0, nospace);
    nospace = line.find_last_not_of(" \t");
    line.erase(nospace+1);
    if (line.empty())
      continue;

    lineno++;

    string line1;
    bool is_string = false;
    for (size_t i = 0; i < line.length(); ++i) {
      char chr = line[i];
      if (chr == '\\' && i == line.length()-1) {
	cerr << "+++ line cannot terminate with a backslash" << endl;
	return;
      } else if (chr == '\\')
	line1 += chr + line[++i];
      else if (chr == '"')
	is_string = ! is_string;
      else if (is_string && chr == ' ')
	line1 += "_";
      else if (is_string && (chr == ',' || chr == ';'))
	line1 += ".";
      else
	line1 += chr;
    }

    string line2;
    for (size_t i = 0; i < line1.length(); ++i)
      line2 += (line1[i] == ',' || line1[i] == ';' ? ' ' : line1[i]);

    vector<string> chunks = split(line2, " \t");
    accountant[chunks[concept_column]]++;
  }
  cerr << "+++ read " << lineno << " nonempty lines" << endl;
}

void write_output () {
  size_t maxval = 0;
  size_t maxnum = 0;
  for (const auto &val : accountant) {
    maxval = max(maxval, val.first.length());
    maxnum = max(maxnum, val.second);
  }
  maxval = max(maxval, 5);
  maxnum = max(maxnum, 999999);
  size_t numlen = to_string(maxnum).length();

  cout << "+++ value";
  const string val0buf(maxval-5, ' ');
  cout << val0buf;
  const string num0buf(numlen-6, ' ');
  cout << num0buf;
  cout << " number";
  cout << " percentage" << endl;

  for (const auto &val : accountant) {
    const string val1buf(maxval-val.first.length(), ' ');
    const string num1buf(numlen-to_string(val.second).length(), ' ');
    const double pc = ((1.0 * val.second) / (1.0 * lineno)) * 100.0;
    cout << "    " << val.first
	 << val1buf
	 << num1buf
	 << " " << val.second
	 << (pc < 10.0 ? "     " : "    ")
	 << pc  << "%"
	 << endl;
  }
  const string val2buf(numlen - to_string(lineno).length(), ' ');
  cout << "... Total " << val0buf << val2buf << lineno << endl;
  cerr << "+++ overview written on " << output << endl;
}

void out2csv () {
  outfile.close();
  cout.rdbuf(backup);

  if (!csvput.empty()) {
    csvfile.open(csvput);
    if (csvfile.is_open()) {
      backup = cout.rdbuf();
      cout.rdbuf(csvfile.rdbuf());
    } else {
      cerr << "+++ Cannot open csv file " << csvput << endl;
      exit(1);
    }
  }
}

void write_csv () {
  cout << "value,number,percentage" << endl;
  for (const auto &val : accountant)
    cout << val.first << ","
	 << val.second << ","
	 << ((1.0 * val.second) / (1.0 * lineno)) * 100.0
	 << endl;
  cerr << "+++ csv file written on " << csvput << endl;
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  read_arg(argc, argv);
  adjust_and_open();
  read_input();
  write_output();
  out2csv();
  if (!csvput.empty())
    write_csv();
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
