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
 *	Version: all                                                      *
 *      File:    mcp-check.cpp                                            *
 *                                                                        *
 *      Copyright (c) 2019 - 2024                                         *
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
#include <iomanip>
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
string headerput;
string formula_input;
ifstream infile;
ifstream form_in;
ifstream headerfile;
ofstream outfile;

Formula formula;

vector<int> names;
// string suffix;
// int arity;
// int nvars;
// int offset;

int tp = 0;		// true positive
int tn = 0;		// true negative
int fp = 0;		// false postitive
int fn = 0;		// false negative
double tpr = RSNTNL;	// true positive rate aka sensitivity aka recall
double tnr = RSNTNL;	// true negative rate aka specificity aka selectivity
double ppv = RSNTNL;	// positive predictive value aka precision
double npv = RSNTNL;	// negative predictive value
double fnr = RSNTNL;	// false negative rate aka miss rate
double fpr = RSNTNL;	// false positive rate aka fall-out
double fdr = RSNTNL;	// false discovery rate
double forate = RSNTNL;	// false omission rate
double pt = RSNTNL;	// prevalence treshhold
double csi = RSNTNL;	// critical success index aka threat score
double acc = RSNTNL;	// accuracy
double ba = RSNTNL;	// balanced accuracy
double f1score = RSNTNL;// F1 score


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void read_arg (int argc, char *argv[]) {	// reads the input parameters
  int argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "--input"
	|| arg == "-i") {
      input = argv[++argument];
    } else if (arg == "-hdr"
	       || arg == "--header") {
      headerput = argv[++argument];
    } else if (arg == "--output"
    	       || arg == "-o") {
      output = argv[++argument];
    } else if (arg == "--formula"
	       || arg == "--logic"
	       || arg == "--log"
	       || arg == "-l") {
      formula_input = argv[++argument];
    } else if (arg == "--pr"
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

  if (print == pVOID)
    print = pMIX;
}

void print_arg () {
  cout << "@@@ Parameters:" << endl;
  cout << "@@@ ===========" << endl;
  cout << "@@@ version       = " << version << endl;
  cout << "@@@ input         = " << input << endl;
  cout << "@@@ header        = " << headerput << endl;
  cout << "@@@ output        = " << output << endl;
  cout << "@@@ var. offset   = " << offset << endl;
  cout << "@@@ print matrix  = " << display_strg[display]
       << (display == yUNDEF ? " (will be changed)" : "") << endl;
  cout << "@@@ print formula = " << print_strg[print] << endl;
  cout << "@@@ formula input = " << (formula_input.empty() ? "none" : formula_input) << endl;
  cout << endl;
}

void read_header () {
  streambuf *backup;

  if (headerput.empty())
    varswitch = false;
  else {
    headerfile.open(headerput);
    if (headerfile.is_open()) {
      backup = cin.rdbuf();
      cin.rdbuf(headerfile.rdbuf());
    } else {
      cerr << "+++ Cannot open header file " << headerput << endl
	   << "... Continue with fake variable names" << endl;
      varswitch = false;
      return;
    }

    cout << "+++ Own names for variables" << endl;
    varswitch = true;

    string line;
    while(getline(cin, line))
      varnames.push_back(line);
    // arity = varnames.size();

    headerfile.close();
    cin.rdbuf(backup);
  }
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

  string line;

  // moved into read_header and changed
  // int ind_a, ind_b;
  // getline(cin, line);
  // istringstream inds(line);
  // inds >> ind_a >> ind_b;
  // cout << "+++ Indication line: " << ind_a << " " << ind_b << endl;

  // if (ind_a == 1) {
  //   cout << "+++ Own names for variables" << endl;
  //   varswitch = true;

  //   getline(cin, line);
  //   istringstream vars(line);
  //   string vname;
  //   while (vars >> vname)
  //     varnames.push_back(vname);
  // }
  // if (ind_b == 1)
  //   getline(cin, line);

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
  for (auto group = matrix.cbegin(); group != matrix.cend(); ++group) {
    cout << "+++ Group " << group->first;
    grps.push_back(group->first);
    Matrix gmtx = group->second;
    cout << " [" << gmtx.size() << "]:" << endl;
    if (display == yPEEK || display == ySHOW)
      cout << gmtx << endl;
  }
  sort(grps.begin(), grps.end());
  cout << "+++ Number of groups = " << grps.size() << endl;

  cout << endl;
  cout << "+++ Satisfying: " << suffix << endl;
  cout << "+++ Falsifying:";
  for (auto group : grps)
    if (group != suffix)
      cout << " " << group;
  cout << endl << endl;
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
    tpr = (1.0 * tp) / (1.0*tp + 1.0*fn);
    fnr = 1.0 - tpr;
  }
  if (tn+fp != 0) {
    tnr = (1.0 * tn) / (1.0*tn + 1.0*fp);
    fpr = 1.0 * fp / (1.0*fp + 1.0*tn);
  }
  if (tp+fp != 0)
    ppv = (1.0 * tp) / (1.0*tp + 1.0*fp);
  if (tp+tn+fp+fn != 0)
    acc = 1.0 * (tp + tn) / (1.0*tp + 1.0*tn + 1.0*fp + 1.0*fn);
  if (tpr+ppv != 0.0)
    f1score = (1.0 * tp) / (1.0*tp + 0.5*fp + 0.5*fn);
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
  cout << left << setw(7);
  if (tpr < 0.0)
    cout << "---";
  else
    cout << tpr * 100.0 << " %";
  cout << right << "\t [tp / (tp + fn)]" << endl;
  
  cout << "+++ miss rate      (fnr) = ";
  cout << left << setw(7);
  if (fnr < 0.0)
    cout << "---";
  else
    cout << fnr * 100.0 << " %";
  cout << right << "\t [fn / (fn + tp)]" << endl;

  cout << "+++ fall-out       (fnr) = ";
  cout << left << setw(7);
  if (fpr < 0.0)
    cout << "---";
  else
    cout << fpr * 100.0 << " %";
  cout << right << "\t [fp / (fp + tn)]" << endl;
    
  cout << "+++ specificity    (tnr) = ";
  cout << left << setw(7);
  if (tnr < 0.0)
    cout << "---";
  else
    cout << tnr * 100.0 << " %";
  cout << right << "\t [tn / (tn + fp)]" << endl;
  
  cout << "+++ precision      (ppv) = ";
  cout << left << setw(7);
  if (ppv < 0.0)
    cout << "---";
  else
    cout << ppv * 100.0 << " %";
  cout << right << "\t [tp / (tp + fp)]" << endl;
  
  cout << "+++ accuracy       (acc) = ";
  cout << left << setw(7);
  if (acc < 0.0)
    cout << "---";
  else
    cout << acc * 100.0 << " %";
  cout << right << "\t [(tp + tn) / (tp + tn + fp +fn)]" << endl;
  
  cout << "+++ F_1 score      (F_1) = ";
  cout << left << setw(7);
  if (f1score < 0.0)
    cout << "---";
  else
    cout << f1score * 100.0 << " %";
  cout << right << "\t [tp / (tp + 0.5 * (fp +fn))]" << endl;

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
  read_header();
  read_matrix(group_of_matrix);
  print_matrix(group_of_matrix);
  print_formula(names, formula);
  sat_test(group_of_matrix, formula);
  print_result();
}

//////////////////////////////////////////////////////////////////////////////
