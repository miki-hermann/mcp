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
 *	Version: sequential                                               *
 *      File:    mcp-seq.cpp                                              *
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
      cerr << "+++ Cannot open latex file " << latex << endl;
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
  cout << "@@@ output        = " << output << endl;
  cout << "@@@ latex output  = "
       << (latex.length() > 0 ? latex : "no")
       << endl;
  cout << "@@@ clustering    = "
       << (cluster == SENTINEL ? "no" : "yes, epsilon = " + to_string(cluster))
       << endl;
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

void clustering(Matrix &batch) {
  Row cl_flag (batch[0].size(), false);
  string new_varid = "z";

  cout << "+++ Original arity = " << arity << endl;
  cout << "+++ Clustering with epsilon = "
       << to_string(cluster)
       << endl;

  Matrix tr_batch = transpose(batch);
  Matrix new_tr;
  queue<int> cl_q;
  int cl_pointer = 0;
  int number_of_clusters = 0;

  if (varswitch)
    cout << "+++ Own variable names canceled by clustering" << endl;
  cout << "+++ Correspondence table with centers and their distance from average" << endl;
  
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
      cout << "\t{ ";
      int cl_ctr= 0;
      for (int num : cl_bag) {
	if (++cl_ctr % CLUSTERLIMIT == 0)
	  cout << endl << "\t  ";
	if (varswitch)
	  cout << varnames[num] + " ";
	else
	  cout << varid + to_string(offset + num) + " ";
      }
      cout << "} ["
	   << to_string(cl_card)
	   << "] ";
      if (cl_bag.size() > 1)
	cout << "\n\t\t";
      cout << "\t-> " << new_varid << to_string(offset + number_of_clusters++);
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
	
	cout << "\t(center = "
	     << (varswitch ? varnames[cindex] : varid + to_string(cindex))
	     << ", delta = " << to_string(hdistance)
	     << ", mean distance = " << to_string(mean_dist)
	     << ", std.dev. = " << to_string(std_dev)
	     << ")";
      }
      cout << endl;
    }

  cout << "+++ Number of clusters = "
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
  cout << "+++ Indication line: " << ind_a << " " << ind_b << endl;

  if (ind_a == 1) {
    cout << "+++ Own names for variables" << endl;
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
    while (nums >> number)
      temp.push_back(number);
    if (arity == 0)
      arity = temp.size();
    else if (arity != temp.size())
      cout << "*** arity discrepancy on line " << numline << endl;

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
    cout << "@@@ print matrix  = " << display_strg[display]
       << " (redefined)" << endl;
  }
}

void print_matrix (const Group_of_Matrix &matrix) {
  // prints the matrices
  cout << "+++ Arity = " << arity << endl;
  for (auto group = matrix.begin(); group != matrix.end(); ++group) {
    cout << "+++ Group " << group->first;
    grps.push_back(group->first);
    auto gmtx = group->second;
    cout << " [" << gmtx.size() << "]:" << endl;
    if (display == yPEEK || display == ySHOW) {
      for (auto i = gmtx.begin(); i != gmtx.end(); ++i) {
	for (auto j = i->begin(); j != i->end(); ++j)
	  // cout << *j << " ";
	  cout << *j;
	cout << endl;
      }
      cout << endl;
    }
  }
  sort(grps.begin(), grps.end());
  cout << "+++ Number of groups = " << grps.size() << endl << endl;
}

Matrix ObsGeq (const Row &a, const Matrix &M) {
  // selects tuples (rows) above the tuple a
  Matrix P;
  for (Row row : M)
    if (row >= a)
      P.push_back(row);
  return P;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Formula learnHornLarge (const Matrix &T, Matrix F) {
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
      cout << "+++ WARNING: vector " << f << " not elminated" << endl;
  }

  cook(H);
  return H;
}

Formula learnBijunctive (const Matrix &T, const Matrix &F) {
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
    	cout << "WARNING: vector " << f
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
    	cout << "WARNING: vector " << t
	     << " does not satisfy the formula" << endl;

  }

  cook(B);
  return B;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Formula post_prod(const vector<int> &A, const Matrix &F, const Formula &formula) {
  Formula schf;
  if (setcover == true) {
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

void OneToOne () {
  // one group as positive agains one group of negative examples

  for (int i = 0; i < grps.size(); ++i) {
    Matrix T = group_of_matrix[grps[i]];
    for (int j = 0; j < grps.size(); ++j) {
      if (j == i) continue;
      Matrix F = group_of_matrix[grps[j]];

      if (closure == clDHORN) {
	cout << "+++ swapping polarity of vectors and treating swapped vectors as Horn" << endl;
	T = polswap_matrix(T);
	F = polswap_matrix(F);
      }

      Row sect = minsect(T, F);
      cout << "+++ Section of groups T=" << grps[i] << " and F=" << grps[j] << ":" << endl;
      if (!disjoint) {
	cout << "+++ Matrices <T> and F are not disjoint, therefore I cannot infer a formula"
	     << endl << endl;
      } else {
	int hw = hamming_weight(sect);
	cout << "+++ Relevant variables [" << hw << "]: ";
	vector<int> A;
	for (int k = 0; k < sect.size(); ++k)
	  if (sect[k] == true) {
	    A.push_back(k);
	    if (varswitch) {
	      vector<string> new_names = split(varnames[k], ':');
	      cout << new_names[nOWN];
	    } else
	      cout << varid + to_string(offset+k);
	    cout << " ";
	  }
	cout << endl;
	cout << "+++ A [" << hw << "] = {";
	for (int coord : A)
	  cout << offset+coord << " ";
	cout << "}" << endl;
	Matrix TsectA = restrict(sect, T);
	Matrix FsectA = restrict(sect, F);

	cout << "+++ T|_A [" << TsectA.size() << "]";
	if (display >= ySECTION) {
	  cout << " = { " << endl;
	  cout << TsectA;
	  cout << "+++ }";
	}
	cout << endl;

	cout << "+++ F|_A [" << FsectA.size() << "]";
	if (display >= ySECTION) {
	  cout << " = { " << endl;
	  cout << FsectA;
	  cout << "+++ }";
	}
	cout << endl;

	Matrix HC;
	if (closure == clHORN || closure == clDHORN) {
	  HC = HornClosure(TsectA);
	  cout << "+++ " << pcl_strg[closure] << "Closure(T|_A) [" << HC.size() << "]";
	  if (display >= ySECTION) {
	    cout << " = { " << endl;
	    cout << HC;
	    cout << "+++ }";
	  }
	  cout << endl;
	} else if (closure == clBIJUNCTIVE)
	  cout << "+++ " << pcl_strg[closure] << "Closure not computed" << endl;

	Formula formula;
	if (closure == clHORN || closure == clDHORN)
	  formula =  strategy == sEXACT
	    ? learnHornExact(HC)
	    : learnHornLarge(TsectA, FsectA);
	else if (closure == clBIJUNCTIVE) {
	  formula = learnBijunctive(TsectA, FsectA);
	  if (formula.empty()) {
	    cout << "+++ bijunctive formula not possible for this configuration" << endl << endl;
	    continue;
	  }
	} else if (closure == clCNF)
	  formula = strategy == sLARGE
	    ? learnCNFlarge(FsectA)
	    : learnCNFexact(TsectA);

	Formula schf = post_prod(A, FsectA, formula);
	if (! formula_output.empty())
	  write_formula(grps[i], grps[j], A, schf);
      }
      disjoint = true;
      cout << endl;
    }
  }
}

void OneToAll () {
  // one group of positive exaples against all other groups together as negative examples
  
  for (int i = 0; i < grps.size(); ++i) {
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
      cout << "+++ swapping polarity of vectors and treating swapped vectors as Horn" << endl;
      T = polswap_matrix(T);
      F = polswap_matrix(F);
    }
    
    Row sect= minsect(T, F);
    cout << "+++ Section of groups T=" << grps[i] << " and F=( ";
    for (string coord : index)
      cout << coord << " ";
    cout << "):" << endl;
    if (!disjoint) {
      cout << "+++ Matrices <T> and F are not disjoint, therefore I cannot infer a formula"
	   << endl << endl;
    } else {
      int hw = hamming_weight(sect);
      cout << "+++ Relevant variables [" << hw << "]: ";
      vector<int> A;
      for (int k = 0; k < sect.size(); ++k)
	if (sect[k]) {
	  A.push_back(k);
	  if (varswitch) {
	    vector<string> new_names = split(varnames[k], ':');
	    cout << new_names[nOWN];
	  } else
	    cout << varid << to_string(offset+k);
	  cout << " ";
	}
      cout << endl;
      cout << "+++ A [" << hw << "] = { ";
      for (int var : A)
	cout << offset+var << " ";
      cout << "}" << endl;
      Matrix TsectA = restrict(sect, T);
      Matrix FsectA = restrict(sect, F);

      cout << "+++ T|_A [" << TsectA.size() << "]";
      if (display >= ySECTION) {
	cout << " = { " << endl;
	cout << TsectA;
	cout << "+++ }";
      }
      cout << endl;

      cout << "+++ F|_A [" << FsectA.size() << "]";
      if (display >= ySECTION) {
	cout << " = { " << endl;
	cout << FsectA;
	cout << "+++ }";
      }
      cout << endl;

      Matrix HC;
      if (closure == clHORN || closure == clDHORN) {
	HC = HornClosure(TsectA);
	cout << "+++ " << pcl_strg[closure] << "Closure(T|_A) [" << HC.size() << "]";
	if (display >= ySECTION) {
	  cout << " = { " << endl;
	  cout << HC;
	  cout << "+++ }";
	}
	cout << endl;
      } else if (closure == clBIJUNCTIVE)
	cout << "+++ " << pcl_strg[closure] << "Closure not computed" << endl;

      Formula formula;
      if (closure == clHORN || closure == clDHORN)
	formula =  strategy == sEXACT
	  ? learnHornExact(HC)
	  : learnHornLarge(TsectA, FsectA);
      else if (closure == clBIJUNCTIVE) {
	formula = learnBijunctive(TsectA, FsectA);
	  if (formula.empty()) {
	    cout << "+++ bijunctive formula not possible for this configuration" << endl << endl;
	    continue;
	  }
      }
      else if (closure == clCNF)
	formula = strategy == sLARGE
	  ? learnCNFlarge(FsectA)
	  : learnCNFexact(TsectA);

      Formula schf = post_prod(A, FsectA, formula);
      if (! formula_output.empty())
	write_formula(grps[i], A, schf);
    }
    disjoint = true;
    cout << endl;
  }
}

void OneToAllNosection () {
  // one group of positive exaples against all other groups together as negative examples
  // no section is done

  
  vector<int> names(arity);
  for (int nms = 0; nms < arity; ++nms)
    names[nms] = nms;

  for (int i = 0; i < grps.size(); ++i) {
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
      cout << "+++ swapping polarity of vectors and treating swapped vectors as Horn" << endl;
      T = polswap_matrix(T);
      F = polswap_matrix(F);
    }

    cout << "+++ Groups T=" << grps[i] << " and F=( ";
    for (string coord : index)
      cout << coord << " ";
    cout << "):" << endl;

    if (display == ySHOW) {
      cout << "+++ T [" << T.size() << "] = { " << endl;
      cout << T;
      cout << "+++ }" << endl;

      cout << "+++ F [" << F.size() << "] = { " << endl;
      cout << F;
      cout << "+++ }" << endl;
    } else {
      cout << "+++ T [" << T.size() << "]" << endl;
      cout << "+++ F [" << F.size() << "]" << endl;
    }

    Matrix HC;
    if (closure == clHORN || closure == clDHORN) {
      HC = HornClosure(T);
      cout << "+++ " << pcl_strg[closure] << "Closure(T) [" << HC.size() << "]";
      if (display == ySHOW) {
	cout << " = { " << endl;
	cout << HC;
	cout << "+++ }";
      }
      cout << endl;
    } else if (closure == clBIJUNCTIVE)
      cout << "+++ " << pcl_strg[closure] << "Closure not computed" << endl;
    
    if (inadmissible(T,F)) {
      cout << "*** F not disjoint from <T>" << endl;
      disjoint = false;
      cout << "+++ Matrices <T> and F are not disjoint, therefore I cannot infer a formula"
	   << endl << endl;
    }

    if (disjoint) {
      Formula formula;
      if (closure == clHORN || closure == clDHORN)
	formula =  strategy == sEXACT
	  ? learnHornExact(HC)
	  : learnHornLarge(T, F);
      else if (closure == clBIJUNCTIVE) {
	formula = learnBijunctive(T, F);
	if (formula.empty()) {
	  cout << "+++ bijunctive formula not possible for this configuration"
	       << endl << endl;
	  continue;
	}
      }
      else if (closure == clCNF)
	formula = strategy == sLARGE
	  ? learnCNFlarge(F)
	  : learnCNFexact(T);

      Formula schf = post_prod(names, F, formula);
      if (! formula_output.empty())
	write_formula(grps[i], names, schf);
    }
    disjoint = true;
    cout << endl;
  }
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  time_t start_time = time(nullptr);

  version += arch_strg[arch];

  read_arg(argc, argv);
  adjust();
  print_arg();
  read_matrix(group_of_matrix);
  print_matrix(group_of_matrix);

  switch (action) {
  case aONE:
    OneToOne ();
    break;
  case aALL:
    OneToAll ();
    break;
  case aNOSECT:
    OneToAllNosection ();
    break;
  }

  time_t finish_time = time(nullptr);
  cout << "+++ time = "
       << time2string(difftime(finish_time, start_time))
       << endl;

  cout << "+++ end of run +++" << endl;
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
