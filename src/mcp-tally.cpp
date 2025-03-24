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
 *      File:    mcp-tally.cpp                                            *
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

void read_input (const size_t concept_column) {
  string line;
  size_t lineno = 0;
  while (getline(cin, line)) {
    lineno++;
    if (line.empty())
      continue;

    total += clear_line(lineno, line);
    if (line.empty())
      continue;
    uncomma_line(line);
    const vector<string> chunks = split(line, " \t");
    if (concept_column >= chunks.size()) {
      cerr << "+++ concept column out of range on line "
	   << lineno
	   << endl;
      exit(2);
    } else
      accountant[chunks[concept_column]]++;
  }
}

size_t tally (const size_t concept_column) {
  read_input(concept_column);
  for (const auto &val : accountant)
    percentage[val.first] = ((1.0 * val.second) / (1.0 * total)) * 100.0;
  return total;
}

//////////////////////////////////////////////////////////////////////////////
