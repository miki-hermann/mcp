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
 *	Version: common to all                                            *
 *      File:    mcp-common.cpp                                           *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Given several  Boolean matrices  representing the  presence/absence of *
 * attributes in  observations, this software generates  Horn, dual Horn, *
 * or bijunctive formulas from positive and negative examples represented *
 * by these matrices.                                                     *
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

bool debug = false;

map<size_t, int> idx2w;		// coordinate index to weight for precedence dir
struct cmp_prec { 
  bool operator() (const size_t &idx1, const size_t &idx2) {
    return idx2w.at(idx1) < idx2w.at(idx2);
    // return idx2w[idx1] < idx2w[idx2];
  }
};

const string STDIN     = "STDIN";
const string STDOUT    = "STDOUT";
const int CLUSTERLIMIT = 15;

Closure closure     = clHORN;
Cooking cooking     = ckWELLDONE;
Direction direction = dBEGIN;
bool setcover       = true;
Strategy strategy   = sLARGE;
string input        = STDIN;
string output       = STDOUT;
string headerput    = "";
string weights      = "";
bool disjoint       = true;
int cluster         = SENTINEL;
string tpath        = "/tmp/";		// directory where the temporary files will be stored
bool np_fit	    = false;
unsigned chunkLIMIT      = 4096;	// heavily hardware dependent; must be optimized
string latex        = "";		// file to store latex output

ifstream infile;
ifstream headerfile;
ifstream precfile;			// file with attribute precedence
ofstream outfile;
ofstream latexfile;
string formula_output;			// prefix of files, where formulas will be stored

const string action_strg[]    = {"One to One", "One to All Others",
				 // "One to All Others, Nosection",
                                 "Selected to All Others"};
const string closure_strg[]   = {"", "Horn",       "dual Horn",  "bijunctive", "affine", "CNF"};
const string cooking_strg[]   = {"", "raw",        "bleu",       "medium",     "well done"};
const string direction_strg[] = {"", "begin",      "end",        "optimum",    "random",
				 "low cardinality", "high cardinality", "precedence"};
const string pcl_strg[]       = {"Horn",       "Horn",       "bijunctive", "affine", "cnf"};
const string strategy_strg[]  = {"large",      "exact"};
const string arch_strg[]      = {"seq",        "mpi",        "pthread",    "hybrid"};

const unordered_map<string, Parameter> param_values = {
  {"", parERROR},
  {"-i", parINPUT},
  {"--input", parINPUT},
  {"-o", parOUTPUT},
  {"--output", parOUTPUT},
  {"--hdr", parHEADER},
  {"--header", parHEADER},
  {"-l", parFORMULA},
  {"--logic", parFORMULA},
  {"--formula", parFORMULA},
  {"--tpath", parTPATH},
  {"--latex", parLATEX},
  {"-w", parWEIGHTS},
  {"--weights", parWEIGHTS},
  //----
  {"-a", parACTION},
  {"--action", parACTION},
  {"--ns", parNOSECTION},
  {"--nosection", parNOSECTION},
  {"--no-section", parNOSECTION},
  {"--no_section", parNOSECTION},
  {"-d", parDIRECTION},
  {"--direction", parDIRECTION},
  {"-c", parCLOSURE},
  {"--closure", parCLOSURE},
  {"--pr", parPRINT},
  {"--print", parPRINT},
  {"-s", parSTRATEGY},
  {"--strategy", parSTRATEGY},
  {"--sc", parSETCOVER},
  {"--setcover", parSETCOVER},
  {"--SetCover", parSETCOVER},
  {"--ck", parCOOKING},
  {"--cook", parCOOKING},
  {"--cooking", parCOOKING},
  {"-m", parMATRIX},
  {"--mtx", parMATRIX},
  {"--matrix", parMATRIX},
  {"--offset", parOFFSET},
  {"--of", parOFFSET},
  {"--shift", parOFFSET},
  {"--sh", parOFFSET},
  {"--chunk", parCHUNK},
  {"--debug", parDEBUG}
};

//--------------------------------------------------------------------------------------------------

inline Parameter str2par (const string &str) {
  if (param_values.count(str) == 0)
    return parERROR;
  return param_values.at(str);
}

void read_arg (int argc, char *argv[]) {	// reads the input parameters
  size_t argument = 1;
  while (argument < argc) {
    const string arg = argv[argument];
    const Parameter parameter = str2par(arg);

    switch (parameter) {
    case parERROR:
      cerr <<  "+++ unknown option " << arg << endl;
      break;
    case parINPUT:
      if (argument < argc-1) {
	input = argv[++argument];
      } else
	cerr << "+++ no input file selected" << endl;
      break;
    case parOUTPUT:
      if (argument < argc-1) {
	output = argv[++argument];
      } else
	cerr << "+++ no output file selected, revert to default" << endl;
      break;
    case parHEADER:
      if (argument < argc-1) {
	headerput = argv[++argument];
      } else
	cerr << "+++ no header file selected, revert to default" << endl;
      break;
    case parFORMULA:
      if (argument < argc-1) {
	formula_output = argv[++argument];
      } else
	cerr << "+++ no formula file selected, revert to default" << endl;
      break;
    case parTPATH:
      if (argument < argc-1) {
	tpath = argv[++argument];
      } else
	cerr << "+++ no tmp path selected, revert to default" << endl;
      break;
    case parLATEX:
      if (argument < argc-1) {
	latex = argv[++argument];
      } else
	cerr << "+++ no latex file selected" << endl;
      break;
    case parWEIGHTS:
      if (argument < argc-1) {
	weights = argv[++argument];
      } else
	cerr << "+++ no weight selected" << endl;
      break;
    case parACTION:
      if (argument < argc-1) {
	const string act = argv[++argument];
	if (act == "one" || act == "1") {
	  action = aONE;
	} else if (act == "all" || act == "a") {
	  action = aALL;
	} else if (act == "selected" || act == "select") {
	  if (argument < argc-1) {
	    action = aSELECTED;
	    selected = argv[++argument];
	  } else
	    cerr << "+++ no group selected, revert to default" << endl;
	}
      } else
	cerr << "+++ no action selected, revert to default" << endl;
      break;
    case parNOSECTION:
      nosection = true;
      break;
    case parDIRECTION:
      if (argument < argc-1) {
	const string dir = argv[++argument];
	if (dir == "begin") {
	  direction = dBEGIN;
	} else if (dir == "end") {
	  direction = dEND;
	} else if (dir == "lowscore" || dir == "lscore") {
	  direction = dLOWSCORE;
	} else if (dir == "highscore" || dir == "hscore") {
	  direction = dHIGHSCORE;
	} else if (dir == "random" || dir == "rand") {
	  direction = dRAND;
	} else if (dir == "precedence" || dir == "prec") {
	  direction = dPREC;
	} else 
	  cerr << "+++ unknown direction option " << dir << endl;
      } else
	cerr << "+++ no direction selected, revert to default" << endl;
      break;
    case parCLOSURE:
      if (argument < argc-1) {
	const string cl = argv[++argument];
	if (cl == "horn" || cl == "Horn" || cl == "HORN") {
	  closure = clHORN;
	} else if (cl == "dhorn" || cl == "dHorn" || cl == "dualHorn"
		   || cl == "dual-Horn" || cl == "dual_Horn") {
	  closure = clDHORN;
	} else if (cl == "bij" || cl == "bijunctive"
		   || cl == "2sat" || cl == "2SAT") {
	  closure = clBIJUNCTIVE;
	} else if (cl == "general" || cl == "gen"
		   || cl == "cnf"  || cl == "CNF") {
	  closure = clCNF;
	} else
	  cerr  << "+++ unknown closure option " << cl << endl;
      } else
	cerr << "+++ no closure selected, revert to default" << endl;
      break;
    case parPRINT:
      if (argument < argc-1) {
	const string prt = argv[++argument];
	if (prt == "clause" || prt == "clausal") {
	  print_val = pCLAUSE;
	} else if (prt == "implication" || prt == "impl") {
	  print_val = pIMPL;
	} else if (prt == "mix" || prt == "mixed") {
	  print_val = pMIX;
	} else if (prt == "dimacs" || prt == "DIMACS") {
	  print_val = pDIMACS;
	} else
	  cerr <<  "+++ unknown print option " << prt << endl;
      } else
	cerr << "+++ no print selected, revert to default" << endl;
      break;
    case parSTRATEGY:
      if (argument < argc-1) {
	const string strtgy = argv[++argument];
	if (strtgy =="exact") {
	  strategy = sEXACT;
	} else if  (strtgy == "large") {
	  strategy = sLARGE;
	} else
	  cerr <<  "+++ unknown strategy option " << strtgy << endl;
      } else
	cerr << "+++ no strategy selected, revert to default" << endl;
      break;
    case parSETCOVER:
      if (argument < argc-1) {
	const string sc = argv[++argument];
	if (sc == "y" || sc == "Y" || sc == "yes" || sc == "YES") {
	  setcover = true;
	} else if (sc == "n" || sc == "N" || sc == "no" || sc == "NO") {
	  setcover = false;
	} else
	  cerr <<  "+++ unknown set cover option " << sc << endl;
      } else
	cerr << "+++ no set cover selected, revert to default" << endl;
      break;
    case parCOOKING:
      if (argument < argc-1) {
	const string ck = argv[++argument];
	if (ck == "raw") {
	  cooking = ckRAW;
	} else if (ck == "bleu") {
	  cooking = ckBLEU;
	} else if (ck == "med" || ck == "medium") {
	  cooking = ckMEDIUM;
	} else if (ck == "done" || ck == "well"
		   || ck == "well_done" || ck == "welldone"
		   || ck == "all") {
	  cooking = ckWELLDONE;
	} else
	  cerr << "+++ unknown cooking option " << ck << endl;
      } else
	cerr << "+++ no cooking selected, revert to default" << endl;
      break;
    case parMATRIX:
      if (argument < argc-1) {
	const string mtx = argv[++argument];
	if (mtx == "yes" || mtx == "y" || mtx == "show") {
	  display = ySHOW;
	} else if (mtx == "peek") {
	  display = yPEEK;
	} else if (mtx == "section") {
	  display = ySECTION;
	} else if (mtx == "no" ||mtx == "n" || mtx == "hide") {
	  display = yHIDE;
	} else if (mtx == "undefined" || mtx == "undef") {
	  display = yUNDEF;
	} else
	  cerr << "+++ unknown matrix print option " << mtx << endl;
      } else
	cerr << "+++ no matrix printing selected, revert to default" << endl;
      break;
    case parOFFSET:
      if (argument < argc-1) {
	try {
	  offset = stoul(argv[++argument]);
	} catch  (invalid_argument err) {
	  cerr << "+++ " << argv[argument]
	       << " is not a valid offset, revert to default"
	       << endl;
	}
      } else
	cerr << "+++ no offset selected, revert to default" << endl;
      break;
    case parCHUNK:
      if (argument < argc-1) {
	try {
	  chunkLIMIT = stoul(argv[++argument]);
	} catch  (invalid_argument err) {
	  cerr << "+++ " << argv[argument]
	       << " is not a valid chunk limit, revert to default"
	       << endl;
	}
      } else
	cerr << "+++ no chunk limit selected, revert to default" << endl;
      break;
    case parDEBUG:
      debug = true;
      break;
    default:
      cerr << "+++ read_arg: you should not be here" << endl;
    }
    ++argument;
  }
}

Matrix transpose (const Matrix &batch) {
  Matrix tr_batch;

  for (size_t column = 0; column < batch[0].size(); ++column) {
    Row temp(batch.size());
    for (size_t line = 0; line < batch.size(); ++line)
      // temp.push_back(batch[line][column]);
      temp[line] = batch[line][column];
    tr_batch.push_back(temp);
  }
  return tr_batch;
}

int hamming_distance (const Row &u, const Row &v) {
  // Hamming distance between two tuples
  if (u.size() != v.size())
    return SENTINEL;

  int sum = 0;
  for (size_t i = 0; i < u.size(); ++i)
    sum += abs(u[i] - v[i]);
  return sum;
}

//--------------------------------------------------------------------------------

string to_string (const Row &a) {
  string s;
  for (size_t i = 0; i < a.size(); ++i)
    s += to_string(a[i]);
  return s;
}

// comparing function for rows, used in sorting algorithm
bool cmp_row (const Row &a, const Row &b) {
  return to_string(a) < to_string(b);
}

// This overloading is necessary because deque implements >= differently
bool operator>= (const Row &a, const Row &b) {
  // overloading >=
  // is row a >= row b?
  if (a.size() != b.size())
    throw;

  for (size_t i = 0; i < a.size(); ++i) {
    if (a[i] < b[i]) return false;
  }
  return true;
}

//--------------------------------------------------------------------------------

// reads the row for from line
// mcp-seq and mcp-parallel, but NOT for mcp-check and mcp-predict
Row read_row (const string &line, string &group) {
  const vector<string> nums = split(line, " \t,");
  group = nums.at(0);
  Row temp(nums.size()-1);
  for (size_t i = 1; i < nums.size(); ++i) {
    assert(nums.at(i).length() == 1);
    const int number = nums.at(i).back() - '0';
    temp[i-1] = number == 0 ? false : true;
    // temp[temp.size() - i] = number == 0 ? false : true;
    // temp.push_back(number == 0 ? false : true);
  }
  return temp;
}

Row Min (const Row &a, const Row &b) {
  // computes the minimum (intersection) of two tuples by coordinates
  Row c(a.size());
  for (size_t i = 0; i < a.size(); ++i)
    // c.push_back(min(a[i],b[i]));
    c[i] = min(a[i],b[i]);
  return  c;
}

Row MIN (const Matrix &M) {
  // computes the minimal tuple of the whole matrix
  Row m(M[0].size());
  m.set();
  for (const Row &row : M)
    m = Min(m, row);
  return m;
}

bool InHornClosure (const Row &row, const Matrix &M) {
  // is the tuple row in the Horn closure of matrix M?
  unique_ptr<Row> P = ObsGeq(row, M);
  if (P == nullptr)
    return false;
  else
    return row == *P;
}

bool SHCPsolvable (const Matrix &T, const Matrix &F) {
  // is the intersection of F and of the Horn closure of T empty?
  // T = MinimizeObs(T);	// Optional, may not be worth the effort
  for (const Row &f : F) 
    if (InHornClosure (f, T))
      return false;
  return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool majority (const bool &a, const bool &b, const bool &c) {
  // majority of 3 boolean values
  size_t cnt[] = {0, 0};
  cnt[a]++;
  cnt[b]++;
  cnt[c]++;
  return (cnt[false] >= cnt[true]) ? false : true;
}

Row Majority (const Row &a, const Row &b, const Row &c) {
  // majority of 3 tuples coordinate wise
  Row maj(a.size());
  for (size_t i = 0; i < a.size(); ++i)
    // maj.push_back(majority(a[i], b[i], c[i]));
    maj[i] = majority(a[i], b[i], c[i]);
  return maj;
}

bool isect_nonempty (const Matrix &T, const Matrix &F) {
  // is the intersection of T and F nonempty?
  unordered_set<Row> orig;

  orig.insert(T.cbegin(), T.cend());
  for (const Row &row : F)
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
  for (const Row &aa : A) {
    Row bb;
    for (size_t i = 0; i < alpha.size(); ++i)
      if (alpha[i])
	bb.push_back(aa[i]);
    B.push_back(bb);
  }
  return B;
}

// Hamming weight of a tuple
size_t hamming_weight (const Row &row) {
  // size_t sum = accumulate(cbegin(row), cend(row), 0);
  // size_t sum = row.count();
  return row.count();
}

Matrix join (const Matrix &head, const Matrix &tail) {
  // join two matrices: join rows
  if (tail.empty())
    return head;
  if (head.empty())
    return tail;

  Matrix join;  
  for (size_t i = 0; i < head.size(); ++i) {
    Row row = head[i];
    // auto row_it = back_inserter(row);
    // copy(tail[i].begin(), tail[i].end(), row_it);
    for (size_t j = 0; j < tail[i].size(); ++j)
      row.push_back(tail[i][j]);
    join.push_back(row);
  }
  return join;
}

static inline Row eliminate (const Matrix &T, const Matrix &F,
			     const vector<size_t> &coords) {
  const size_t lngt  = T[0].size();
  const size_t Tsize = T.size();
  const size_t Fsize = F.size();

  Row A(lngt, true);
  Matrix Thead, Fhead, Ttail, Ftail;
  
  for (const Row &t : T) {
    Row row(lngt);
    for (size_t j = 0; j < lngt; ++j)
      // row.push_back(t[coords[j]]);
      row[j] = t[coords[j]];
    Ttail.push_back(row);
  }
  for (const Row &f : F) {
    Row row(lngt);
    for (size_t j = 0; j < lngt; ++j)
      // row.push_back(f[coords[j]]);
      row[j] = f[coords[j]];
    Ftail.push_back(row);
  }
  
  for (size_t i = 0; i < lngt; ++i) {
    A[coords[i]] = false;
    Row Tcolumn, Fcolumn;
    // if (!Ttail.empty())
    for (size_t j = 0; j < Tsize; ++j) {
      Tcolumn.push_back(front(Ttail[j]));
      pop_front(Ttail[j]);
    }
    // if (!Ftail.empty())
    for (size_t j = 0; j < Fsize; ++j) {
      Fcolumn.push_back(front(Ftail[j]));
      pop_front(Ftail[j]);
    }
    Matrix Ta = join(Thead, Ttail);
    Matrix Fa = join(Fhead, Ftail);
    if (inadmissible(Ta,Fa)) {
      A[coords[i]] = true;
      if (Thead.empty())
	for (size_t j = 0; j < Tsize; ++j) {
	  Row row(1, Tcolumn[j]);
	  // row.push_back(Tcolumn[j]);
	  Thead.push_back(row);
	}
      else
	for (size_t j = 0; j < Tsize; ++j)
	  Thead[j].push_back(Tcolumn[j]);
      if (Fhead.empty())
	for (size_t j = 0; j < Fsize; ++j) {
	  Row row(1, Fcolumn[j]);
	  // row.push_back(Fcolumn[j]);
	  Fhead.push_back(row);
	}
      else
	for (size_t j = 0; j < Fsize; ++j)
	  Fhead[j].push_back(Fcolumn[j]);
    }
    // we must keep at least one coordinate
    if (hamming_weight(A) == 0) {
      // we search for coordinate with unequal values in T and F
      // or both T and F have all Boolean values on that coordinate
      for (size_t j = 0; j < coords.size(); ++j) {
	unordered_set<bool> Tval, Fval;
	for (size_t k = 0; k < Tsize; ++k)
	  Tval.insert(T[k][coords[j]]);
	for (size_t k = 0; k < Fsize; ++k)
	  Fval.insert(F[k][coords[j]]);
	if (Fval != Tval || Fval.size() == 2 && Tval.size() == 2) {
	  A[coords[j]] = true;
	  return A;
	}
      }
    }
  }
  return A;
}

// computes the minimal section
Row minsect (const Matrix &T, const Matrix &F) {
  const size_t lngt  = T[0].size();

  if (inadmissible(T,F)) {
    disjoint = false;
    const Row emptyrow(lngt, false);
    return emptyrow;
  } else if (nosection) {
    const Row fullrow(lngt, true);
    return fullrow;
  }

  vector<size_t> coord(lngt);	// sequence of coordinates to inspect

  switch (direction) {
  case dBEGIN:
    for (size_t i = 0; i < lngt; ++i)
      coord[i] = lngt - 1 - i;
    break;
  case dEND:
    for (size_t i = 0; i < lngt; ++i)
      coord[i] = i;
    break;
  case dRAND: {
    random_device rd;
    static uniform_int_distribution<int> uni_dist(0,lngt-1);
    static default_random_engine dre(rd());

    for (size_t i = 0; i < lngt; ++i)
      coord[i] = i;
    shuffle(coord.begin(), coord.end(), dre);
    cout << "+++ precedence of coordinates:";
    for (size_t i = 0; i < coord.size(); ++i)
      cout << " " << coord[i];
    cout << endl;
  } break;
  case dLOWSCORE:
  case dHIGHSCORE: {
    int card[lngt] = {};
    vector<size_t> indicator(lngt, 0);

    for (const Row &t : T)
      for (size_t i = 0; i < t.size(); ++i) {
	card[i] += t[i];
	indicator[i] += t[i];
      }
    // for (const Row &f : F)
    //   for (size_t i = 0; i < f.size(); ++i) {
    // 	card[i] += f[i];
    // 	indicator[i] += f[i];
    //   }
    // for (const Row &f : F)
    //   for (size_t i = 0; i < f.size(); ++i) {
    // 	card[i] -= f[i];
    // 	indicator[i] -= f[i];
    //   }

    if (direction == dLOWSCORE)
      stable_sort(indicator.begin(), indicator.end(), greater<size_t>());
    else if (direction == dHIGHSCORE)
      stable_sort(indicator.begin(), indicator.end());

    for (size_t i = 0; i < lngt; ++i) {
      size_t j = 0;
      while (card[j] != indicator[i])
	++j;
      coord[i] = j;
      card[j] = SENTINEL;
    }
    cout << "+++ precedence of coordinates:";
    for (size_t i = 0; i < lngt; ++i)
      cout << " " << coord[i];
    cout << endl;
  } break;
  case dPREC: {
    size_t idx;			// coordinate index
    int w;			// weight
    for (size_t i = 0; i < lngt; ++i) {
      coord[i] = i;
      idx2w[i] = 50;		// 50 is the default value
    }

    if (input != STDIN && weights.empty()) {
      string::size_type pos = input.rfind('.');
      weights = (pos == string::npos ? input : input.substr(0, pos)) + ".prc";
    }
    ifstream weightstream;
    weightstream.open(weights);
    if (! weightstream.is_open()) {
      cerr << "+++ cannot open file " << weights << endl
	   << "+++ reverting to default direction (" << direction_strg[dBEGIN] << ")"
	   << endl;
      direction = dBEGIN;
      return minsect(T, F);
    } else {
      while (weightstream >> idx >> w)
	if (idx >= lngt)
	  cerr << "+++ coordinate " << idx << " out of bounds ignored" << endl;
	else
	  idx2w[idx] = w;
      weightstream.close();
    }
    stable_sort(coord.begin(), coord.end(), cmp_prec());
    cout << "+++ precedence of coordinates:";
    for (size_t i = 0; i < coord.size(); ++i)
      cout << " " << coord[i];
    cout << endl;
  } break;
  }
  return eliminate(T, F, coord);
}

void w_f (const string &filename, const string suffix,
	  const vector<size_t> &names, const Formula &formula) {
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
    for (const size_t n : names)
      formfile << " " << n+offset;
    formfile << endl;
    formfile << formula2dimacs(names, formula) << endl;
    offset = old_offset;
    formfile.close();
  }
}

void write_formula (const string &suffix1, const string &suffix2,
		    const vector<size_t> &names, const Formula &formula) {
  // write formula to a file in DIMACS format
  // offset begins at 1, if not set otherwise
  w_f(formula_output + "_" + suffix1 + "_" + suffix2 + ".log",
      suffix1, names, formula);
}

void write_formula (const string &suffix,
		    const vector<size_t> &names, const Formula &formula) {
  // write formula to a file in DIMACS format
  // offset begins at 1, if not set otherwise
  w_f(formula_output + "_" + suffix + ".log",
      suffix, names, formula);
}

bool satisfied_by (const Clause &clause, const Matrix &T) {
  // is the clause satified by all tuples in T?
  for (const Row &t : T) {
    bool satisfied = false;
    for (size_t i = 0; i < clause.size(); ++i) {
      if (clause[i] == lpos && t[i]
	  ||
	  clause[i] == lneg && ! t[i]) {
	satisfied = true;
	break;
      }
    }
    if (!satisfied)
      return false;
  }
  return true;
}

size_t numlit (const Clause &clause) {
  // number of literals in a clause
  size_t i = 0;
  for (const Literal lit : clause)
    if (lit != lnone)
      i++;
  return i;
}

size_t firstlit (const Clause &clause) {
  // coordinate of first literal
  size_t i = 0;
  while (i < clause.size() && clause[i] == lnone)
    i++;
  return i;
}

bool clauseLT (const Clause &a, const Clause &b) {
  // is clause a < clause b in literals ?
  for (size_t i = 0; i < a.size(); ++i)
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

// struct cmp_numlit {
//   bool operator() (const Clause &a, const Clause &b) {
//     return numlit(a) < numlit(b);
//   }
// };

// bool operator< (const Clause &a, const Clause &b) {
//   if (a.length() != b.length())
//     throw;
//   for (size_t i = 0; i < a.length(); ++i)
//     if (a[i] < b[i])
//       return true;
//   return false;
// }

// struct cmp_clause {
//   bool operator() (const Clause &a, const Clause &b) {
//     return numlit(a) < numlit(b)
//       || firstlit(a) < firstlit(b)
//       || a < b;
//   }
// };

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
  for (const Row &m : M)
    pointer[m] = 0;

  while (! newM.empty()) {
    for (const Row &nm : newM)
      HC.push_back(nm);
    newM.clear();
    for (const Row &m : M)
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
  for (size_t i = 1; i < M.size(); ++i)
    mh = Min(mh,M[i]);
  return mh;
}

// unit resolution
Formula unitres (const Formula &formula) {	
  Formula units;
  Formula clauses;

  for (const Clause &cl : formula)
    if (numlit(cl) == 1)
      units.push_back(cl);
    else
      clauses.push_back(cl);

  Formula resUnits = units;
  while (!units.empty() && !clauses.empty()) {
    Clause unit = units.front();
    units.pop_front();
    size_t index = 0;
    while (unit[index] == lnone)
      index++;
    for (size_t j = 0; j < clauses.size(); j++) {
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
  for (int i = 0; i < ru_bound-1; ++i) {	// must remain int
    Clause unit_i = resUnits[i];
    size_t index = 0;
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

  for (const Clause &cl : formula)
    if (numlit(cl) == 2)
      bins.push_back(cl);
    else
      clauses.push_back(cl);

  Formula resBins = bins;
  while (!bins.empty() && !clauses.empty()) {
    Clause bincl = bins.front();
    bins.pop_front();
    size_t index1 = 0;
    while (bincl[index1] == lnone)
      index1++;
    size_t index2 = index1 + 1;
    while (bincl[index2] == lnone)
      index2++;
    for (size_t j = 0; j < clauses.size(); j++) {
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
  for (size_t i = 0; i < cla.size(); ++i)
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
  for (const Literal lit : clause)
    if (lit != lnone)
      return false;
  return true;
}

// eliminating redundant clauses
// clauses must be sorted by length --- IS GUARANTEED
Formula redundant (const Formula &formula) {	
  const int lngt = formula[0].size();

  Formula prefix, suffix;
  // prefix.insert(prefix.end(), formula.begin(), formula.end());
  prefix = formula;
  // sort_mf not necessary, clauses are GUARANTEED SORTED
  // sort(prefix.begin(), prefix.end(), cmp_numlit);
  // sort_mf(prefix, 0, prefix.size()-1);

  size_t left = 0;
  while (left < prefix.size() && numlit(prefix[left]) == 1)
    left++;

  while (left < prefix.size()) {
    Clause pivot = prefix.back();
    prefix.pop_back();
    Formula newUnits;
    for (size_t i = 0; i < pivot.size(); ++i) {
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
    for (const Clause &bgcl : bogus)
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

  for (const Row &tuple : Universe) {
    R.push_back(tuple);
    for (Clause clause : SubSets)
      incidence[make_pair(tuple, clause)]
	= sat_clause(tuple, clause) ? ABSENT : PRESENT;
  }

  // perform set cover
  while (!R.empty()) {
    // How many tuples does the clause falsify (intersect)?
    map<Clause, size_t> intersect;
    for (const Clause &clause : SubSets)
      intersect[clause] = 0;
#pragma omp parallel for
    for (const Row &tuple : R)
#pragma omp parallel for
      for (const Clause &clause : SubSets)
	if (incidence[make_pair(tuple, clause)] == PRESENT)
	  ++intersect[clause];

    size_t maxw = 0;
    Clause maxset;
    for (const Clause &clause : SubSets)
      if (intersect[clause] > maxw) {
	maxw = intersect[clause];
	maxset = clause;
      }

    if (maxw == 0)
      break;
    selected.push_back(maxset);
    
    auto it = R.begin();
    while (it != R.end())
      if (incidence[make_pair(*it, maxset)] == PRESENT) {
	for (const Clause &clause : SubSets)
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
map<Row, int> predecessor (const Matrix &R) {
  map<Row, int> pred;
  pred[R[0]] = SENTINEL;
  Row p = R[0];
  for (size_t i = 1; i < R.size(); ++i) {
    Row m = R[i];
    size_t j = 0;
    for (size_t k = 0; k < m.size(); ++k)
      if (m[k] == p[k])
	j++;
      else
	break;
    pred[R[i]] = j;
    p = m;
  }
  return pred;
}

// sucessor function of Zanuttini's algorithm
// R must be lexicographically sorted
map<Row, int> successor (const Matrix &R) {
  map<Row, int> succ;
  Row m = R[0];
  for (size_t i = 1; i < R.size(); ++i) {
    Row s = R[i];
    size_t j = 0;
    for (size_t k = 0; k < m.size(); ++k)
      if (m[k] == s[k])
	j++;
      else
	break;
    succ[R[i-1]] = j;
    m = s;
  }
  succ[R[R.size()-1]] = SENTINEL;
  return succ;
}

// sim array of Zanuttini's algorithm
map<Row, vector<int>> simsim (const Matrix &R, const map<Row, int> &succ) {
  map<Row, vector<int>> sim;
  for (const Row &mm : R) {
    const vector<int> dummy(mm.size(), SENTINEL);
    sim[mm] = dummy;
    for (const Row &m1m : R) {
      if (mm == m1m)
	continue;
      int j0 = 0;
      while (mm[j0] == m1m[j0])
	++j0;
      int j = j0;
      while (j < m1m.size() && (m1m[j] || ! mm[j])) {
	if (j > succ.at(mm) && ! mm[j] && m1m[j])
	  sim[mm][j] = max(sim[mm][j], j0);
	j++;
      }
    }
  }
  return sim;
}

// generate clauses with Zanuttini's algorithm
Clause hext (const Row &m, const int &j,
	     const map<Row, int> &pred, const map<Row, int> &succ,
	     const map<Row, vector<int>> &sim) {
  Clause clause(m.size(), lnone);
  for (size_t i = 0; i < j; ++i)
    if (m[i])
      clause[i] = lneg;
  if (j > pred.at(m) && m[j])
    clause[j] = lpos;
  else if (j > succ.at(m) && ! m[j] && sim.at(m)[j] == SENTINEL)
    clause[j] = lneg;
  else if (j > succ.at(m) && ! m[j] && sim.at(m)[j] != SENTINEL) {
    clause[j] = lneg;
    clause[sim.at(m)[j]] = lpos;
  }
  return clause;
}

// phi is a CNF formula and M is a set of tuples, such that sol(phi) = M
// constructs a reduced prime formula phiPrime, such that sol(phi) = sol(phiPrime)
Formula primality (const Formula &phi, const Matrix &M) {
  const size_t card = M.size();
  const size_t lngt = M[0].size();
  Formula phiPrime;

  auto last = make_unique<int []>(card);	// smart pointer
  // int *last = new int[card];			// hard pointer
  for (Clause clause : phi) {
    if (clause.size() != lngt) {
      cerr << "+++ Clause size and vector length do not match" << endl;
      exit(2);
    }
    for (size_t k = 0; k < card; ++k) {
      last[k] = SENTINEL;
      for (size_t j = 0; j < clause.size(); ++j)
	if (M[k][j] && clause[j] == lpos
	    ||
	    ! M[k][j] && clause[j] == lneg)
	  last[k] = j;
    }
    Clause cPrime(clause.size(), lnone);
    for (size_t j = 0; j < clause.size(); ++j) {
      bool d;
      if (clause[j] == lpos) {
	d = true;
	for (size_t k = 0; k < card; ++k)
	  if (last[k] == j)
	    d = d && M[k][j];
	if (d)
	  cPrime[j] = lpos;
      } else if (clause[j] == lneg) {
	d = false;
	for (size_t k = 0; k < card; ++k)
	  if (last[k] == j)
	    d = d || M[k][j];
	if (!d)
	  cPrime[j] = lneg;
      }
      if (cPrime[j] != lnone)
	for (size_t k = 0; k < card; ++k)
	  for (size_t i = 0; i < lngt; ++i)
	    if (M[k][i] && cPrime[j] == lpos
		||
		M[k][i] && cPrime[j] == lneg) {
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
  if (T.empty()) {
    cerr << "*** learnHornExact: matrix is empty" << endl;
    exit(2);
  }
  
  const size_t lngt = T[0].size();
  if (T.size() == 1)
    // T has only one row / tuple
    for (size_t i = 0; i < lngt; ++i) {
      Clause clause(lngt, lnone);
      clause[i] = T[0][i] ? lpos : lneg;
      H.push_back(clause);
    }
  else if (T.size() > 1) {
    // T has more than one row / tuple
    sort(T.begin(), T.end(), cmp_row);
    // sort_mf(T, 0, T.size()-1);

    map<Row, int> succ = successor(T);		// successor function for Zanuttini's algorithm
    map<Row, int> pred = predecessor(T);	// predecessor function for Zanuttini's algorithm
    map<Row, vector<int>> sim = simsim(T, succ);// sim table for Zanuttini's algorithm
    
    for (const Row &m : T)
      for (size_t j = 0; j < lngt; ++j)
	if (j > pred.at(m) && m[j] || j > succ.at(m) && ! m[j])
	  H.push_back(hext(m,j, pred, succ, sim));
    
    H = primality(H, T);
    cook(H);
  }
  // sort_mf(H, 0, H.size()-1);
  // auto last3 = unique(H.begin(), H.end());
  // H.erase(last3, H.end());
  return H;
}

// learn general CNF formula with large strategy
// from negative examples F
Formula learnCNFlarge (const Matrix &F) {
  Formula formula;
  for (const Row &row : F) {
    Clause clause;
    // for (bool bit : row)
    for (size_t i = 0; i < row.size(); ++i)
      clause.push_back(! row[i] ? lpos : lneg);
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
    clause[j] = ! m[j] ? lpos : lneg;
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
  for (size_t ell = 0; ell < T.size(); ++ell) {
    for (size_t i = frk[ell]+1; i < arity; ++i)
      if (T[ell][i])
	formula.push_back(negLeft(T[ell], i));
    for (size_t i = frk[ell+1]+1; i < arity; ++i)
      if (! T[ell][i])
	formula.push_back(negRight(T[ell], i));
  }
  formula = primality(formula, T);
  cook(formula);
  return formula;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// // swap the polarity of values in a tuple
// void polswap_row (Row &row) {
//   row = ~row;
//   // Row swapped = ~row;
//   // for (bool bit : row)
//   //   swapped.push_back(! bit);
// }

// swap polarity of every tuple in a matrix
void polswap_matrix (Matrix &A) {
  for (Row &row : A)
    // polswap_row(row);
    row = ~row;
}

// swap polarity of literals in a clause
Clause polswap_clause (const Clause &clause) {
  Clause swapped;
  for (const Literal literal : clause)
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
  for (const Clause &clause : formula)
    swapped.push_back(polswap_clause(clause));
  return swapped;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

string time2string (size_t mlseconds) {
  enum TimeUnit : char {mlsec = 0, sec = 1, min = 2, hour = 3, day = 4};
  const string tu_name[] = {" millisecond(s) ",
			    " second(s) ",
			    " minute(s) ",
			    " hour(s) ",
			    " day(s) "};
  const size_t timevalue[] = {1000, 60, 60, 24};
  size_t timeunit[5] = {0, 0, 0, 0, 0};

  if (mlseconds == 0)
    return "0 milliseconds";

  for (size_t t = mlsec; t < day; ++t) {
    timeunit[t] = mlseconds % timevalue[t];
    mlseconds /= timevalue[t];
  }
  if (mlseconds > 0)
    timeunit[day] = mlseconds;

  string output;
  for (int t = day; t > sec; --t)
    if (timeunit[t] > 0)
      output += to_string(timeunit[t]) + tu_name[t];
  output += to_string(timeunit[sec])
    + "."
    + to_string(timeunit[mlsec])
    + tu_name[sec];
  return output;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
