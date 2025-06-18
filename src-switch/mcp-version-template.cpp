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
 *      File:    mcp-version.cpp                                          *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Template for version management                                        *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <string>
#include <map>
#include "mcp-basic.hpp"
#include "mcp-version.hpp"

using namespace std;

string default_version; /*** default version ***/
// string version;		// real version
// VERSION map VERSION[module][version]
//  module,    version, sha3
// map<string, map<string, string>> VERSION;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void set_version () {
  /*** VERSION map ***/

  if (version.empty()) {
    char *env = getenv(global_version.c_str());
    if (env == nullptr) {
      cerr << "+++ variable " << global_version
	   << " missing or incorrect, proceed with default version "
	   << default_version
	   << endl;
      version = default_version;
    } else {
      version = env;
      if (version.empty()) {
	cerr << "+++ variable " << global_version
	     << " empty, proceed with default version "
	     << default_version
	     << endl;
	version = default_version;
      }
    }
  }

  version_string[0] = version;
  size_t i = mekong;
  while (version_string[i] != version)
    --i;
  if (i == 0) {
    cerr << "+++ non-recognized version " << version << endl;
    exit(2);
  }
}
