/**************************************************************************
 *                                                                        *
 *                                                                        *
 *	       Multiple Characterization Problem (MCP)                    *
 *                                                                        *
 *	Author:   Miki Hermann                                            *
 *	e-mail:   hermann@lix.polytechnique.fr                            *
 *	Address:  LIX (CNRS UMR 7161), Ecole Polytechnique, France        *
 *                                                                        *
 *	Author: Gernot Salzer                                             *
 *	e-mail: gernot.salzer@tuwien.ac.at                                *
 *	Address: Technische Universitaet Wien, Vienna, Austria            *
 *                                                                        *
 *	Version: parallel with POSIX threads and hybrid interior          *
 *      File:    mcp-posix.cpp                                            *
 *                                                                        *
 *      Copyright (c) 2019 - 2023                                         *
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

void OGchunk (const Row &a, const Matrix &M,
	      unique_ptr<Row> &result,
	      int left, int right) {
  // selects tuples (rows) in M[left..right-1] above the tuple a
  // usefull for distribution among threads
  for (int i = left; i < right; ++i)
    if (M[i] >= a)
      result = make_unique<Row>(result == nullptr ? M[i] : Min(*result, M[i]));
}

unique_ptr<Row> ObsGeq (const Row &a, const Matrix &M) {
  // selects tuples (rows) above the tuple a
  unique_ptr<Row> P;
  const unsigned msize = M.size();
  if (msize > chunkLIMIT) {
    int nchunks = (msize / chunkLIMIT) + (msize % chunkLIMIT > 0);
    unique_ptr<Row> chunk[nchunks];
    vector<thread> chunk_threads;
    for (unsigned i = 0; i < nchunks; ++i)
      chunk_threads.push_back(std::thread(OGchunk,
					  ref(a), ref(M), ref(chunk[i]),
					  i*chunkLIMIT,
					  min((i+1)*chunkLIMIT, msize)
					  )
			      );
    for (auto &ct : chunk_threads)
      ct.join();
    for (int i = 0; i < nchunks; ++i)
      if (chunk[i] != nullptr)
	P = make_unique<Row>(P == nullptr ? *chunk[i] : Min(*P, *chunk[i]));
  } else
    for (Row row : M)
      if (row >= a)
	P = make_unique<Row>(P == nullptr ? row : Min(*P, row));
  return P;
}

//==================================================================================================
