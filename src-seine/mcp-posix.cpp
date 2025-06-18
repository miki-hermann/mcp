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
 *	Version: parallel with POSIX threads and hybrid interior          *
 *      File:    mcp-posix.cpp                                            *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Given several  Boolean matrices  representing the  presence/absence of *
 * attributes in  observations, this software generates  Horn, dual Horn, *
 * or bijunctive formulas from positive and negative examples represented *
 * by these matrices.                                                     *
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
	      size_t left, size_t right) {
  // selects tuples (rows) in M[left..right-1] above the tuple a
  // usefull for distribution among threads
  for (size_t i = left; i < right; ++i)
    if (M[i] >= a)
      result = make_unique<Row>(result == nullptr ? M[i] : Min(*result, M[i]));
}

unique_ptr<Row> ObsGeq (const Row &a, const Matrix &M) {
  // selects tuples (rows) above the tuple a
  unique_ptr<Row> P;
  const size_t msize = M.size();
  if (msize > chunkLIMIT) {
    size_t nchunks = (msize / chunkLIMIT) + (msize % chunkLIMIT > 0);
    unique_ptr<Row> chunk[nchunks];
    vector<thread> chunk_threads;
    for (size_t i = 0; i < nchunks; ++i)
      chunk_threads.push_back(std::thread(OGchunk,
					  ref(a), ref(M), ref(chunk[i]),
					  i*chunkLIMIT,
					  min((i+1)*chunkLIMIT, msize)
					  )
			      );
    for (auto &ct : chunk_threads)
      ct.join();
    for (size_t i = 0; i < nchunks; ++i)
      if (chunk[i] != nullptr)
	P = make_unique<Row>(P == nullptr ? *chunk[i] : Min(*P, *chunk[i]));
  } else
    for (const Row &row : M)
      if (row >= a)
	P = make_unique<Row>(P == nullptr ? row : Min(*P, row));
  return P;
}

//==================================================================================================
