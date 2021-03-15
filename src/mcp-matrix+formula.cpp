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
 *      File:    mcp-matrix+formula.cpp                                   *
 *                                                                        *
 *      Copyright (c) 2019 - 2021                                         *
 *                                                                        *
 * Data structures for row, matrices, literals, clauses, and formula.     *
 *                                                                        *
 * This software has been created within the ACCA Project.                *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#include <sstream>
#include <vector>
#include "mcp-matrix+formula.hpp"

using namespace std;

//------------------------------------------------------------------------------

string version = GLOBAL_VERSION;
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
Print print         = pVOID;
Display display     = yUNDEF;
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

vector<string> split (const string &strg, char delimiter) {
  // splits a string into chunks separated by delimiter (split in perl)
  vector<string> chunks;
  string token;
  istringstream iss(strg);
  while (getline(iss, token, delimiter))
    chunks.push_back(token);
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
    vector<string> new_names = split(varnames[litname], ':');
    
    if (new_names.size() > 1)		// positive or negative
      output += (lit == lneg && print == pCLAUSE)
	? new_names[nNEGATIVE]
	: new_names[nPOSITIVE];
    else				// own name only
      output += (lit == lneg && print == pCLAUSE)
	? "-" + new_names[nOWN]
	: new_names[nOWN];
  } else				// variable without name
    output += (lit == lneg && print == pCLAUSE)
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
      output += literal2string(names[lit], lneg) + " ";
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
    vector<string> new_names = split(varnames[litname], ':');
    
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
      output += literal2latex(names[lit], lneg) + " ";
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
