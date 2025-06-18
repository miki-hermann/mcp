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

#pragma once

#include <string>
#include <map>

using namespace std;

enum Module : short {
  dummy   = 0,
  trans   = 1,
  seq     = 2,
  pthread = 3,
  check   = 4,
  predict = 5
};
extern string module_string[];

enum Version : short {
  none    = 0,
  seine   = 1,
  danube  = 2,
  mekong  = 3
};
extern string version_string[];

extern string version;		// real version
// VERSION map VERSION[module][version]
//  module,    version, sha3
extern map<string, map<string, string>> VERSION;
