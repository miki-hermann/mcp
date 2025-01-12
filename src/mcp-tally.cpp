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
 *      File:    mcp-overview.cpp                                         *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Given the position of the concept, this software computes the tally    *
 * of the values and their percentual representation in the dataset.      *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "mcp-matrix+formula.hpp"

using namespace std;

size_t total = 0;
map<string, size_t> accountant;
map<string, double> percentage;

//------------------------------------------------------------------------------

void read_input (const int concept_column) {
  string line;
  size_t lineno = 0;
  while (getline(cin, line)) {
    lineno++;
    if (line.empty())
      continue;

    // erase leading and trailing whitespace
    auto nospace = line.find_first_not_of(" \t");
    line.erase(0, nospace);
    nospace = line.find_last_not_of(" \t");
    line.erase(nospace+1);
    if (line.empty())
      continue;

    total++;

    string line1;
    bool is_string = false;
    for (size_t i = 0; i < line.length(); ++i) {
      char chr = line[i];
      if (chr == '\\' && i == line.length()-1) {
	cerr << "+++ line " << lineno
	     << " cannot terminate with a backslash"
	     << endl;
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
    if (concept_column >= chunks.size()) {
      cerr << "+++ concept column out of range on line "
	   << lineno
	   << endl;
      exit(2);
    } else
      accountant[chunks[concept_column]]++;
  }
  // cerr << "+++ read " << total << " nonempty lines" << endl;
}

size_t tally (const long concept_column) {
  read_input(concept_column);
  for (const auto &val : accountant)
    percentage[val.first] = ((1.0 * val.second) / (1.0 * total)) * 100.0;
  return total;
}

//////////////////////////////////////////////////////////////////////////////
