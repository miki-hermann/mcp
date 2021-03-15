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
 *	Version: hybrid (MPI + POSIX threads)                             *
 *      File:    mcp-hybrid.cpp                                           *
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
#include <mpi.h>
#include <thread>
// #include <mutex>
#include "mcp-matrix+formula.hpp"
#include "mcp-common.hpp"
#include "mcp-parallel.hpp"
#include "mcp-posix.hpp"

using namespace std;

Arch arch = archHYBRID;

int main(int argc, char **argv)
{
  int num_procs;
  int process_rank;

  time_t start_time = time(nullptr);

  version += arch_strg[arch];
  set_terminate(crash);

  read_arg(argc, argv);
  adjust();
  print_arg();
  read_matrix(group_of_matrix);
  print_matrix(group_of_matrix);

  const string temp_prefix = tpath + "mcp-tmp-";
  const string basename = temp_prefix + to_string(start_time);
  ofstream *process_outfile = new ofstream[grps.size()];
  ofstream *latex_outfile   = new ofstream[grps.size()];
  // auto process_outfile = make_unique<ofstream []>(grps.size());
  // smart pointers clash with POSIX threads

  int ierr = MPI_Init(&argc, &argv);
  if (ierr != 0) {
    cerr << endl << "*** MCP - fatal error" << endl;
    cerr << "*** MPI_Init returned error = " << ierr << endl;
    exit(1);
  }

  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

  if (process_rank == 0) {
    outfile << "+++ Number of processes = " << num_procs << endl << endl;
    if (np_fit && grps.size() < num_procs) {
      cerr << "+++ " << num_procs - grps.size() << " unused processes" << endl;
      outfile << "+++ " << num_procs - grps.size() << " unused processes" << endl;
      cerr << "+++ launch 'mpirun -np " << grps.size() << " " << argv[0] << " --fit yes ...'" << endl;
      outfile << "+++ launch 'mpirun -np " << grps.size() << " " << argv[0] << " --fit yes ...'" << endl;
      MPI_Abort(MPI_COMM_WORLD, 99);
    }
  }
  
  string filename  = basename + "-" + to_string(process_rank) + ".txt";
  string latexname = basename + "-" + to_string(process_rank) + ".tex";

  if (process_rank < grps.size()) {
    // semaphore.lock();
    process_outfile[process_rank].open(filename);
    // semaphore.unlock();
    if (! process_outfile[process_rank].is_open()) {
      cerr << "+++ Cannot open output file " << filename << endl;
      exit(2);
    }
    process_outfile[process_rank] << "+++ Start output of process " << process_rank << endl << endl;

    if (latex.length() > 0) {
      latex_outfile[process_rank].open(latexname);
      if (! latex_outfile[process_rank].is_open()) {
	cerr << "+++ Cannot open latex file " << latexname << endl;
	exit(2);
      }
      latex_outfile[process_rank] << "% Start output of process " << process_rank << endl << endl;
    }
  
    for (int process_id = process_rank; process_id < grps.size(); process_id += num_procs)
      split_action(process_outfile[process_rank], latex_outfile[process_rank], process_id);

    process_outfile[process_rank] << "+++ End output of process " << process_rank << endl;
    // semaphore.lock();
    process_outfile[process_rank].close();
    // semaphore.unlock();

    if (latex.length() > 0) {
      latex_outfile[process_rank] << "% End output of process " << process_rank << endl;
      latex_outfile[process_rank].close();
    }
  }

  // MPI_Barrier(MPI_COMM_WORLD); 
  MPI_Finalize();

  if (process_rank == 0) {
    delete [] process_outfile;
    delete [] latex_outfile;

    int gsize = grps.size();
    for (int proc_id = 0; proc_id < min(num_procs, gsize); ++proc_id) {
      string filename = basename + "-" + to_string(proc_id) + ".txt";

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
	string latexname = basename + "-" + to_string(proc_id) + ".tex";
	ifstream in(latexname);

	while (getline(in, line))
	  latexfile << line << endl;
	latexfile << endl;
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
  }
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
