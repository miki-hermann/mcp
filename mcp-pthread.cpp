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

void thread_split (ofstream thread_outfile[], ofstream thread_latexfile[],
		   string basename, int rank) {
  string filename  = basename + "-" + to_string(rank) + ".txt";
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

  if (latex.length() > 0) {
    string latexname = basename + "-" + to_string(rank) + ".tex";
    thread_latexfile[rank].open(filename);
    if (! thread_latexfile[rank].is_open()) {
      cerr << "+++ Cannot open latex file " << latexname << endl;
      exit(2);
    }
    thread_latexfile[rank]
    << endl
    << "% Start output of thread " << rank
    << endl << endl;
  }

  split_action(thread_outfile[rank], thread_latexfile[rank], rank);
  
  thread_outfile[rank] << "+++ End output of thread " << rank << endl;
  // semaphore.lock();
  thread_outfile[rank].close();
  // semaphore.unlock();

  if (latex.length() > 0) {
    thread_latexfile[rank] << "% End output of thread " << rank << endl;
    thread_latexfile[rank].close();
  }
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
  ofstream *thread_outfile   = new ofstream[grps.size()];
  ofstream *thread_latexfile = new ofstream[grps.size()];

  for (int rank = 0; rank < grps.size(); ++rank)
    threads.push_back(thread(thread_split, thread_outfile, thread_latexfile, basename, rank));

  for (auto &t : threads)
    t.join();
  delete [] thread_outfile;
  delete [] thread_latexfile;

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

    if (latex.length() > 0) {
      string latexname = basename + "-" + to_string(rank) + ".tex";
      ifstream in(latexname);

      while (getline(in, line))
	latexfile << line << endl;
      latexfile << endl;
      // semaphore.lock();
      in.close();
      remove(latexname.c_str());
    }
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
