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
#include <numeric>
// #include <mutex>

#include "mcp-bucket.hpp"
#include "mcp-common.hpp"
#include "mcp-matrix+formula.hpp"
#include "mcp-parallel.hpp"
#include "mcp-mesh.hpp"

using namespace std;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// adjust input parameters
void adjust () {
  if (!tpath.empty() && tpath.back() != '/')
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

  if (offset <= 0 && print_val == pDIMACS) {
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
  outfile << "@@@ latex output  = " << (latex.length() > 0 ? latex : "no") << endl;
  outfile << "@@@ action        = " << action_strg[action] << endl;
  outfile << "@@@ closure       = " << closure_strg[closure] << endl;
  outfile << "@@@ direction     = " << direction_strg[direction] << endl;
  if (direction == dRAND)
    outfile << "@@@ random seed   = " << random_seed << endl;
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
  outfile << "@@@ out   formula = "
	  << (formula_output.empty() ? "none" : formula_output) << endl;
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
      vector<string> hds = split(line, ":");
      string name = hds[nOWN];
      Token tk = reverse_string.at(hds[1]);
      integer dmax = integer(stoull(hds[2]));
      vector<string> elems;
      move(hds.begin()+3, hds.end(), back_inserter(elems));
      Headline hdl(name, tk, dmax, elems);
      headlines.push_back(hdl);
    }
    arity = headlines.size();

    headerfile.close();
  }
}

// reads the input matrices
void read_matrix (Group_of_Matrix &matrix) {
  string line;

  string group;
  size_t numline = 0;
  while (getline(cin, line)) {
    numline++;
    const vector<string> nums = split(line, " \t,");
    group = nums.at(0);
    if (arity == 0)
      arity = nums.size()-1;
    else if (arity != nums.size()-1)
      outfile << "*** arity discrepancy on line " << numline << endl;
    Row temp;
    for (size_t i = 1; i < nums.size(); ++i) {
      integer x = integer(stoull(nums.at(i)));
      temp.push_back(x);
    }
    if (matrix.find(group) == matrix.end())
      matrix.insert({group, Matrix()});
    matrix[group].add_row(std::move(temp));
  }

  if (input != STDIN)
    infile.close();

  if (display == yUNDEF) {
    display = (numline * arity > MTXLIMIT) ? yHIDE : yPEEK;
    outfile << "@@@ print matrix  = " << display_strg[display] << " (redefined)"
	    << endl;
  }
}

// prints the matrices
void print_matrix (const Group_of_Matrix &matrix) {
  outfile << "+++ Arity = " << arity << endl;
  // for (auto group = matrix.cbegin(); group != matrix.cend(); ++group) {
  for (const auto &group : matrix) {
    outfile << "+++ Group " << group.first;
    grps.push_back(group.first);
    const Matrix &gmtx = group.second;
    outfile << " [" << gmtx.num_rows() << "]:" << endl;
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

// selects rows of matrix above the input row, computes their minimum
// then checks wether the row is equal to the minimum
template <typename R, typename M>
bool InHornClosure(const R &row, const M &matrix) {
  if (matrix.empty())
    return false;
  Row MIN;
  bool any_above = false;
  for (size_t i = 0; i < matrix.num_rows(); ++i) {
    const R &r = matrix[i];
    if (r >= row) {
      if (any_above)
        MIN.inplace_minimum(r);
      else {
        MIN = r.to_row();
        any_above = true;
      }
    }
  }
  return any_above && row == MIN;
}

bool InHornClosure(const RowView &r, const MatrixMask &m) {
  return InHornClosure<RowView, MatrixMask>(r, m);
}
bool InHornClosure(const Row &r, const Matrix &m) {
  return InHornClosure<Row, Matrix>(r, m);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// learn a Horn clause from positive samples T and negative samples F
// with the large strategy
Formula learnHornLarge (ofstream &p_outfile,
			const Matrix &positiveT, const Matrix &negativeF,
			const vector<size_t> &A) {
  Formula varphi;
  size_t arity = positiveT.num_cols();

  vector<size_t> names(arity);
  for (size_t i = 0; i < arity; ++i)
    names[i] = i;

  for (size_t i = 0; i < negativeF.num_rows(); ++i) {
    const Row &f = negativeF[i];
    if (!sat_formula(f, varphi)) {
      continue;
    }

    Clause clause;
    clause.reserve(arity);
    for (size_t i = 0; i < arity; ++i) {
      Sign sign = f[i] > 0 ? lneg : lnone;
      integer val = f[i] > 0 ? f[i] - 1 : 0;
      Literal lit(sign, 0, val);
      clause.push_back(lit);
    }
    bool found = sat_clause(positiveT, clause);
    size_t j = 0;
    Literal old;
    while (!found && j < arity) {
      if (f[j] < headlines[A[j]].DMAX) {
	old = clause[j];
	clause[j].sign = (Sign)(clause[j].sign | lpos);
	clause[j].pval = f[j] + 1;
	found = sat_clause(positiveT, clause);
	if (!found)
	  clause[j] = old;
      }
      j++;
    }
    if (!found) {
      p_outfile << "+++ negative example present in Horn closure of T" << endl;
      p_outfile << "+++ the negative culprit is '" << f << "'" << endl;
      exit(2);
    }
    varphi.push_back(std::move(clause));
  }

  cook(varphi);
  return varphi;
}

// learn a bijunctive clause from positive samples T and negative samples F
Formula learn2sat (ofstream &p_outfile, const Matrix &positiveT, const Matrix &negativeF) {
  bucket::Bucket bucket;
  mesh::Strip strip;
  mesh::Mesh mesh;

  const size_t arity = positiveT.num_cols();
  mesh::init(mesh, arity);
  mesh::init(strip, arity);
  mesh::populate(positiveT, strip, mesh, arity);

  for (size_t k = 0; k < negativeF.num_rows(); ++k) {
    const Row &f = negativeF[k];

    if (!(bucket::sat_bucket(f, bucket)))
      continue;

    bucket::Clause c;
    bool eliminated = false;
    size_t i = 0;

    while (!eliminated && i < arity) {
      c = mesh::isolation({f[i], f[i]}, {i, i}, mesh::SE);
      // Test if f[i] is in strip[i] and valid
      if (strip[i].count(f[i]) == 0 && bucket::valid(c))
        eliminated = true;

      size_t j = i + 1;
      while (!eliminated && j < arity) {
        const bucket::Point f2 = {f[i], f[j]};
        const std::array<size_t, 2> ij = {i, j};
        // Test if f2 can be isolated by trying all directions

        mesh::Direction q = mesh::NW;
        while (!eliminated && q <= mesh::SW) {
          if (mesh[i][j].isolated(f2, q)) {
            c = mesh::isolation(f2, ij, q);
            eliminated = bucket::valid(c);
          }
          q = mesh::Direction(q + 1);
        }
        j++;
      }

      if (eliminated)
        bucket::insert(c, bucket);
      i++;
    }
    if (!eliminated) {
      p_outfile << "+++ 2SAT formula not possible:    No isolated point" << endl;
      // exit(2);
      const Formula empty_formula;
      return empty_formula;
    }
  }

  Formula B = get_formula(bucket, arity);
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
    if (latex.length() > 0) {
      latex_outfile << "% " << pcl_strg[closure] << " formula before set cover ["
                << formula.size() << "] =" << endl;
      latex_outfile << "\\varphi = " << endl;
      latex_outfile << formula2latex(A, formula) << endl << endl;
    }

    schf = SetCover(F, formula);
    process_outfile << "+++ " << pcl_strg[closure]
	 << " formula after set cover [" << schf.size() << "] ="  << endl;
    process_outfile << formula2string(A, schf) << endl;
  } else {
    process_outfile << "+++ " << pcl_strg[closure]
	 << " formula [" << formula.size() << "] =" << endl;
    process_outfile << formula2string(A, formula) << endl;
    if (latex.length() > 0) {
      latex_outfile << "% " << pcl_strg[closure] << " formula [" << formula.size()
		    << "] =" << endl;
      latex_outfile << "\\varphi = " << endl;
      latex_outfile << formula2latex(A, formula) << endl << endl;
    }
    schf = formula;
  }

  if (closure == clDHORN) {
    process_outfile << "+++ swapping the formula back to dual Horn" << endl;
    polswap_formula(schf);
    process_outfile << "+++ final dual Horn formula [" << schf.size() << "] =" << endl;
    process_outfile << formula2string(A, schf) << endl;
    if (latex.length() > 0) {
      latex_outfile << "% swapping the formula back to dual Horn" << endl;
      latex_outfile << "% final dual Horn formula [" << schf.size()
		    << "] =" << endl;
      latex_outfile << "\\varphi = " << endl;
      latex_outfile << formula2latex(A, schf) << endl << endl;
    }
  }
  return schf;
}

Formula post_prod(ofstream &process_outfile, ofstream &latex_outfile,
		  const Matrix &F, const Formula &formula) {
  vector<size_t> names(formula[0].size());
  for (size_t i = 0; i < formula[0].size(); ++i)
    // names.push_back(i);
    names[i] = i;

  return post_prod(process_outfile, latex_outfile, names, F, formula);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// one group as positive agains one group of negative samples
void one2one (ofstream &process_outfile, ofstream &latex_outfile, const int &i) {
  Matrix T = group_of_matrix.at(grps[i]).clone();
  for (size_t j = 0; j < grps.size(); ++j) {
    if (j == i)
      continue;
    Matrix F = group_of_matrix.at(grps[j]).clone();

    if (closure == clDHORN) {
      process_outfile << "+++ swapping polarity of vectors and treating swapped vectors as Horn"
		      << endl;
      polswap_matrix(T);
      polswap_matrix(F);
    }

    Mask sect = minsect(T, F);
    if (nosection)
      process_outfile << "+++ Groups ";
    else
      process_outfile << "+++ Section of groups ";
    process_outfile << "T=" << grps[i] << " and F=" << grps[j] << ":" << endl;
    
    if (!disjoint) {
      process_outfile << "+++ Matrices <T> and F are not disjoint, therefore I cannot infer a formula"
		      << endl << endl;
    } else {
      vector<size_t> A;
      if (!nosection) {
	// int hw = hamming_weight(sect);
	int hw = std::accumulate(sect.cbegin(), sect.cend(), 0);
	process_outfile << "+++ Relevant variables [" << hw << "]: ";
	for (size_t k = 0; k < sect.size(); ++k)
	  if (sect[k]) {
	    A.push_back(k);
	    if (varswitch) {
	      process_outfile << headlines[k].name;
	    } else
	      process_outfile << varid << to_string(offset+k);
	    process_outfile << " ";
	  }
	process_outfile << endl;
	process_outfile << "+++ A [" << hw << "] = {";
	for (const size_t &coord : A)
	  process_outfile << offset + coord << " ";
	process_outfile << "}" << endl;
	T.restrict(sect);
	F.restrict(sect);

	process_outfile << "+++ T|_A [" << T.num_rows() << "]";
	if (display >= ySECTION) {
	  process_outfile << " = { " << endl;
	  process_outfile << T;
	  process_outfile << "+++ }";
	}
	process_outfile << endl;

	process_outfile << "+++ F|_A [" << F.num_rows() << "]";
	if (display >= ySECTION) {
	  process_outfile << " = { " << endl;
	  process_outfile << F;
	  process_outfile << "+++ }";
	}
	process_outfile << endl;
      }

      Formula formula;
      if (closure == clHORN || closure == clDHORN)
	formula =
	  strategy == sEXACT ? learnHornExact(T, A) : learnHornLarge(process_outfile, T, F, A);
      else if (closure == clBIJUNCTIVE) {
	formula = learn2sat(process_outfile, T, F);
	if (formula.empty()) {
	  process_outfile << "+++ 2SAT formula not possible for this configuration" << endl << endl;
	  continue;
	}
      } else if (closure == clCNF)
	formula = strategy == sLARGE ? learnCNFlarge(F, A) : learnCNFexact(T);

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
}

// selected group of positive samples against all other groups together as negative samples
void selected2all (ofstream &process_outfile, ofstream &latex_outfile, const int &i) {
  Matrix T = group_of_matrix.at(grps[i]).clone();
  Matrix F;
  vector<string> index;
  for (size_t j = 0; j < grps.size(); ++j) {
    if (j == i)
      continue;
    F.reserve(F.num_rows() + group_of_matrix[grps[j]].num_rows());
    for (size_t i = 0; i < group_of_matrix[grps[j]].num_rows(); ++i)
      F.add_row(group_of_matrix[grps[j]][i].clone());
    // F.insert(F.end(), group_of_matrix[grps[j]].begin(), group_of_matrix[grps[j]].end());
    index.push_back(grps[j]);
  }
  sort(index.begin(), index.end());

  if (closure == clDHORN) {
    process_outfile << "+++ swapping polarity of vectors and treating swapped vectors as Horn" << endl;
    polswap_matrix(T);
    polswap_matrix(F);
  }
    
  Mask sect= minsect(T, F);
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
      // int hw = hamming_weight(sect);
      int hw = std::accumulate(sect.cbegin(), sect.cend(), 0);
      process_outfile << "+++ Relevant variables [" << hw << "]: ";
      for (size_t k = 0; k < sect.size(); ++k)
	if (sect[k]) {
	  A.push_back(k);
	  if (varswitch) {
	    process_outfile << headlines[k].name;
	  } else
	    process_outfile << varid << to_string(offset+k);
	  process_outfile << " ";
	}
      process_outfile << endl;
      process_outfile << "+++ A [" << hw << "] = { ";
      for (const size_t &var : A)
	process_outfile << offset + var << " ";
      process_outfile << "}" << endl;
      T.restrict(sect);
      F.restrict(sect);

      process_outfile << "+++ T|_A [" << T.num_rows() << "," << T.num_cols() << "]";
      if (display >= ySECTION) {
	process_outfile << " = { " << endl;
	process_outfile << T;
	process_outfile << "+++ }";
      }
      process_outfile << endl;

      process_outfile << "+++ F|_A [" << F.num_rows() << "," << F.num_cols() << "]";
      if (display >= ySECTION) {
	process_outfile << " = { " << endl;
	process_outfile << F;
	process_outfile << "+++ }";
      }
      process_outfile << endl;
    }

    Formula formula;
    if (closure == clHORN || closure == clDHORN)
      formula =
	strategy == sEXACT ? learnHornExact(T, A) : learnHornLarge(process_outfile, T, F, A);
    else if (closure == clBIJUNCTIVE) {
      formula = learn2sat(process_outfile, T, F);
      if (formula.empty()) {
	process_outfile << "+++ 2SAT formula not possible for this configuration" << endl << endl;
	return;
      }
    } else if (closure == clCNF)
      formula = strategy == sLARGE ? learnCNFlarge(F, A) : learnCNFexact(T);

    vector<size_t> names(arity);
    if (nosection)
      for (size_t nms = 0; nms < arity; ++nms)
	names[nms] = nms;
    Formula schf = post_prod(process_outfile, latex_outfile,
			     nosection ? names : A, F, formula);
    if (! formula_output.empty())
      write_formula(grps[i], nosection ? names : A, schf);
  }
  disjoint = true;
  process_outfile << endl;
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

void erase_tmp () {
  const string temp_prefix = tpath + "mcp-tmp-";
  const string erase = "rm -f " + temp_prefix + "*.txt";
  int syserr = system(erase.c_str());
}

// terminal handler: we erase the temporary files in case of a crash
void crash (int signal) {
  erase_tmp();
  cerr << endl << "\t*** Segmentation fault ***" << endl << endl;
  exit(signal);
}

// terminal handler: we erase the temporary files in case of an interrupt
void interrupt (int signal) {
  erase_tmp();
  cerr << endl << "\t*** Interrupt ***" << endl << endl;
  exit(signal);
}
