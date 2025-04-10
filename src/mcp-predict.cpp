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
 *	Version: all                                                      *
 *      File:    mcp-predict.cpp                                          *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 *  Takes a testing file and optionally a pivot file, produces the        *
 *  prediction. Similar to mcp-check. The difference is that              *
 *  mcp-predict does not have the information which row belongs to        *
 *  which group, but predicts it.                                         *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "mcp-matrix+formula.hpp"

using namespace std;

const string STDIN  = "STDIN";
const string STDOUT = "STDOUT";
const string test_group = "test_group";

string input  = STDIN;
string output = STDOUT;
string headerput = "";
string formula_prefix;
ifstream infile;
ifstream headerfile;
ofstream outfile;
string pivot_file = "";
string predict = "";
ofstream pdxfile;

unordered_map<string, Formula> formula;
vector<size_t> names;
vector<string> pivot;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void read_args(int argc, char *argv[]) {	// reads the input parameters
  int argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "--input"
	|| arg == "-i") {
      input = argv[++argument];
    } else if (arg == "--output"
    	       || arg == "-o") {
      output = argv[++argument];
    } else if (arg == "-hdr"
	       || arg == "--header") {
      headerput = argv[++argument];
    } else if (arg == "--pivot"
	       || arg == "--pvt") {
      pivot_file = argv[++argument];
    } else if (arg == "--formula"
	       || arg == "--logic"
	       || arg == "--log"
	       || arg == "-l") {
      formula_prefix = argv[++argument];
    } else if (arg == "--prediction"
	       || arg == "--predict"
	       || arg == "--pdx") {
      predict = argv[++argument];
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
  if (formula_prefix.empty()) {
    cerr << "+++ Formulas missing" << endl;
    exit(2);
  }

  if (input != STDIN && headerput.empty()) {
    string::size_type pos = input.rfind('.');
    headerput = (pos == string::npos ? input : input.substr(0, pos)) + ".hdr";
  }

  if (input != STDIN && output == STDOUT) {
    string::size_type pos = input.rfind('.');
    output = (pos == string::npos ? input : input.substr(0, pos)) + ".pdt";
  }

  if (output != STDOUT && predict.empty()) {
    string::size_type pos = output.rfind('.');
    predict = (pos == string::npos ? output : output.substr(0, pos)) + ".pdx";
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
  cout << "@@@ pivot         = " << (pivot_file.empty() ? "none" : pivot_file) << endl;
  cout << "@@@ prediction    = " << (predict.empty() ? "none" : predict) << endl;
  cout << "@@@ var. offset   = " << offset << endl;
  cout << "@@@ print matrix  = " << display_strg[display]
       << (display == yUNDEF ? " (will be changed)" : "") << endl;
  cout << "@@@ print formula = " << print_strg[print] << endl;
  cout << "@@@ formula input = " << formula_prefix + "_*.log" << endl;
  cout << endl;

}

void get_formulas () {
  time_t start_time = time(nullptr);
  const string lsname = "/tmp/mcp-ls-" + to_string(start_time) + ".txt";
  const string filestar = formula_prefix + "_*.log";
  const string lscommand = "ls " + filestar + " > " + lsname;
  const string lserase = "rm -f " + lsname;

  int syserr = system(lscommand.c_str());

  if (syserr != 0) {
    cerr << "+++ No " << filestar << " files present" << endl;
    remove(lsname.c_str());
    exit(2);
  }

  ifstream ls_in;
  ls_in.open(lsname);
  if (!ls_in.is_open()) {
    cerr << "+++ Cannot open ls file " << lsname << endl;
    exit(2);
  }

  string file_string;
  unordered_map<string, string> formula_file;
  while (ls_in >> file_string) {
    string temp1 = file_string.substr(formula_prefix.length()+1);
    string gp = temp1.substr(0, temp1.length()-4);
    grps.push_back(gp);
    formula_file[gp] = file_string;
  }
  ls_in.close();
  syserr = remove(lsname.c_str());
  if (syserr != 0)
    cerr << "+++ WARNING: ls file " << lsname << " could not be erased" << endl;
  if (grps.empty()) {
    cerr << "+++ Formulas missing" << endl;
    exit(2);
  }

  streambuf *backup;
  backup = cin.rdbuf();
  ifstream form_in;
  for (const string &gp : grps) {
    form_in.open(formula_file[gp]);
    if (form_in.is_open())
      cin.rdbuf(form_in.rdbuf());
    else {
      cerr << "+++ Cannot open formula file " << formula_file[gp] << endl;
      exit(2);
    }

    names.clear();
    read_formula(names, formula[gp]);

    form_in.close();

    // cerr << "*** group   = " << gp << endl;
    // cerr << "*** formula = " << endl;
    // cerr << formula2string(names, formula[gp]) << endl;
    // cerr << endl;
  }
  cin.rdbuf(backup);

  cout << "+++ Groups [" << grps.size() << "]:";
  for (const string &gp : grps)
    cout << " " << gp;
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
      const vector<string> &hdl = split(line, ":");
      varnames.push_back(hdl);
    }
    // arity = varnames.size();

    headerfile.close();
  }
}

void read_matrix (Group_of_Matrix &matrix) {
  streambuf *backup;
  if (input != STDIN) {
    backup = cin.rdbuf();
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

  // indication line abandoned and header reading moved to read_header

  // string group;		// there will be no groups here
  int numline = 0;
  while (getline(cin, line)) {
    numline++;
    istringstream nums(line);
    // nums >> group;		// group identifier is test_group
    Row temp;
    int number;
    while (nums >> number)
      temp.push_back(number);
    if (arity != temp.size())
      cout << "*** arity discrepancy on line " << numline << endl;

    matrix[test_group].push_back(temp);
  }
  
  if (input != STDIN) {
    infile.close();
    cin.rdbuf(backup);
  }

  if (display == yUNDEF) {
    display = (numline * arity > MTXLIMIT) ? yHIDE : yPEEK;
    cout << "@@@ print matrix  = " << display_strg[display]
	 << " (redefined)" << endl;
  }
}

void print_matrix (Matrix &gmtx) {
  cout << "+++ Arity = " << arity << endl;
  cout << "+++ Test Group [" << gmtx.size() << "]:" << endl;
  if (display == yPEEK || display == ySHOW)
    cout << gmtx << endl;
  sort(grps.begin(), grps.end());
}

void print_formula (const vector<size_t> &names,
		    const Formula &formula,
		    const string gp) {
  string strg_fm = formula2string(names, formula);
  cout << "+++ Formula for '" << gp << "' [" << formula.size() << "] =" << endl;
  cout << strg_fm << endl;
}

void sat_test (Group_of_Matrix &matrix) {
  const Matrix gmtx = matrix[test_group];
  bool no_id = pivot_file.empty();
  long ctr = SENTINEL;

  size_t it_pivot = 0;
  cout << endl
       << "+++ Result: " << gmtx.size() << " row(s) in "
       << predict
       << endl;
  if (output != STDOUT)
    cerr << "+++ " << gmtx.size() << " row(s) in " << predict << endl;
  cout << "+++ end of run +++" << endl;
  if (output != STDOUT)
    outfile.close();

  if (!predict.empty()) {
    pdxfile.open(predict);
    if (! pdxfile.is_open()) {
      cerr << "+++ Cannot open predict file " << predict << endl;
      exit(2);
    }
      
    ctr = SENTINEL;
    it_pivot = 0;
    for (const Row &row : gmtx) {
      if (pivot_file.empty())
	pdxfile << "row_" << ++ctr;
      else
	pdxfile << pivot[it_pivot++];
      string separator = ",";
      for (const string &gp : grps)
	if (sat_formula(row, formula[gp])) {
	  pdxfile << separator << gp;
	  separator = "+";
	}
      pdxfile << endl;
    }
    pdxfile.close();
  }
}

void get_pivot (const Matrix &matrix) {
  if (pivot_file.empty())
    return;

  ifstream pivot_in;
  pivot_in.open(pivot_file);
  if (!pivot_in.is_open()) {
    cerr << "+++ Cannot open pivot file " << pivot_file << endl;
    exit(2);
  }

  string pivot_string;
  while (pivot_in >> pivot_string)
    pivot.push_back(pivot_string);
  pivot_in.close();

  if (pivot.size() != matrix.size()) {
    cerr << "+++ Size discrepancy between pivot[" << pivot.size()
	 << "] and matrix[" << matrix.size()
	 << endl;
    exit(2);
  }
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  version += "predict";
  cerr << "+++ version = " << version << endl;

  read_args(argc, argv);
  adjust_and_open();
  print_arg();
  get_formulas();
  read_header();
  read_matrix(group_of_matrix);
  print_matrix(group_of_matrix[test_group]);
  get_pivot(group_of_matrix[test_group]);
  for (const string &gp : grps)
    print_formula(names, formula[gp], gp);
  sat_test(group_of_matrix);
}

//////////////////////////////////////////////////////////////////////////////
