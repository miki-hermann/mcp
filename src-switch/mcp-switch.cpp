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
 *      File:    mcp-switch.cpp                                           *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Establishes switching between different versions of MCP modules        *
 *                                                                        *
 **************************************************************************/

// compile with
// g++ -O4 -o mcp-switch mcp-switch.cpp -lssl -lcrypto

#include <iostream>
#include <fstream>
#include <string>
#include "mcp-basic.hpp"
#include "mcp-sha3.hpp"
#include "mcp-version.hpp"

using namespace std;

ifstream input;
ofstream output;
const string mcp_version_prefix = "mcp-version";
string default_version; // default version

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// reads the input parameters
void read_arg (int argc, char *argv[]) {
  if (argc == 1)
    return;
  size_t argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "--default"
	|| arg == "-d") {
      if (argument < argc-1) {
	default_version = argv[++argument];
      } else
	cerr << "+++ no default version selected, revert to default" << endl;
    } else
      cerr <<  "+++ unknown option " << arg << endl;
    ++argument;
  }
}

void io_open () {
  const string  infile(mcp_version_prefix + "-template.cpp");
  const string outfile(mcp_version_prefix + ".cpp");

  input.open(infile);
  if (!input.is_open()) {
    cerr << "+++ Cannot open template file " << infile << endl;
    exit(2);
  }
  output.open(outfile);
  if (!output.is_open()) {
    cerr << "+++ Cannot open target file " << outfile << endl;
    exit(2);
  }
}

void io_close () {
  input.close();
  output.close();
}

//////////////////////////////////////////////////////////////////////////////

int main (int argc, char **argv) {
  read_arg(argc, argv);
  if (!default_version.empty())
    cerr << "+++ default version will be " << default_version << endl;

  io_open();
  output << "// WARNING: this is a generated file by mcp-switch"
	 << endl << endl;
  string line;
  getline(input, line);
  while (line.find("*** default version ***") == string::npos) {
    output << line << endl;
    getline(input, line);
  }
  output << "string default_version(\""
	 << default_version
	 << "\"); // default version" << endl;
  getline(input, line);
  while (line.find("*** VERSION map ***") == string::npos) {
    output << line << endl;
    getline(input, line);
  }
  output << endl;

  for (short module = trans; module <= predict; ++module) {
    const string mod_file("mcp-" + module_string[module]);
    bool errflag = false;

    for (short version = seine; version <= mekong; ++version) {
      const string mv_file(mod_file + "-" + version_string[version]);
      string whis;
      size_t pos = search_file(mv_file, whis);
      if (pos == string::npos) {
	cerr << "+++ binary file " << whis << " not found" << endl;
	errflag = true;
	break;
      } else {
	const string sha3 = sha3_file(whis);
	// produce VERSION map
	output << "  VERSION[\"" << module_string[module]
	       << "\"][\""       << version_string[version]
	       << "\"] ="
	       << endl
	       << "\"" << sha3 << "\";" << endl << endl;
      }
    }
    if (!errflag)
      cerr << "+++ branching for MCP module " << mod_file << " produced" << endl;
  }
  
  while(getline(input, line))
    output << line << endl;
  io_close();
}

//////////////////////////////////////////////////////////////////////////////
