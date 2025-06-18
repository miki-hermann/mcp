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
 *      File:    template for mcp-check.cpp                               *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Caller module for mcp-trans, mcp-seq, mcp-pthread, mcp-check,          *
 *                   and mcp-predict                                      *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cstdlib>
#include "mcp-basic.hpp"
#include "mcp-sha3.hpp"
#include "mcp-version.hpp"

using namespace std;

vector<string> arg_list;
string module_name;	// name of the real module to be called
string module;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// reads the input parameters
void read_arg (int argc, char *argv[]) {
  module_name = argv[0];
  module = module_name.substr(4); // eliminate prefix mcp-
  module_string[0] = module;
  size_t i = predict;
  while (module_string[i] != module)
    i--;
  if (i == 0) {
    cerr << "+++ module " << module_name << " incorrect" << endl;
    exit(2);
  }

  size_t argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "--version" || arg == "-v") {
      if (argument < argc-1) {
	version = argv[++argument];
      } else
	cerr << "+++ no version selected, revert to default" << endl;
    } else
      arg_list.push_back(arg);
    ++argument;
  }
}

string join (const vector<string> &arglist) {
  string arg_string;
  if (arglist.empty())
    cerr << "+++ " << module_name << ": no parameters to join";
  else {
    arg_string = arglist[0];
    for (size_t i = 1; i < arglist.size(); ++i)
      arg_string += " " + arglist[i];
  }
  return arg_string;
}

int main (int argc, char **argv) {
  read_arg(argc, argv);
  set_version();
  const string arg_string = join(arg_list);
  const string binary(module_name + "-" + version);
  string whis;
  const size_t pos = search_file(binary, whis);
  if (pos == string::npos) {
    cerr << "+++ binary file " << whis << " not found" << endl;
    exit(2);
  }
  const string sha3 = sha3_file(whis);
  if (sha3 != VERSION.at(module).at(version)) {
    cerr << "+++ binary file " << binary << " compromised" << endl
	 << "... recompile the binaries for " << version << " version"
	 << endl;
    exit(2);
  } else {
    const string system_call(binary + " " + arg_string);
    int sysout = system(system_call.c_str());
    return sysout;
  }
}
