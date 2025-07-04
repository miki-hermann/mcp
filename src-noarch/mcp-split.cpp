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
 *      File:    mcp-split.cpp                                            *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Given an input  file with matrices, this procedure splits  it into two *
 * files: one  with vectors to LEARN  the fomula, second one  to CHECK if *
 * the learned formulas correspond to the vectors.                        *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include "mcp-defs.hpp"

using namespace std;

string version = NOARCH_VERSION;

string input = STDIN;
string learn_output;
string check_output;
ifstream infile;
ofstream learnfile;
ofstream checkfile;

int ratio = 10;
vector<string> matlines;
vector<size_t> checklines;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// reads the input parameters
void read_arg (int argc, char *argv[]) {
  size_t argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "--input"
	|| arg == "-i") {
      if (argument < argc-1) {
	input = argv[++argument];
      } else
	cerr << "+++ no input file selected" << endl;
    } else if (arg == "--learn"
	       || arg == "--lrn"
	       || arg == "-l") {
      if (argument < argc-1) {
	learn_output = argv[++argument];
      } else
	cerr << "+++ no learn file selected, revert to default" << endl;
    } else if (arg == "--check"
	       || arg == "--chk"
	       || arg == "-c") {
      if (argument < argc-1) {
	check_output = argv[++argument];
      } else
	cerr << "+++ no check file selected, revert to default" << endl;
    } else if (arg == "--ratio") {
      if (argument < argc-1) {
	ratio = stoi(argv[++argument]);
      } else
	cerr << "+++ no ratio selected, revert to default" << endl;
    } else
      cerr << "+++ unknown option " << arg << endl;
    ++argument;
  }
}

// adjusts the input parameters
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

  string basename;
  if (learn_output.empty() || check_output.empty()) {
    if (input == STDIN) {
      cerr << "+++ No learn and/or check output file" << endl;
      exit(2);
    }
    auto pos = input.rfind('.');
    basename = pos == string::npos ? input : input.substr(0, pos);
  }

  if (learn_output.empty())
    learn_output = basename + ".lrn";
  learnfile.open(learn_output);
  if (!learnfile.is_open()) {
    cerr << "+++ Cannot open learn file " << learn_output << endl;
    exit(2);
  }

  if (check_output.empty())
    check_output = basename + ".chk";
  checkfile.open(check_output);
  if (!checkfile.is_open()) {
    cerr << "+++ Cannot open check file " << check_output << endl;
    exit(2);
  }

  if (ratio <= 0 || ratio >= 100) {
    cerr << "+++ Ratio must be bigger than 0 and smaller than 100" << endl;
    exit(3);
  } else if (ratio >= 80)
    cerr << "+++ Your ratio of " << ratio
	 << "% may not produce the desired results"
	 << endl;
}

void read_input (vector<string> &matlines) {
  string line;

  while (getline(cin, line))
    matlines.push_back(line);
}

vector<size_t> rand_vectors (const vector<string> &matlines) {
  random_device rd;
  static uniform_int_distribution<int> uni_dist(0,matlines.size()-1);
  static default_random_engine dre(rd());

  const size_t rand_card = matlines.size() * (1.0 * ratio / 100.0);
  vector<size_t> rand_nums;
  
  // Needs to be generated through a loop and checked if every
  // generated value is new

  while (rand_nums.size() < rand_card) {
    size_t rnd = uni_dist(dre);
    auto it = find(rand_nums.cbegin(), rand_nums.cend(), rnd);
    if (it == cend(rand_nums))
      rand_nums.push_back(rnd);
  }
  sort(rand_nums.begin(), rand_nums.end());

  return rand_nums;
}

void distribute (const vector<string> &matlines,
		 const vector<size_t> &checklines) {
  size_t cl = 0;	// checkline pointer
  for (size_t i = 0; i < matlines.size(); ++i) {
    if (cl < checklines.size()
	&& checklines[cl] == i) {
      checkfile << matlines[i] << endl;
      cl++;
    } else
      learnfile << matlines[i] << endl;
  }
}

void cleanup (const size_t &matsize, const size_t &checksize) {
  learnfile.close();
  checkfile.close();

  cerr << "+++ check ratio = " << ratio << "%" << endl;
  cerr << "+++ learn " << learn_output << ": "
       << matsize - checksize
       << endl;
  cerr << "+++ check " << check_output << ": "
       << checksize
       << endl;
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  version += "split";
  cerr << "+++ version = " << version << endl;

  read_arg(argc, argv);
  adjust_and_open();
  read_input(matlines);
  checklines = rand_vectors(matlines);
  distribute(matlines, checklines);
  cleanup(matlines.size(), checklines.size());
}

//////////////////////////////////////////////////////////////////////////////
