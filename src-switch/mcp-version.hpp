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
 *      File:    mcp-version.hpp                                          *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Version management                                                     *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <iostream>
#include <map>
#include "mcp-basic.hpp"

using namespace std;

const  string global_version = "MCP_VERSION";
// extern string default_version; // default version
extern string version;	       // real version
// VERSION map VERSION[module][version]
//         module,    version, sha3
extern map<string, map<string, string>> VERSION;

extern void set_version();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
