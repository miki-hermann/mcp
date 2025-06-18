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
 * Basic structures for version management                                *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#include <string>
#include <map>

using namespace std;

string module_string[] = {
  // must correspond with enum Module
  "",
  "trans",
  "seq",
  "pthread",
  "check",
  "predict"
};
string version_string[] = {
  // must correspond with enum Version
  "",
  "seine",
  "danube",
  "mekong"
};

string version;		// real version
// VERSION map VERSION[module][version]
//  module,    version, sha3
map<string, map<string, string>> VERSION;
