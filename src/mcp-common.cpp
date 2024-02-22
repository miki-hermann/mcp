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
 *	Version: common to all                                            *
 *      File:    mcp-common.cpp                                           *
 *                                                                        *
 *      Copyright (c) 2019 - 2024                                         *
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
// #include <set>
#include <unordered_set>
#include <algorithm>
#include <sstream>
#include <random>
#include <memory>
#include "mcp-matrix+formula.hpp"
#include "mcp-common.hpp"
#include <string>

using namespace std;

// string version = GLOBAL_VERSION;
bool debug = false;
// string varid = "x";
// bool varswitch = false;
// vector<string> varnames;

map<Row, int> pred;		// predecessor function for Zanuttini's algorithm
map<Row, int> succ;		// successor function for Zanuttini's algorithm
map<Row, vector<int>> sim;	// sim table for Zanuttini's algorithm

// const int SENTINEL     = -1;
const string STDIN     = "STDIN";
const string STDOUT    = "STDOUT";
// const int MTXLIMIT     = 4000;
const int CLUSTERLIMIT = 15;

// Action action       = aALL;
Closure closure     = clHORN;
Cooking cooking     = ckWELLDONE;
Direction direction = dBEGIN;
// Print print         = pVOID;
bool setcover       = true;
Strategy strategy   = sLARGE;
// Display display     = yUNDEF;
string input        = STDIN;
string output       = STDOUT;
bool disjoint       = true;
// int arity           = 0;
int cluster         = SENTINEL;
// int offset          = 0;
string tpath        = "/tmp/";		// directory where the temporary files will be stored
bool np_fit	    = false;
unsigned chunkLIMIT      = 4096;	// heavily hardware dependent; must be optimized
string latex        = "";		// file to store latex output

ifstream infile;
ofstream outfile;
ofstream latexfile;
string formula_output;			// prefix of files, where formulas will be stored

const string action_strg[]    = {"One to One", "One to All Others", "One to All Others, Nosection",
                                 "Selected to All Others"};
const string closure_strg[]   = {"Horn",       "dual Horn",  "bijunctive", "affine", "CNF"};
const string cooking_strg[]   = {"raw",        "bleu",       "medium",     "well done"};
const string direction_strg[] = {"begin",      "end",        "optimum",    "random",
				 "low cardinality", "high cardinality"};
const string pcl_strg[]       = {"Horn",       "Horn",       "bijunctive", "affine", "cnf"};
const string strategy_strg[]  = {"large",      "exact"};
// const string print_strg[]     = {"void",       "clause",     "implication", "mixed",   "DIMACS"};
// const string display_strg[]   = {"undefined",  "hide",       "peek",        "section", "show"};
const string arch_strg[]      = {"seq",        "mpi",        "pthread",    "hybrid"};

//--------------------------------------------------------------------------------------------------

void read_arg (int argc, char *argv[]) {	// reads the input parameters
  int argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "--action"
	|| arg == "-a") {
      string act = argv[++argument];
      if (act == "one"
	  || act == "1") {
	action = aONE;
      } else if (act == "all"
		 || act == "a") {
	action =aALL;
      } else if (act == "nosection"
		 || act == "nosect"
		 || act == "nos"
		 || act == "no"
		 || act == "ns"
		 || act == "n" ) {
	action = aNOSECT;
      } else if (act == "selected"
		 || act == "select"
		 || act == "sel"
		 || act == "s") {
	if (argument < argc-1) {
	  action = aSELECTED;
	  selected = argv[++argument];
	} else
	  cerr << "+++ no group selected, revert to default" << endl;
      } else
	cerr << "+++ unknown action " << act << endl;
    } else if  (arg == "-d"
		|| arg == "--direction") {
      string dir = argv[++argument];
      if (dir == "begin"
	  || dir == "b") {
	direction = dBEGIN;
      } else if (dir == "end"
		 || dir == "e") {
	direction = dEND;
      } else if (dir == "lowcard"
		 || dir == "lcard"
		 || dir ==  "lc") {
	direction = dLOWCARD;
      } else if (dir == "highcard"
		 || dir == "hcard"
		 || dir ==  "hc") {
	direction = dHIGHCARD;
      } else if (dir == "random"
		 || dir == "rand"
		 || dir ==  "r") {
	direction =dRAND;
      // } else if (dir == "optimum"
      // 		 || dir == "optimal"
      // 		 || dir == "opt"
      // 		 || dir ==  "x") {
      // 	direction = dOPT;
      } else 
	cerr << "+++ unknown direction option " << dir << endl;
    } else if (arg == "--closure"
	       || arg == "-clo") {
      string cl = argv[++argument];
      if (cl == "horn"
	  || cl == "Horn"
	  || cl == "HORN"
	  || cl == "h") {
	closure = clHORN;
      } else if (cl == "dhorn"
		 || cl == "dHorn"
		 || cl == "dualHorn"
		 || cl == "dual-Horn"
		 || cl == "dual_Horn"
		 || cl == "dh") {
	closure = clDHORN;
      } else if (cl == "bij"
		 || cl == "bijunctive"
		 || cl == "b") {
	closure = clBIJUNCTIVE;
      } else if (cl == "general"
		 || cl == "gen"
		 || cl == "cnf"
		 || cl == "CNF") {
	closure = clCNF;
      } else
	cerr  << "+++ unknown closure option " << cl << endl;
    } else if (arg == "--cluster"
	       || arg == "--epsilon"
	       || arg == "-clu"
	       || arg == "-eps") {
      cluster = stoi(argv[++argument]);
    } else if (arg == "-pr"
	       || arg == "--print") {
      string prt = argv[++argument];
      if (prt == "clause"
	  || prt == "clausal"
	  || prt == "cl"
	  || prt == "c") {
	print = pCLAUSE;
      } else if (prt == "implication"
		 || prt == "impl"
		 || prt == "imp"
		 || prt == "im"
		 || prt == "i") {
	print = pIMPL;
      } else if (prt == "mix"
		 || prt == "mixed"
		 || prt == "m") {
	print = pMIX;
      } else if (prt == "dimacs"
		 || prt == "DIMACS") {
	print = pDIMACS;
      } else
	cerr <<  "+++ unknown print option " << prt << endl;
    } else if (arg == "-s"
	       || arg == "--strategy") {
      string strtgy = argv[++argument];
      if (strtgy == "e"
	  || strtgy == "ex"
	  || strtgy =="exact") {
	strategy = sEXACT;
      } else if  (strtgy == "l"
		  || strtgy == "lg"
		  || strtgy == "large") {
	strategy = sLARGE;
      } else
	cerr <<  "+++ unknown strategy option " << strtgy << endl;
    } else if (arg == "-sc"
	       || arg == "--setcover"
	       || arg == "--SetCover") {
      string sc = argv[++argument];
      if (sc == "y"
	  || sc == "Y"
	  || sc == "yes"
	  || sc == "YES") {
	setcover = true;
      } else if (sc == "n"
		 || sc == "N"
		 || sc == "no"
		 || sc == "NO") {
	setcover = false;
      } else
	cerr <<  "+++ unknown set cover option " << sc << endl;
    } else if ((arch == archMPI || arch == archHYBRID) &&
	       (arg == "-f"
		|| arg == "--fit")) {
      string fnp = argv[++argument];
      if (fnp == "y"
	  || fnp == "Y"
	  || fnp == "yes"
	  || fnp == "YES") {
	np_fit = true;
      } else if (fnp == "n"
		 || fnp == "N"
		 || fnp == "no"
		 || fnp == "NO") {
	np_fit = false;
      } else
	cerr <<  "+++ unknown fit number of processes option " << fnp << endl;
    } else if (arg == "-ck"
	       || arg == "--cook"
	       || arg == "--cooking") {
      string ck = argv[++argument];
      if (ck == "r"
	  || ck == "raw") {
	cooking = ckRAW;
      } else if (ck == "b"
		 || ck == "bl"
		 || ck == "bleu") {
	cooking = ckBLEU;
      } else if (ck == "m"
		 || ck == "med"
		 || ck == "medium") {
	cooking = ckMEDIUM;
      } else if (ck == "wd"
		 || ck == "done"
		 || ck == "well"
		 || ck == "well_done"
		 || ck == "welldone"
		 || ck == "all") {
	cooking = ckWELLDONE;
      } else
	cerr << "+++ unknown cooking option " << ck << endl;
    } else if (arg == "--input"
	       || arg == "-i") {
      input = argv[++argument];
    } else if (arg == "--output"
	       || arg == "-o") {
      output = argv[++argument];
    } else if (arg == "--formula"
	       || arg == "--logic"
	       || arg == "-l") {
      formula_output = argv[++argument];
    } else if (arg == "--matrix"
	       || arg == "--mtx"
	       || arg == "-m") {
      string mtx = argv[++argument];
      if (mtx == "yes"
	  || mtx == "y"
	  || mtx == "show") {
	display = ySHOW;
      } else if (mtx == "peek") {
	display = yPEEK;
      } else if (mtx == "section") {
	display = ySECTION;
      } else if (mtx == "no"
		 || mtx == "n"
		 || mtx == "hide") {
	display = yHIDE;
      } else if (mtx == "undefined"
		 || mtx == "undef"
		 || mtx == "u") {
	display = yUNDEF;
      } else
	cerr << "+++ unknown matrix print option " << mtx << endl;
    } else if (arg == "--offset"
	       || arg == "-of"
	       || arg == "--shift"
	       || arg == "-sh") {
      offset = stoi(argv[++argument]);
    } else if (arch > archMPI &&
	       (arg == "--chunk"
		|| arg == "-ch")) {
      chunkLIMIT = stoi(argv[++argument]);
    } else if (arch != archSEQ &&
	       (arg == "--tpath"
		|| arg == "-tp")) {
      tpath = argv[++argument];
    } else if (arg == "--latex") {
      latex = argv[++argument];
    } else if (arg == "--debug") {
      debug = true;
    } else
      cerr <<  "+++ unknown option " << arg << endl;
    ++argument;
  }
}

Matrix transpose (Matrix &batch) {
  Matrix tr_batch;

  for (int column = 0; column < batch[0].size(); ++column) {
    Row temp;
    for (int line = 0; line < batch.size(); ++line)
      temp.push_back(batch[line][column]);
    tr_batch.push_back(temp);
  }
  return tr_batch;
}

int hamming_distance (const Row &u, const Row &v) {
  // Hamming distance between two tuples
  if (u.size() != v.size())
    return SENTINEL;

  int sum = 0;
  for (int i = 0; i < u.size(); ++i)
    sum += abs(u[i] - v[i]);
  return sum;
}

// This overloading is necessay because deque implements >= differently
bool operator>= (const Row &a, const Row &b) {
  // overloading >=
  // is row a >= row b?
  if (a.size() != b.size())
    throw;

  for (int i = 0; i < a.size(); ++i) {
    if (a[i] < b[i]) return false;
  }
  return true;
}

Row Min (const Row &a, const Row &b) {
  // computes the minimum (intersection) of two tuples by coordinates
  Row c;
  for (int i = 0; i < a.size(); ++i)
    c.push_back(min(a[i],b[i]));
  return  c;
}

Row MIN (const Matrix &M) {
  // computes the minimal tuple of the whole matrix
  Row m(M[0].size());
  m.set();
  for (Row row : M)
    m = Min(m, row);
  return m;
}

bool InHornClosure (const Row &row, const Matrix &M) {
  // is the tuple row in the Horn closure of matrix M?
  unique_ptr<Row> P = ObsGeq(row, M);
  if (P == nullptr) return false;
  else return row == *P;
}

bool SHCPsolvable (const Matrix &T, const Matrix &F) {
  // is the intersection of F and of the Horn closure of T empty?
  // T = MinimizeObs(T);	// Optional, may not be worth the effort
  for (Row f : F) 
    if (InHornClosure (f, T)) return false;
  return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool majority (const bool &a, const bool &b, const bool &c) {
  // majority of 3 boolean values
  int cnt[] = {0, 0};
  cnt[a]++;
  cnt[b]++;
  cnt[c]++;
  return (cnt[false] >= cnt[true]) ? false : true;
}

Row Majority (const Row &a, const Row &b, const Row &c) {
  // majority of 3 tuples coordinate wise
  Row maj;
  for (int i = 0; i < a.size(); ++i)
    maj.push_back(majority(a[i], b[i], c[i]));
  return maj;
}

bool isect_nonempty (const Matrix &T, const Matrix &F) {
  // is the intersection of T and F nonempty?
  unordered_set<Row> orig;

  orig.insert(T.cbegin(), T.cend());
  for (Row row : F)
    if (orig.find(row) != orig.end())
      return true;
  return false;
}

bool inadmissible (const Matrix &T, const Matrix &F) {
  if (closure < clBIJUNCTIVE)
    return ! SHCPsolvable(T,F);
  else
    return isect_nonempty(T,F);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Matrix section (const Row &alpha, const Matrix &A) {
  // section of matrix A to columns determined by bitvector alpha
  Matrix B;
  for (Row aa : A) {
    Row bb;
    for (int i = 0; i < alpha.size(); ++i)
      if (alpha[i])
	bb.push_back(aa[i]);
    B.push_back(bb);
  }
  return B;
}

int hamming_weight (const Row &row) {	// Hamming weight of a tuple
  // int sum = accumulate(cbegin(row), cend(row), 0);
  // int sum = row.count();
  return row.count();
}

Matrix join (const Matrix &head, const Matrix &tail) {
  // join two matrices: join rows
  if (tail.empty())
    return head;
  if (head.empty())
    return tail;

  Matrix join;  
  for (int i = 0; i < head.size(); ++i) {
    Row row = head[i];
    // auto row_it = back_inserter(row);
    // copy(tail[i].begin(), tail[i].end(), row_it);
    for (int j = 0; j < tail[i].size(); ++j)
      row.push_back(tail[i][j]);
    join.push_back(row);
  }
  return join;
}

Row minsect (const Matrix &T, const Matrix &F) {
  // computes the minimal section for Horn or dual Horn closures
  const int lngt  = T[0].size();
  const int Tsize = T.size();
  const int Fsize = F.size();

  if (inadmissible(T,F)) {
    disjoint = false;
    Row emptyrow(lngt, false);
    return emptyrow;
  }

  Row A(lngt, true);
  if (direction == dBEGIN) {
    // from right to left; favors variables on the left
    Matrix Thead = T, Fhead = F, Ttail, Ftail;
    for (int i = lngt-1; i >= 0; --i) {
      A[i] = false;
      Row Tcolumn, Fcolumn;
      // if (!Thead.empty())
      for (int j = 0; j < Tsize; ++j) {
	// Tcolumn.push_back(Thead[j].back());
	Tcolumn.push_back(back(Thead[j]));
	Thead[j].pop_back();
      }
      // if (!Fhead.empty())
      for (int j = 0; j < Fsize; ++j) {
	Fcolumn.push_back(back(Fhead[j]));
	Fhead[j].pop_back();
      }
      Matrix Ta = join(Thead, Ttail);
      Matrix Fa = join(Fhead, Ftail);
      if (inadmissible(Ta,Fa)) {
	A[i] = true;
	if (Ttail.empty())
	  for (int j = 0; j < Tsize; ++j) {
	    Row row(1, Tcolumn[j]);
	    Ttail.push_back(row);
	  }
	else
	  for (int j = 0; j < Tsize; ++j)
	    // Ttail[j].push_front(Tcolumn[j]);
	    push_front(Ttail[j], Tcolumn[j]);
	if (Ftail.empty())
	  for (int j = 0; j < Fsize; ++j) {
	    Row row(1, Fcolumn[j]);
	    // row.push_back(Fcolumn[j]);
	    Ftail.push_back(row);
	  }
	else
	  for (int j = 0; j < Fsize; ++j)
	    // Ftail[j].push_front(Fcolumn[j]);
	    push_front(Ftail[j], Fcolumn[j]);
      }
      // for (int i = lngt-1; i >= 0; --i) {
      //   A[i] = false;
      //   Matrix Ta = section(A, T);
      //   Matrix Fa = section(A, F);
      //   if (inadmissible(Ta,Fa)) A[i] = true;
    }
  } else if (direction == dEND) {
    // from left to right; favors variables on the right
    Matrix Thead, Fhead, Ttail = T, Ftail = F;
    for (int i = 0; i < lngt; ++i) {
      A[i] = false;
      Row Tcolumn, Fcolumn;
      // if (!Ttail.empty())
      for (int j = 0; j < Tsize; ++j) {
	// Tcolumn.push_back(Ttail[j].front());
	Tcolumn.push_back(front(Ttail[j]));
	pop_front(Ttail[j]);
      }
      // if (!Ftail.empty())
      for (int j = 0; j < Fsize; ++j) {
	// Fcolumn.push_back(front(Ftail[j]));
	Fcolumn.push_back(front(Ftail[j]));
	pop_front(Ftail[j]);
      }
      Matrix Ta = join(Thead, Ttail);
      Matrix Fa = join(Fhead, Ftail);
      if (inadmissible(Ta,Fa)) {
	A[i] = true;
	if (Thead.empty())
	  for (int j = 0; j < Tsize; ++j) {
	    Row row(1, Tcolumn[j]);
	    // row.push_back(Tcolumn[j]);
	    Thead.push_back(row);
	  }
	else
	  for (int j = 0; j < Tsize; ++j)
	    Thead[j].push_back(Tcolumn[j]);
	if (Fhead.empty())
	  for (int j = 0; j < Fsize; ++j) {
	    Row row(1, Fcolumn[j]);
	    // row.push_back(Fcolumn[j]);
	    Fhead.push_back(row);
	  }
	else
	  for (int j = 0; j < Fsize; ++j)
	    Fhead[j].push_back(Fcolumn[j]);
      }
      // for (int i = 0; i < lngt; ++i) {
      //   A[i] = false;
      //   Matrix Ta = section(A,T);
      //   Matrix Fa = section(A,F);
      //   if (inadmissible(Ta,Fa)) A[i] = true;
    }
  // } else if (direction == dOPT) {	// optimum = exponential
  //   // search for the smallest number of selected columns
  //   cerr << "+++ Optimal direction takes forever" << endl;
  //   cerr << "+++ Therefore it has been suspended" << endl;
  //   exit(99);

  //   // Matrix Q;
  //   // int mincard = lngt;
  //   // Row minsct(lngt,false);
  //   // Q.push_back(minsct);
  //   // int counter = 0;
  //   // while (! Q.empty()) {
  //   //   if (++counter % 10000 == 0)
  //   // 	cerr << "*** loop " << counter << ", \t|queue| = " << Q.size() << endl;
  //   //   A = Q.front();
  //   //   Q.pop_front();
  //   //   Matrix Ta = section(A,T);
  //   //   Matrix Fa = section(A,F);
  //   //   if (inadmissible(Ta,Fa)) {
  //   // 	for (int i = 0; i < lngt; ++i)
  //   // 	  if (A[i] == false) {
  //   // 	    A[i] = true;
  //   // 	    Q.push_back(A);
  //   // 	    A[i] = false;
  //   // 	  }
  //   //   } else {
  //   // 	int hw = hamming_weight(A);
  //   // 	if (hw < mincard) {
  //   // 	  mincard = hw;
  //   // 	  minsct = A;
  //   // 	}
  //   //   }
  //   // }
  //   // A = minsct;
  } else if (direction == dRAND) {	// random order
    random_device rd;
    static uniform_int_distribution<int> uni_dist(0,lngt-1);
    static default_random_engine dre(rd());

    vector<int> coord;
    for (int i = 0; i < lngt; ++i)
      coord.push_back(i);
    shuffle(coord.begin(), coord.end(), dre);
    
    Matrix Thead, Fhead, Ttail, Ftail;
    for (Row t : T) {
      Row row;
      for (int j = 0; j < lngt; ++j)
	row.push_back(t[coord[j]]);
      Ttail.push_back(row);
    }
    for (Row f : F) {
      Row row;
      for (int j = 0; j < lngt; ++j)
	row.push_back(f[coord[j]]);
      Ftail.push_back(row);
    }
    for (int i = 0; i < lngt; ++i) {
      A[coord[i]] = false;
      Row Tcolumn, Fcolumn;
      // if (!Ttail.empty())
      for (int j = 0; j < Tsize; ++j) {
	Tcolumn.push_back(front(Ttail[j]));
	pop_front(Ttail[j]);
      }
      // if (!Ftail.empty())
      for (int j = 0; j < Fsize; ++j) {
	Fcolumn.push_back(front(Ftail[j]));
	pop_front(Ftail[j]);
      }
      Matrix Ta = join(Thead, Ttail);
      Matrix Fa = join(Fhead, Ftail);
      if (inadmissible(Ta,Fa)) {
	A[coord[i]] = true;
	if (Thead.empty())
	  for (int j = 0; j < Tsize; ++j) {
	    Row row(1, Tcolumn[j]);
	    // row.push_back(Tcolumn[j]);
	    Thead.push_back(row);
	  }
	else
	  for (int j = 0; j < Tsize; ++j)
	    Thead[j].push_back(Tcolumn[j]);
	if (Fhead.empty())
	  for (int j = 0; j < Fsize; ++j) {
	    Row row(1, Fcolumn[j]);
	    // row.push_back(Fcolumn[j]);
	    Fhead.push_back(row);
	  }
	else
	  for (int j = 0; j < Fsize; ++j)
	    Fhead[j].push_back(Fcolumn[j]);
      }
    }
    // for (int i = 0; i < A.size(); ++i) {
    //   A[coord[i]] = false;
    //   Matrix Ta = section(A,T);
    //   Matrix Fa = section(A,F);
    //   if (inadmissible(Ta,Fa)) A[coord[i]] = true;
    // }
  } else if (direction == dLOWCARD || direction == dHIGHCARD) {
    // cardinality preference
    int card[lngt] = {};
    vector<int> indicator(lngt, 0);
    for (Row t : T)
      for (int i = 0; i < t.size(); ++i) {
	card[i] += t[i];
	indicator[i] += t[i];
      }
    // for (Row f : F)
    //   for (int i = 0; i < f.size(); ++i) {
    // 	card[i] += f[i];
    // 	indicator[i] += f[i];
    //   }
    // for (Row f : F)
    //   for (int i = 0; i < f.size(); ++i) {
    // 	card[i] -= f[i];
    // 	indicator[i] -= f[i];
    //   }

    if (direction == dLOWCARD)
      sort(indicator.begin(), indicator.end(), greater<int>());
    else if (direction == dHIGHCARD)
      sort(indicator.begin(), indicator.end());

    int coord[lngt];
    for (int i = 0; i < lngt; ++i) {
      int j = 0;
      while (card[j] != indicator[i]) ++j;
      coord[i] = j;
      card[j] = SENTINEL;
    }

    Matrix Thead, Fhead, Ttail, Ftail;
    for (Row t : T) {
      Row row;
      for (int j = 0; j < lngt; ++j)
    	row.push_back(t[coord[j]]);
      Ttail.push_back(row);
    }
    for (Row f : F) {
      Row row;
      for (int j = 0; j < lngt; ++j)
    	row.push_back(f[coord[j]]);
      Ftail.push_back(row);
    }
    for (int i = 0; i < lngt; ++i) {
      A[coord[i]] = false;
      Row Tcolumn, Fcolumn;
      // if (!Ttail.empty())
      for (int j = 0; j < Tsize; ++j) {
    	Tcolumn.push_back(front(Ttail[j]));
    	pop_front(Ttail[j]);
      }
      // if (!Ftail.empty())
      for (int j = 0; j < Fsize; ++j) {
    	Fcolumn.push_back(front(Ftail[j]));
    	pop_front(Ftail[j]);
      }
      Matrix Ta = join(Thead, Ttail);
      Matrix Fa = join(Fhead, Ftail);
      if (inadmissible(Ta,Fa)) {
    	A[coord[i]] = true;
    	if (Thead.empty())
    	  for (int j = 0; j < Tsize; ++j) {
    	    Row row(1, Tcolumn[j]);
    	    // row.push_back(Tcolumn[j]);
    	    Thead.push_back(row);
    	  }
    	else
    	  for (int j = 0; j < Tsize; ++j)
    	    Thead[j].push_back(Tcolumn[j]);
    	if (Fhead.empty())
    	  for (int j = 0; j < Fsize; ++j) {
    	    Row row(1, Fcolumn[j]);
    	    // row.push_back(Fcolumn[j]);
    	    Fhead.push_back(row);
    	  }
    	else
    	  for (int j = 0; j < Fsize; ++j)
    	    Fhead[j].push_back(Fcolumn[j]);
      }
    }
    // for (int i = 0; i < A.size(); ++i) {
    //   A[coord[i]] = false;
    //   Matrix Ta = section(A,T);
    //   Matrix Fa = section(A,F);
    //   if (inadmissible(Ta,Fa)) A[coord[i]] = true;
    // }
  }
  return A;
}

void w_f (const string &filename, const string suffix,
	  const vector<int> &names, const Formula &formula) {
  // open the file and write the formula in it
  ofstream formfile;
  formfile.open(filename);
  if (! formfile.is_open()) {
    cerr << "+++ Cannot open formula output file " << filename << endl;
    cerr << "+++ Formula not written" << endl;
  } else {
    formfile << suffix
	     << " " << arity
	     << " " << formula[0].size()
	     << " " << offset  << endl;
    int old_offset = offset;
    offset = 1;
    for (int n : names)
      formfile << " " << n+offset;
    formfile << endl;
    formfile << formula2dimacs(names, formula) << endl;
    offset = old_offset;
    formfile.close();
  }
}

void write_formula (const string &suffix1, const string &suffix2,
		    const vector<int> &names, const Formula &formula) {
  // write formula to a file in DIMACS format
  // offset begins at 1, if not set otherwise
  w_f(formula_output + "_" + suffix1 + "_" + suffix2 + ".log",
      suffix1, names, formula);
}

void write_formula (const string &suffix,
		    const vector<int> &names, const Formula &formula) {
  // write formula to a file in DIMACS format
  // offset begins at 1, if not set otherwise
  w_f(formula_output + "_" + suffix + ".log",
      suffix, names, formula);
}

bool satisfied_by (const Clause &clause, const Matrix &T) {
  // is the clause satified by all tuples in T?
  for (Row t : T) {
    bool satisfied = false;
    for (int i = 0; i < clause.size(); ++i) {
      if (clause[i] == lpos && t[i] == true
	  ||
	  clause[i] == lneg && t[i] == false) {
	satisfied = true;
	break;
      }
    }
    if (!satisfied)
      return false;
  }
  return true;
}

int numlit (const Clause &clause) {
  // number of literals in a clause
  int i=0;
  for (Literal lit : clause)
    if (lit != lnone) ++i;
  return i;
}

int firstlit (const Clause &clause) {
  // coordinate of first literal
  int i=0;
  while (i < clause.size() && clause[i] == lnone) i++;
  return i;
}

bool clauseLT (const Clause &a, const Clause &b) {
  // is clause a < clause b in literals ?
  for (int i = 0; i < a.size(); ++i)
    if (a[i] < b[i])
      return true;
  return false;
}

// This overloading is necessay because deque implements >= differently
bool operator>= (const Clause &a, const Clause &b) {
  // overloading >=
  // is clause a >= clause b?
  // order on clauses:
  // 1. number of literals
  // 2. coordinate of the first literal
  // 3. order on positive / negative literals
  if (a == b)
    return true;
  if (numlit(a) < numlit(b))
    return false;
  if (numlit(a) == numlit(b)
      && firstlit(a) < firstlit(b))
    return false;
  if (numlit(a) == numlit(b)
      && firstlit(a) == firstlit(b))
    return clauseLT(b, a);
  return true;
}

// struct {
//   bool operator() (const Clause &a, const Clause &b) {
//     return numlit(a) < numlit(b);
//   }
// } cmp_numlit;

// bool operator< (const Clause &a, const Clause &b) {
//   if (a.length() != b.length())
//     throw;
//   for (int i = 0; i < a.length(); ++i)
//     if (a[i] < b[i])
//       return true;
//   return false;
// }

// struct {
//   bool operator() (const Clause &a, const Clause &b) {
//     return numlit(a) < numlit(b)
//       || firstlit(a) < firstlit(b)
//       || a < b;
//   }
// } cmp_clause;

// partition matrix or formula
template <typename U, template<typename> typename T>
int partition_mf (T<U> &t, int low, int high)
{
  U pivot = t[high];
  int p_index = low;
    
  for (int i = low; i < high; i++)
    if (t[i] <= pivot)
      swap(t[i], t[p_index++]);
  swap(t[high], t[p_index]);
    
  return p_index;
}

// sort matrix or formula
template <typename T>
void sort_mf (T &t, int low, int high) {
  if (low < high) {
    int p_index = partition_mf(t, low, high);
    sort_mf(t, low, p_index-1);
    sort_mf(t, p_index+1, high);
  }
}

Matrix restrict (const Row &sect, const Matrix &A) {
  // restricts matrix A to columns determined by the bitvector sect
  Matrix AA = section(sect, A);
  // sort(AA.begin(), AA.end());
  sort_mf(AA, 0, AA.size()-1);
  auto ip = unique(AA.begin(), AA.end());
  AA.resize(distance(AA.begin(), ip));
  return AA;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// computes the Horn closure of a matrix
// UNUSED
Matrix HornClosure (const Matrix &M) {	
  Matrix newM = M;
  Matrix HC;

  map<Row, int> pointer;
  for (Row m : M)
    pointer[m] = 0;

  while (! newM.empty()) {
    for (Row nm : newM)
      HC.push_back(nm);
    newM.clear();
    for (Row m : M)
      while (pointer[m] < HC.size()) {
	Row c = Min(m, HC[pointer[m]]);
	pointer[m]++;
	if (pointer.count(c) == 0) {
	  pointer[c] = 0;
	  newM.push_back(c);
	}
      }
  }

  // sort(HC.begin(), HC.end());
  sort_mf(HC, 0, HC.size()-1);
  return HC;
}

// computes the minimal tuple of a matrix coordinate wise
Row minHorn (const Matrix &M) {
  Row mh = M[0];
  for (int i = 1; i < M.size(); ++i)
    mh = Min(mh,M[i]);
  return mh;
}

// unit resolution
Formula unitres (const Formula &formula) {	
  Formula units;
  Formula clauses;

  for (Clause cl : formula)
    if (numlit(cl) == 1)
      units.push_back(cl);
    else
      clauses.push_back(cl);

  Formula resUnits = units;
  while (!units.empty() && !clauses.empty()) {
    Clause unit = units.front();
    units.pop_front();
    int index = 0;
    while (unit[index] == lnone) index++;
    for (int j = 0; j < clauses.size(); j++) {
      Clause clause = clauses[j];
      if (unit[index] == lpos && clause[index] == lneg
	  ||
	  unit[index] == lneg && clause[index] == lpos)
	clauses[j][index] = lnone;
    }

    auto it = clauses.begin();
    while (it != clauses.end())
      if (numlit(*it) == 1) {
	units.push_back(*it);
	resUnits.push_back(*it);
	it = clauses.erase(it);
      } else
	++it;
  }

  // sort(resUnits.begin(), resUnits.end());
  sort_mf(resUnits, 0, resUnits.size()-1);
  auto last1 = unique(resUnits.begin(), resUnits.end());
  resUnits.erase(last1, resUnits.end());

  // test if there are no two unit clauses having literals of opposite parity
  int ru_bound = resUnits.size();
  for (int i = 0; i < ru_bound-1; ++i) {
    Clause unit_i = resUnits[i];
    int index = 0;
    while (index < unit_i.size() && unit_i[index] == lnone)
      index++;
    if (index < ru_bound) {
      for (int j=i+1; j < ru_bound; ++j) {
	Clause unit_j = resUnits[j];
	if (unit_i[index] == lpos && unit_j[index] == lneg
	    ||
	    unit_i[index] == lneg && unit_j[index] == lpos) {
	  Clause emptyClause(unit_i.size(), lnone);
	  Formula emptyFormula;
	  emptyFormula.push_back(emptyClause);
	  return emptyFormula;
	}
      }
    }
  }

  clauses.insert(clauses.end(), resUnits.begin(), resUnits.end());
  // sort(clauses.begin(), clauses.end()), cmp_numlit;
  sort_mf(clauses, 0, clauses.size()-1);
  auto last2 = unique(clauses.begin(), clauses.end());
  clauses.erase(last2, clauses.end());

  return clauses;
}

Formula binres (const Formula &formula) {
  Formula bins;
  Formula clauses;

  for (Clause cl : formula)
    if (numlit(cl) == 2)
      bins.push_back(cl);
    else
      clauses.push_back(cl);

  Formula resBins = bins;
  while (!bins.empty() && !clauses.empty()) {
    Clause bincl = bins.front();
    bins.pop_front();
    int index1 = 0;
    while (bincl[index1] == lnone) index1++;
    int index2 = index1 + 1;
    while (bincl[index2] == lnone) index2++;
    for (int j = 0; j < clauses.size(); j++) {
      Clause clause = clauses[j];
      if (clause[index1] == bincl[index1]
	  &&
	  clause[index2] == -1 * bincl[index2])
	clauses[j][index2] = lnone;
      else if (clause[index1] == -1 * bincl[index1]
	  &&
	  clause[index2] == bincl[index2])
	clauses[j][index1] = lnone;
    }

    auto it = clauses.begin();
    while (it != clauses.end())
      if (numlit(*it) == 2) {
	bins.push_back(*it);
	resBins.push_back(*it);
	it = clauses.erase(it);
      } else
	++it;
  }
  
  clauses.insert(clauses.end(), resBins.begin(), resBins.end());
  // sort(clauses.begin(), clauses.end(), cmp_numlit);
  sort_mf(clauses, 0, clauses.size()-1);
  auto last3 = unique(clauses.begin(), clauses.end());
  clauses.erase(last3, clauses.end());

  return clauses;
}

bool subsumes (const Clause &cla, const Clause &clb) {
  // does clause cla subsume clause clb ?
  // cla must be smaller than clb
  for (int i=0; i < cla.size(); ++i)
    if (cla[i] != lnone && cla[i] != clb[i])
      return false;
  return true;
}

// perform subsumption on clauses of a formula
// clauses must be sorted by length --- IS GUARANTEED
Formula subsumption (Formula formula) {	
  Formula res;
  // sort_mf not necessary, clauses are GUARANTEED SORTED
  // sort(formula.begin(), formula.end(), cmp_numlit);
  // sort_mf(formula, 0, formula.size()-1);
  while (! formula.empty()) {
    Clause clause = formula.front();
    formula.pop_front();
    auto it = formula.begin();
    while (it != formula.end())
      if (subsumes(clause, *it))
	it = formula.erase(it);
      else
	++it;
    res.push_back(clause);
  }
  return res;
}

// is the clause empty ?
bool empty_clause (const Clause &clause) {	
  for (Literal lit : clause)
    if (lit != lnone)
      return false;
  return true;
}

// eliminating redundant clauses
// clauses must be sorted by length --- IS UARANTEED
Formula redundant (const Formula &formula) {	
  const int lngt = formula[0].size();

  Formula prefix, suffix;
  // prefix.insert(prefix.end(), formula.begin(), formula.end());
  prefix = formula;
  // sort_mf not necessary, clauses are GUARANTEED SORTED
  // sort(prefix.begin(), prefix.end(), cmp_numlit);
  // sort_mf(prefix, 0, prefix.size()-1);

  int left = 0;
  while (left < prefix.size() && numlit(prefix[left]) == 1)
    left++;

  while (left < prefix.size()) {
    Clause pivot = prefix.back();
    prefix.pop_back();
    Formula newUnits;
    for (int i = 0; i < pivot.size(); ++i) {
      if (pivot[i] != lnone) {
	Clause newclause(lngt, lnone);
	newclause[i] = (pivot[i] == lpos) ? lneg : lpos;
	newUnits.push_back(newclause);
      }
    }
    newUnits.insert(newUnits.end(), prefix.begin(), prefix.end());
    newUnits.insert(newUnits.end(), suffix.begin(), suffix.end());
    // sort(newUnits.begin(), newUnits.end(), cmp_numlit);
    sort_mf(newUnits, 0, newUnits.size()-1);
    auto last3 = unique(newUnits.begin(), newUnits.end());
    newUnits.erase(last3, newUnits.end());
    Formula bogus = unitres(newUnits);
    bool keep = true;
    for (Clause bgcl : bogus)
      if (empty_clause(bgcl)) {
	keep = false;
	break;
      }
    if (keep)
      suffix.push_front(pivot);
  }
  prefix.insert(prefix.end(), suffix.begin(), suffix.end());
  return prefix;
}

// perform set cover optimizing the clauses as subsets falsified by tuples as universe
// Universe = tuples in F
// SubSets  = clauses of a formula
Formula SetCover (const Matrix &Universe, const Formula &SubSets) {
  enum Presence {NONE = 0, ABSENT = 1, PRESENT = 2};
  typedef pair<Row, Clause> Falsification;	
  map<Falsification, Presence> incidence;	// Incidence matrix indicating
						// which tuple (row) falsifies which clause
  Matrix R;					// tuples still active == not yet falsified
  Formula selected;				// selected clauses for falsification

  for (Row tuple : Universe) {
    R.push_back(tuple);
    for (Clause clause : SubSets)
      incidence[make_pair(tuple, clause)]
	= sat_clause(tuple, clause) ? ABSENT : PRESENT;
  }

  // perform set cover
  while (!R.empty()) {
    map<Clause, int> intersect;	// How many tuples does the clause falsify (intersect)?
    for (Clause clause : SubSets)
      intersect[clause] = 0;
    for (Row tuple : R)
      for (Clause clause : SubSets)
	if (incidence[make_pair(tuple, clause)] == PRESENT)
	  ++intersect[clause];

    int maxw = 0;
    Clause maxset;
    for (Clause clause : SubSets)
      if (intersect[clause] > maxw) {
	maxw = intersect[clause];
	maxset = clause;
      }

    if (maxw == 0) break;
    selected.push_back(maxset);
    
    auto it = R.begin();
    while (it != R.end())
      if (incidence[make_pair(*it, maxset)] == PRESENT) {
	for (Clause clause : SubSets)
	  incidence[make_pair(*it, clause)] = ABSENT;
	it = R.erase(it);
      } else
	++it;
  }
  // sort(selected.begin(), selected.end(), cmp_numlit);
  sort_mf(selected, 0, selected.size()-1);
  return selected;
}

// perform redundancy elimination on formula according to cooking
void cook (Formula &formula) {
  if (! formula.empty()) {
    if (cooking == ckRAW)      sort_mf(formula, 0, formula.size()-1);
      // sort(formula.begin(), formula.end(), cmp_numlit);
    if (cooking >= ckBLEU)     {formula = unitres(formula);
				formula = binres(formula);}
    if (cooking >= ckMEDIUM)   formula = subsumption(formula);
    if (cooking == ckWELLDONE) formula = redundant(formula);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// predecessor function of Zanuttini's algorithm
// R must be lexicographically sorted
void predecessor (const Matrix &R) {
  pred.clear();
  pred[R[0]] = SENTINEL;
  Row p = R[0];
  for (int i = 1; i < R.size(); ++i) {
    Row m = R[i];
    int j = 0;
    for (int k = 0; k < m.size(); ++k)
      if (m[k] == p[k])
	j++;
      else
	break;
    pred[R[i]] = j;
    p = m;
  }
}

// sucessor function of Zanuttini's algorithm
// R must be lexicographically sorted
void successor (const Matrix &R) {
  succ.clear();
  Row m = R[0];
  for (int i = 1; i < R.size(); ++i) {
    Row s = R[i];
    int j = 0;
    for (int k = 0; k < m.size(); ++k)
      if (m[k] == s[k])
	j++;
      else
	break;
    succ[R[i-1]] = j;
    m = s;
  }
  succ[R[R.size()-1]] = SENTINEL;
}

void simsim (const Matrix &R) {	// sim array of Zanuttini's algorithm
  sim.clear();
  for (Row mm : R) {
    const vector<int> dummy(mm.size(), SENTINEL);
    sim[mm] = dummy;
    for (Row m1m : R) {
      if (mm == m1m) continue;
      int j0 = 0;
      while (mm[j0] == m1m[j0]) ++j0;
      int j = j0;
      while (j < m1m.size() &&
	     (m1m[j] == true
	      || mm[j] == false)) {
	if (j > succ[mm]
	    && mm[j] == false
	    && m1m[j] == true)
	  sim[mm][j] = max(sim[mm][j], j0);
	j++;
      }
    }
  }
}

// generate clauses with Zanuttini's algorithm
Clause hext (const Row &m, const int &j) {
  Clause clause(m.size(), lnone);
  for (int i = 0; i < j; ++i)
    if (m[i] == true)
      clause[i] = lneg;
  if (j > pred[m]
      && m[j] == true)
    clause[j] = lpos;
  else if (j > succ[m]
	   && m[j] == false
	   && sim[m][j] == SENTINEL)
    clause[j] = lneg;
  else if (j > succ[m]
	   && m[j] == false
	   && sim[m][j] != SENTINEL) {
    clause[j] = lneg;
    clause[sim[m][j]] = lpos;
  }
  return clause;
}

// phi is a CNF formula and M is a set of tuples, such that sol(phi) = M
// constructs a reduced prime formula phiPrime, such that sol(phi) = sol(phiPrime)
Formula primality (const Formula &phi, const Matrix &M) {
  const int card = M.size();
  const int lngt = M[0].size();
  Formula phiPrime;

  auto last = make_unique<int []>(card);	// smart pointer
  // int *last = new int[card];			// hard pointer
  for (Clause clause : phi) {
    if (clause.size() != lngt) {
      cerr << "+++ Clause size and vector length do not match" << endl;
      exit(2);
    }
    for (int k = 0; k < card; ++k) {
      last[k] = SENTINEL;
      for (int j = 0; j < clause.size(); ++j)
	if (M[k][j] == true && clause[j] == lpos
	    ||
	    M[k][j] == false && clause[j] == lneg)
	  last[k] = j;
    }
    Clause cPrime(clause.size(), lnone);
    for (int j = 0; j < clause.size(); ++j) {
      bool d;
      if (clause[j] == lpos) {
	d = true;
	for (int k = 0; k < card; ++k)
	  if (last[k] == j)
	    d = d && M[k][j];
	if (d == true)
	  cPrime[j] = lpos;
      } else if (clause[j] == lneg) {
	d = false;
	for (int k = 0; k < card; ++k)
	  if (last[k] == j)
	    d = d || M[k][j];
	if (d == false)
	  cPrime[j] = lneg;
      }
      if (cPrime[j] != lnone)
	for (int k = 0; k < card; ++k)
	  for (int i = 0; i < lngt; ++i)
	    if (M[k][i] == true && cPrime[j] == lpos
		||
		M[k][i] == false && cPrime[j] == lneg) {
	      last[k] = SENTINEL;
	      break;
	    }
    }
    phiPrime.push_back(cPrime);
  }
  // delete [] last;	// only for hard pointer, comment out if use smart pointer
  return phiPrime;
}

// learn the exact Horn clause from positive examples T
// uses Zanuttini's algorithm
Formula learnHornExact (Matrix T) {
  Formula H;
  const int lngt = T[0].size();
  if (T.size() == 1) {		// T has only one row / tuple
    Row t = T[0];
    for (int i = 0; i < lngt; ++i) {
      Clause clause(lngt, lnone);
      clause[i] = t[i] == true ? lpos : lneg;
      H.push_back(clause);
    }
    return H;
  }
  // sort(T.begin(), T.end());
  sort_mf(T, 0, T.size()-1);
  successor(T);
  predecessor(T);
  simsim(T);

  for (Row m : T)
    for (int j = 0; j < lngt; ++j)
      if (j > pred[m] && m[j] == true
	  || j > succ[m] && m[j] == false)
	H.push_back(hext(m,j));

  H = primality(H, T);
  cook(H);
  return H;
}

// learn general CNF formula with large strategy
// from negative examples F
Formula learnCNFlarge (const Matrix &F) {
  Formula formula;
  for (Row row : F) {
    Clause clause;
    // for (bool bit : row)
    for (int i = 0; i < row.size(); ++i)
      clause.push_back(row[i] == false ? lpos : lneg);
    formula.push_back(clause);
  }
  cook(formula);
  return formula;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int fork (const Row &m1, const Row &m2) {
  int i = 0;
    // we can guarantee that always m1 != m2, therefore
    // we can drop i < m1.size()
  while (m1[i] == m2[i])
    ++i;
  return i;
}

deque<int> fork (const Matrix &M) {
  deque<int> frk(M.size()-1, 0);
  for (int ell = 0; ell < M.size()-1; ++ell)
    frk[ell] = fork(M[ell], M[ell+1]);
  frk.push_front(SENTINEL);
  frk.push_back(SENTINEL);
  return frk;
}

Clause negTerm (const Row &m, const int &i) {
  Clause clause(m.size(), lnone);
  for (int j = 0; j < i; ++j)
    clause[j] = m[j] == false ? lpos : lneg;
  return clause;
}

Clause negLeft (const Row &m, const int &i) {
  Clause clause = negTerm(m, i);
  clause[i] = lpos;		// negLT
  return clause;
}

Clause negRight (const Row &m, const int &i) {
  Clause clause = negTerm(m, i);
  clause[i] = lneg;		// negGT
  return clause;
}

// learn general CNF formula with exact strategy
// from positive examples T
Formula learnCNFexact (Matrix T) {
  // sort(T.begin(), T.end());
  sort_mf(T, 0, T.size()-1);
  auto ip = unique(T.begin(), T.end());
  T.resize(distance(T.begin(), ip));

  deque<int> frk = fork(T);		// frk.size() == T.size()+1
  Formula formula;
  for (int ell = 0; ell < T.size(); ++ell) {
    for (int i = frk[ell]+1; i < arity; ++i)
      if (T[ell][i] == true)
	formula.push_back(negLeft(T[ell], i));
    for (int i = frk[ell+1]+1; i < arity; ++i)
      if (T[ell][i] == false)
	formula.push_back(negRight(T[ell], i));
  }
  formula = primality(formula, T);
  cook(formula);
  return formula;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// swap the polarity of values in a tuple
Row polswap_row (const Row &row) {
  Row swapped = ~row;
  // for (bool bit : row)
  //   swapped.push_back(! bit);
  return swapped;
}

// swap polarity of every tuple in a matrix
Matrix polswap_matrix (const Matrix &A) {
  Matrix swapped;
  for (Row row : A)
    swapped.push_back(polswap_row(row));
  return swapped;
}

// swap polarity of literals in a clause
Clause polswap_clause (const Clause &clause) {
  Clause swapped;
  for (Literal literal : clause)
    if (literal != lnone)
      // swapped.push_back(literal == lpos ? lneg : lpos);
      swapped += literal == lpos ? lneg : lpos;
    else
      swapped += lnone;
  return swapped;
}

// swap polarity of every clause of a formula
Formula polswap_formula (const Formula &formula) {
  Formula swapped;
  for (Clause clause : formula)
    swapped.push_back(polswap_clause(clause));
  return swapped;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

string time2string (int seconds) {
  enum TimeUnit {second = 0, minute = 1, hour = 2, day = 3};
  const string tu_name[] = {" second(s) ", " minute(s) ", " hour(s) ", " day(s) "};
  const int timevalue[] = {60, 60, 24};

  int timeunit[4] = {0, 0, 0, 0};

  if (seconds == 0)
    return "0 seconds";

  for (int t = second; t < day; t++) {
    timeunit[t] = seconds % timevalue[t];
    seconds /= timevalue[t];
  }
  if (seconds > 0)
    timeunit[day] = seconds;

  string output;
  for (int t = day; t >= second; t--) {
    if (timeunit[t] > 0)
      output += to_string(timeunit[t]) + tu_name[t];
  }
  return output;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
