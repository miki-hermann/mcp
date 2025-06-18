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
 *     File:    src-mekong/mcp-common.cpp                                 *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 **************************************************************************/

#include "mcp-common.hpp"
#include "mcp-matrix+formula.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
// #include <algorithm>

// do we really need omp.h ?
#include <omp.h>

using namespace std;

// string version = GLOBAL_VERSION;
bool debug = false;
// bool varswitch = false;

// predecessor function for Zanuttini's algorithm
unordered_map<Row, size_t> pred;
// successor function for Zanuttini's algorithm
unordered_map<Row, size_t> succ;
// sim table for Zanuttini's algorithm
unordered_map<Row, vector<size_t>> sim;

map<size_t, int> idx2w;		// coordinate index to weight for precedence dir
struct cmp_prec { 
  bool operator() (const size_t &idx1, const size_t &idx2) {
    return idx2w.at(idx1) < idx2w.at(idx2);
    // return idx2w[idx1] < idx2w[idx2];
  }
};

// const int SENTINEL     = -1;
const string STDIN = "STDIN";
const string STDOUT = "STDOUT";
// const int MTXLIMIT     = 4000;

// Action action       = aALL;
Closure closure = clHORN;
Cooking cooking = ckWELLDONE;
Direction direction = dBEGIN;
// Print print         = pVOID;
// setcover take very much time, therefore we change the default to false
// bool setcover = true;
bool setcover = false;
Strategy strategy = sLARGE;
// Display display     = yUNDEF;
string input = STDIN;
string output = STDOUT;
string headerput    = "";
string weights      = "";
bool disjoint = true;
// int arity = 0;

// int offset          = 0;
string tpath = "/tmp/"; // directory where the temporary files will be stored
bool np_fit = false;
int chunkLIMIT = 4096; // heavily hardware dependent; must be optimized
string latex = "";     // file to store latex output

ifstream infile;
ifstream headerfile;
ifstream weightstream;
ofstream outfile;
ofstream latexfile;
string formula_output; // prefix of files, where formulas will be stored

const string action_strg[] = {"One to One", "One to All Others",
                              // "One to All Others, Nosection",
                              "Selected to All Others"};
const string closure_strg[] = {"Horn", "dual Horn", "2SAT", "affine",
                               "CNF"};
const string cooking_strg[] = {"raw", "bleu", "medium", "well done"};
const string direction_strg[] = {
  "begin", "end", "optimum", "random", "low cardinality", "high cardinality"};
const string pcl_strg[] = {"Horn", "Horn", "2SAT", "affine", "cnf"};
const string strategy_strg[] = {"large", "exact"};
// const string print_strg[]     = {"void",       "clause",     "implication",
// "mixed",   "DIMACS"}; const string display_strg[]   = {"undefined",  "hide",
// "peek",        "section", "show"};
const string arch_strg[] = {"seq", "mpi", "pthread", "hybrid"};

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

unsigned int random_seed;

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

  if (direction == dRAND)
    std::srand(random_seed);
}

size_t hamming_distance(const Row &u, const Row &v) {
  // Hamming distance between two tuples
  if (u.size() != v.size())
    return size_t(SENTINEL);

  size_t sum = 0;
  for (size_t i = 0; i < u.size(); ++i)
    sum += size_t(abs((long int)u[i] - (long int)v[i]));
  return sum;
}

// is the tuple row in the Horn closure of matrix M?
// TODO: REWRITE in a zero-copy way in
// [ ] posix
// [x] seq
/*
  bool InHornClosure(const RowView &row, const Matrix &M) {
  Matrix P = ObsGeq(row, M);

  if (P.empty()) {
  return false;
  } else if (row == MIN(P)) {
  return true;
  } else {
  return false;
  }
  }
*/

// is the intersection of F and of the Horn closure of T empty?
template <typename M> bool SHCPsolvable(const M &T, const M &F) {
  for (size_t i = 0; i < F.num_rows(); ++i) {
    if (InHornClosure(F[i], T))
      return false;
  }
  return true;
}

bool SHCPsolvable(const Matrix &T, const Matrix &F) {
  return SHCPsolvable<Matrix>(T, F);
}

bool SHCPsolvable(const MatrixMask &T, const MatrixMask &F) {
  return SHCPsolvable<MatrixMask>(T, F);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool isect_nonempty(const Matrix &T, const Matrix &F) {
  unordered_set<reference_wrapper<const Row>, std::hash<Row>,
                std::equal_to<Row>>
    orig{};

  // insert rowviews into the hashset...
  for (size_t i = 0; i < T.num_rows(); ++i) {
    orig.insert(T[i]);
  }

  // check for the presence of any row from F in the hashset
  for (size_t i = 0; i < F.num_rows(); ++i) {
    if (orig.find(F[i]) != orig.end())
      return true;
  }

  return false;
}

// checks if the intersection of T and F is non empty
bool isect_nonempty(const MatrixMask &T, const MatrixMask &F) {
  unordered_set<RowView> orig{};

  // insert rowviews into the hashset...
  for (size_t i = 0; i < T.num_rows(); ++i) {
    orig.insert(T[i]);
  }

  // check for the presence of any row from F in the hashset
  for (size_t i = 0; i < F.num_rows(); ++i) {
    if (orig.find(F[i]) != orig.end())
      return true;
  }

  return false;
}

bool inadmissible(const Matrix &T, const Matrix &F) {
  if (closure == clHORN || closure == clDHORN)
    return !SHCPsolvable(T, F);
  else
    return isect_nonempty(T, F);
}

bool inadmissible(const MatrixMask &T, const MatrixMask &F) {
  if (closure == clHORN || closure == clDHORN)
    return !SHCPsolvable(T, F);
  else
    return isect_nonempty(T, F);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// TODO: statistics heuristic? -> Z score, Median Absolute Deviation...
// These heuristics need global data to be calculated, however.

// Hamming weight of a tuple
size_t hamming_weight(const vector<bool> &row) {
  int sum = accumulate(cbegin(row), cend(row), 0);
  return sum;
}

static inline Mask eliminate(const Matrix &T, const Matrix &F,
                             const vector<size_t> &coords) {
  const size_t n = T.num_cols();

  // boolean mask
  Mask mask(n, true);
  MatrixMask Tm(T), Fm(F);

  for (const size_t &i : coords) {
    mask[i] = false;
    Tm.hide_column(i);
    Fm.hide_column(i);
    if (inadmissible(Tm, Fm)) {
      mask[i] = true;
      Tm = MatrixMask(T, mask);
      Fm = MatrixMask(F, mask);
    }
    // we keep at least one coordinate
    if (hamming_weight(mask) == 0) {
      for (const size_t &j : coords)
	if (i == j)
	  continue;
	else {
	  unordered_set<bool> Tval, Fval;
	  for (size_t k = 0; k < T.num_rows(); ++k)
	    Tval.insert(T[k][coords[j]]);
	  for (size_t k = 0; k < F.num_rows(); ++k)
	    Fval.insert(F[k][coords[j]]);
	  if (Fval != Tval || Fval.size() == 2 && Tval.size() == 2) {
	    mask[j] = true;
	    return mask;
	  }
	}
    }
  }

  return mask;
}

// computes the minimal section for Horn or dual Horn closures
Mask minsect(const Matrix &T, const Matrix &F) {
  const size_t n = T.num_cols();

  if (inadmissible(T, F)) {
    disjoint = false;
    Mask emptymask(n, false);
    return emptymask;
  } else if (nosection) {
    Mask fullmask(n, true);
    return fullmask;
  }

  vector<size_t> coords(n);

  switch (direction) {
  case dBEGIN:
    for (size_t i = 0; i < n; ++i)
      coords[i] = n - 1 - i;
    break;
  case dEND:
    for (size_t i = 0; i < n; ++i)
      coords[i] = i;
    break;
  case dRAND: {
    for (size_t i = 0; i < n; ++i)
      coords[i] = i;

    std::random_device rd;
    static std::uniform_int_distribution<int> uni_dist(0,n-1);
    static std::default_random_engine dre(rd());

    // std::random_shuffle(coords.begin(), coords.end());
    // std::shuffle(coords.begin(), coords.end());
    std::shuffle(coords.begin(), coords.end(), dre);
    std::cout << "+++ precedence of coordinates:";
    for (size_t i = 0; i < coords.size(); ++i)
      std::cout << " " << coords[i];
    std::cout << endl;
    break;
  }
  case dPREC: {
    for (size_t i = 0; i < n; ++i) {
      coords[i] = i;
      idx2w[i] = 50;			// 50 is the default value
    }
    
    if (input != STDIN && weights.empty()) {
      string::size_type pos = input.rfind('.');
      weights = (pos == string::npos ? input : input.substr(0, pos))
	+ ".prc";
    }
    weightstream.open(weights);
    if (! weightstream.is_open()) {
      std::cerr << "+++ cannot open file " << weights << std::endl
		<< "+++ reverting to default direction ("
		<< direction_strg[dBEGIN] << ")"
		<< std::endl;
      direction = dBEGIN;
      return minsect(T, F);
    } else {
      size_t idx;			// coordinate index
      int w;				// weight
      while (weightstream >> idx >> w)
	if (idx >= n)
	  std::cerr << "+++ coordinate " << idx
		    << " out of bounds ignored" << std::endl;
	else
	  idx2w[idx] = w;
      weightstream.close();
    }
    std::stable_sort(coords.begin(), coords.end(), cmp_prec());
    std::cout << "+++ precedence of coordinates:";
    for (size_t i = 0; i < coords.size(); ++i)
      std::cout << " " << coords[i];
    std::cout << endl;
    break;
  }
  case dLOWSCORE:
  case dHIGHSCORE: {
    double score[n] = {};
    vector<double> indicator(n, 0.0);
    vector<bool> mask(n, true);

    Matrix trT = T.transpose();
    for (size_t row = 0; row < trT.num_rows(); ++row) {
      score[row] = trT[row].zscore();
      indicator[row] = score[row];
    }

    if (direction == dLOWSCORE)
      stable_sort(indicator.begin(), indicator.end(), greater<double>());
    else if (direction == dHIGHSCORE)
      stable_sort(indicator.begin(), indicator.end());

    for (size_t i = 0; i < n; ++i) {
      size_t j = 0;
      while (score[j] != indicator[i] || ! mask[j])
	++j;
      coords[i] = j;
      mask[j] = false;
    }
  } break;
  case dOPT:
    std::cerr << "Unsupported direction: dOPT" << std::endl;
    exit(2);
    break;
  default:
    std::cerr << "minsect: you should not be here" << std::endl;
    exit(2);
    break;
  }

  return eliminate(T, F, coords);
}

// open the file and write the formula in it
void w_f(const string &filename, const string suffix,
         const vector<size_t> &names, const Formula &formula) {
  ofstream formfile;
  formfile.open(filename);
  if (!formfile.is_open()) {
    cerr << "+++ Cannot open formula output file " << filename << endl;
    cerr << "+++ Formula not written" << endl;
  } else {
    formfile << suffix << " " << arity << " " << formula.cbegin()->size() << " "
             << offset << endl;
    size_t old_offset = offset;
    offset = 1;
    for (size_t n : names)
      formfile << " " << n + offset;
    formfile << endl;
    formfile << formula2dimacs(names, formula) << endl;
    offset = old_offset;
    formfile.close();
  }
}

// write formula to a file in DIMACS format
// offset begins at 1, if not set otherwise
void write_formula(const string &suffix1, const string &suffix2,
                   const vector<size_t> &names, const Formula &formula) {
  w_f(formula_output + "_" + suffix1 + "_" + suffix2 + ".log", suffix1, names,
      formula);
}

// write formula to a file in DIMACS format
// offset begins at 1, if not set otherwise
void write_formula(const string &suffix, const vector<size_t> &names,
                   const Formula &formula) {
  w_f(formula_output + "_" + suffix + ".log", suffix, names, formula);
}

// is the clause satified by all tuples in T?
bool satisfied_by(const Clause &clause, const Matrix &T) {
  for (size_t i = 0; i < T.num_rows(); ++i) {
    bool satisfied = false;
    for (size_t j = 0; j < clause.size(); ++j) {
      if (clause[j].sat(T.get(i, j))) {
        satisfied = true;
        break;
      }
    }
    if (!satisfied)
      return false;
  }
  return true;
}

// number of literals in a clause
size_t numlit(const Clause &clause) {
  size_t i = 0;
  for (Literal lit : clause)
    switch (lit.sign) {
    case lneg:
    case lpos:
      ++i;
      break;
    case lboth:
      i += 2;
      break;
    case lnone:
      break;
    }
  return i;
}

// coordinate of first literal
size_t firstlit(const Clause &clause) {
  size_t i = 0;
  while (i < clause.size() && clause[i].sign == lnone)
    i++;
  return i;
}

// is clause a < clause b in literals ?
bool clauseLT(const Clause &a, const Clause &b) {
  for (size_t i = 0; i < a.size(); ++i)
    if (a[i] < b[i])
      return true;
  return false;
}

// This overloading is necessay because deque implements >= differently
bool clauseGE(const Clause &a, const Clause &b) {
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
  if (numlit(a) == numlit(b) && firstlit(a) < firstlit(b))
    return false;
  if (numlit(a) == numlit(b) && firstlit(a) == firstlit(b))
    return clauseLT(b, a);
  return true;
}

size_t partition_formula(Formula &formula, size_t low, size_t high) {
  Clause &pivot = formula[high];
  size_t p_index = low;

  for (size_t i = low; i < high; i++) {
    if (clauseGE(pivot, formula[i])) {
      if (p_index != i) {
        swap(formula[i], formula[p_index]);
      }
      p_index++;
    }
  }
  if (p_index < high)
    swap(formula[high], formula[p_index]);

  return p_index;
}

void sort_formula(Formula &formula, size_t low, size_t high) {
  if (high < formula.size() && low < high) {
    size_t p_index = partition_formula(formula, low, high);
    if (p_index > 0 && low < p_index - 1)
      sort_formula(formula, low, p_index - 1);
    if (p_index < numeric_limits<size_t>::max() && p_index + 1 < high)
      sort_formula(formula, p_index + 1, high);
  }
}

// restricts matrix A to columns determined by the bitvector sect
void restrict(const Mask &sect, Matrix &A) {
  A.restrict(sect);
  A.sort();
  A.remove_duplicates();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Row minHorn(const Matrix &M) {
  // computes the minimal tuple of a matrix coordinate wise
  Row mh = M[0].clone();
  for (size_t i = 1; i < M.num_rows(); ++i)
    mh.inplace_minimum(M[i]);
  return mh;
}

// compare and eliminate unit clauses with same variable and sign,
// but different value
// x <= d1 && x <= d2 becomes x <= min(d1, d2)
// x >= d1 && x >= d2 becomes x >= max(d1, d2)
void unit2unit (Formula &units) {
  map<size_t, map<Sign, set<integer>>> uclauses;
  const size_t arity = units.front().size();
  
  for (const Clause &clause : units) {
    size_t index = firstlit(clause);
    Literal lit = clause[index];
    Sign sgn = lit.sign;
    uclauses[index][sgn].insert(sgn == lneg ? lit.nval : lit.pval);
  }

  // clear units, they are now in uclauses
  units.clear();
  for (const auto &un : uclauses) {
    size_t index = un.first;
    for (const auto &sg : un.second) {
      Literal lit = sg.first == lneg
	? Literal::neg(*(sg.second.cbegin()))
	: Literal::pos(*(sg.second.crbegin()));
      Clause new_clause(arity, Literal::none());
      new_clause[index] = lit;
      units.push_back(new_clause);
    }
  }
}

// unit resolution
Formula unitres(const Formula &formula) {
  Formula units;
  Formula clauses;

  for (const Clause &cl : formula)
    if (numlit(cl) == 1)
      units.push_back(cl);
    else
      clauses.push_back(cl);

  Formula resUnits;
  while (!units.empty() && !clauses.empty()) {
    copy(units.cbegin(), units.cend(), back_inserter(resUnits));
    const Clause unit = units.front();
    units.pop_front();
    const size_t index = firstlit(unit);
    for (size_t j = 0; j < clauses.size(); j++) {
      Clause &clause = clauses[j];

      if (unit[index].sign == lpos && clause[index].sign & lpos &&
          unit[index].pval >= clause[index].pval) {
        // x >= d & (x >= d' + c), && d >= d' => x >= d
        clause = Clause();
      } else if (unit[index].sign == lneg && clause[index].sign & lneg &&
                 unit[index].nval <= clause[index].nval) {
        // x <= d & (x <= d' + c), && d <= d' => x <= d
        clause = Clause();
      } else if (unit[index].sign == lpos && clause[index].sign & lneg &&
                 unit[index].pval > clause[index].nval) {
        // x >= p & (x <= n + c), && p > n => x >= p & c
        clause[index].sign = Sign(clause[index].sign ^ lneg);
      } else if (unit[index].sign == lneg && clause[index].sign & lpos &&
                 unit[index].nval < clause[index].pval) {
        // x <= n & (x >= p + c), && n < p => x <= n & c
        clause[index].sign = Sign(clause[index].sign ^ lpos);
      }
    }

    auto it = clauses.begin();
    while (it != clauses.end()) {
      size_t num = numlit(*it);
      if (num == 0) {
        it = clauses.erase(it);
      } else if (num == 1) {
        units.push_back(*it);
        // resUnits.push_back(*it);
        it = clauses.erase(it);
      } else {
        ++it;
      }
    }
  }
  if (! units.empty())
    move(units.cbegin(), units.cend(), back_inserter(resUnits));
  unit2unit(resUnits);

  // sort(resUnits.begin(), resUnits.end());
  sort_formula(resUnits, 0, resUnits.size() - 1);
  auto last1 = unique(resUnits.begin(), resUnits.end());
  resUnits.erase(last1, resUnits.end());

  // test if there are no two unit clauses having literals of opposite parity
  size_t ru_bound = resUnits.size();
  if (ru_bound > 1)
    for (size_t i = 0; i < ru_bound - 1; ++i) {
      Clause &unit_i = resUnits[i];
      size_t index = 0;
      while (index < unit_i.size() && unit_i[index].sign == lnone)
        index++;
      if (index < ru_bound) {
        for (size_t j = i + 1; j < ru_bound; ++j) {
          Clause &unit_j = resUnits[j];
          if ((unit_i[index].sign == lpos && unit_j[index].sign == lneg &&
               unit_i[index].pval > unit_j[index].nval) ||
              (unit_i[index].sign == lneg && unit_j[index].sign == lpos &&
               unit_i[index].nval < unit_j[index].pval)) {
            Clause emptyClause(unit_i.size(), Literal::none());
            Formula emptyFormula;
            emptyFormula.push_back(emptyClause);
            return emptyFormula;
          }
        }
      }
    }

  clauses.insert(clauses.end(), resUnits.begin(), resUnits.end());
  // sort(clauses.begin(), clauses.end()), cmp_numlit;
  sort_formula(clauses, 0, clauses.size() - 1);
  auto last2 = unique(clauses.begin(), clauses.end());
  clauses.erase(last2, clauses.end());

  return clauses;
}

bool subsumes(const Clause &cla, const Clause &clb) {
  // does clause cla subsume clause clb ?
  // cla must be smaller than clb
  for (size_t i = 0; i < cla.size(); ++i) {
    if (cla[i].sign != lnone) {
      if (cla[i].sign & lneg && !(clb[i].sign & lneg))
        return false;
      if (cla[i].sign & lneg && clb[i].sign & lneg && cla[i].nval > clb[i].nval)
        return false;
      if (cla[i].sign & lpos && !(clb[i].sign & lpos))
        return false;
      if (cla[i].sign & lpos && clb[i].sign & lpos && cla[i].pval < clb[i].pval)
        return false;
    }
  }
  return true;
}

// perform subsumption on clauses of a formula
// clauses must be sorted by length --- IS GUARANTEED
Formula subsumption(Formula &formula) {
  Formula res;
  // sort_formula not necessary, clauses are GUARANTEED SORTED
  // sort(formula.begin(), formula.end(), cmp_numlit);
  // sort_formula(formula, 0, formula.size()-1);
  while (!formula.empty()) {
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

bool empty_clause(const Clause &clause) { // is the clause empty ?
  for (Literal lit : clause)
    if (lit.sign != lnone)
      return false;
  return true;
}

Formula redundant(const Formula &formula) { // eliminating redundant clauses
                                            // clauses must be sorted by length
                                            // --- IS GUARANTEED
  const size_t lngt = formula[0].size();

  Formula prefix, suffix;
  // prefix.insert(prefix.end(), formula.begin(), formula.end());
  prefix = formula;
  // sort_formula not necessary, clauses are GUARANTEED SORTED
  // sort(prefix.begin(), prefix.end(), cmp_numlit);
  // sort_formula(prefix, 0, prefix.size()-1);

  size_t left = 0;
  while (left < prefix.size() && numlit(prefix[left]) == 1)
    left++;

  while (left < prefix.size()) {
    Clause pivot = prefix.back();
    prefix.pop_back();
    Formula newUnits;
    for (size_t i = 0; i < pivot.size(); ++i) {
      if (pivot[i].sign != lnone) {
        Clause newclause(lngt, Literal::none());
        Literal lit = pivot[i].negate(i);
        if (lit.sign != lboth && lit.sign != lnone) {
          newclause[i] = lit;
          newUnits.push_back(newclause);
        } else {
          lit.sign = lpos;
          newclause[i] = lit;
          newUnits.push_back(newclause);
          lit.sign = lneg;
          newclause[i] = lit;
          newUnits.push_back(newclause);
        }
      }
    }
    newUnits.insert(newUnits.end(), prefix.begin(), prefix.end());
    newUnits.insert(newUnits.end(), suffix.begin(), suffix.end());
    // sort(newUnits.begin(), newUnits.end(), cmp_numlit);
    sort_formula(newUnits, 0, newUnits.size() - 1);
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

Formula SetCover(const Matrix &Universe, const Formula &SubSets) {
  // perform set cover optimizing the clauses as subsets falsified by tuples as
  // universe Universe = tuples in F
  // SubSets  = clauses of a formula
  enum Presence { NONE = 0, ABSENT = 1, PRESENT = 2 };

  typedef pair<reference_wrapper<const Row>, reference_wrapper<const Clause>>
    Falsification;
  struct falsification_hasher {
    size_t operator()(const Falsification &f) const noexcept {
      return hash_combine(hash_combine(std::hash<Row>{}(f.first), 0xff),
                          std::hash<Clause>{}(f.second));
    }
  };
  struct falsification_eq {
    bool operator()(const Falsification &a,
                    const Falsification &b) const noexcept {
      return a.first.get() == b.first.get() && a.second.get() == b.second.get();
    }
  };

  // Incidence matrix indicating which tuple (row) falsifies which clause
  unordered_map<Falsification, Presence, falsification_hasher, falsification_eq>
    incidence;
  vector<size_t> R; // tuples still active == not yet falsified
  Formula selected; // selected clauses for falsification

  for (size_t i = 0; i < Universe.num_rows(); ++i) {
    R.push_back(i);

    const Row &tuple = Universe[i];
    for (const Clause &clause : SubSets)
      incidence[make_pair(cref(tuple), cref(clause))] =
	sat_clause(tuple, clause) ? ABSENT : PRESENT;
  }

  // perform set cover
  while (!R.empty()) {
    cerr << "\r" << R.size() << flush;

    // How many tuples does the clause falsify (intersect)?
    unordered_map<reference_wrapper<const Clause>, size_t, std::hash<Clause>>
      intersect;
    for (const Clause &clause : SubSets)
      intersect[clause] = 0;

    for (const Clause &clause : SubSets) {
#pragma omp parallel for
      for (size_t i : R) {
        const Row &tuple = Universe[i];
        if (incidence[make_pair(cref(tuple), cref(clause))] == PRESENT) {
          ++intersect[clause];
        }
      }
    }

    struct maxclause {
      size_t maxw;
      const Clause *maxset;

      static maxclause max(maxclause a, maxclause b) {
        return a.maxw > b.maxw ? a : b;
      }
    };

#pragma omp declare reduction(maxVal:maxclause : omp_out =	\
			      maxclause::max(omp_out, omp_in))

    maxclause m = {.maxw = 0, .maxset = &SubSets[0]};

#pragma omp parallel for reduction(maxVal : m)
    for (const Clause &clause : SubSets) {
      if (intersect[clause] > m.maxw) {
        m.maxw = intersect[clause];
        m.maxset = &clause;
      }
    }

    size_t maxw = m.maxw;
    // cannot be invalidated: SubSets is immutable.
    const Clause *maxset = m.maxset;

    if (maxw == 0)
      break;
    selected.push_back(*maxset);

    size_t i = 0;
    while (i < R.size()) {
      const Row &tuple = Universe[R[i]];
      if (incidence[make_pair(cref(tuple), cref(*maxset))] == PRESENT) {
        for (const Clause &clause : SubSets) {
          incidence[make_pair(cref(tuple), cref(clause))] = ABSENT;
        }
        if (i != R.size() - 1) {
          swap(R[i], R[R.size() - 1]);
        }
        R.resize(R.size() - 1);
      } else {
        ++i;
      }
    }
  }

  cerr << "\r" << flush;

  // sort(selected.begin(), selected.end(), cmp_numlit);
  sort_formula(selected, 0, selected.size() - 1);
  return selected;
}

// perform redundancy elimination on formula according to cooking
void cook(Formula &formula) {
  if (!formula.empty()) {
    if (cooking == ckRAW)
      sort_formula(formula, 0, formula.size() - 1);
    if (cooking >= ckBLEU) {
      formula = unitres(formula);
    }
    if (cooking >= ckMEDIUM)
      formula = subsumption(formula);
    if (cooking == ckWELLDONE)
      formula = redundant(formula);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void predecessor(const Matrix &R) {
  // predecessor function of Zanuttini's algorithm
  // R must be lexicographically sorted
  pred.clear();
  pred[R[0].clone()] = size_t(SENTINEL);
  // pred.insert({R[0].clone(), SENTINEL});

  auto p = cref(R[0]);
  for (size_t i = 1; i < R.num_rows(); ++i) {
    const Row &m = R[i];
    size_t j = 0;
    for (size_t k = 0; k < m.size(); ++k)
      if (m[k] == p.get()[k])
        j++;
      else
        break;
    pred[R[i].clone()] = j;
    p = m;
  }
}

void successor(const Matrix &R) {
  // sucessor function of Zanuttini's algorithm
  // R must be lexicographically sorted
  succ.clear();
  auto m = cref(R[0]);
  for (size_t i = 1; i < R.num_rows(); ++i) {
    const Row &s = R[i];
    size_t j = 0;
    for (size_t k = 0; k < m.get().size(); ++k)
      if (m.get()[k] == s[k])
        j++;
      else
        break;
    succ[R[i - 1].clone()] = j;
    m = s;
  }
  succ[R[R.num_rows() - 1].clone()] = size_t(SENTINEL);
}

void simsim(const Matrix &R) { // sim array of Zanuttini's algorithm
  sim.clear();
  for (size_t i = 0; i < R.num_rows(); ++i) {
    const Row &mm = R[i];
    const vector<size_t> dummy(mm.size(), size_t(SENTINEL));
    sim[mm.clone()] = std::move(dummy);

    for (size_t k = 0; k < R.num_rows(); ++k) {
      const Row &m1m = R[k];
      if (mm == m1m)
        continue;
      size_t j0 = 0;
      while (mm[j0] == m1m[j0])
        ++j0;
      size_t j = j0;
      while (j < m1m.size() && (m1m[j] == true || mm[j] == false)) {
        if (j > succ[mm.clone()] && mm[j] == false && m1m[j] == true)
          sim[mm.clone()][j] = max(sim[mm.clone()][j], j0);
        j++;
      }
    }
  }
}

// TODO: what the frick
Clause hext(const Row &m, const int &j) {
  // generate clauses with Zanuttini's algorithm
  Clause clause(m.size(), Literal::none());
  /*
    for (int i = 0; i < j; ++i)
    if (m[i] == true)
    clause[i] = lneg;
    if (j > pred[m] && m[j] == true)
    clause[j] = lpos;
    else if (j > succ[m] && m[j] == false && sim[m][j] == SENTINEL)
    clause[j] = lneg;
    else if (j > succ[m] && m[j] == false && sim[m][j] != SENTINEL) {
    clause[j] = lneg;
    clause[sim[m][j]] = lpos;
    }
  */
  return clause;
}

// TODO: What the frick
// phi is a CNF formula and M is a set of tuples, such that sol(phi) = M
// constructs a reduced prime formula phiPrime, such that sol(phi) =
// sol(phiPrime)
Formula primality(const Formula &phi, const Matrix &M) {
  Formula phiPrime;
  /*
    const int card = M.num_rows();
    const int lngt = M.num_cols();
    auto last = make_unique<int[]>(card);

    for (Clause clause : phi) {
    if (clause.size() != lngt) {
    cerr << "+++ Clause size and vector length do not match" << endl;
    exit(2);
    }
    for (int k = 0; k < card; ++k) {
    last[k] = SENTINEL;
    for (int j = 0; j < clause.size(); ++j)
    if (M[k][j] == true && clause[j] == lpos ||
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
    if (M[k][i] == true && cPrime[j] == lpos ||
    M[k][i] == false && cPrime[j] == lneg) {
    last[k] = SENTINEL;
    break;
    }
    }
    phiPrime.push_back(cPrime);
    }
  */
  return phiPrime;
}

// TODO: What the frick
// learn the exact Horn clause from positive examples T
// uses Zanuttini's algorithm
Formula learnHornExact(const Matrix &T, const vector<size_t> &A) {
  Formula H;

  const size_t lngt = T.num_cols();
  if (T.num_rows() == 1) { // T has only one row / tuple
    const Row &t = T[0];
    for (size_t i = 0; i < lngt; ++i) {
      Clause clause(lngt, Literal::none());
      clause[i] = Literal::pos(t[i]);
      H.push_back(clause);
      clause[i] = Literal::neg(t[i]);
      H.push_back(clause);
    }
    return H;
  }

  /*
  // What
  // The
  // Frick
  sort_matrix(T, 0, T.num_rows() - 1);
  successor(T);
  predecessor(T);
  simsim(T);

  for (size_t i = 0; i < T.num_rows(); ++i) {
  const Row &m = T[i];
  for (int j = 0; j < lngt; ++j)
  if ((j > pred[m] && m[j] == true) || (j > succ[m] && m[j] == false))
  H.push_back(hext(m, j));
  }

  */
  H = primality(H, T);
  cook(H);
  return H;
}

// learn general CNF formula with large strategy
// from negative examples F
Formula learnCNFlarge(const Matrix &F, const vector<size_t> &A) {
  Formula formula;
  for (size_t j = 0; j < F.num_rows(); ++j) {
    const Row &row = F[j];
    Clause clause(row.size());
    // for (bool bit : row)
    for (size_t i = 0; i < row.size(); ++i) {
      // isolate row[i]
      Literal lit;
      if (row[i] > 0) {
        lit.sign = lneg;
        lit.nval = row[i] - 1;
      }
      if (row[i] < headlines[A[i]].DMAX) {
        lit.sign = Sign(lit.sign | lpos);
        lit.pval = row[i] + 1;
      }
      // clause.push_back(lit);
      clause[i] = lit;
    }
    formula.push_back(clause);
  }

  cook(formula);
  return formula;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// UNSAFE if m1 == m2
size_t fork(const Row &m1, const Row &m2) {
  size_t i = 0;
  // we can guarantee that always m1 != m2, therefore
  // we can drop i < m1.size()
  while (m1[i] == m2[i])
    ++i;
  return i;
}

deque<size_t> fork(const Matrix &M) {
  deque<size_t> frk(M.num_rows() - 1, 0);
  for (size_t ell = 0; ell < M.num_rows() - 1; ++ell)
    frk[ell] = fork(M[ell], M[ell + 1]);
  frk.push_front(size_t(SENTINEL));
  frk.push_back(size_t(SENTINEL));
  return frk;
}

/*
  Clause negTerm(const Row &m, const int &i) {
  Clause clause(m.size(), Literal::none());
  for (int j = 0; j < i; ++j)
  clause[j] = m[j] == false ? lpos : lneg;
  return clause;
  }

  Clause negLeft(const Row &m, const int &i) {
  Clause clause = negTerm(m, i);
  clause[i] = lpos; // negLT
  return clause;
  }

  Clause negRight(const Row &m, const int &i) {
  Clause clause = negTerm(m, i);
  clause[i] = lneg; // negGT
  return clause;
  }
*/

// TODO: What the frick
// learn general CNF formula with exact strategy
// from positive examples T
Formula learnCNFexact(const Matrix &T) {
  Formula formula;
  /*
  // sort(T.begin(), T.end());
  sort_matrix(T, 0, T.size() - 1);
  auto ip = unique(T.begin(), T.end());
  T.resize(distance(T.begin(), ip));

  deque<int> frk = fork(T); // frk.size() == T.size()+1
  for (int ell = 0; ell < T.size(); ++ell) {
  for (int i = frk[ell] + 1; i < arity; ++i)
  if (T[ell][i] == true)
  formula.push_back(negLeft(T[ell], i));
  for (int i = frk[ell + 1] + 1; i < arity; ++i)
  if (T[ell][i] == false)
  formula.push_back(negRight(T[ell], i));
  }
  formula = primality(formula, T);
  */
  cook(formula);
  return formula;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// swap the polarity of values in a tuple
void polswap_row(Row &row) {
  for (size_t i = 0; i < row.size(); ++i) {
    row[i] = headlines[i].DMAX - row[i];
  }
}

// swap polarity of every tuple in a matrix
void polswap_matrix(Matrix &A) {
  for (size_t i = 0; i < A.num_rows(); ++i) {
    polswap_row(A[i]);
  }
}

// swap polarity of literals in a clause
void polswap_clause(Clause &clause) {
  for (size_t i = 0; i < clause.size(); ++i)
    clause[i] = clause[i].swap(i);
  // for (Literal &literal : clause)
  //   literal = literal.swap();
}

// swap polarity of every clause of a formula
void polswap_formula(Formula &formula) {
  for (Clause &clause : formula)
    polswap_clause(clause);
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
