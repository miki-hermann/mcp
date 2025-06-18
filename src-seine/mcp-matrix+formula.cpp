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
 *      File:    mcp-matrix+formula.cpp                                   *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Data structures for row, matrices, literals, clauses, and formula.     *
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

#define SPACE " \t"
#define ENDSPACE " \t\n\v\f\r"

const string print_strg[]     = {"void",
				 "clause",
				 "implication",
				 "mixed",
				 "DIMACS"};
const string display_strg[]   = {"undefined",
				 "hide",
				 "peek",
				 "section",
				 "show"};

// const string neg[2]  = {"-", "\\neg "};
// const string disj[2] = {" + ", " \\lor "};
// const string conj[2] = {"* ", "\\land "};

Group_of_Matrix group_of_matrix;
vector<string> grps;

string varid = "x";
bool varswitch = false;
// vector<string> varnames;
vector<vector<string>> varnames;

Action action   = aALL;
string selected = "";
Print print_val = pVOID;
Display display = yUNDEF;
string suffix;
size_t arity    = 0;
size_t offset   = 0;
bool nosection  = false;

//------------------------------------------------------------------------------

bool sat_clause (const Row &tuple, const Clause &clause) {
  // does the tuple satisfy the clause?
  for (int i = 0; i < tuple.size(); ++i)
    if (clause[i] == lpos && tuple[i]
	||
	clause[i] == lneg && ! tuple[i])
      return true;
  return false;
}

bool sat_formula (const Row &tuple, const Formula &formula) {
  // does the tuple satisfy the formula?
  for (const Clause &cl : formula)
    if (!sat_clause(tuple, cl))
      return false;
  return true;
}

// true if line has been cleared and is nonempty
bool clear_line (const size_t lineno, string &line) {
  // erase leading and trailing whitespace
  auto nospace = line.find_first_not_of(SPACE);
  line.erase(0, nospace);
  nospace = line.find_last_not_of(ENDSPACE);
  line.erase(nospace+1);
  if (line.empty())
    return false;

  // treat back slashed characters and strings
  string line1;
  bool is_string = false;
  size_t i = 0;
  // for (size_t i = 0; i < line.length(); ++i) {
  while (i < line.length()) {
    const char chr = line[i];
    if (chr == '\\' && i == line.length()-1) {
      cerr << "+++ line " << lineno
	   << " cannot terminate with a backslash"
	   << endl;
      return false;
    } else if (chr == '\\')
      line1 += chr + line[++i];
    else if (chr == '"')
      is_string = ! is_string;
    else if (is_string && chr == ' ')
      line1 += "_";
    else if (is_string && (chr == ',' || chr == ';'))
      line1 += ":";
    else
      line1 += chr;
    i++;
  }
  // line.clear();
  line = move(line1);
  return true;
}

// commas (,) and semicolons (;) are replaced with spaces
// outside strings
void uncomma_line (string &line) {
  bool is_string = false;
  // replace commas and semicolons by a space
  for (size_t i = 0; i < line.length(); ++i) {
    char chr = line[i];
    if (chr == '"')
      is_string = ! is_string;
    else if (! is_string && (line[i] == ',' || line[i] == ';'))
      line[i] = ' ';
  }
}

// splits a string into chunks separated by delimiters (split in perl)
vector<string> split (string strg, const string &delimiters) {
  vector<string> chunks;

  // get rid of non-printable characters at the end of the string
  // without warning
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
  for (size_t i = 0; i < strg.length(); ++i)
    if (! isprint(strg[i])) {
      cerr << "+++ string on input has a non-printable character on position "
	   << i
	   << endl;
      if (i >= strg.length()-1)
	cout << "... use dos2unix to fix it" << endl;
      exit(2);
    }

  // proper split
  size_t pointer = 0;
  while (pointer < strg.length()) {
    const size_t found = strg.find_first_of(delimiters, pointer);
    if (found == string::npos) {
      chunks.push_back(strg.substr(pointer));
      break;
    }
    chunks.push_back(strg.substr(pointer, found - pointer));
    pointer = found+1;
  }
  return chunks;
}

// transforms clause into readable clausal form in DIMACS format to print
string clause2dimacs (const vector<size_t> &names, const Clause &clause) {
  string output = "\t";
  bool plus = false;
  for (int lit = 0; lit < clause.size(); ++lit) {
      if (clause[lit] != lnone) {
	if (plus)
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

// transforms formula into readable clausal form in DIMACS format to print
string formula2dimacs (const vector<size_t> &names, const Formula &formula) {
  if (formula.empty())
    return " ";

  string output;
  for (const Clause &clause : formula)
    output += clause2dimacs(names, clause) + "\n";
  return output;
}

string literal2string (const int &litname, const Literal lit) {
  string output;
  if (varswitch) {
    // vector<string> new_names = split(varnames[litname], ":");
    const vector<string> &new_names = varnames[litname];
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

string rlcl2string (const vector<size_t> &names, const Clause &clause) {
  string output;
  bool plus = false;
  for (size_t lit = 0; lit < clause.size(); ++lit) {
    if (clause[lit] != lnone) {
      if (plus)
	output += " + ";
      else
	plus = true;
      output += literal2string(names[lit], clause[lit]);
    }
  }
  return output;
}

string impl2string (const vector<size_t> &names, const Clause &clause) {
  string output;
  for (size_t lit = 0; lit < clause.size(); ++lit)
    if (clause[lit] == lneg)
      output += literal2string(names[lit], lpos) + " ";
  output += "->";
  for (size_t lit = 0; lit < clause.size(); ++lit)
    if (clause[lit] == lpos)
      output += " " + literal2string(names[lit], lpos);
  return output;
}

string clause2string (const vector<size_t> &names, const Clause &clause) {
  string output = "(";
  if (print_val == pCLAUSE) {
    output += rlcl2string(names, clause);
  } else if (print_val == pIMPL) {
    output += impl2string(names, clause);
  } else if (print_val == pMIX) {
    size_t pneg = 0;
    size_t ppos = 0;
    for (size_t lit = 0; lit < clause.size(); ++lit)
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

// transforms formula into readable clausal, implication or mixed form
// to print
string formula2string (const vector<size_t> &names, const Formula &formula) {
  if (formula.empty())
    return " ";

  if (print_val == pDIMACS)
    return formula2dimacs(names, formula);

  string output;
  bool times = false;
  for (Clause clause : formula) {
    output += times ? "\n\t* " : "\t  ";
    times = true;
    output += clause2string(names, clause);
  }
  return output;
}

string literal2latex (const int &litname, const Literal lit) {
  string output;
  if (varswitch) {
    const vector<string> &new_names = varnames[litname];
    
    if (new_names.size() > 1)		// positive or negative
      output +=
	lit == lneg && print_val == pCLAUSE
	? new_names[nNEGATIVE]
	: new_names[nPOSITIVE];
    else				// own name only
      output += (lit == lneg && print_val == pCLAUSE
		 ? "\\neg " + new_names[nOWN]
		 : new_names[nOWN]);
  } else				// variable without name
    output += (lit == lneg && print_val == pCLAUSE)
      ? "\\neg " + varid + "_" + to_string(offset + litname)
      : varid + "_" + to_string(offset + litname);
  return output;
}

string rlcl2latex (const vector<size_t> &names, const Clause &clause) {
  string output;
  bool lor = false;
  for (size_t lit = 0; lit < clause.size(); ++lit) {
    if (clause[lit] != lnone) {
      if (lor)
	output += " \\lor ";
      else
	lor = true;
      output += literal2latex(names[lit], clause[lit]);
    }
  }
  return output;
}

string impl2latex (const vector<size_t> &names, const Clause &clause) {
  string output;
  for (size_t lit = 0; lit < clause.size(); ++lit)
    if (clause[lit] == lneg)
      output += literal2latex(names[lit], lpos) + " ";
  output += "\\to";
  for (size_t lit = 0; lit < clause.size(); ++lit)
    if (clause[lit] == lpos)
      output += " " + literal2latex(names[lit], lpos);
  return output;
}

string clause2latex (const vector<size_t> &names, const Clause &clause) {
  string output = "(";
  if (print_val == pCLAUSE) {
    output += rlcl2latex(names, clause);
  } else if (print_val == pIMPL) {
    output += impl2latex(names, clause);
  } else if (print_val == pMIX) {
    size_t pneg = 0;
    size_t ppos = 0;
    for (size_t lit = 0; lit < clause.size(); ++lit)
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

// transforms formula into readable clausal form in LaTeX format to print
string formula2latex (const vector<size_t> &names, const Formula &formula) {
  if (formula.empty())
    return " ";

  string output;
  bool land = false;
  for (Clause clause : formula) {
    output += (land) ? "\n\t\\land " : "\t  ";
    land = true;
    output += clause2latex(names, clause);
  }
  return output;
}

// formula read instructions
void read_formula (vector<size_t> &names, Formula &formula) {

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

  for (size_t i = 0; i < arity; ++i)
    names.push_back(i);

  int lit;
  Clause clause(arity, lnone);
  while (cin >> lit)
    if (lit == 0) {			// end of clause in DIMACS
      formula.push_back(clause);
      for (int i = 0; i < arity; ++i)
	clause[i] = lnone;
    } else if (find(cbegin(validID), cend(validID),
		    abs(lit)) == cend(validID)) {
      cerr << "+++ " << abs(lit) << " outside allowed variable names" << endl;
      exit(2);
    } else
      clause[abs(lit)-1-offset] = lit < 0 ? lneg : lpos;
}

// overloading ostream to print a row
// transforms a tuple (row) to a printable form
ostream& operator<< (ostream &output, const Row &row) {
  // for (bool bit : row)
  for (size_t i = 0; i < row.size(); ++i)
    // output << to_string(bit); // bit ? 1 : 0;
    // output << to_string(row[i]);
    output << row[i];
  return output;
}

// overloading ostream to print a matrix
// transforms a matrix to a printable form
ostream& operator<< (ostream &output, const Matrix &M) {
  for (Row row : M)
    output << "\t" << row << endl;
  return output;
}

// void push_front (Row &row1, const Row &row2) {
//   Row newrow(row1.size()+row2.size(), row2.to_ulong());
//   row1.resize(newrow.size());
//   row1 <<= row2.size();
//   newrow |= row1;
//   row1 = newrow;
// }

void push_front (Row &row1, const Row &row2) {
  Row newrow = row2;
  move(row1.begin(), row1.end(), back_inserter(newrow));
  row1 = move(newrow);
}

// void push_front (Row &row, const bool b) {
//   Row newrow(row.size()+1, b);
//   row.resize(newrow.size());
//   row <<= 1;
//   row |= newrow;
// }

// bool front (const Row &row) {
//   return row[0];
// }

// bool back (const Row &row) {
//   return row[row.size()-1];
// }

// void pop_front (Row &row) {
//   row >>= 1;
//   row.resize(row.size()-1);
// }
