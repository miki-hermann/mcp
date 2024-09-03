/**************************************************************************
 *                                                                        *
 *                                                                        *
 *	       Multiple Classification   Problem (MCP)                    *
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
 *      File:    mcp-matrix+formula.cpp                                   *
 *                                                                        *
 *      Copyright (c) 2019 - 2024                                         *
 *                                                                        *
 * Data structures for row, matrices, literals, clauses, and formula.     *
 *                                                                        *
 * This software has been created within the ACCA Project.                *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#include <iostream>
// #include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include "mcp-matrix+formula.hpp"

using namespace std;

//------------------------------------------------------------------------------

string version       = GLOBAL_VERSION;
const int SENTINEL   = -1;
const double RSNTNL  = -1.0;
const int MTXLIMIT   = 4000;

const string print_strg[]     = {"void",       "clause",     "implication", "mixed",   "DIMACS"};
const string display_strg[]   = {"undefined",  "hide",       "peek",        "section", "show"};

// const string neg[2]  = {"-", "\\neg "};
// const string disj[2] = {" + ", " \\lor "};
// const string conj[2] = {"* ", "\\land "};

Group_of_Matrix group_of_matrix;
vector<string> grps;

string varid = "x";
bool varswitch = false;
vector<string> varnames;

Action action       = aALL;
string selected     = "";
Print print         = pVOID;
Display display     = yUNDEF;
string suffix;
int arity           = 0;
int offset          = 0;

//------------------------------------------------------------------------------

bool sat_clause (const Row &tuple, const Clause &clause) {
  // does the tuple satisfy the clause?
  for (int i = 0; i < tuple.size(); ++i)
    if (clause[i] == lpos && tuple[i] == true
	||
	clause[i] == lneg && tuple[i] == false)
      return true;
  return false;
}

bool sat_formula (const Row &tuple, const Formula &formula) {
  // does the tuple satisfy the formula?
  for (Clause cl : formula)
    if (!sat_clause(tuple, cl))
      return false;
  return true;
}

vector<string> split (string strg, string delimiters) {
  // splits a string into chunks separated by delimiters (split in perl)
  vector<string> chunks;

  // get rid of non-printable characters at the end of the string without warning
  // because Linux, Windows, and MacOS all terminate the string differently
  // doing it the hard way
  if (! strg.empty()) {
    size_t i = strg.length()-1;
    while (i >= 0 && ! isprint(strg[i]))
      i--;
    if (i < 0)
      strg.clear();
    else
      strg = strg.substr(0, i+1);
  }
  // the following code does not work 
  // while (! strg.empty() && ! isprint(strg.back()))
  // // while (! strg.empty() && strg.back() == '\r')
  //   strg.pop_back();
  for (size_t i = 0; i < strg.length(); ++i)
    if (! isprint(strg[i])) {
      cerr << "+++ string on input has a non-printable character on position "
	   << i
	   << endl;
      if (i >= strg.length()-1)
	cout << "... use dos2unix to fix it" << endl;
      exit(2);
    }

  size_t pointer = 0;
  while (!strg.empty()) {
    size_t found = strg.find_first_of(delimiters, pointer);
    if (found == string::npos) {
      chunks.push_back(strg.substr(pointer));
      break;
    }
    chunks.push_back(strg.substr(pointer, found - pointer));
    pointer = found+1;
  }
  return chunks;
}

string clause2dimacs (const vector<int> &names, const Clause &clause) {
  // transforms clause into readable clausal form in DIMACS format to print
  string output = "\t";
  bool plus = false;
  for (int lit = 0; lit < clause.size(); ++lit) {
      if (clause[lit] != lnone) {
	if (plus == true)
	  output += " ";
	else
	  plus = true;
	if (clause[lit] == lneg)
	  output += '-';
	output += to_string(offset + names[lit]);
      }
  }
  output += " 0";
  return output;
}

string formula2dimacs (const vector<int> &names, const Formula &formula) {
  // transforms formula into readable clausal form in DIMACS format to print
  if (formula.empty())
    return " ";

  string output;
  for (Clause clause : formula)
    output += clause2dimacs(names, clause) + "\n";
  return output;
}

string literal2string (const int &litname, const Literal lit) {
  string output;
  if (varswitch) {
    vector<string> new_names = split(varnames[litname], ":");
    if (new_names.size() > 1)		// positive or negative
      output += (lit == lneg)
	? new_names[nNEGATIVE]
	: new_names[nPOSITIVE];
    else				// own name only
      output += (lit == lneg)
	? "-" + new_names[nOWN]
	: new_names[nOWN];
  } else				// variable without name
    output += (lit == lneg)
      ? "-" + varid + to_string(offset + litname)
      : varid + to_string(offset + litname);
  return output;
}

string rlcl2string (const vector<int> &names, const Clause &clause) {
  string output;
  bool plus = false;
  for (int lit = 0; lit < clause.size(); ++lit) {
    if (clause[lit] != lnone) {
      if (plus == true)
	output += " + ";
      else
	plus = true;
      output += literal2string(names[lit], clause[lit]);
    }
  }
  return output;
}

string impl2string (const vector<int> &names, const Clause &clause) {
  string output;
  for (int lit = 0; lit < clause.size(); ++lit)
    if (clause[lit] == lneg)
      output += literal2string(names[lit], lpos) + " ";
  output += "->";
  for (int lit = 0; lit < clause.size(); ++lit)
    if (clause[lit] == lpos)
      output += " " + literal2string(names[lit], lpos);
  return output;
}

string clause2string (const vector<int> &names, const Clause &clause) {
  string output = "(";
  if (print == pCLAUSE) {
    output += rlcl2string(names, clause);
  } else if (print == pIMPL) {
    output += impl2string(names, clause);
  } else if (print == pMIX) {
    int pneg = 0;
    int ppos = 0;
    for (int lit = 0; lit < clause.size(); ++lit)
      if (clause[lit] == lneg)
	pneg++;
      else if (clause[lit] == lpos)
	ppos++;
    output += (pneg == 0 || ppos == 0)
      ? rlcl2string(names, clause)
      : impl2string(names, clause);
  }
  output += ')';
  return output;
}

string formula2string (const vector<int> &names, const Formula &formula) {
  // transforms formula into readable clausal, implication or mixed form to print
  if (formula.empty())
    return " ";

  if (print == pDIMACS)
    return formula2dimacs(names, formula);

  string output;
  bool times = false;
  for (Clause clause : formula) {
    output += (times == true) ? "\n\t* " : "\t  ";
    times = true;
    output += clause2string(names, clause);
  }
  return output;
}

string literal2latex (const int &litname, const Literal lit) {
  string output;
  if (varswitch) {
    vector<string> new_names = split(varnames[litname], ":");
    
    if (new_names.size() > 1)		// positive or negative
      output +=
	lit == lneg && print == pCLAUSE
	? new_names[nNEGATIVE]
	: new_names[nPOSITIVE];
    else				// own name only
      output += (lit == lneg && print == pCLAUSE
		 ? "\\neg " + new_names[nOWN]
		 : new_names[nOWN]);
  } else				// variable without name
    output += (lit == lneg && print == pCLAUSE)
      ? "\\neg " + varid + "_" + to_string(offset + litname)
      : varid + "_" + to_string(offset + litname);
  return output;
}

string rlcl2latex (const vector<int> &names, const Clause &clause) {
  string output;
  bool lor = false;
  for (int lit = 0; lit < clause.size(); ++lit) {
    if (clause[lit] != lnone) {
      if (lor == true)
	output += " \\lor ";
      else
	lor = true;
      output += literal2latex(names[lit], clause[lit]);
    }
  }
  return output;
}

string impl2latex (const vector<int> &names, const Clause &clause) {
  string output;
  for (int lit = 0; lit < clause.size(); ++lit)
    if (clause[lit] == lneg)
      output += literal2latex(names[lit], lpos) + " ";
  output += "\\to";
  for (int lit = 0; lit < clause.size(); ++lit)
    if (clause[lit] == lpos)
      output += " " + literal2latex(names[lit], lpos);
  return output;
}

string clause2latex (const vector<int> &names, const Clause &clause) {
  string output = "(";
  if (print == pCLAUSE) {
    output += rlcl2latex(names, clause);
  } else if (print == pIMPL) {
    output += impl2latex(names, clause);
  } else if (print == pMIX) {
    int pneg = 0;
    int ppos = 0;
    for (int lit = 0; lit < clause.size(); ++lit)
      if (clause[lit] == lneg)
	pneg++;
      else if (clause[lit] == lpos)
	ppos++;
    output += (pneg == 0 || ppos == 0)
      ? rlcl2latex(names, clause)
      : impl2latex(names, clause);
  }
  output += ')';
  return output;
}

string formula2latex (const vector<int> &names, const Formula &formula) {
  // transforms formula into readable clausal form in LaTeX format to print
  if (formula.empty())
    return " ";

  string output;
  bool land = false;
  for (Clause clause : formula) {
    output += (land == true) ? "\n\t\\land " : "\t  ";
    land = true;
    output += clause2latex(names, clause);
  }
  return output;
}

void read_formula (vector<int> &names, Formula &formula) {
  // formula read instructions

  int nvars;
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

ostream& operator<< (ostream &output, const Row &row) {
  // overloading ostream to print a row
  // transforms a tuple (row) to a printable form
  // for (bool bit : row)
  for (int i = 0; i < row.size(); ++i)
    // output << to_string(bit); // bit == true ? 1 : 0;
    // output << to_string(row[i]);
    output << row[i];
  return output;
}

ostream& operator<< (ostream &output, const Matrix &M) {
  // overloading ostream to print a matrix
  // transforms a matrix to a printable form
  for (Row row : M)
    output << "\t" << row << "\n";
  return output;
}

void push_front (Row &row1, const Row &row2) {
  Row newrow(row1.size()+row2.size(), row2.to_ulong());
  row1.resize(newrow.size());
  row1 <<= row2.size();
  newrow |= row1;
  row1 = newrow;
}

void push_front (Row &row, const bool b) {
  Row newrow(row.size()+1, b);
  row.resize(newrow.size());
  row <<= 1;
  row |= newrow;
}

bool front (const Row &row) {
  return row[0];
}

bool back (const Row &row) {
  return row[row.size()-1];
}

void pop_front (Row &row) {
  row >>= 1;
  row.resize(row.size()-1);
}
