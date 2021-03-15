/**************************************************************************
 *                                                                        *
 *                                                                        *
 *	       Multiple Characterization Problem (MCP)                    *
 *                                                                        *
 *	Author:   Miki Hermann                                            *
 *	e-mail:   hermann@lix.polytechnique.fr                            *
 *	Address:  LIX (CNRS UMR 7161), Ecole Polytechnique, France         *
 *                                                                        *
 *	Author: Gernot Salzer                                             *
 *	e-mail: gernot.salzer@tuwien.ac.at                                *
 *	Address: Technische Universitaet Wien, Vienna, Austria            *
 *                                                                        *
 *	Version: all parallel                                             *
 *      File:    mcp-parallel.hpp                                         *
 *                                                                        *
 *      Copyright (c) 2019 - 2021                                         *
 *                                                                        *
 * Given several  Boolean matrices  representing the  presence/absence of *
 * attributes in  observations, this software generates  Horn, dual Horn, *
 * or bijunctive formulas from positive and negative examples represented *
 * by these matrices.                                                     *
 *                                                                        *
 * This software has been created within the ACCA Project.                *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <string>
#include "mcp-matrix+formula.hpp"

using namespace std;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void adjust ();
void print_arg ();
Formula learnHornLarge (ofstream &process_outfile, const Matrix &T, Matrix F);
Formula learnBijunctive (ofstream &process_outfile, const Matrix &T, const Matrix &F);
// void OneToOne (ofstream &process_outfile, const int &i);
// void OneToAll (ofstream &process_outfile, const int &i);
// void OneToAllNosection (ofstream &process_outfile, const int &i);
void split_action (ofstream &popr, ofstream &latpr, const int &process_id);
void crash();

//==================================================================================================
