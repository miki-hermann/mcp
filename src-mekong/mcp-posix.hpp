/**************************************************************************
 *                                                                        *
 *                                                                        *
 *        Multiple Characterization Problem (MCP)                         *
 *                                                                        *
 * Author:   Miki Hermann                                                 *
 * e-mail:   hermann@lix.polytechnique.fr                                 *
 * Address:  LIX (CNRS UMR 7161), Ecole Polytechnique, France             *
 *                                                                        *
 * Author:   Gernot Salzer                                                *
 * e-mail:   gernot.salzer@tuwien.ac.at                                   *
 * Address:  Technische Universitaet Wien, Vienna, Austria                *
 *                                                                        *
 * Author:   César Sagaert                                                *
 * e-mail:   cesar.sagaert@ensta-paris.fr                                 *
 * Address:  ENSTA Paris, Palaiseau, France                               *
 *                                                                        *
 * Version: all                                                           *
 *     File:    src/mcp-posix.hpp                                         *
 *                                                                        *
 *      Copyright (c) 2019 - 2023                                         *
 *                                                                        *
 **************************************************************************/

#pragma once

#include "mcp-matrix+formula.hpp"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OGchunk(const Row &a, const Matrix &M, Matrix &result, int left,
             int right);
Matrix gather(const Matrix &A, const Matrix &B);

//==================================================================================================
