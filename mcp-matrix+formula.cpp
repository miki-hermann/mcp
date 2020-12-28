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
 *      Copyright (c) 2019 - 2020                                         *
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

const int MTXLIMIT     = 4000;

Group_of_Matrix group_of_matrix;
vector<string> grps;

string varid = "x";
bool varswitch = false;
vector<string> varnames;

Action action       = aALL;
Print print         = pVOID;
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

// The following function needs to be overloaded. We need it once with and the
// second time without the names.
// First version WITH names
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