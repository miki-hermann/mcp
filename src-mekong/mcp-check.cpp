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
 *     File:    src-mekong/mcp-check.cpp                                  *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 **************************************************************************/

#include "mcp-matrix+formula.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

const string STDIN = "STDIN";
const string STDOUT = "STDOUT";

string input = STDIN;
string output = STDOUT;
string headerput;
string formula_input;
ifstream infile;
ifstream form_in;
ifstream headerfile;
ofstream outfile;

Formula formula;

vector<size_t> names;
// string suffix;
// int arity;
// int nvars;
// int offset;

int tp = 0;              // true positive
int tn = 0;              // true negative
int fp = 0;              // false postitive
int fn = 0;              // false negative
double tpr = RSNTNL;     // true positive rate aka sensitivity aka recall
double tnr = RSNTNL;     // true negative rate aka specificity aka selectivity
double ppv = RSNTNL;     // positive predictive value aka precision
double npv = RSNTNL;     // negative predictive value
double fnr = RSNTNL;     // false negative rate aka miss rate
double fpr = RSNTNL;     // false positive rate aka fall-out
double fdr = RSNTNL;     // false discovery rate
double forate = RSNTNL;  // false omission rate
double pt = RSNTNL;      // prevalence treshhold
double csi = RSNTNL;     // critical success index aka threat score
double acc = RSNTNL;     // accuracy
double ba = RSNTNL;      // balanced accuracy
double f1score = RSNTNL; // F1 score

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void read_arg (int argc, char *argv[]) {	// reads the input parameters
  int argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "--input"
	|| arg == "-i") {
      if (argument < argc-1) {
	input = argv[++argument];
      } else
	cerr << "+++ no input file selected" << endl;
    } else if (arg == "--hdr"
	       || arg == "--header") {
      if (argument < argc-1) {
	headerput = argv[++argument];
      } else
	cerr << "+++ no header file selected, revert to default" << endl;
    } else if (arg == "--output"
    	       || arg == "-o") {
      if (argument < argc-1) {
	output = argv[++argument];
      } else
	cerr << "+++ no output file selected, revert to default" << endl;
    } else if (arg == "--formula"
	       || arg == "--logic"
	       || arg == "--log"
	       || arg == "-l") {
      if (argument < argc-1) {
	formula_input = argv[++argument];
      } else
	cerr << "+++ no formula file prefix selected, revert to default" << endl;
    } else if (arg == "--pr"
	       || arg == "--print") {
      if (argument < argc-1) {
	string prt = argv[++argument];
	if (prt == "clause"
	    || prt == "clausal"
	    || prt == "cl"
	    || prt == "c") {
	  print_val = pCLAUSE;
	} else if (prt == "implication"
		   || prt == "impl"
		   || prt == "imp"
		   || prt == "im"
		   || prt == "i") {
	  print_val = pIMPL;
	} else if (prt == "mix"
		   || prt == "mixed"
		   || prt == "m") {
	  print_val = pMIX;
	} else if (prt == "dimacs"
		   || prt == "DIMACS") {
	  print_val = pDIMACS;
	} else
	  cerr <<  "+++ unknown print option " << prt << endl;
      } else
	cerr << "+++ no print option selected, revert to default" << endl;
    } else if (arg == "--matrix"
	       || arg == "--mtx"
	       || arg == "-m") {
      if (argument < argc-1) {
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
	cerr << "+++ no matrix printing selected, revert to default" << endl;
    } else
      cerr << "+++ unknown option " << arg << endl;
    ++argument;
  }
}

void adjust_and_open() {
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

  if (print_val == pVOID)
    print_val = pMIX;
}

void print_arg() {
  cout << "@@@ Parameters:" << endl;
  cout << "@@@ ===========" << endl;
  cout << "@@@ version       = " << version << endl;
  cout << "@@@ input         = " << input << endl;
  cout << "@@@ header        = " << headerput << endl;
  cout << "@@@ output        = " << output << endl;
  cout << "@@@ var. offset   = " << offset << endl;
  cout << "@@@ print matrix  = " << display_strg[display]
       << (display == yUNDEF ? " (will be changed)" : "") << endl;
  cout << "@@@ print formula = " << print_strg[print_val] << endl;
  cout << "@@@ formula input = "
       << (formula_input.empty() ? "none" : formula_input) << endl;
  cout << endl;
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
      vector<string> hds = split(line, ":");
      string name = hds[nOWN];
      Token tk = reverse_string.at(hds[1]);
      integer dmax = integer(stoull(hds[2]));
      vector<string> elems;
      move(hds.begin()+3, hds.end(), back_inserter(elems));
      Headline hdl(name, tk, dmax, elems);
      headlines.push_back(hdl);
    }

    headerfile.close();
  }
}

void read_matrix(Group_of_Matrix &matrix) {
  if (input != STDIN) {
    infile.open(input);
    if (infile.is_open())
      cin.rdbuf(infile.rdbuf());
    else {
      cerr << "+++ Cannot open input file " << input << endl;
      exit(2);
    }
  }

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
      cout << "*** arity discrepancy on line " << numline << endl;
    Row temp;
    for (size_t i = 1; i < nums.size(); ++i) {
      integer x = integer(stoull(nums.at(i)));
      temp.push_back(x);
    }
    if (matrix.find(group) == matrix.end()) {
      matrix.insert({group, Matrix()});
    }
    matrix[group].add_row(std::move(temp));
  }

  if (input != STDIN)
    infile.close();

  if (display == yUNDEF) {
    display = (numline * arity > MTXLIMIT) ? yHIDE : yPEEK;
    cout << "@@@ print matrix  = " << display_strg[display] << " (redefined)"
         << endl;
  }
}

// copied from mcp-seq, but slightly changed (use gmtx.size() instead numline)
// need to do something to have only one copy
void print_matrix(const Group_of_Matrix &matrix) {
  // prints the matrices
  cout << "+++ Arity = " << arity << endl;
  for (auto group = matrix.begin(); group != matrix.end(); ++group) {
    cout << "+++ Group " << group->first;
    grps.push_back(group->first);
    const Matrix &gmtx = group->second;
    cout << " [" << gmtx.num_rows() << "]:" << endl;
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

void print_formula(const vector<size_t> &names, const Formula &formula) {
  const string strg_fm = formula2string(names, formula);
  cout << "+++ Formula [" << formula.size() << "] =" << endl;
  cout << strg_fm << endl;
}

void sat_test(const Group_of_Matrix &matrix, const Formula &formula) {
  for (auto group = matrix.begin(); group != matrix.end(); ++group) {
    const Matrix &gmtx = group->second;
    for (size_t i = 0; i < gmtx.num_rows(); ++i) {
      const Row &row = gmtx[i];
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
    if (tp + fn != 0) {
      tpr = (1.0 * tp) / (1.0 * tp + 1.0 * fn);
      fnr = 1.0 - tpr;
    }
    if (tn + fp != 0) {
      tnr = (1.0 * tn) / (1.0 * tn + 1.0 * fp);
      fpr = 1.0 * fp / (1.0 * fp + 1.0 * tn);
    }
    if (tp + fp != 0)
      ppv = (1.0 * tp) / (1.0 * tp + 1.0 * fp);
    if (tp + tn + fp + fn != 0)
      acc = 1.0 * (tp + tn) / (1.0 * tp + 1.0 * tn + 1.0 * fp + 1.0 * fn);
    if (tpr + ppv != 0.0)
      f1score = (1.0 * tp) / (1.0 * tp + 0.5 * fp + 0.5 * fn);
  }
}

inline size_t max (const size_t a, const size_t b) {
  return a >= b ? a : b;
}

void print_result() {
  size_t maxnum = 0;
  maxnum = max(maxnum, tp);
  maxnum = max(maxnum, tn);
  maxnum = max(maxnum, fp);
  maxnum = max(maxnum, fn);

  const size_t perclen = 6;
  const size_t numlen = max(to_string(maxnum).length(), perclen)+1;

  cout << endl;
  cout << "+++ Statistics:" << endl;
  cout << "    ===========" << endl;
  cout << "+++ true  positive (tp)  = " << setw(numlen) << right << tp << endl;
  cout << "+++ true  negative (tn)  = " << setw(numlen) << right << tn << endl;
  cout << "+++ false positive (fp)  = " << setw(numlen) << right << fp << endl;
  cout << "+++ false negative (fn)  = " << setw(numlen) << right << fn << endl;

  cout << "+++ sensitivity    (tpr) = ";
  if (tpr < 0.0)
    cout << " ---.--  ";
  else
    cout << right << setw(perclen+1) << setprecision(2) << fixed << showpoint
	 << tpr * 100.0 << " %";
  cout << right << "\t [tp / (tp + fn)]" << endl;

  cout << "+++ miss rate      (fnr) = ";
  if (fnr < 0.0)
    cout <<  " ---.--  ";
  else
    cout << right << setw(perclen+1) << setprecision(2) << fixed << showpoint
	 << fnr * 100.0 << " %";
  cout << right << "\t [fn / (fn + tp)]" << endl;

  cout << "+++ fall-out       (fnr) = ";
  if (fpr < 0.0)
    cout <<  " ---.--  ";
  else
    cout << right << setw(perclen+1) << setprecision(2) << fixed << showpoint
	 << fpr * 100.0 << " %";
  cout << right << "\t [fp / (fp + tn)]" << endl;

  cout << "+++ specificity    (tnr) = ";
  if (tnr < 0.0)
    cout <<  " ---.--  ";
  else
    cout << right << setw(perclen+1) << setprecision(2) << fixed << showpoint
	 << tnr * 100.0 << " %";
  cout << right << "\t [tn / (tn + fp)]" << endl;

  cout << "+++ precision      (ppv) = ";
  if (ppv < 0.0)
    cout <<  " ---.--  ";
  else
    cout << right << setw(perclen+1) << setprecision(2) << fixed << showpoint
	 << ppv * 100.0 << " %";
  cout << right << "\t [tp / (tp + fp)]" << endl;

  cout << "+++ accuracy       (acc) = ";
  if (acc < 0.0)
    cout <<  " ---.--  ";
  else
    cout << right << setw(perclen+1) << setprecision(2) << fixed << showpoint
	 << acc * 100.0 << " %";
  cout << right << "\t [(tp + tn) / (tp + tn + fp + fn)]" << endl;

  cout << "+++ F_1 score      (F_1) = ";
  if (f1score < 0.0)
    cout <<  " ---.--  ";
  else
    cout << right << setw(perclen+1) << setprecision(2) << fixed << showpoint
	 << f1score * 100.0 << " %";
  cout << right << "\t [tp / (tp + 0.5 * (fp +fn))]" << endl;

  form_in.close();
  if (output != STDOUT)
    outfile.close();
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  version += "check";
  cerr << "+++ version = " << version << endl;

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
