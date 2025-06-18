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
 * Computes the SHA3 hash of a (binary) file                              *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

// compile with
// g++ -O4 -o mcp-switch mcp-switch.cpp -lssl -lcrypto

#include <iostream>
#include <string>

using namespace std;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

string sha3_file (const string &filename);
size_t search_file (const string &mv_file, string &whis);

//////////////////////////////////////////////////////////////////////////////
