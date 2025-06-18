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
 *	Version: both                                                     *
 *      File:    mcp-compare.cpp                                          *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Compares original concept values with prediction results               *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#include <array>
// #include <algorithm>
#include "mcp-matrix+formula.hpp"

using namespace std;

const string STDIN  = "STDIN";
const string STDOUT = "STDOUT";

string output = STDOUT;
string original;
string predict;
ofstream outfile;
ifstream origfile;
ifstream predfile;

enum State : short {eql = 0, in = 1, out = 2};
map<string, array<size_t, 3>> aggregate;
map<string, size_t> tally;
size_t numline = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// reads the input parameters
void read_args (int argc, char *argv[]) {
  int argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "--original"
	|| arg == "-orig"
	|| arg == "-or") {
      original = argv[++argument];
    } else if (arg == "--prediction"
	       || arg == "-predict"
	       || arg == "-pdx") {
      predict = argv[++argument];
    } else if (arg == "--output"
	       || arg == "-o") {
      output = argv[++argument];
    } else
      cerr << "+++ unknown option " << arg << endl;
    ++argument;
  }
}

void adjust_and_open () {
  if (original.empty()) {
    cerr << "*** original dataset missing" << endl;
    exit(2);
  }

  if (predict.empty()) {
    string::size_type pos = original.rfind('.');
    predict = (pos == string::npos ? original : original.substr(0, pos)) + ".pdx";
  }

  if (output == STDOUT) {
    string::size_type pos = original.rfind('.');
    output = (pos == string::npos ? original : original.substr(0, pos)) + "-compare.txt";
  }

  origfile.open(original);
  if (!origfile.is_open()) {
      cerr << "+++ Cannot open original dataset file " << original << endl;
      exit(2);
  }

  predfile.open(predict);
  if (!predfile.is_open()) {
      cerr << "+++ Cannot open predict file " << predict << endl;
      exit(2);
  }

  outfile.open(output);
  if (outfile.is_open())
    cout.rdbuf(outfile.rdbuf());
  else {
      cerr << "+++ Cannot open output file " << output << endl;
      exit(2);
  }
}

void print_arg () {
  cout << "@@@ Parameters:" << endl;
  cout << "@@@ ===========" << endl;
  cout << "@@@ version    = " << version << endl;
  cout << "@@@ original   = " << original << endl;
  cout << "@@@ prediction = " << predict << endl;
  cout << "@@@ output     = " << output << endl;
  cout << endl;
}

void error (const string &error_message) {
  cerr << "+++ " << error_message << endl;
  remove(output.c_str());
  cerr << "+++ output file " << output << " deleted" << endl;
  exit(2);
}

void read_both () {
  string orig_line;
  string pred_line;
  while (!origfile.eof() || !predfile.eof()) {
    getline(origfile, orig_line);
    getline(predfile, pred_line);
    if (!origfile.eof() && !predfile.eof()) {
      numline++;

      if (orig_line.empty())
	error("empty line " + to_string(numline) + " in original dataset " + original);
      else if (pred_line.empty())
	error("empty line " + to_string(numline) + " in prediction file " + predict);

      vector<string> ochunks = split(orig_line, " ");
      string leader = ochunks[0];
      tally[leader]++;
      ochunks.clear();
      vector<string> pchunks = split(pred_line, ",");

      State state;
      if (pchunks.size() == 1)
	state = out;
      else if (pchunks.size() == 2) {
	vector<string> vchunks = split(pchunks[1], "+");
	bool present = find(vchunks.cbegin(), vchunks.cend(), leader) != vchunks.cend();
	if (!present)
	  state = out;
	else
	  state = vchunks.size() == 1 ? eql : in;
	aggregate[leader][state]++;
      } else
	error("line "
	      + to_string(numline)
	      + " in prediction file "
	      + predict
	      + " compromised");
    } else if (!origfile.eof() && predfile.eof()) {
      error("discrepancy: prediction file "
	    + predict
	    + " has fewer lines than original data set "
	    + original);
    } else if (origfile.eof() && !predfile.eof()) {
      error("discrepancy: original data set "
	    + original
	    + " has fewer lines than prediction file "
	    + predict);
    }
  }
}

inline size_t max (size_t a, size_t b) {
  return a >= b ? a : b;
}

void statistics () {
  map<State, size_t> maxlen;
  size_t leader_len = 0;
  for (const auto &t : aggregate) {
    leader_len = max(t.first.length(), leader_len);
    for (short state = eql; state <= out; ++state)
      maxlen[(State)state] = max(to_string(t.second[state]).length(), maxlen[(State)state]);
  }
  size_t totallen = 0;
  for (const auto &t : tally)
    totallen = max(to_string(t.second).length(), totallen);
  totallen = max(totallen, 6)+1;

  const size_t ldlen      = max(leader_len, 9);
  const size_t eqlen      = max(maxlen.at(eql), 6)+1;
  const size_t inlen      = max(maxlen.at(in),  3)+1;
  const size_t outlen     = max(maxlen.at(out), 4)+1;
  const size_t percentlen = 11;
  const size_t dashlen    = ldlen+totallen+eqlen+inlen+outlen+3*(percentlen+1);
  const string dash(dashlen, '-');

  cout << right << setw(ldlen) << "concept"
       << right << setw(totallen) << "total"
       << right << setw(eqlen) << "equal"
       << right << setw(percentlen+1) << "percentage"
       << right << setw(inlen) << "in"
       << right << setw(percentlen+1) << "percentage"
       << right << setw(outlen) << "out"
       << right << setw(percentlen+1) << "percentage"
       << endl;
  // cout << "===========================================" << endl;
  cout << dash << endl;
  for (const auto &t : aggregate)
    if (tally.count(t.first) == 0)
      continue;
    else
      cout << right << setw(ldlen) << t.first
	   << right << setw(totallen) << tally.at(t.first)
	   << right << setw(eqlen) << t.second[eql]
	   << right << setw(percentlen) << setprecision(2) << fixed << showpoint
	   << (100.0 * t.second[eql]) / tally.at(t.first) << "%"
	   << right << setw(inlen) << t.second[in]
	   << right << setw(percentlen) << setprecision(2) << fixed << showpoint
	   << (100.0 * t.second[in]) / tally.at(t.first) << "%"
	   << right << setw(outlen) << t.second[out]
	   << right << setw(percentlen) << setprecision(2) << fixed << showpoint
	   << (100.0 * t.second[out]) / tally.at(t.first) << "%"
	   << endl;

  cerr << "+++ output written on " << output << endl;
}

//////////////////////////////////////////////////////////////////////////////

int main (int argc, char **argv) {
  version = NOARCH_VERSION "compare";
  cerr << "+++ version = " << version << endl;

  read_args(argc, argv);
  adjust_and_open();
  print_arg();
  read_both();
  statistics();
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
