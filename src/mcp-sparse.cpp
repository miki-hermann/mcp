/**************************************************************************
 *                                                                        *
 *                                                                        *
 *	       Multiple Classification   Problem (MCP)                    *
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
 *      File:    mcp-sparse.cpp                                           *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Transform files from sparse format to full format                      *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "mcp-matrix+formula.hpp"

using namespace std;

#define STDIN    "STDIN"
#define STDOUT   "STDOUT"

string input   = STDIN;
string output  = STDOUT;
string dflt    = "0";

ifstream infile;
ifstream metafile;
ofstream outfile;
streambuf *backup;

int max_idx = 0;

//------------------------------------------------------------------------------

bool isblank(const string &id) {
  if (id.empty())
    return true;
  for (int i = 0; i < id.length(); ++i)
    if (isspace(id[i]) || iscntrl(id[i]) || !isprint(id[i]))
      return true;
  return false;
}

void read_arg  (int argc, char *argv[]) {	// reads the input parameters
  int argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "-i"
	|| arg == "--input") {
      input = argv[++argument];
    } else if (arg == "-o"
	       || arg == "--output") {
      output = argv[++argument];
    } else if (arg == "-d"
	       || arg == "--default") {
      dflt = argv[++argument];
    } else {
    cerr << "+++ argument error: " << arg << " " << argv[argument] << endl;
    exit(1);
    }
    ++argument;
  }
  if (input != STDIN && output == STDOUT) {
    string::size_type pos = input.rfind('.');
    output = (pos == string::npos ? input : input.substr(0, pos)) + ".data";
  }
  if (isblank(dflt)) {
    cerr << "*** Default string cannot be white space or control char" << endl;
    exit(1);
  }
}

void IO_open () {
  if (input != STDIN) {
    infile.open(input);
    if (infile.is_open()) {
      cin.rdbuf(infile.rdbuf());
    } else {
      cerr << "+++ Cannot open input file " << input << endl;
      exit(1);
    }
  }
  
  if (output != STDOUT) {
    outfile.open(output);
    if (outfile.is_open()) {
      backup = cout.rdbuf();
      cout.rdbuf(outfile.rdbuf());
    } else {
      cerr << "+++ Cannot open output file " << output << endl;
      exit(1);
    }
  }
}

void IO_close () {
  if (input != STDIN)
    infile.close();
  if (output != STDOUT) {
    outfile.close();
    cout.rdbuf(backup);
  }
}

void I_reopen () {
  if (input != STDIN) {
    infile.close();
    infile.open(input);
    if (infile.is_open()) {
      cin.rdbuf(infile.rdbuf());
    } else {
      cerr << "+++ Cannot reopen input file " << input << endl;
      exit(1);
    }
  }
}

void spread (const string &line) {
  vector<string> parts = split(line, " \t");
  cout << parts[0];

  int ptr = 1;
  for (int i = 1; i < parts.size(); ++i) {
    vector<string> pair = split(parts[i], ":");
    
    while (ptr < stoi(pair[0])) {
      cout << " " << dflt;
      ptr++;
    }
    cout << " " << pair[1];
    ptr++;
  }
  while (ptr <= max_idx) {
    cout << " " << dflt;
    ptr++;
  }
  cout << endl;
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  string line;
  vector<string> sparse_data;

  read_arg(argc, argv);
  IO_open();

  while (getline(cin, line)) {
    if (input == STDIN)
      sparse_data.push_back(line);

    vector<string> parts = split(line, " \t");
    vector<string> last = split(parts[parts.size()-1], ":");
    max_idx = max(max_idx, stoi(last[0]));
  }

  I_reopen();

  if (input == STDIN)
    for (int ln = 0; ln < sparse_data.size(); ++ln)
      spread(sparse_data[ln]);
  else
    while (getline(cin, line))
      spread(line);
  
  IO_close();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
