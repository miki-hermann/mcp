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
 *	Version: parallel with POSIX threads and hybrid interior          *
 *      File:    mcp-posix.hpp                                            *
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

#include "mcp-matrix+formula.hpp"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OGchunk (const Row &a, const Matrix &M, Matrix &result, int left, int right);
Matrix gather (const Matrix &A, const Matrix &B);

//==================================================================================================
