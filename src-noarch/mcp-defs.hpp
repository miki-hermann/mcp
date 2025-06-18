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
 *      File:    mcp-defs.hpp                                             *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 *  Basic definitions                                                     *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <string>

using namespace std;

const string NOARCH_VERSION = "1.04f-noarch-";

const string SPACE = " \t";
const string ENDSPACE = " \t\n\v\f\r";
const int SENTINEL = -1;

const string STDIN  = "STDIN";
const string STDOUT = "STDOUT";
