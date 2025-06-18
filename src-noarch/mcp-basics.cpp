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
 *      File:    mcp-basics.cpp                                           *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 *  Basic algorithms                                                      *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <vector>
#include <string>
#include "mcp-defs.hpp"

using namespace std;

// true if line has been cleared and is nonempty
bool clear_line (const size_t lineno, string &line) {
  // erase leading and trailing whitespace
  auto nospace = line.find_first_not_of(SPACE);
  line.erase(0, nospace);
  nospace = line.find_last_not_of(ENDSPACE);
  line.erase(nospace+1);
  if (line.empty())
    return false;

  // treat back slashed characters and strings
  string line1;
  bool is_string = false;
  size_t i = 0;
  // for (size_t i = 0; i < line.length(); ++i) {
  while (i < line.length()) {
    const char chr = line[i];
    if (chr == '\\' && i == line.length()-1) {
      cerr << "+++ line " << lineno
	   << " cannot terminate with a backslash"
	   << endl;
      return false;
    } else if (chr == '\\')
      line1 += chr + line[++i];
    else if (chr == '"')
      is_string = ! is_string;
    else if (is_string && chr == ' ')
      line1 += "_";
    else if (is_string && (chr == ',' || chr == ';'))
      line1 += ":";
    else
      line1 += chr;
    i++;
  }
  // line.clear();
  line = move(line1);
  return true;
}

// commas (,) and semicolons (;) are replaced with spaces
// outside strings
void uncomma_line (string &line) {
  bool is_string = false;
  // replace commas and semicolons by a space
  for (size_t i = 0; i < line.length(); ++i) {
    char chr = line[i];
    if (chr == '"')
      is_string = ! is_string;
    else if (! is_string && (line[i] == ',' || line[i] == ';'))
      line[i] = ' ';
  }
}

// splits a string into chunks separated by delimiters (split in perl)
vector<string> split (string strg, const string &delimiters) {
  vector<string> chunks;

  // get rid of non-printable characters at the end of the string
  // without warning
  // because Linux, Windows, and MacOS all terminate the string differently
  // doing it the hard way
  if (! strg.empty()) {
    size_t i = strg.length()-1;
    while (i >= 0 && ! isprint(strg[i]))
      i--;
    if (i < 0)
      strg.clear();
    else
      strg = strg.substr(0, i+1);
  }
  for (size_t i = 0; i < strg.length(); ++i)
    if (! isprint(strg[i])) {
      cerr << "+++ string on input has a non-printable character on position "
	   << i
	   << endl;
      if (i >= strg.length()-1)
	cout << "... use dos2unix to fix it" << endl;
      exit(2);
    }

  // proper split
  size_t pointer = 0;
  while (pointer < strg.length()) {
    const size_t found = strg.find_first_of(delimiters, pointer);
    if (found == string::npos) {
      chunks.push_back(strg.substr(pointer));
      break;
    }
    chunks.push_back(strg.substr(pointer, found - pointer));
    pointer = found+1;
  }
  return chunks;
}
