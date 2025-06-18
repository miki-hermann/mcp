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
 *	Version: all parallel                                             *
 *      File:    mcp-parallel.cpp                                         *
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
#include <csignal>
#include "mcp-matrix+formula.hpp"
#include "mcp-common.hpp"
#include "mcp-parallel.hpp"

using namespace std;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void adjust () {				// adjust input parameters
  if (!tpath.empty() && tpath[tpath.size()-1] != '/')
    tpath += '/';

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

  if (output == STDOUT) {
    cout << "+++ Cannot write to STDOUT because of parallel processes" << endl;
    cout << "+++ Specify an output file" << endl;
    exit(1);
  } else {
    outfile.open(output);
    if (! outfile.is_open()) {
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
      cerr << "+++ Cannot open latex file " << latex << endl;
      exit(2);
    }
  }

  if (offset < 0 && print_val != pDIMACS) {
    outfile << "+++ WARNING: offset reset to 0" << endl;
    offset = 0;
  } else if (offset <= 0 && print_val == pDIMACS) {
    outfile << "+++ WARNING: offset reset to 1" << endl;
    offset = 1;
  }

  if (print_val == pVOID)
    print_val = (closure <= clDHORN) ? pMIX : pCLAUSE;
  if (strategy == sEXACT && setcover) {
    outfile << "+++ WARNING: set cover abandoned" << endl;
    setcover = false;
  }
}

void print_arg () {
  outfile << "@@@ Parameters:" << endl;
  outfile << "@@@ ===========" << endl;
  outfile << "@@@ version       = " << version << endl;
  outfile << "@@@ input         = " << input << endl;
  outfile << "@@@ header        = " << headerput << endl;
  if (direction == dPREC)
    outfile << "@@@ prec. weights = " << weights << endl;
  outfile << "@@@ output        = " << output << endl;
  outfile << "@@@ latex output  = "
	  << (latex.length() > 0 ? latex : "no")
	  << endl;
  // outfile << "@@@ clustering    = "
  // 	  << (cluster == SENTINEL ? "no" : "yes, epsilon = " + to_string(cluster))
  // 	  << endl;
  outfile << "@@@ action        = " << action_strg[action] << endl;
  outfile << "@@@ closure       = " << closure_strg[closure] << endl;
  outfile << "@@@ direction     = " << direction_strg[direction] << endl;
  outfile << "@@@ strategy      = " << strategy_strg[strategy] << endl;
  outfile << "@@@ cooking       = " << cooking_strg[cooking] << endl;
  outfile << "@@@ set cover     = " << (setcover ? "yes" : "no") << endl;
  outfile << "@@@ var. offset   = " << offset << endl;
  if (arch != archMPI)
    outfile << "@@@ chunk limit   = " << chunkLIMIT << endl;
  if (arch != archPTHREAD)
    outfile << "@@@ proc.num. fit = " << (np_fit ? "yes" : "no") << endl;
  outfile << "@@@ print matrix  = " << display_strg[display]
	  << (display == yUNDEF ? " (will be changed)" : "") << endl;
  outfile << "@@@ print formula = " << print_strg[print_val] << endl;
  outfile << "@@@ out   formula = " << (formula_output.empty() ? "none" : formula_output) << endl;
  outfile << "@@@ tmp path      = " << tpath << endl;
  outfile << "@@@ debug         = " << (debug ? "yes" : "no") << endl << endl;
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

    outfile << "+++ Own names for variables" << endl;
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
  string line;

  // vector<string> gqueue;	// queue of group leading indicators
  // Matrix batch;		// stored tuples which will be clustered

  string group;
  int numline = 0;
  while (getline(cin, line)) {
    numline++;
    Row temp = read_row(line, group);
    if (arity == 0)
      arity = temp.size();
    else if (arity != temp.size())
      outfile << "*** arity discrepancy on line " << numline << endl;

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
    outfile << "@@@ print matrix  = " << display_strg[display]
	    << " (redefined)" << endl;
  }
}

void print_matrix (const Group_of_Matrix &matrix) {
  // prints the matrices
  outfile << "+++ Arity = " << arity << endl;
  for (auto group = matrix.cbegin(); group != matrix.cend(); ++group) {
    outfile << "+++ Group " << group->first;
    grps.push_back(group->first);
    Matrix gmtx = group->second;
    outfile << " [" << gmtx.size() << "]:" << endl;
    if (display == yPEEK || display == ySHOW)
      outfile << gmtx << endl;
  }
  sort(grps.begin(), grps.end());
  outfile << "+++ Number of groups = " << grps.size() << endl;
  
  if (action == aSELECTED) {
    if (find(grps.begin(), grps.end(), selected) == grps.end()) {
      outfile << "+++ Selected group " << selected << " does not exist" << endl;
      exit(2);
    } else
      outfile << "@@@ selected group   = " << selected << endl;
  }

  outfile << endl;
}

// learn a Horn clause from positive examples T and negative examples F
// with the large strategy
Formula learnHornLarge (ofstream &process_outfile,
			const Matrix &T, const Matrix &F) {
  Formula H;

  for (const Row &f : F) {
    Clause clause;
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
      process_outfile << "+++ WARNING: vector " << f << " not elminated" << endl;
  }

  cook(H);
  return H;
}

Formula learn2sat (ofstream &process_outfile, const Matrix &T, const Matrix &F) {
  // learn a bijunctive clause from positive examples T and negative examples F
  Formula B;
  const size_t lngt = T[0].size();
  const Literal literals[] = {lpos, lneg};

  // Put here the production of a bijunctive formula
  // Is it necessary to generate the majority closure?
  // T = MajorityClosure(T);

  if (strategy == sEXACT) {

    if (T.size() == 1) {
      Row t = T[0];
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
    	process_outfile << "WARNING: vector " << f
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
    	process_outfile << "WARNING: vector " << t
			<< " does not satisfy the formula" << endl;

  }

  cook(B);
  return B;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Formula post_prod(ofstream &process_outfile, ofstream &latex_outfile,
	       const vector<size_t> &A, const Matrix &F, const Formula &formula) {
  Formula schf;
  if (setcover) {
    process_outfile << "+++ " << pcl_strg[closure]
	 << " formula before set cover [" << formula.size() << "] =" << endl;
    process_outfile << formula2string(A, formula) << endl;
    schf = SetCover(F, formula);
    process_outfile << "+++ " << pcl_strg[closure]
	 << " formula after set cover [" << schf.size() << "] ="  << endl;
    process_outfile << formula2string(A, schf) << endl;
  } else {
    process_outfile << "+++ " << pcl_strg[closure]
	 << " formula [" << formula.size() << "] =" << endl;
    process_outfile << formula2string(A, formula) << endl;
    schf = formula;
  }

  if (closure == clDHORN) {
    process_outfile << "+++ swapping the formula back to dual Horn" << endl;
    // Formula dschf = polswap_formula(schf);
    schf = polswap_formula(schf);
    process_outfile << "+++ final dual Horn formula [" << schf.size() << "] =" << endl;
    process_outfile << formula2string(A, schf) << endl;
  }

  return schf;
}

Formula post_prod(ofstream &process_outfile, ofstream &latex_outfile,
		  const Matrix &F, const Formula &formula) {
  vector<size_t> names;
  for (size_t i = 0; i < formula[0].size(); ++i)
    names.push_back(i);

  return post_prod(process_outfile, latex_outfile, names, F, formula);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void one2one (ofstream &process_outfile, ofstream &latex_outfile, const int &i) {
  // one group as positive agains one group of negative examples

  Matrix T = group_of_matrix[grps[i]];
  for (size_t j = 0; j < grps.size(); ++j) {
    if (j == i) continue;
    Matrix F = group_of_matrix[grps[j]];

    if (closure == clDHORN) {
      process_outfile << "+++ swapping polarity of vectors and treating swapped vectors as Horn" << endl;
      polswap_matrix(T);
      polswap_matrix(F);
    }

    Row sect = minsect(T, F);
    if (nosection)
      process_outfile << "+++ Groups ";
    else
      process_outfile << "+++ Section of groups ";
    process_outfile << "T=" << grps[i] << " and F=" << grps[j] << ":" << endl;
    // process_outfile << "    " << sect << endl;
    
    if (!disjoint) {
      process_outfile << "+++ Matrices <T> and F are not disjoint, therefore I cannot infer a formula"
		      << endl << endl;
    } else {
      vector<size_t> A;
      if (!nosection) {
	size_t hw = hamming_weight(sect);
	process_outfile << "+++ Relevant variables [" << hw << "]: ";
	for (size_t k = 0; k < sect.size(); ++k)
	  if (sect[k]) {
	    A.push_back(k);
	    if (varswitch) {
	      // vector<string> new_names = split(varnames[k], ":");
	      process_outfile << varnames[k][nOWN];
	    } else
	      process_outfile << varid << to_string(offset+k);
	    process_outfile << " ";
	  }
	process_outfile << endl;
	process_outfile << "+++ A [" << hw << "] = {";
	for (const size_t &coord : A)
	  process_outfile << offset + coord << " ";
	process_outfile << "}" << endl;
	T = restrict(sect, T);
	F = restrict(sect, F);

	process_outfile << "+++ T|_A [" << T.size() << "]";
	if (display >= ySECTION) {
	  process_outfile << " = { " << endl;
	  process_outfile << T;
	  process_outfile << "+++ }";
	}
	process_outfile << endl;

	process_outfile << "+++ F|_A [" << F.size() << "]";
	if (display >= ySECTION) {
	  process_outfile << " = { " << endl;
	  process_outfile << F;
	  process_outfile << "+++ }";
	}
	process_outfile << endl;
      }

      Formula formula;
      if (closure == clHORN || closure == clDHORN)
	formula =  strategy == sEXACT
	  ? learnHornExact(T)
	  : learnHornLarge(process_outfile, T, F);
      else if (closure == clBIJUNCTIVE) {
	formula = learn2sat(process_outfile, T, F);
	if (formula.empty()) {
	  process_outfile << "+++ 2SAT formula not possible for this configuration" << endl << endl;
	  continue;
	}
      }
      else if (closure == clCNF)
	formula = strategy == sLARGE
	  ? learnCNFlarge(F)
	  : learnCNFexact(T);

      vector<size_t> names(arity);
      if (nosection)
	for (size_t nms = 0; nms < arity; ++nms)
	  names[nms] = nms;
      Formula schf = post_prod(process_outfile, latex_outfile,
			       nosection ? names : A, F, formula);
      if (! formula_output.empty())
	write_formula(grps[i], grps[j], nosection ? names : A, schf);
    }
    disjoint = true;
    process_outfile << endl;
  }
  // }
}

void selected2all (ofstream &process_outfile, ofstream &latex_outfile, const int &i) {
  // selected group of positive exaples against all other groups together as negative examples
  
  Matrix T = group_of_matrix[grps[i]];
  Matrix F;
  vector<string> index;
  for (size_t j = 0; j < grps.size(); ++j) {
    if (j == i) continue;
    F.insert(F.end(), group_of_matrix[grps[j]].begin(), group_of_matrix[grps[j]].end());
    index.push_back(grps[j]);
  }
  sort(index.begin(), index.end());

  if (closure == clDHORN) {
    process_outfile << "+++ swapping polarity of vectors and treating swapped vectors as Horn" << endl;
    polswap_matrix(T);
    polswap_matrix(F);
  }
    
  const Row sect = minsect(T, F);
  if (nosection)
    process_outfile << "+++ Groups ";
  else
    process_outfile << "+++ Section of groups ";
  process_outfile << "T=" << grps[i] << " and F=( ";
  for (const string &coord : index)
    process_outfile << coord << " ";
  process_outfile << "):" << endl;

  if (!disjoint) {
    process_outfile << "+++ Matrices <T> and F are not disjoint, therefore I cannot infer a formula"
		    << endl << endl;
  } else {
    vector<size_t> A;
    if (!nosection) {
      size_t hw = hamming_weight(sect);
      process_outfile << "+++ Relevant variables [" << hw << "]: ";
      for (size_t k = 0; k < sect.size(); ++k)
	if (sect[k]) {
	  A.push_back(k);
	  if (varswitch) {
	    // vector<string> new_names = split(varnames[k], ":");
	    process_outfile << varnames[k][nOWN];
	  } else
	    process_outfile << varid << to_string(offset+k);
	  process_outfile << " ";
	}
      process_outfile << endl;
      process_outfile << "+++ A [" << hw << "] = { ";
      for (size_t var : A)
	process_outfile << offset + var << " ";
      process_outfile << "}" << endl;
      T = restrict(sect, T);
      F = restrict(sect, F);

      process_outfile << "+++ T|_A [" << T.size() << "]";
      if (display >= ySECTION) {
	process_outfile << " = { " << endl;
	process_outfile << T;
	process_outfile << "+++ }";
      }
      process_outfile << endl;

      process_outfile << "+++ F|_A [" << F.size() << "]";
      if (display >= ySECTION) {
	process_outfile << " = { " << endl;
	process_outfile << F;
	process_outfile << "+++ }";
      }
      process_outfile << endl;
    }

    Formula formula;
    if (closure == clHORN || closure == clDHORN)
      formula =  strategy == sEXACT
	? learnHornExact(T)
	: learnHornLarge(process_outfile, T, F);
    else if (closure == clBIJUNCTIVE) {
      formula = learn2sat(process_outfile, T, F);
      if (formula.empty()) {
	process_outfile << "+++ 2SAT formula not possible for this configuration" << endl << endl;
	disjoint = true;
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
    const Formula schf = post_prod(process_outfile, latex_outfile,
			     nosection ? names : A, F, formula);
    if (! formula_output.empty())
      write_formula(grps[i], nosection ? names : A, schf);
  }
  disjoint = true;
  process_outfile << endl;
  // }
}

void split_action (ofstream &popr, ofstream &latpr, const int &process_id) {
  switch (action) {
  case aONE:
    one2one(popr, latpr, process_id);
    break;
  case aALL:
    selected2all(popr, latpr, process_id);
    break;
  case aSELECTED:
    cerr << endl;
    cerr << "*** no need to run the selected action in parallel" << endl;
    cerr << "*** use mcp-seq to run this example" << endl;
    exit(2);
    break;
  }
}

// terminal handler: we erase the temporary files in case of a crash
void crash (int signal) {
  const string temp_prefix = tpath + "mcp-tmp-";
  const string erase = "rm -f " + temp_prefix + "*.txt";
  int syserr = system(erase.c_str());

  cerr << endl << "\t*** Segmentation fault ***" << endl << endl;
  exit(signal);
}

// terminal handler: we erase the temporary files in case of an interrupt
void interrupt (int signal) {
  const string temp_prefix = tpath + "mcp-tmp-";
  const string erase = "rm -f " + temp_prefix + "*.txt";
  int syserr = system(erase.c_str());

  cerr << endl << "\t*** Interrupt ***" << endl << endl;
  exit(signal);
}
