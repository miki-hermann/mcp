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
 *	Version: all                                                      *
 *      File:    mcp-check.cpp                                            *
 *                                                                        *
 *      Copyright (c) 2019 - 2021                                         *
 *                                                                        *
 * Given a  formula and  a set  of boolean  vectors, check  which vectors *
 * satisfy  and which  falsify the  given formula.  Report precision  and *
 * recall (https://en.wikipedia.org/wiki/Precision_and_recall).           *
 *                                                                        *
 * This software has been created within the ACCA Project.                *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "mcp-matrix+formula.hpp"

using namespace std;

const string STDIN  = "STDIN";
const string STDOUT = "STDOUT";

string input  = STDIN;
string output = STDOUT;
string formula_input;
ifstream infile;
ifstream form_in;
ofstream outfile;

Formula formula;

vector<int> names;
string suffix;
// int arity;
int nvars;
// int offset;

int tp = 0;		// true positive
int tn = 0;		// true negative
int fp = 0;		// false postitive
int fn = 0;		// false negative
double tpr = -1.0;	// true positive rate aka sensitivity aka recall
double tnr = -1.0;	// true negative rate aka specificity aka selectivity
double ppv = -1.0;	// positive predictive value aka precision
double npv = -1.0;	// negative predictive value
double fnr = -1.0;	// false negative rate aka miss rate
double fpr = -1.0;	// false positive rate aka fall-out
double fdr = -1.0;	// false discovery rate
double forate = -1.0;	// false omission rate
double pt = -1.0;	// prevalence treshhold
double csi = -1.0;	// critical success index aka threat score
double accuracy = -1.0;	// accuracy
double ba = -1.0;	// balanced accuracy
double f1score = -1.0;	// F1 score


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void read_arg (int argc, char *argv[]) {	// reads the input parameters
  int argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "--input"
	|| arg == "-i") {
      input = argv[++argument];
    } else if (arg == "--output"
    	       || arg == "-o") {
      output = argv[++argument];
    } else if (arg == "--formula"
	       || arg == "--logic"
	       || arg == "--log"
	       || arg == "-l") {
      formula_input = argv[++argument];
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
    } else
      cerr << "+++ unknown option " << arg << endl;
    ++argument;
  }
}

void adjust_and_open () {
  form_in.open(formula_input);
  if (form_in.is_open())
    cin.rdbuf(form_in.rdbuf());
  else {
    cerr << "+++ Cannot open formula input file " << formula_input << endl;
    exit(2);
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

  if (print == pVOID)
    print = pMIX;
}

void print_arg () {
  cout << "@@@ Parameters:" << endl;
  cout << "@@@ ===========" << endl;
  cout << "@@@ version       = " << version << endl;
  cout << "@@@ input         = " << input << endl;
  cout << "@@@ output        = " << output << endl;
  cout << "@@@ var. offset   = " << offset << endl;
  cout << "@@@ print matrix  = " << display_strg[display]
       << (display == yUNDEF ? " (will be changed)" : "") << endl;
  cout << "@@@ print formula = " << print_strg[print] << endl << endl;
}

void read_formula (vector<int> &names, Formula &formula) {
  // formula read instructions

  cin >> suffix >> arity >> nvars >> offset;

  // cerr << "*** suffix = " << suffix
  //      << ", arity = " << arity
  //      << ", #vars = " << nvars
  //      << ", offset = " << offset
  //      << endl;

  vector<int> validID;
  int dummy;
  for (int i = 0; i < nvars; ++i) {
    cin >> dummy;
    validID.push_back(dummy);
  }

  for (int i = 0; i < arity; ++i)
    names.push_back(i);

  int lit;
  Clause clause(arity, lnone);
  while (cin >> lit)
    if (lit == 0) {			// end of clause in DIMACS
      formula.push_back(clause);
      for (int i = 0; i < arity; ++i)
	clause[i] = lnone;
    } else if (find(cbegin(validID), cend(validID), abs(lit)) == cend(validID)) {
      cerr << "+++ " << abs(lit) << " outside allowed variable names" << endl;
      exit(2);
    } else
      clause[abs(lit)-1-offset] = lit < 0 ? lneg : lpos;
}

void read_matrix (Group_of_Matrix &matrix) {
  if (input != STDIN) {
    infile.open(input);
    if (infile.is_open())
      cin.rdbuf(infile.rdbuf());
    else {
      cerr << "+++ Cannot open input file " << input << endl;
      exit(2);
    }
  }

  // matrix read instructions
  // maybe to be changed and replaced with the one in mcp-seq
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
  }
  if (ind_b == 1)
    getline(cin, line);

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
    if (arity != temp.size())
      cout << "*** arity discrepancy on line " << numline << endl;

    matrix[group].push_back(temp);
  }
  
  if (input != STDIN)
    infile.close();

  if (display == yUNDEF) {
    display = (numline * arity > MTXLIMIT) ? yHIDE : yPEEK;
    cout << "@@@ print matrix  = " << display_strg[display]
       << " (redefined)" << endl;
  }
}

// copied from mcp-seq, but slightly changed (use gmtx.size() instead numline)
// need to do something to have only one copy
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
  cout << "+++ Number of groups = " << grps.size() << endl;

  cout << endl;
  cout << "+++ Satisfying: " << suffix << endl;
  cout << "+++ Falsifying:";
  for (auto group : grps)
    if (group != suffix)
      cout << " " << group;
  cout << endl << endl;;
}

void print_formula (const vector<int> &names, const Formula &formula) {
  string strg_fm = formula2string(names, formula);
  cout << "+++ Formula [" << formula.size() << "] =" << endl;
  cout << strg_fm << endl;
}

void sat_test (const Group_of_Matrix &matrix, const Formula &formula) {
  for (auto group = matrix.begin(); group != matrix.end(); ++group) {
    Matrix gmtx = group->second;
    for (Row row : gmtx)
      if (group->first == suffix) {		
	// must satisfy
	if (sat_formula(row, formula))
	  tp++;
	else
	  fn++;
      } else {
	// must falsify
	if (sat_formula(row, formula))
	  fp++;
	else
	  tn++;
      }
  }
  if (tp+fn != 0) {
    tpr = 1.0 * tp / (1.0*tp + 1.0*fn);
    fnr = 1.0 - tpr;
  }
  if (tn+fp != 0)
    tnr = 1.0 * tn / (1.0*tn + 1.0*fp);
  if (tp+fp != 0)
    ppv = 1.0 * tp / (1.0*tp + 1.0*fp);
}

void print_result () {
  cout << endl;
  cout << "+++ Statistics:" << endl;
  cout << "    ===========" << endl;
  cout << "+++ true  positive (tp)  = " << tp << endl;
  cout << "+++ true  negative (tn)  = " << tn << endl;
  cout << "+++ false positive (fp)  = " << fp << endl;
  cout << "+++ false negative (fn)  = " << fn << endl;
  cout << "+++ sensitivity    (tpr) = ";
  if (tpr < 0.0)
    cout << "---"
	 << "\t [tp / (tp + fn)]"
	 << endl;
  else
    cout << tpr * 100.0 << " %"
	 << "\t [tp / (tp + fn)]"
	 << endl;
  cout << "+++ miss rate      (fnr) = ";
  if (fnr < 0.0)
    cout << "---"
	 << "\t [fn / (fn + tp)]"
	 << endl;
  else
    cout << fnr * 100.0 << " %"
	 << "\t [fn / (fn + tp)]"
	 << endl;
  cout << "+++ specificity    (tnr) = ";
  if (tnr < 0.0)
    cout << "---"
	 << "\t [tn / (tn + fp)]"
	 << endl;
  else
    cout << tnr * 100.0 << " %"
	 << "\t [tn / (tn + fp)]"
	 << endl;
  cout << "+++ precision      (ppv) = ";
  if (ppv < 0.0)
    cout << "---"
	 << "\t [tp / (tp + fp)]"
	 << endl;
  else
    cout << ppv * 100.0 << " %"
	 << "\t [tp / (tp + fp)]"
	 << endl;
  
  form_in.close();
  if (output != STDOUT)
    outfile.close();
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  version += "check";

  read_arg(argc, argv);
  adjust_and_open();
  print_arg();
  read_formula(names, formula);
  read_matrix(group_of_matrix);
  print_matrix(group_of_matrix);
  print_formula(names, formula);
  sat_test(group_of_matrix, formula);
  print_result();
}

//////////////////////////////////////////////////////////////////////////////
