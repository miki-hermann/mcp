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
 *      File:    mcp-posix.cpp                                            *
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

#include <thread>
#include "mcp-common.hpp"
#include "mcp-matrix+formula.hpp"
#include "mcp-parallel.hpp"
#include "mcp-posix.hpp"

using namespace std;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OGchunk (const Row &a, const Matrix &M, Matrix &result, int left, int right) {
  // selects tuples (rows) in M[left..right-1] above the tuple a
  // usefull for distribution among threads
  for (int i = left; i < right; ++i)
    if (M[i] >= a)
      result.push_back(M[i]);
}

Matrix gather (const Matrix &A, const Matrix &B) {
  if (A.empty())
    return B;
  if (B.empty())
    return A;

  Matrix C = A;
  copy(B.begin(), B.end(), C.end());
  return C;
}

Matrix ObsGeq (const Row &a, const Matrix &M) {
  // selects tuples (rows) above the tuple a
  Matrix P;
  const int msize = M.size();
  if (msize > chunkLIMIT) {
    int nchunks = (msize / chunkLIMIT) + (msize % chunkLIMIT > 0);
    Matrix *chunk = new Matrix[nchunks];
    vector<thread> chunk_threads;
    for (int i = 0; i < nchunks; ++i)
      chunk_threads.push_back(thread(OGchunk,
				     ref(a), ref(M), ref(chunk[i]),
				     i*chunkLIMIT, min((i+1)*chunkLIMIT, msize)));
    for (auto &ct : chunk_threads)
      ct.join();
    for (int i = 0; i < nchunks; ++i)
      P = gather(P, chunk[i]);
    delete [] chunk;
  } else
    for (Row row : M)
      if (row >= a)
	P.push_back(row);
  return P;
}

//==================================================================================================
