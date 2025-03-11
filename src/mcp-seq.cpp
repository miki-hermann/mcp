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
 *	Version: sequential                                               *
 *      File:    mcp-seq.cpp                                              *
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
#include <sstream>
#include <fstream>
#include <queue>
#include <algorithm>
#include <cmath>
#include <chrono>
#include "mcp-matrix+formula.hpp"
#include "mcp-common.hpp"

using namespace std;

Arch arch = archSEQ;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void adjust () {			// adjusts the input parameters
  if (input != STDIN) {
    infile.open(input);
    if (infile.is_open())
      cin.rdbuf(infile.rdbuf());
    else {
      cerr << "+++ Cannot open input file " << input << endl;
      exit(2);
    }
  }

  if (input != STDIN && headerput.empty()) {
    string::size_type pos = input.rfind('.');
    headerput = (pos == string::npos ? input : input.substr(0, pos)) + ".hdr";
  }

  if (output != STDOUT) {
    outfile.open(output);
    if (outfile.is_open())
      cout.rdbuf(outfile.rdbuf());
    else {
      cerr << "+++ Cannot open output file " << output << endl;
      exit(2);
    }
  }

  if (latex.length() > 0) {
    if (latex.find(".") == string::npos
	|| latex.length() < 4
	|| latex.substr(latex.length()-4) != ".tex")
      latex += ".tex";
    latexfile.open(latex);
    if (!latexfile.is_open()){
      cout << "+++ Cannot open latex file " << latex << endl;
      exit(2);
    }
  }

  if (offset < 0 && print != pDIMACS) {
    cout << "+++ WARNING: offset reset to 0" << endl;
    offset = 0;
  } else if (offset <= 0 && print == pDIMACS) {
    cout << "+++ WARNING: offset reset to 1" << endl;
    offset = 1;
  }

  if (print == pVOID)
    print = (closure <= clDHORN) ? pMIX : pCLAUSE;
  if (strategy == sEXACT && setcover) {
    cout << "+++ WARNING: set cover abandoned" << endl;
    setcover = false;
  }
}

void print_arg () {
  cout << "@@@ Parameters:" << endl;
  cout << "@@@ ===========" << endl;
  cout << "@@@ version       = " << version << endl;
  cout << "@@@ input         = " << input << endl;
  cout << "@@@ header        = " << headerput << endl;
  if (direction == dPREC)
    cout << "@@@ prec. weights = " << weights << endl;
  cout << "@@@ output        = " << output << endl;
  cout << "@@@ latex output  = "
       << (latex.length() > 0 ? latex : "no")
       << endl;
  // cout << "@@@ clustering    = "
  //      << (cluster == SENTINEL ? "no" : "yes, epsilon = " + to_string(cluster))
  //      << endl;
  cout << "@@@ action        = " << action_strg[action] << endl;
  cout << "@@@ closure       = " << closure_strg[closure] << endl;
  cout << "@@@ direction     = " << direction_strg[direction] << endl;
  cout << "@@@ strategy      = " << strategy_strg[strategy] << endl;
  cout << "@@@ cooking       = " << cooking_strg[cooking] << endl;
  cout << "@@@ set cover     = " << (setcover ? "yes" : "no") << endl;
  cout << "@@@ var. offset   = " << offset << endl;
  cout << "@@@ print matrix  = " << display_strg[display]
       << (display == yUNDEF ? " (will be changed)" : "") << endl;
  cout << "@@@ print formula = " << print_strg[print] << endl;
  cout << "@@@ out   formula = " << (formula_output.empty() ? "none" : formula_output) << endl;
  cout << "@@@ debug         = " << (debug ? "yes" : "no") << endl << endl;
}

void read_header () {
  if (headerput.empty())
    varswitch = false;
  else {
    headerfile.open(headerput);
    if (! headerfile.is_open()) {
      cerr << "+++ Cannot open header file " << headerput << endl
	   << "... Continue with fake variable names" << endl;
      varswitch = false;
      return;
    }

    cout << "+++ Own names for variables" << endl;
    varswitch = true;

    string line;
    while(getline(headerfile, line)) {
      const vector<string> &hdl = split(line, ":");
      varnames.push_back(hdl);
    }
    arity = varnames.size();

    headerfile.close();
  }
}

// reads the input matrices
void read_matrix (Group_of_Matrix &matrix) {

  // vector<string> gqueue;	// queue of group leading indicators
  // Matrix batch;			// stored tuples which will be clustered

  string group;
  int numline = 0;
  string line;
  
  while (getline(cin, line)) {
    numline++;
    // istringstream nums(line);
    // nums >> group;
    // Row temp;
    // int number;
    // while (nums >> number)
    //   temp.push_back(number);
    const vector<string> nums = split(line, " \t,");
    group = nums.at(0);
    Row temp;
    for (size_t i = 1; i < nums.size(); ++i) {
      int number = stoi(nums.at(i));
      temp.push_back(number);
    }
  
    if (arity == 0)
      arity = temp.size();
    else if (arity != temp.size())
      cout << "*** arity discrepancy on line " << numline << endl;

    // if (cluster <= SENTINEL)
      matrix[group].push_back(temp);
    // else {
    //   gqueue.push_back(group);
    //   batch.push_back(temp);
    // }
  }

  if (input != STDIN)
    infile.close();

  // if (cluster > SENTINEL) {
  //   clustering(batch);
  //   numline = batch.size();
  //   for (int i = 0; i < gqueue.size(); ++i)
  //     matrix[gqueue[i]].push_back(batch[i]);
  // }

  if (display == yUNDEF) {
    display = (numline * arity > MTXLIMIT) ? yHIDE : yPEEK;
    cout << "@@@ print matrix  = " << display_strg[display]
       << " (redefined)" << endl;
  }
}

void print_matrix (const Group_of_Matrix &matrix) {
  // prints the matrices
  cout << "+++ Arity = " << arity << endl;
  for (auto group = matrix.cbegin(); group != matrix.cend(); ++group) {
    cout << "+++ Group " << group->first;
    grps.push_back(group->first);
    auto gmtx = group->second;
    cout << " [" << gmtx.size() << "]:" << endl;
    if (display == yPEEK || display == ySHOW)
      cout << gmtx << endl;
  }
  sort(grps.begin(), grps.end());
  cout << "+++ Number of groups = " << grps.size() << endl;

  if (action == aSELECTED) {
    if (find(grps.begin(), grps.end(), selected) == grps.end()) {
      cout << "+++ Selected group " << selected << " does not exist" << endl;
      exit(2);
    } else
      cout << "@@@ selected group   = " << selected << endl;
  }

  cout << endl;
}

unique_ptr<Row> ObsGeq (const Row &a, const Matrix &M) {
  // selects tuples (rows) above the tuple a
  unique_ptr<Row> P;
  for (const Row &row : M)
    if (row >= a)
      P = make_unique<Row>(P == nullptr ? row : Min(*P, row));
  return P;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Formula learnHornLarge (const Matrix &T, const Matrix &F) {
  // learn a Horn clause from positive examples T and negative examples F
  // with the large strategy
  Formula H;

  for (const Row &f : F) {
    Clause clause;			// Clause type is a string
    bool eliminated = false;
    for (size_t i = 0; i < f.size(); ++i)
      clause += f[i] ? lneg : lnone;
    if (satisfied_by(clause, T)) {
      H.push_back(clause);
      eliminated = true;
    } else
      for (size_t i = 0; i < f.size(); ++i)
	if (! f[i]) {
	  clause[i] = lpos;
	  if (satisfied_by(clause, T)) {
	    H.push_back(clause);
	    eliminated = true;
	    break;
	  }
	  clause[i] = lnone;
	}
    if (! eliminated)
      cout << "+++ WARNING: vector " << f << " not elminated" << endl;
  }

  if (debug)
    cerr << "*** before cook in learnHornLarge" << endl;
  cook(H);
  if (debug)
    cerr << "*** after cook" << endl;
  return H;
}

Formula learn2sat (const Matrix &T, const Matrix &F) {
  // learn a bijunctive clause from positive examples T and negative examples F
  Formula B;
  const size_t lngt = T[0].size();
  const Literal literals[] = {lpos, lneg};

  // Put here the production of a bijunctive formula
  // Is it necessary to generate the majority closure?
  // T = MajorityClosure(T);

  if (strategy == sEXACT) {

    if (T.size() == 1) {
      const Row t = T[0];
      for (size_t i = 0; i < lngt; ++i) {
	Clause clause(lngt, lnone);
	clause[i] = t[i] ? lpos : lneg;
	B.push_back(clause);
      }
      return B;
    }

    for (size_t j = 0; j < lngt; ++j) {
      Clause clause(lngt, lnone);
      clause[j] = lpos;
      if (satisfied_by(clause, T))
	B.push_back(clause);
      clause[j] = lneg;
      if (satisfied_by(clause, T))
	B.push_back(clause);
    }

    for (int j1 = 0; j1 < lngt-1; ++j1)
      for (int j2 = j1+1; j2 < lngt; ++j2)
	for (const Literal lit1 : literals) {
	  Clause clause(lngt, lnone);
	  clause[j1] = lit1;
	  for (const Literal lit2 : literals) {
	    clause[j2] = lit2;
	    if (satisfied_by(clause, T))
	      B.push_back(clause);
	  }
	}

    for (const Row &f : F)
      if (sat_formula(f, B))
    	cout << "WARNING: vector " << f
	     << " not elminated from F" << endl;

    B = primality(B, T);
  } else if (strategy == sLARGE) {
    
    for (size_t j = 0; j < lngt; ++j) {
      Clause clause(lngt, lnone);

      clause[j] = lpos;
      if (! satisfied_by(clause, F)
	  && satisfied_by(clause, T))
	B.push_back(clause);

      clause[j] = lneg;
      if (! satisfied_by(clause, F)
	  && satisfied_by(clause, T))
	B.push_back(clause);
    }

    for (int j1 = 0; j1 < lngt-1; ++j1)
      for (int j2 = j1+1; j2 < lngt; ++j2)
	for (const Literal lit1 : literals) {
	  Clause clause(lngt, lnone);
	  clause[j1] = lit1;
	  for (const Literal lit2 : literals) {
	    clause[j2] = lit2;
	    if (! satisfied_by(clause, F)
		&& satisfied_by(clause, T))
	      B.push_back(clause);
	  }
	}
    
    for (const Row &t : T)
      if (! sat_formula(t, B))
    	cout << "WARNING: vector " << t
	     << " does not satisfy the formula" << endl;

  }

  cook(B);
  return B;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Formula post_prod (const vector<size_t> &A, const Matrix &F, const Formula &formula) {
  Formula schf;
  if (setcover) {
    cout << "+++ " << pcl_strg[closure]
	 << " formula before set cover [" << formula.size() << "] =" << endl;
    cout << formula2string(A, formula) << endl;
    if (latex.length() > 0) {
      latexfile << "% " << pcl_strg[closure]
	 << " formula before set cover [" << formula.size() << "] =" << endl;
      latexfile << "\\varphi = " << endl;
      latexfile << formula2latex(A, formula) << endl << endl;
    }
    
    schf = SetCover(F, formula);
    cout << "+++ " << pcl_strg[closure]
	 << " formula after set cover [" << schf.size() << "] ="  << endl;
    cout << formula2string(A, schf) << endl;
    if (latex.length() > 0) {
      latexfile << "% " << pcl_strg[closure]
		<< " formula after set cover [" << schf.size() << "] ="  << endl;
      latexfile << "\\varphi = " << endl;
      latexfile << formula2latex(A, schf) << endl << endl;
    }
  } else {
    cout << "+++ " << pcl_strg[closure]
	 << " formula [" << formula.size() << "] =" << endl;
    cout << formula2string(A, formula) << endl;
    if (latex.length() > 0) {
      latexfile << "% " << pcl_strg[closure]
		<< " formula [" << formula.size() << "] =" << endl;
      latexfile << "\\varphi = " << endl;
      latexfile << formula2latex(A, formula) << endl << endl;
    }    
    schf = formula;
  }

  if (closure == clDHORN) {
    cout << "+++ swapping the formula back to dual Horn" << endl;
    // Formula dschf = polswap_formula(schf);
    schf = polswap_formula(schf);
    cout << "+++ final dual Horn formula [" << schf.size() << "] =" << endl;
    cout << formula2string(A, schf) << endl;
    if (latex.length() > 0) {
      latexfile << "% swapping the formula back to dual Horn" << endl;
      latexfile << "% final dual Horn formula [" << schf.size() << "] =" << endl;
      latexfile << "\\varphi = " << endl;
      latexfile << formula2latex(A, schf) << endl << endl;
    }
  }
  return schf;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void one2one () {
  // one group as positive agains one group of negative examples

  for (size_t i = 0; i < grps.size(); ++i) {
    Matrix T = group_of_matrix[grps[i]];
    for (size_t j = 0; j < grps.size(); ++j) {
      if (j == i) continue;
      Matrix F = group_of_matrix[grps[j]];

      if (closure == clDHORN) {
	cout << "+++ swapping polarity of vectors and treating swapped vectors as Horn" << endl;
	polswap_matrix(T);
	polswap_matrix(F);
      }

      const Row sect = minsect(T, F);
      if (nosection)
	cout << "+++ Groups ";
      else
	cout << "+++ Section of groups ";
      cout << "T=" << grps[i] << " and F=" << grps[j] << ":" << endl;

      if (!disjoint) {
	cout << "+++ Matrices <T> and F are not disjoint, therefore I cannot infer a formula"
	     << endl << endl;
      } else {
	vector<size_t> A;
	if (!nosection) {
	  size_t hw = hamming_weight(sect);
	  cout << "+++ Relevant variables [" << hw << "]: ";
	  for (size_t k = 0; k < sect.size(); ++k)
	    if (sect[k]) {
	      A.push_back(k);
	      if (varswitch) {
		// vector<string> new_names = split(varnames[k], ":");
		cout << varnames[k][nOWN];
	      } else
		cout << varid + to_string(offset+k);
	      cout << " ";
	    }
	  cout << endl;
	  cout << "+++ A [" << hw << "] = {";
	  for (const size_t &coord : A)
	    cout << offset + coord << " ";
	  cout << "}" << endl;
	  T = restrict(sect, T);
	  F = restrict(sect, F);

	  cout << "+++ T|_A [" << T.size() << "]";
	  if (display >= ySECTION) {
	    cout << " = { " << endl;
	    cout << T;
	    cout << "+++ }";
	  }
	  cout << endl;

	  cout << "+++ F|_A [" << F.size() << "]";
	  if (display >= ySECTION) {
	    cout << " = { " << endl;
	    cout << F;
	    cout << "+++ }";
	  }
	  cout << endl;
	}

	Formula formula;
	if (closure == clHORN || closure == clDHORN)
	  formula =  strategy == sEXACT
	    ? learnHornExact(T)
	    : learnHornLarge(T, F);
	else if (closure == clBIJUNCTIVE) {
	  formula = learn2sat(T, F);
	  if (formula.empty()) {
	    cout << "+++ 2SAT formula not possible for this configuration"
		 << endl
		 << endl;
	    continue;
	  }
	} else if (closure == clCNF)
	  formula = strategy == sLARGE
	    ? learnCNFlarge(F)
	    : learnCNFexact(T);

	vector<size_t> names(arity);
	if (nosection)
	  for (size_t nms = 0; nms < arity; ++nms)
	    names[nms] = nms;
	const Formula schf = post_prod(nosection ? names : A, F, formula);
	if (! formula_output.empty())
	  write_formula(grps[i], grps[j], nosection ? names : A, schf);
      }
      disjoint = true;
      cout << endl;
    }
  }
}

void selected2all (const string &grp) {
  // selected group of positive exaples against all other groups together as negative examples

  Matrix T = group_of_matrix[grp];
  Matrix F;
  vector<string> index;
  for (size_t j = 0; j < grps.size(); ++j) {
    if (grps[j] == grp)
      continue;
    F.insert(F.end(),
	     group_of_matrix[grps[j]].begin(), group_of_matrix[grps[j]].end());
    index.push_back(grps[j]);
  }
  sort(index.begin(), index.end());

  if (closure == clDHORN) {
    cout << "+++ swapping polarity of vectors and treating swapped vectors as Horn" << endl;
    polswap_matrix(T);
    polswap_matrix(F);
  }
    
  const Row sect= minsect(T, F);
  if (nosection)
    cout << "+++ Groups ";
  else
    cout << "+++ Section of groups ";
  cout << "T=" << grp << " and F=( ";
  for (const string &coord : index)
    cout << coord << " ";
  cout << "):" << endl;
  
  if (!disjoint)
    cout << "+++ Matrices <T> and F are not disjoint, therefore I cannot infer a formula"
	 << endl << endl;
  else {
    vector<size_t> A;
    if (!nosection) {
      size_t hw = hamming_weight(sect);
      cout << "+++ Relevant variables [" << hw << "]: ";
      for (size_t k = 0; k < sect.size(); ++k)
	if (sect[k]) {
	  A.push_back(k);
	  if (varswitch) {
	    // vector<string> new_names = split(varnames[k], ":");
	    cout << varnames[k][nOWN];
	  } else
	    cout << varid << to_string(offset+k);
	  cout << " ";
	}
      cout << endl;
      cout << "+++ A [" << hw << "] = { ";
      for (const size_t &var : A)
	cout << offset + var << " ";
      cout << "}" << endl;
      T = restrict(sect, T);
      F = restrict(sect, F);

      cout << "+++ T|_A [" << T.size() << "]";
      if (display >= ySECTION) {
	cout << " = { " << endl;
	cout << T;
	cout << "+++ }";
      }
      cout << endl;

      cout << "+++ F|_A [" << F.size() << "]";
      if (display >= ySECTION) {
	cout << " = { " << endl;
	cout << F;
	cout << "+++ }";
      }
      cout << endl;
    }

    Formula formula;
    if (closure == clHORN || closure == clDHORN)
      formula =  strategy == sEXACT
	? learnHornExact(T)
	: learnHornLarge(T, F);
    else if (closure == clBIJUNCTIVE) {
      formula = learn2sat(T, F);
      if (formula.empty()) {
	cout << "+++ 2SAT formula not possible for this configuration"
	     << endl << endl;
	return;
      }
    } else if (closure == clCNF)
      formula = strategy == sLARGE
	? learnCNFlarge(F)
	: learnCNFexact(T);

    vector<size_t> names(arity);
    if (nosection)
      for (size_t nms = 0; nms < arity; ++nms)
	names[nms] = nms;
    const Formula schf = post_prod(nosection ? names : A, F, formula);
    if (! formula_output.empty())
      write_formula(grp, nosection ? names : A, schf);
  }
  disjoint = true;
  cout << endl;
}

void one2all () {
  // one group of positive exaples against all other groups together as negative examples
  
  for (const auto &grp : grps)
    selected2all(grp);
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  version += arch_strg[arch];

  read_arg(argc, argv);
  adjust();
  print_arg();
  read_header();
  read_matrix(group_of_matrix);
  print_matrix(group_of_matrix);

  // start clock
  auto clock_start = chrono::high_resolution_clock::now();

  switch (action) {
  case aONE:
    one2one();
    break;
  case aALL:
    one2all();
    break;
  case aSELECTED:
    selected2all(selected);
    break;
  }

  // stop the clock
  auto clock_stop = chrono::high_resolution_clock::now();
  auto duration = chrono::duration_cast<chrono::milliseconds>(clock_stop - clock_start);
  size_t dtime = duration.count();

  cout << "+++ time = "
       << time2string(dtime)
       << endl;

  cout << "+++ end of run +++" << endl;
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
