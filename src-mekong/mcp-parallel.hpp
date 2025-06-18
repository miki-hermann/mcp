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
 *	Version: all parallel                                             *
 *      File:    mcp-parallel.hpp                                         *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Given several  Boolean matrices  representing the  presence/absence of *
 * attributes in  observations, this software generates  Horn, dual Horn, *
 * or bijunctive formulas from positive and negative examples represented *
 * by these matrices.                                                     *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <string>
#include "mcp-matrix+formula.hpp"

using namespace std;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void adjust ();
void read_header ();
void print_arg ();
Formula learnHornLarge (ofstream &process_outfile,
			const Matrix &T, const Matrix &F, const vector<size_t> &A);
Formula learn2sat (ofstream &process_outfile, const Matrix &T, const Matrix &F);
void split_action (ofstream &popr, ofstream &latpr, const int &process_id);
void crash(int signal);
void interrupt (int signal);

//==================================================================================================
