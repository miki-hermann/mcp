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
 *      File:    mcp-uniq.cpp                                             *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 *  Takes a matrix and deletes rows with same values but different        *
 *  leading group identifier. Technical support oft mcp-trans.            *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include "mcp-defs.hpp"
#include "mcp-basics.hpp"

using namespace std;

string version = NOARCH_VERSION;

string input  = STDIN;
string output = STDOUT;
ifstream infile;
ofstream outfile;

bool use_hash = true;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void read_arg(int argc, char *argv[]) {	// reads the input parameters
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
    	       || arg == "-o") {
      if (argument < argc-1) {
	output = argv[++argument];
      } else
	cerr << "+++ no output file selected, revet to default" << endl;
    } else if (arg == "--hash") {
      if (argument < argc-1) {
	string hash_par = argv[++argument];
	if (hash_par == "yes"
	    || hash_par == "y"
	    || hash_par == "1")
	  use_hash = true;
	else if (hash_par == "no"
		 || hash_par == "n"
		 || hash_par == "0")
	  use_hash = false;
	else {
	  cerr << "+++ argument error: " << arg << " " << hash_par << endl;
	  exit(1);
	}
      } else
	cerr << "+++ no hash option selected, revet to default" << endl;
    } else
      cerr << "+++ unknown option " << arg << endl;
    ++argument;
  }
}

void IO_open () {
  if (input != STDIN) {
    infile.open(input);
    if (infile.is_open())
      cin.rdbuf(infile.rdbuf());
    else {
      cerr << "+++ Cannot open input file " << input << endl;
      exit(2);
    }
  }

  if (input != STDIN && output == STDOUT) {
    string::size_type pos = input.rfind('.');
    output = (pos == string::npos ? input : input.substr(0, pos)) + ".unq";
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

void IO_close () {
  if (input != STDIN)
    infile.close();
  if (output != STDOUT)
    outfile.close();
}

// void header () {
//   int ind_a, ind_b;
//   string line;

//   getline(cin, line);
//   istringstream inds(line);
//   inds >> ind_a >> ind_b;
//   cout << ind_a << " " << ind_b << endl;
//   if (ind_a == 1) {
//     getline(cin, line);
//     cout << line << endl;
//   }
//   if (ind_b == 1) {
//     getline(cin, line);
//     cout << line << endl;
//   }
// }

void matrix () {
  string line;
  int numline = SENTINEL;
  
  vector<string> line_tab;
  vector<string> group_tab;
  map<size_t, set<int>> hash_tab;
  map<string, set<int>> nohash_tab;

  while (getline(cin, line)) {
    ++numline;
    clear_line(numline, line);
    if (line.empty())
      continue;
    line_tab.push_back(line);
    vector<string> chunk = split(line, " \t");
    group_tab.push_back(chunk[0]);

    string blob = "";
    for (int i = 1; i < chunk.size(); ++i)
      blob += "#" + chunk[i];
    if (use_hash) {
      size_t hash_blob = hash<string>{}(blob);
      hash_tab[hash_blob].insert(numline);
    } else
      nohash_tab[blob].insert(numline);
  }

  cerr << "+++ " << line_tab.size() << " rows read" << endl;
  cerr << "+++ (no)hash table size = "
       << (use_hash ? hash_tab.size() : nohash_tab.size())
       << endl;
  vector<bool> row_del_indicator(line_tab.size(), false);

  int del_num = 0;
  if (use_hash) {
    for (auto it = hash_tab.begin(); it != hash_tab.end(); ++it) {
      auto item = it->second;
      if (item.size() > 1) {
	set<string> grp;
	for (auto it2 = item.begin(); it2 != item.end(); ++it2)
	  grp.insert(group_tab[*it2]);
	if (grp.size() > 1) {
	  for (auto it2 = item.begin(); it2 != item.end(); ++it2) {
	    row_del_indicator[*it2] = true;
	    del_num++;
	  }
	}
      }
    }
  } else {
    for (auto it = nohash_tab.begin(); it != nohash_tab.end(); ++it) {
      auto item = it->second;
      if (item.size() > 1) {
	set<string> grp;
	for (auto it2 = item.begin(); it2 != item.end(); ++it2)
	  grp.insert(group_tab[*it2]);
	if (grp.size() > 1) {
	  for (auto it2 = item.begin(); it2 != item.end(); ++it2) {
	    row_del_indicator[*it2] = true;
	    del_num++;
	  }
	}
      }
    }
  }
  cerr << "+++ " << del_num << " rows deleted" << endl;

  int row_written = 0;
  for (int i = 0; i < line_tab.size(); ++i)
    if (!row_del_indicator[i]) {
      cout << line_tab[i] << endl;
      row_written++;
    }
  cerr << "+++ " << row_written << " rows written on " << output << endl;
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  version += "uniq";
  cerr << "+++ version = " << version << endl;

  read_arg(argc, argv);
  IO_open();
  // header();
  matrix();
  IO_close();
}

//////////////////////////////////////////////////////////////////////////////
