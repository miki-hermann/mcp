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
 *	Version: parallel with POSIX threads                              *
 *      File:    mcp-pthread.cpp                                          *
 *                                                                        *
 *      Copyright (c) 2019 - 2020                                         *
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

#include <iostream>
#include <fstream>
#include <thread>
// #include <mutex>
#include "mcp-matrix+formula.hpp"
#include "mcp-common.hpp"
#include "mcp-parallel.hpp"
#include "mcp-posix.hpp"

using namespace std;

Arch arch = archPTHREAD;

//--------------------------------------------------------------------------------------------------

void thread_split (ofstream thread_outfile[], string basename, int rank) {
  string filename = basename + "-" + to_string(rank) + ".txt";
  // semaphore.lock();
  thread_outfile[rank].open(filename);
  // semaphore.unlock();
  if (! thread_outfile[rank].is_open()) {
    cerr << "+++ Cannot open output file " << filename << endl;
    exit(2);
  }
  thread_outfile[rank]
    << endl
    << "+++ Start output of thread " << rank
    << endl << endl;

  split_action(thread_outfile[rank], rank);
  
  thread_outfile[rank] << "+++ End output of thread " << rank << endl;
  // semaphore.lock();
  thread_outfile[rank].close();
  // semaphore.unlock();
}

int main(int argc, char **argv)
{
  time_t start_time = time(nullptr);

  version += arch_strg[arch];;
  set_terminate(crash);

  read_arg(argc, argv);
  adjust();
  print_arg();
  read_matrix(group_of_matrix);
  print_matrix(group_of_matrix);

  vector<thread> threads;
  const string temp_prefix = tpath + "mcp-tmp-";
  const string basename = temp_prefix + to_string(start_time);
  ofstream *thread_outfile = new ofstream[grps.size()];

  for (int rank = 0; rank < grps.size(); ++rank)
    threads.push_back(thread(thread_split, thread_outfile, basename, rank));

  for (auto &t : threads)
    t.join();
  delete [] thread_outfile;

  for (int rank = 0; rank < grps.size(); ++rank) {
    string filename = basename + "-" + to_string(rank) + ".txt";
    ifstream in(filename);
    string line;

    while (getline(in, line))
      outfile << line << endl;
    outfile << endl;
    // semaphore.lock();
    in.close();
    remove(filename.c_str());
    // semaphore.unlock();
  }

  time_t finish_time = time(nullptr);
  outfile << "+++ time = "
	  << time2string(difftime(finish_time, start_time))
	  << endl;
  
  outfile << "+++ end of run +++" << endl;
  outfile.close();
  return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
