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
 *	Version: all parallel                                             *
 *      File:    mcp-parallel.cpp                                         *
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
#include <sstream>
#include <fstream>
#include <queue>
#include <algorithm>
#include <cmath>
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

  if (offset < 0 && print != pDIMACS) {
    outfile << "+++ WARNING: offset reset to 0" << endl;
    offset = 0;
  } else if (offset <= 0 && print == pDIMACS) {
    outfile << "+++ WARNING: offset reset to 1" << endl;
    offset = 1;
  }

  if (print == pVOID)
    print = (closure <= clDHORN) ? pMIX : pCLAUSE;
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
  outfile << "@@@ output        = " << output << endl;
  outfile << "@@@ latex output  = "
	  << (latex.length() > 0 ? latex : "no")
	  << endl;
  outfile << "@@@ clustering    = "
	  << (cluster == SENTINEL ? "no" : "yes, epsilon = " + to_string(cluster))
	  << endl;
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
  outfile << "@@@ print formula = " << print_strg[print] << endl;
  outfile << "@@@ out   formula = " << (formula_output.empty() ? "none" : formula_output) << endl;
  outfile << "@@@ tmp path      = " << tpath << endl;
  outfile << "@@@ debug         = " << (debug ? "yes" : "no") << endl << endl;
}

void clustering(Matrix &batch) {
  Row cl_flag (batch[0].size(), false);
  string new_varid = "z";

  outfile << "+++ Original arity = " << arity << endl;
  outfile << "+++ Clustering with epsilon = "
	  << to_string(cluster)
	  << endl;

  Matrix tr_batch = transpose(batch);
  Matrix new_tr;
  queue<int> cl_q;
  int cl_pointer = 0;
  int number_of_clusters = 0;

  if (varswitch)
    outfile << "+++ Own variable names canceled by clustering" << endl;
  outfile << "+++ Correspondence table with centers and their distance from average" << endl;
  
  for (int i = 0; i < cl_flag.size(); ++i)
    if (cl_flag[i] == false) {
      cl_q.push(i);
      cl_flag[i] = true;
      vector<int> cl_bag;
      cl_bag.push_back(i);

      vector<int> center;
      for (int j = 0; j < tr_batch[i].size(); ++j)
	center.push_back(tr_batch[i][j]);
      int cl_card = 1;

      while (! cl_q.empty()) {
	int index = cl_q.front();
	cl_q.pop();

	for (int j = i+1; j < cl_flag.size(); ++j)
	  if (cl_flag[j] == false
	      &&
	      (hamming_distance(tr_batch[j], tr_batch[index]) <= cluster)) {
	    cl_q.push(j);
	    cl_flag[j] = true;
	    cl_card++;
	    cl_bag.push_back(j);
	    for (int k = 0; k < tr_batch[i].size(); ++k)
	      center[k] += tr_batch[j][k];
	  }
      }
      
      Row false_center;
      for (int j = 0; j < center.size(); ++j)
	false_center.push_back(center[j] / cl_card + 0.5 < 1.0 ? false : true);
      int hdistance = tr_batch.size()+1;
      int cindex;
      Row true_center = false_center;
      for (int j : cl_bag) {
	int hd = hamming_distance(false_center, tr_batch[j]);
	if (hd < hdistance) {
	  hdistance = hd;
	  true_center = tr_batch[j];
	  cindex = j;
	}
      }
      new_tr.push_back(true_center);

      sort(cl_bag.begin(), cl_bag.end());
      outfile << "\t{ ";
      int cl_ctr= 0;
      for (int num : cl_bag) {
	if (++cl_ctr % CLUSTERLIMIT == 0)
	  outfile << endl << "\t  ";
	if (varswitch)
	  outfile << varnames[num] + " ";
	else
	  outfile << varid + to_string(offset + num) + " ";
      }
      outfile << "} ["
	      << to_string(cl_card)
	      << "] ";
      if (cl_bag.size() > 1)
	outfile << "\n\t\t";
      outfile << "\t-> " << new_varid << to_string(offset + number_of_clusters++);
      if (cl_bag.size() > 1) {
	vector<int> ham_dist;
	float mean_dist = 0.0;
	for (int j : cl_bag) {
	  int hd = hamming_distance(true_center, tr_batch[j]);
	  ham_dist.push_back(hd);
	  mean_dist += hd;
	}
	mean_dist /= cl_card;

	float std_dev = 0.0;
	for (int hd : ham_dist)
	  std_dev += (mean_dist - hd) * (mean_dist - hd);
	std_dev = sqrt(std_dev / cl_card);
	
	outfile << "\t(center = "
		<< (varswitch ? varnames[cindex] : varid + to_string(cindex))
		<< ", delta = " << to_string(hdistance)
		<< ", mean distance = " << to_string(mean_dist)
		<< ", std.dev. = " << to_string(std_dev)
		<< ")";
      }
      outfile << endl;
    }

  outfile << "+++ Number of clusters = "
	  << to_string(number_of_clusters)
	  << endl;

  batch = transpose(new_tr);
  arity = batch[0].size();
  varid = new_varid;
  varswitch = false;
}

void read_matrix (Group_of_Matrix &matrix) {
  // reads the input matrices
  int ind_a, ind_b;
  string line;

  getline(cin, line);
  istringstream inds(line);
  inds >> ind_a >> ind_b;
  outfile << "+++ Indication line: " << ind_a << " " << ind_b << endl;

  if (ind_a == 1) {
    outfile << "+++ Own names for variables" << endl;
    varswitch = true;

    getline(cin, line);
    istringstream vars(line);
    string vname;
    while (vars >> vname)
      varnames.push_back(vname);
    arity = varnames.size();
  }
  if (ind_b == 1)
    getline(cin, line);

  vector<string> gqueue;	// queue of group leading indicators
  Matrix batch;		// stored tuples which will be clustered

  string group;
  int numline = 0;
  while (getline(cin, line)) {
    numline++;
    istringstream nums(line);
    nums >> group;
    Row temp;
    int number;
    while (nums >> number) {
      temp.push_back(number);
    }
    if (arity == 0)
      arity = temp.size();
    else if (arity != temp.size())
      outfile << "*** arity discrepancy on line " << numline << endl;

    if (cluster <= SENTINEL)
      matrix[group].push_back(temp);
    else {
      gqueue.push_back(group);
      batch.push_back(temp);
    }
  }

  if (input != STDIN)
    infile.close();

  if (cluster > SENTINEL) {
    clustering(batch);
    numline = batch.size();
    for (int i = 0; i < gqueue.size(); ++i)
      matrix[gqueue[i]].push_back(batch[i]);
  }

  if (display == yUNDEF) {
    display = (numline * arity > MTXLIMIT) ? yHIDE : yPEEK;
    outfile << "@@@ print matrix  = " << display_strg[display]
	    << " (redefined)" << endl;
  }
}

void print_matrix (const Group_of_Matrix &matrix) {
  // prints the matrices
  outfile << "+++ Arity = " << arity << endl;
  for (auto group = matrix.begin(); group != matrix.end(); ++group) {
    outfile << "+++ Group " << group->first;
    grps.push_back(group->first);
    auto gmtx = group->second;
    outfile << " [" << gmtx.size() << "]:" << endl;
    if (display == yPEEK || display == ySHOW) {
      for (auto i = gmtx.begin(); i != gmtx.end(); ++i) {
	for (auto j = i->begin(); j != i->end(); ++j)
	  // outfile << *j << " ";
	  outfile << *j;
	outfile << endl;
      }
      outfile << endl;
    }
  }
  sort(grps.begin(), grps.end());
  outfile << "+++ Number of groups = " << grps.size() << endl;
}

Formula learnHornLarge (ofstream &process_outfile, const Matrix &T, Matrix F) {
  // learn a Horn clause from positive examples T and negative examples F
  // with the large strategy
  Formula H;

  // if (T.size() == 1) {		// T has only one vector
  //   Row t = T.front();
  //   Clause clause;
  //   for (int i = 0; i < t.size(); ++i) {
  //     if (t[i] == true)
  // 	clause.push_back(lpos);
  //     else if (t[i] == false)
  // 	clause.push_back(lneg);
  //     else
  // 	clause.push_back(lnone);
  //     H.push_back(clause);
  //   }
  //   return H;
  // }

  Row tmin = minHorn(T);
  for (int i = 0; i < tmin.size(); ++i)
    if (tmin[i] == true) {
      Clause clause(tmin.size(), lnone);
      clause[i] = lpos;
      H.push_back(clause);
    }

  Matrix newF;
  while (! F.empty()) {
    Row ff = F.front();
    bool insert = true;
    F.pop_front();
    for (int i = 0; i < ff.size(); ++i)
      if (tmin[i] == true && ff[i] == false) {
	insert = false;
	break;
      }
    if (insert == true)
      newF.push_back(ff);
  }
  F = newF;

  for (Row f : F) {
    Clause clause;
    bool eliminated = false;
    for (int i = 0; i < f.size(); ++i)
      if (f[i] == true)
	clause.push_back(lneg);
      else
	clause.push_back(lnone);
    if (satisfied_by(clause, T)) {
      H.push_back(clause);
      eliminated = true;
    }
    else
      for (int i = 0; i < f.size(); ++i)
	if (f[i] == false) {
	  clause[i] = lpos;
	  if (satisfied_by(clause, T)) {
	    H.push_back(clause);
	    eliminated = true;
	  }
	  clause[i] = lnone;
	}
    if (eliminated == false)
      process_outfile << "+++ WARNING: vector " << f << " not elminated" << endl;
  }

  cook(H);
  return H;
}

Formula learnBijunctive (ofstream &process_outfile, const Matrix &T, const Matrix &F) {
  // learn a bijunctive clause from positive examples T and negative examples F
  Formula B;
  const int lngt = T[0].size();
  Literal literals[] = {lpos, lneg};

  if (T.size() == 1) {
    Row t = T[0];
    for (int i = 0; i < lngt; ++i) {
      Clause clause(lngt, lnone);
      clause[i] = t[i] == true ? lpos : lneg;
      B.push_back(clause);
    }
    return B;
  }

  // Put here the production of a bijunctive formula
  // Is it necessary to generate the majority closure?
  // T = MajorityClosure(T);

  if (strategy == sEXACT) {

    for (int j = 0; j < lngt; ++j) {
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
	for (Literal lit1 : literals) {
	  Clause clause(lngt, lnone);
	  clause[j1] = lit1;
	  for (Literal lit2 : literals) {
	    clause[j2] = lit2;
	    if (satisfied_by(clause, T))
	      B.push_back(clause);
	  }
	}

    for (Row f : F)
      if (sat_formula(f, B))
    	process_outfile << "WARNING: vector " << f
			<< " not elminated from F" << endl;

    B = primality(B, T);
  } else if (strategy == sLARGE) {
    
    for (int j = 0; j < lngt; ++j) {
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
	for (Literal lit1 : literals) {
	  Clause clause(lngt, lnone);
	  clause[j1] = lit1;
	  for (Literal lit2 : literals) {
	    clause[j2] = lit2;
	    if (! satisfied_by(clause, F)
		&& satisfied_by(clause, T))
	      B.push_back(clause);
	  }
	}
    
    for (Row t : T)
      if (! sat_formula(t, B))
    	process_outfile << "WARNING: vector " << t
			<< " does not satisfy the formula" << endl;

  }

  cook(B);
  return B;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Formula post_prod(ofstream &process_outfile, ofstream &latex_outfile,
	       const vector<int> &A, const Matrix &F, const Formula &formula) {
  Formula schf;
  if (setcover == true) {
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
  vector<int> names;
  for (int i = 0; i < formula[0].size(); ++i)
    names.push_back(i);

  return post_prod(process_outfile, latex_outfile, names, F, formula);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OneToOne (ofstream &process_outfile, ofstream &latex_outfile, const int &i) {
  // one group as positive agains one group of negative examples

  // for (int i = 0; i < grps.size(); ++i) {
  Matrix T = group_of_matrix[grps[i]];
  for (int j = 0; j < grps.size(); ++j) {
    if (j == i) continue;
    Matrix F = group_of_matrix[grps[j]];

    if (closure == clDHORN) {
      process_outfile << "+++ swapping polarity of vectors and treating swapped vectors as Horn" << endl;
      T = polswap_matrix(T);
      F = polswap_matrix(F);
    }

    Row sect = minsect(T, F);
    process_outfile << "+++ Section of groups T=" << grps[i] << " and F=" << grps[j] << ":" << endl;
    // process_outfile << "    " << sect << endl;
    if (!disjoint) {
      process_outfile << "+++ Matrices <T> and F are not disjoint, therefore I cannot infer a formula"
		      << endl << endl;
    } else {
      int hw = hamming_weight(sect);
      process_outfile << "+++ Relevant variables [" << hw << "]: ";
      vector<int> A;
      for (int k = 0; k < sect.size(); ++k)
	if (sect[k] == true) {
	  A.push_back(k);
	  if (varswitch) {
	    vector<string> new_names = split(varnames[k], ':');
	    process_outfile << new_names[nOWN];
	  } else
	    process_outfile << varid << to_string(offset+k);
	  process_outfile << " ";
	}
      process_outfile << endl;
      process_outfile << "+++ A [" << hw << "] = {";
      for (int coord : A)
	process_outfile << offset+coord << " ";
      process_outfile << "}" << endl;
      Matrix TsectA = restrict(sect, T);
      Matrix FsectA = restrict(sect, F);

      if (display >= ySECTION) {
	process_outfile << "+++ T|_A [" << TsectA.size() << "] = { " << endl;
	process_outfile << TsectA;
	process_outfile << "+++ }" << endl;
	
	process_outfile << "+++ F|_A [" << FsectA.size() << "] = { " << endl;
	process_outfile << FsectA;
	process_outfile << "+++ }" << endl;
      } else {
	process_outfile << "+++ T|_A [" << TsectA.size() << "]" << endl;
	process_outfile << "+++ F|_A [" << FsectA.size() << "]" << endl;
      }

      Matrix HC;
      if (closure == clHORN || closure == clDHORN) {
	HC = HornClosure(TsectA);
	process_outfile << "+++ " << pcl_strg[closure] << "Closure(T|_A) [" << HC.size() << "]";
	if (display >= ySECTION) {
	  process_outfile << " = { " << endl;
	  process_outfile << HC;
	  process_outfile << "+++ }";
	}
	process_outfile << endl;
      } else if (closure == clBIJUNCTIVE)
	process_outfile << "+++ " << pcl_strg[closure] << "Closure not computed" << endl;

      Formula formula;
      if (closure == clHORN || closure == clDHORN)
	formula =  strategy == sEXACT
	  ? learnHornExact(HC)
	  : learnHornLarge(process_outfile, TsectA, FsectA);
      else if (closure == clBIJUNCTIVE) {
	formula = learnBijunctive(process_outfile, TsectA, FsectA);
	if (formula.empty()) {
	  process_outfile << "+++ bijunctive formula not possible for this configuration" << endl << endl;
	  continue;
	}
      }
      else if (closure == clCNF)
	formula = strategy == sLARGE
	  ? learnCNFlarge(FsectA)
	  : learnCNFexact(TsectA);

      Formula schf = post_prod(process_outfile, latex_outfile, A, FsectA, formula);
      if (! formula_output.empty())
	write_formula(grps[i], grps[j], A, schf);
    }
    disjoint = true;
    process_outfile << endl;
  }
  // }
}

void OneToAll (ofstream &process_outfile, ofstream &latex_outfile, const int &i) {
  // one group of positive exaples against all other groups together as negative examples
  
  // for (int i = 0; i < grps.size(); ++i) {
  Matrix T = group_of_matrix[grps[i]];
  Matrix F;
  vector<string> index;
  for (int j = 0; j < grps.size(); ++j) {
    if (j == i) continue;
    F.insert(F.end(), group_of_matrix[grps[j]].begin(), group_of_matrix[grps[j]].end());
    index.push_back(grps[j]);
  }
  sort(index.begin(), index.end());

  if (closure == clDHORN) {
    process_outfile << "+++ swapping polarity of vectors and treating swapped vectors as Horn" << endl;
    T = polswap_matrix(T);
    F = polswap_matrix(F);
  }
    
  Row sect= minsect(T, F);
  process_outfile << "+++ Section of groups T=" << grps[i] << " and F=( ";
  for (string coord : index)
    process_outfile << coord << " ";
  process_outfile << "):" << endl;
  // process_outfile << "    " << sect << endl;
  if (!disjoint) {
    process_outfile << "+++ Matrices <T> and F are not disjoint, therefore I cannot infer a formula"
		    << endl << endl;
  } else {
    int hw = hamming_weight(sect);
    process_outfile << "+++ Relevant variables [" << hw << "]: ";
    vector<int> A;
    for (int k = 0; k < sect.size(); ++k)
      if (sect[k]) {
	A.push_back(k);
	if (varswitch) {
	  vector<string> new_names = split(varnames[k], ':');
	  process_outfile << new_names[nOWN];
	} else
	  process_outfile << varid << to_string(offset+k);
	process_outfile << " ";
      }
    process_outfile << endl;
    process_outfile << "+++ A [" << hw << "] = { ";
    for (int var : A)
      process_outfile << offset+var << " ";
    process_outfile << "}" << endl;
    Matrix TsectA = restrict(sect, T);
    Matrix FsectA = restrict(sect, F);

    if (display >= ySECTION) {
      process_outfile << "+++ T|_A [" << TsectA.size() << "] = { " << endl;
      process_outfile << TsectA;
      process_outfile << "+++ }" << endl;
      
      process_outfile << "+++ F|_A [" << FsectA.size() << "] = { " << endl;
      process_outfile << FsectA;
      process_outfile << "+++ }" << endl;
    } else {
      process_outfile << "+++ T|_A [" << TsectA.size() << "]" << endl;
      process_outfile << "+++ F|_A [" << FsectA.size() << "]" << endl;
    }

    Matrix HC;
    if (closure == clHORN || closure == clDHORN) {
      HC = HornClosure(TsectA);
      process_outfile << "+++ " << pcl_strg[closure] << "Closure(T|_A) [" << HC.size() << "]";
      if (display >= ySECTION) {
	process_outfile << " = { " << endl;
	process_outfile << HC;
	process_outfile << "+++ }";
      }
      process_outfile << endl;
    } else if (closure == clBIJUNCTIVE)
      process_outfile << "+++ " << pcl_strg[closure] << "Closure not computed" << endl;

    Formula formula;
    if (closure == clHORN || closure == clDHORN)
      formula =  strategy == sEXACT
	? learnHornExact(HC)
	: learnHornLarge(process_outfile, TsectA, FsectA);
    else if (closure == clBIJUNCTIVE) {
      formula = learnBijunctive(process_outfile, TsectA, FsectA);
      if (formula.empty()) {
	process_outfile << "+++ bijunctive formula not possible for this configuration" << endl << endl;
	disjoint = true;
	return;
      }
    }
    else if (closure == clCNF)
      formula = strategy == sLARGE
	? learnCNFlarge(FsectA)
	: learnCNFexact(TsectA);

    Formula schf = post_prod(process_outfile, latex_outfile, A, FsectA, formula);
    if (! formula_output.empty())
      write_formula(grps[i], A, schf);
  }
  disjoint = true;
  process_outfile << endl;
  // }
}

void OneToAllNosection (ofstream &process_outfile, ofstream &latex_outfile, const int &i) {
  // one group of positive exaples against all other groups together as negative examples
  // no section is done

  vector<int> names(arity);
  for (int nms = 0; nms < arity; ++nms)
    names[nms] = nms;
  
  // for (int i = 0; i < grps.size(); ++i) {
  Matrix T = group_of_matrix[grps[i]];
  Matrix F;
  vector<string> index;
  for (int j = 0; j < grps.size(); ++j) {
    if (j == i) continue;
    F.insert(F.end(), group_of_matrix[grps[j]].begin(), group_of_matrix[grps[j]].end());
    index.push_back(grps[j]);
  }
  sort(index.begin(), index.end());

  if (closure == clDHORN) {
    process_outfile << "+++ swapping polarity of vectors and treating swapped vectors as Horn" << endl;
    T = polswap_matrix(T);
    F = polswap_matrix(F);
  }

  process_outfile << "+++ Groups T=" << grps[i] << " and F=( ";
  for (string coord : index)
    process_outfile << coord << " ";
  process_outfile << "):" << endl;

  if (display == ySHOW) {
    process_outfile << "+++ T [" << T.size() << "] = { " << endl;
    process_outfile << T;
    process_outfile << "+++ }" << endl;

    process_outfile << "+++ F [" << F.size() << "] = { " << endl;
    process_outfile << F;
    process_outfile << "+++ }" << endl;
  } else {
    process_outfile << "+++ T [" << T.size() << "]" << endl;
    process_outfile << "+++ F [" << F.size() << "]" << endl;
  }

  Matrix HC;
  if (closure == clHORN || closure == clDHORN) {
    HC = HornClosure(T);
    process_outfile << "+++ " << pcl_strg[closure] << "Closure(T) [" << HC.size() << "]";
    if (display == ySHOW) {
      process_outfile << " = { " << endl;
      process_outfile << HC;
      process_outfile << "+++ }";
    }
    process_outfile << endl;
  } else if (closure == clBIJUNCTIVE)
    process_outfile << "+++ " << pcl_strg[closure] << "Closure not computed" << endl;
    
  if (inadmissible(T,F)) {
    process_outfile << "*** F not disjoint from <T>" << endl;
    disjoint = false;
    process_outfile << "+++ Matrices <T> and F are not disjoint, therefore I cannot infer a formula"
		    << endl << endl;
  }

  if (disjoint) {
    Formula formula;
    if (closure == clHORN || closure == clDHORN)
      formula =  strategy == sEXACT
	? learnHornExact(HC)
	: learnHornLarge(process_outfile, T, F);
    else if (closure == clBIJUNCTIVE) {
      formula = learnBijunctive(process_outfile, T, F);
      if (formula.empty()) {
	process_outfile << "+++ bijunctive formula not possible for this configuration" << endl << endl;
	disjoint = true;
	return;
      }
    }
    else if (closure == clCNF)
      formula = strategy == sLARGE
	? learnCNFlarge(F)
	: learnCNFexact(T);

    Formula schf = post_prod(process_outfile, latex_outfile, F, formula);
    if (! formula_output.empty())
      write_formula(grps[i], names, schf);
  }
  disjoint = true;
  process_outfile << endl;
  // }
}

void split_action (ofstream &popr, ofstream &latpr, const int &process_id) {
  switch (action) {
  case aONE:
    OneToOne (popr, latpr, process_id);
    break;
  case aALL:
    OneToAll (popr, latpr, process_id);
    break;
  case aNOSECT:
    OneToAllNosection (popr, latpr, process_id);
    break;
  }
}

void crash() {
  // terminal handler: we erase the temporary files in case of a crash
  const string temp_prefix = tpath + "mcp-tmp-";
  const string erase = "rm -f " + temp_prefix + "*.txt";
  int syserr = system(erase.c_str());

  throw;
}
