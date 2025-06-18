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
 *      Author:   César Sagaert                                           *
 *      e-mail:   cesar.sagaert@ensta-paris.fr                            *
 *      Address:  ENSTA Paris, Palaiseau, France                          *
 *                                                                        *
 *	Version: parallel with POSIX threads                              *
 *      File:    mcp-pthread.cpp                                          *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <csignal>
// #include <mutex>
#include "mcp-matrix+formula.hpp"
#include "mcp-common.hpp"
#include "mcp-parallel.hpp"
#include "mcp-posix.hpp"

using namespace std;

Arch arch = archPTHREAD;
// mutex semaphore;

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
  version += arch_strg[arch];;
  cerr << "+++ version = " << version << endl;
  // set_terminate(crash);
  signal(SIGSEGV, crash);
  if (!debug)
    signal(SIGINT, interrupt);

  read_arg(argc, argv);
  adjust();
  print_arg();
  read_header();
  read_matrix(group_of_matrix);
  print_matrix(group_of_matrix);

  if (action == aSELECTED) {
    cerr << endl;
    cerr << "*** no need to run the selected action in parallel" << endl;
    cerr << "*** use mcp-seq to run this example" << endl;
    exit(2);
  }

  vector<thread> threads;
  const string temp_prefix = tpath + "mcp-tmp-";
  time_t start_time = time(nullptr);
  const string basename = temp_prefix + to_string(start_time);
  ofstream *thread_outfile   = new ofstream[grps.size()];
  ofstream *thread_latexfile = new ofstream[grps.size()];

  // start clock
  auto clock_start = chrono::high_resolution_clock::now();

  for (size_t rank = 0; rank < grps.size(); ++rank)
    threads.push_back(thread(thread_split, thread_outfile, thread_latexfile, basename, rank));

  for (auto &t : threads)
    t.join();

  delete [] thread_outfile;
  delete [] thread_latexfile;

  for (size_t rank = 0; rank < grps.size(); ++rank) {
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

  // stop the clock
  auto clock_stop = chrono::high_resolution_clock::now();
  auto duration = chrono::duration_cast<chrono::milliseconds>(clock_stop - clock_start);
  size_t dtime = duration.count();
  
  outfile << "+++ time = "
	  << time2string(dtime)
	  << endl;
  
  outfile << "+++ end of run +++" << endl;
  outfile.close();
  return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
