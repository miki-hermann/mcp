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
#include "mcp-tally.hpp"

using namespace std;

size_t total = 0;
account<string> concept_items;
account<size_t> empty_items;
account<size_t> qmark_items;

//------------------------------------------------------------------------------

void read_input (const size_t concept_column) {
  string line;
  size_t lineno = 0;
  while (getline(cin, line)) {
    lineno++;
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
    } else {
      concept_items.number[chunks[concept_column]]++;
      for (size_t i = 0; i < chunks.size(); ++i)
	if (i == concept_column)
	  continue;
	else if (chunks[i] == "?")
	  qmark_items.number[i]++;
	else if (chunks[i].empty())
	  empty_items.number[i]++;
    }
  }
}

size_t tally (const size_t concept_column) {
  read_input(concept_column);
  for (const auto &val : concept_items.number)
    concept_items.percent[val.first] =
      ((1.0 * val.second) / (1.0 * total)) * 100.0;
  for (const auto &val : empty_items.number)
    empty_items.percent[val.first] =
      ((1.0 * val.second) / (1.0 * total)) * 100.0;
  for (const auto &val : qmark_items.number)
    qmark_items.percent[val.first] =
      ((1.0 * val.second) / (1.0 * total)) * 100.0;
  return total;
}

//////////////////////////////////////////////////////////////////////////////
