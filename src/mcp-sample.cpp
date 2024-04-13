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
 *      File:    mcp-sample.cpp                                           *
 *                                                                        *
 *      Copyright (c) 2019 - 2024                                         *
 *                                                                        *
 * Output a sample from an input file.                                    *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include "mcp-matrix+formula.hpp"	// for split

using namespace std;

#define STDIN  "STDIN"
#define STDOUT "STDOUT"

string input  = STDIN;
string output = STDOUT;
ifstream infile;
ofstream outfile;

string confidence;
double conf;
string error_bound;
string proportion;
double prop;
string concept_column;
size_t ccol;
string value;
bool big = false;

size_t n_of_lines = 0;
size_t sample_size = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// reads the input parameters
void read_arg (int argc, char *argv[]) {
  int argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "--input"
	|| arg == "-i") {
      input = argv[++argument];
    } else if (arg == "--output"
	       || arg == "-o") {
      output = argv[++argument];
    } else if (arg == "--confidence"
	       || arg == "--interval"
	       || arg == "-W"
	       || arg == "-w") {
      confidence = argv[++argument];
    } else if (arg == "--bound"
	       || arg == "--error"
	       || arg == "-B"
	       || arg == "-b") {
      error_bound = argv[++argument];
    } else if (arg == "--proportion"
	       || arg == "-p") {
      proportion = argv[++argument];
    } else if (arg == "--concept"
	       || arg == "-c") {
      concept_column = argv[++argument];
    } else if (arg == "--value"
	       || arg == "-v") {
      value = argv[++argument];
    } else if (arg == "--big"
	       || arg == "--BIG") {
      big = true;
    } else
      cerr << "+++ unknown option " << arg << endl;
    ++argument;
  }
}

inline double string2double (const string &s) {
  return (s.back() == '%' ? stod(s.substr(0, s.length()-1)) * 0.01 : stod(s));
}

// adjusts input parameters and open files
void adjust_and_open () {
  if (input != STDIN) {
    infile.open(input);
    if (infile.is_open())
      cin.rdbuf(infile.rdbuf());
    else {
      cerr << "+++ Cannot open input file " << input << endl;
      exit(2);
    }
  }

  if (output == STDOUT && input != STDIN) {
    auto pos = input.rfind('.');
    output = pos == string::npos
      ? input + "_sample.csv"
      : input.substr(0, pos) + "_sample" + input.substr(pos);
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

  if (! confidence.empty() && ! error_bound.empty()) {
    cerr << "+++ Both confidence interval and error bound specified" << endl;
    exit(2);
  } else if (!error_bound.empty())
    conf = 2.0 * string2double(error_bound);
  else if (!confidence.empty())
    conf = string2double(confidence);
  else {
    cerr << "+++ No confidence interval or error bound specified" << endl;
    exit(2);
  }

  if (!proportion.empty() && (!concept_column.empty() || !value.empty())) {
    cerr << "+++ Both proportion and concept specified" << endl;
    exit(2);
  } else if (!concept_column.empty() && value.empty()) {
    cerr << "+++ Concept column vithout value" << endl;
    exit(2);
  } else if (concept_column.empty() && !value.empty()) {
    cerr << "+++ Value without concept column" << endl;
    exit(2);
  }

  if (concept_column.empty()) {
    if (proportion.empty())
      prop = 0.5;
    else if (proportion.back() == '%')
      prop = stod(proportion.substr(0, proportion.length()-1)) / 100.0;
    else
      prop = stod(proportion);
  } else
    ccol = stoul(concept_column);

}

void cleanup () {
  if (output != STDOUT)
    outfile.close();
  if (input != STDIN)
    infile.close();
  cerr << "+++ sample size = " << sample_size << endl
       << "+++ sample written on " << output << endl;
}

void fst_pass () {
  string line;
  size_t pcount = 0;
  bool warning = false;
  
  while (getline(cin, line)) {
    n_of_lines++;
    if (!concept_column.empty()) {
      vector<string> chunks = split(line, ", \t");
      if (ccol < chunks.size() && chunks[ccol] == value)
	pcount++;
      else if (ccol >= chunks.size()) {
	cerr << "++ arity discrepancy on input line " << n_of_lines << endl;
	warning = true;
      }
    }
  }
  if (warning) {
    cerr << "+++ STOP because of arity discrepancies" << endl;
    exit(2);
  }

  // clear eof flags and seek to the beginning of input file
  cin.clear();
  infile.clear();
  cin.seekg(0);
  infile.seekg(0);

  // compute proportion
  if (! concept_column.empty())
    prop = (1.0 * pcount) / (1.0 * n_of_lines);
}

size_t sample_cardinality (const double &prop, const double &conf) {
  double val = (16.0 * prop * (1.0 - prop)) / (conf * conf);
  size_t sz = val;
  if (val > sz)
    sz++;
  return sz;
}

void snd_pass () {
  string line;
  vector<string> data_lines, sample_lines;

  while (getline(cin, line))
    data_lines.push_back(line);
  sample(data_lines.cbegin(), data_lines.cend(), back_inserter(sample_lines),
	 sample_size,
	 mt19937 {random_device{}()});

  for (const string &sline : sample_lines)
    cout << sline << endl;
}

void big_pass () {
  // install the random device
  random_device rd;
  static uniform_int_distribution<int> uni_dist(1, n_of_lines);
  static default_random_engine dre(rd());

  // generate the random numbers of chosen input lines and sort them
  vector<size_t> rand_nums;
  while (rand_nums.size() < sample_size) {
    size_t rnd = uni_dist(dre);
    auto it = find(rand_nums.cbegin(), rand_nums.cend(), rnd);
    if (it == cend(rand_nums))
      rand_nums.push_back(rnd);
  }
  sort(rand_nums.begin(), rand_nums.end());

  // search the sample lines in the big file
  string line;
  size_t linenum = 0;
  size_t pointer = 0;
  while (getline(cin, line)) {
    linenum++;
    if (linenum == rand_nums[pointer]) {
      cout << line << endl;
      pointer++;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  read_arg(argc, argv);
  adjust_and_open();
  if (big || !concept_column.empty())
    fst_pass();
  sample_size = sample_cardinality(prop, conf);
  if (big)
    big_pass();
  else
    snd_pass();
  cleanup();
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
