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
 *      File:    mcp-basics.hpp                                           *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 *  Basic algorithms                                                      *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <vector>
#include <string>

using namespace std;

// true if line has been cleared and is nonempty
bool clear_line (const size_t lineno, string &line);

// commas (,) and semicolons (;) are replaced with spaces
// outside strings
void uncomma_line (string &line);

// splits a string into chunks separated by delimiters (split in perl)
vector<string> split (string strg, const string &delimiters);
