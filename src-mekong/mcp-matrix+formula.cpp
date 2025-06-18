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
 *     File:    src-mekong/mcp-matrix+formula.cpp                         *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 **************************************************************************/

#include <cstdint>
#include <iomanip>
#include <iostream>
// #include <sstream>
// #include <mutex>
#include "mcp-matrix+formula.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include <cmath>
#include "mcp-trans.hpp"

using namespace std;

//------------------------------------------------------------------------------

string version = GLOBAL_VERSION;
const int SENTINEL = -1;
const double RSNTNL = -1.0;
const size_t MTXLIMIT = 4000;

#define SPACE " \t"
#define ENDSPACE " \t\n\v\f\r"

const string print_strg[] = {"void", "clause", "implication", "mixed",
                             "DIMACS"};
const string display_strg[] = {"undefined", "hide", "peek", "section", "show"};
const string sign_string[] = {/*lnone*/ "?",
                              /*lneg*/ "<=", /*lpos*/ ">=", /*lboth*/ "<=>"};

vector<integer> DMAX;
Group_of_Matrix group_of_matrix;
vector<string> grps;

string varid = "x";
bool varswitch = false;
vector<Headline> headlines;

Action action = aALL;
string selected = "";
Print print_val = pVOID;
Display display = yUNDEF;
string suffix;
size_t arity = 0;
size_t offset = 0;
bool nosection = false;

//------------------------------------------------------------------------------

void Row::inplace_minimum(const Row &other) & {
  for (size_t i = 0; i < other.size(); ++i) {
    if (operator[](i) > other[i]) {
      operator[](i) = other[i];
    }
  }
}

void Row::inplace_minimum(const RowView &other) & {
  for (size_t i = 0; i < other.size(); ++i) {
    if (operator[](i) > other[i]) {
      operator[](i) = other[i];
    }
  }
}

void Row::restrict(const Mask &m) {
  size_t col = 0;
  for (size_t j = 0; j < m.size(); ++j) {
    if (m[j]) {
      // invariant: col <= j
      data[col] = data[j];
      col++;
    }
  }
  data.resize(col);
}

bool Row::operator>=(const Row &rhs) const {
  if (rhs.size() != size())
    return false;

  for (size_t i = 0; i < rhs.size(); ++i) {
    if (rhs[i] > operator[](i))
      return false;
  }
  return true;
}

bool Row::operator>(const Row &rhs) const {
  if (rhs.size() != size())
    return false;

  for (size_t i = 0; i < rhs.size(); ++i) {
    if (rhs[i] >= operator[](i))
      return false;
  }
  return true;
}

bool Row::operator==(const Row &rhs) const {
  if (rhs.size() != size())
    return false;

  for (size_t i = 0; i < rhs.size(); ++i) {
    if (rhs[i] != operator[](i))
      return false;
  }
  return true;
}

double Row::zscore () const {
  double mean = 0.0;
  for (size_t i = 0; i < this->size(); ++i)
    mean += 1.0 * (*this)[i];;
  mean /= this->size();

  double variance = 0.0;
  for (size_t i = 0; i < this->size(); ++i) {
    double delta = 1.0 * (*this)[i] - mean;
    variance += delta * delta;
  }
  variance /= this->size();
  double std_dev = sqrt(variance);

  double sum = 0.0;
  for (size_t i = 0; i < this->size(); ++i)
    sum += ((*this)[i] - mean) / std_dev;
  return sum;
}

bool RowView::operator>=(const RowView &rhs) const {
  if (rhs.size() != size())
    return false;

  for (size_t i = 0; i < rhs.size(); ++i) {
    if (rhs[i] > operator[](i))
      return false;
  }
  return true;
}

bool RowView::operator>=(const Row &rhs) const {
  if (rhs.size() != size())
    return false;

  for (size_t i = 0; i < rhs.size(); ++i) {
    if (rhs[i] > operator[](i))
      return false;
  }
  return true;
}

bool RowView::operator>(const RowView &rhs) const {
  if (rhs.size() != size())
    return false;

  for (size_t i = 0; i < rhs.size(); ++i) {
    if (rhs[i] >= operator[](i))
      return false;
  }
  return true;
}

bool RowView::operator>(const Row &rhs) const {
  if (rhs.size() != size())
    return false;

  for (size_t i = 0; i < rhs.size(); ++i) {
    if (rhs[i] >= operator[](i))
      return false;
  }
  return true;
}

bool RowView::operator==(const RowView &rhs) const {
  if (rhs.size() != size())
    return false;

  for (size_t i = 0; i < rhs.size(); ++i) {
    if (rhs[i] != operator[](i))
      return false;
  }
  return true;
}

bool RowView::operator==(const Row &rhs) const {
  if (rhs.size() != size())
    return false;

  for (size_t i = 0; i < rhs.size(); ++i) {
    if (rhs[i] != operator[](i))
      return false;
  }
  return true;
}

Row RowView::to_row() const {
  Row res(cols.size());
  for (size_t i = 0; i < cols.size(); ++i) {
    res[i] = data[cols[i]];
  }
  return res;
}

void Matrix::delete_row(size_t index) {
  // swap and forget last row
  size_t last = num_rows() - 1;

  if (index != last)
    std::swap(data[index], data[last]);
  data.resize(last);
}

Matrix MatrixMask::to_matrix() const & {
  vector<Row> data;
  data.reserve(num_rows());

  for (size_t i = 0; i < num_rows(); ++i) {
    data.push_back(operator[](i).to_row());
  }

  return Matrix(std::move(data));
}

Matrix Matrix::transpose() const & {
  Matrix tr;
  for (size_t col = 0; col < this->num_cols(); ++col) {
    Row temp(this->num_rows());
    for (size_t row = 0; row < this->num_rows(); ++row)
      temp[row] = (*this)[row][col];
    tr.add_row(std::move(temp));
  }
  return tr;
}

void Matrix::restrict(const Mask &m) {
  // can be parallelised easily
  // #pragma omp parallel for
  for (size_t i = 0; i < num_rows(); ++i) {
    data[i].restrict(m);
  }
}

size_t partition_matrix(Matrix &mtx, size_t low, size_t high) {
  const Row &pivot = mtx[high];
  size_t p_index = low;

  for (size_t i = low; i < high; i++) {
    if (mtx[i].total_order(pivot) <= 0) {
      std::swap(mtx[i], mtx[p_index]);
      p_index++;
    }
  }
  std::swap(mtx[high], mtx[p_index]);

  return p_index;
}

void sort_matrix(Matrix &mtx, size_t low, size_t high) {
  if (low < high) {
    size_t p_index = partition_matrix(mtx, low, high);
    sort_matrix(mtx, low, p_index - 1);
    sort_matrix(mtx, p_index + 1, high);
  }
}

void Matrix::sort() { sort_matrix((Matrix &)(*this), 0, num_rows() - 1); }

void Matrix::remove_duplicates() {
  auto ip = unique(data.begin(), data.end());
  data.resize(size_t(distance(data.begin(), ip)));
}

//------------------------------------------------------------------------------

// template <typename T>
// bool sat_clause(const T &tuple, const Clause &clause) {
//   // does the tuple satisfy the clause?
//   for (size_t i = 0; i < tuple.size(); ++i) {
//     if (clause[i].sat(tuple[i])) {
//       return true;
//     }
//   }
//   return false;
// }

// template <typename T>
// bool sat_formula(const T &tuple, const Formula &formula) {
//   // does the tuple satisfy the formula?
//   for (Clause cl : formula) {
//     if (!sat_clause(tuple, cl)) {
//       return false;
//     }
//   }
//   return true;
// }

// does the tuple satisfy the clause?
bool sat_clause(const RowView &tuple, const Clause &clause) {
  for (size_t i = 0; i < tuple.size(); ++i) {
    if (clause[i].sat(tuple[i])) {
      return true;
    }
  }
  return false;
}

// does the tuple satisfy the formula?
bool sat_formula(const RowView &tuple, const Formula &formula) {
  for (const Clause &cl : formula) {
    if (!sat_clause(tuple, cl)) {
      return false;
    }
  }
  return true;
}

// does the tuple satisfy the clause?
bool sat_clause(const Row &tuple, const Clause &clause) {
  for (size_t i = 0; i < tuple.size(); ++i) {
    if (clause[i].sat(tuple[i])) {
      return true;
    }
  }
  return false;
}

// does the tuple satisfy the formula?
bool sat_formula(const Row &tuple, const Formula &formula) {
  for (const Clause &cl : formula) {
    if (!sat_clause(tuple, cl)) {
      return false;
    }
  }
  return true;
}

bool sat_clause(const Matrix &matrix, const Clause &clause) {
  for (size_t i = 0; i < matrix.num_rows(); ++i) {
    if (!sat_clause(matrix[i], clause)) {
      return false;
    }
  }
  return true;
}

bool sat_formula(const Matrix &matrix, const Formula &formula) {
  for (size_t i = 0; i < matrix.num_rows(); ++i) {
    if (!sat_formula(matrix[i], formula)) {
      return false;
    }
  }
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
vector<string> split(string strg, const string &delimiters) {
  vector<string> chunks;

  // get rid of non-printable characters at the end of the string
  // without warning because Linux, Windows, and MacOS all terminate
  // the string differently doing it the hard way
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
    if (!isprint(strg[i])) {
      cerr << "+++ string on input has a non-printable character on position "
           << i << endl;
      exit(2);
    }

  // non-destructive version
  size_t pointer = 0;
  while (pointer < strg.length()) {
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

// transforms clause into readable clausal form in (extended) DIMACS
// format to print
string clause2dimacs(const vector<size_t> &names, const Clause &clause) {
  string output = "\t";
  for (size_t lit = 0; lit < clause.size(); ++lit) {
    string var = to_string(offset + names[lit]);
    if (clause[lit].sign & lpos) {
      output += var + ":" + to_string(clause[lit].pval) + " ";
    }
    if (clause[lit].sign & lneg) {
      output += "-" + var + ":" + to_string(clause[lit].nval) + " ";
    }
  }
  output += "0";
  return output;
}

// transforms formula into readable clausal form in 'extended) DIMACS format
// to print
string formula2dimacs(const vector<size_t> &names, const Formula &formula) {
  if (formula.empty())
    return " ";

  string output;
  for (const Clause &clause : formula)
    output += clause2dimacs(names, clause) + "\n";
  return output;
}

// print inequalities according to headline token
string ineq(const Sign sgn,
	    const integer &val,
	    const Headline &headline) {
  string output;
  switch (headline.token) {
  case BOOL:
  case ENUM:
  case UP:
  case DOWN:
    // values are indices
    output += headline.elements[val];
    break;
  case INT:
    // values itself
    output += to_string(val);
    break;
  case DISJOINT:
  case OVERLAP:
  case SPAN:
  case WARP:
    // values are intervals
    if (sgn == lpos)
      output += headline.elements[val];
    else if (sgn == lneg)
      output += headline.elements[val+1];
    break;
  case CHECKPOINTS:
    // special treatment
    if (headline.elements[val] == token_string.at(CARET))
      output += sgn == lpos ? "-infinity" : headline.elements[val];
    else if (headline.elements[val] == token_string.at(DOLLAR))
      output += sgn == lpos ? "***IMPOSSIBLE***" : "+infinity";
    else
      output += sgn == lpos ? headline.elements[val] : headline.elements[val+1];
    break;
  default:
    cerr << "ineq: you should not be here" << endl;
    exit(2);
  }
  return output;
}

string literal2string(const size_t &litname, const Literal &lit) {
  string output;
  string var_name;

  if (varswitch) {
    Headline hdl = headlines[litname];
    if (lit.sign & lpos)
      output += hdl.name + ">=" + ineq(lpos, lit.pval, hdl);
    if (lit.sign == lboth)
      output += " + ";
    if (lit.sign & lneg)
      output += hdl.name + "<=" + ineq(lneg, lit.nval, hdl);
  } else { // variable without name
    var_name = varid + to_string(offset + litname);
    if (lit.sign & lpos)
      output += var_name + ">=" + to_string(lit.pval);
    if (lit.sign == lboth)
      output += " + ";
    if (lit.sign & lneg)
      output += var_name + "<=" + to_string(lit.nval);
  }
  return output;
}

string rlcl2string(const vector<size_t> &names, const Clause &clause) {
  string output;
  bool plus = false;
  for (size_t lit = 0; lit < clause.size(); ++lit)
    if (clause[lit].sign != lnone) {
      if (plus == true)
        output += " + ";
      else
        plus = true;
      output += literal2string(names[lit], clause[lit]);
    }
  return output;
}

string impl2string(const vector<size_t> &names, const Clause &clause) {
  string output;
  for (size_t lit = 0; lit < clause.size(); ++lit)
    if (clause[lit].sign & lneg) {
      Literal l = clause[lit];
      l.sign = lpos;
      l.pval = l.nval + 1;
      output += literal2string(names[lit], l) + " ";
    }
  output += "->";
  for (size_t lit = 0; lit < clause.size(); ++lit)
    if (clause[lit].sign & lpos) {
      Literal l = clause[lit];
      l.sign = lpos;
      output += " " + literal2string(names[lit], l);
    }
  return output;
}

string clause2string(const vector<size_t> &names, const Clause &clause) {
  string output = "(";
  if (print_val == pCLAUSE) {
    output += rlcl2string(names, clause);
  } else if (print_val == pIMPL) {
    output += impl2string(names, clause);
  } else if (print_val == pMIX) {
    size_t pneg = 0;
    size_t ppos = 0;
    for (size_t lit = 0; lit < clause.size(); ++lit) {
      if (clause[lit].sign & lneg)
        pneg++;
      if (clause[lit].sign & lpos)
        ppos++;
    }
    output += (pneg != 0 && ppos == 1)
      ? impl2string(names, clause)
      : rlcl2string(names, clause);
  }
  output += ')';
  return output;
}

// transforms formula into readable clausal, implication or mixed form to
// print
string formula2string(const vector<size_t> &names, const Formula &formula) {
  if (formula.empty())
    return " ";

  if (print_val == pDIMACS)
    return formula2dimacs(names, formula);

  string output;
  bool times = false;
  for (const Clause &clause : formula) {
    output += (times == true) ? "\n\t* " : "\t  ";
    times = true;
    output += clause2string(names, clause);
  }
  return output;
}

string literal2latex(const size_t &litname, const Literal lit) {
  string output;

  if (varswitch) {
    Headline hdl = headlines[litname];
    if (lit.sign & lpos)
      output += hdl.name + "\\geq" + ineq(lpos, lit.pval, hdl);
    if (lit.sign == lboth)
      output += " \\lor ";
    if (lit.sign & lneg)
      output += hdl.name + "\\leq" + ineq(lneg, lit.nval, hdl);
  } else { // variable without name
    string var_name = varid + to_string(offset + litname);
    if (lit.sign & lpos) {
      output += var_name + "\\geq" + to_string(lit.pval);
    }
    if (lit.sign == lboth) {
      output += " \\lor ";
    }
    if (lit.sign & lneg) {
      output += var_name + "\\leq" + to_string(lit.nval);
    }
  }
  return output;
}

string rlcl2latex(const vector<size_t> &names, const Clause &clause) {
  string output;
  bool lor = false;
  for (size_t lit = 0; lit < clause.size(); ++lit) {
    if (clause[lit].sign != lnone) {
      if (lor == true)
        output += " \\lor ";
      else
        lor = true;
      output += literal2latex(names[lit], clause[lit]);
    }
  }
  return output;
}

string impl2latex(const vector<size_t> &names, const Clause &clause) {
  string output;
  for (size_t lit = 0; lit < clause.size(); ++lit)
    if (clause[lit].sign & lneg) {
      Literal l = clause[lit];
      l.sign = lpos;
      l.pval = l.nval + 1;
      output += literal2latex(names[lit], l) + " ";
    }
  output += "\\to";
  for (size_t lit = 0; lit < clause.size(); ++lit)
    if (clause[lit].sign & lpos) {
      Literal l = clause[lit];
      l.sign = lpos;
      output += " " + literal2latex(names[lit], l);
    }
  return output;
}

string clause2latex(const vector<size_t> &names, const Clause &clause) {
  string output = "\\left(";
  if (print_val == pCLAUSE) {
    output += rlcl2latex(names, clause);
  } else if (print_val == pIMPL) {
    output += impl2latex(names, clause);
  } else if (print_val == pMIX) {
    int pneg = 0;
    int ppos = 0;
    for (size_t lit = 0; lit < clause.size(); ++lit)
      if (clause[lit].sign & lneg)
        pneg++;
      else if (clause[lit].sign & lpos)
        ppos++;
    output += (pneg == 0 || ppos == 0) ? rlcl2latex(names, clause)
      : impl2latex(names, clause);
  }
  output += "\\right)";
  return output;
}

string formula2latex(const vector<size_t> &names, const Formula &formula) {
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

// formula read instructions
void read_formula(vector<size_t> &names, Formula &formula) {

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

  string lit;
  Clause clause(arity, Literal::none());
  while (cin >> lit) {
    if (lit == "0") { // end of clause in DIMACS
      formula.push_back(clause);
      for (size_t i = 0; i < arity; ++i)
        clause[i] = Literal::none();
    } else {
      vector<string> parts = split(lit, ":");
      if (parts.size() == 0 || parts.size() > 2) {
        cerr << "+++ " << quoted(lit)
             << " is not a valid Extended DIMACS literal" << endl;
        exit(2);
      }
      try {
        long long var = stoll(parts[0]);

        if (find(cbegin(validID), cend(validID), abs(var)) == cend(validID)) {
          cerr << "+++ " << abs(var) << " outside allowed variable names"
               << endl;
          exit(2);
        }

        unsigned long long val =
	  parts.size() == 2 ? stoull(parts[1]) : (var < 0 ? 0 : 1);
        if (val > (unsigned long long)(std::numeric_limits<integer>::max()))
          throw out_of_range(parts[1]);

        Literal l = clause.at(size_t(abs(var)) - 1 - offset);
        if (var < 0) {
          l.sign = Sign(l.sign | lneg);
          l.nval = integer(val);
        } else {
          l.sign = Sign(l.sign | lpos);
          l.pval = integer(val);
        }
        clause.at(size_t(abs(var)) - 1 - offset) = l;

      } catch (invalid_argument const &ex) {
        cerr << "+++ The Extended DIMACS literal " << quoted(lit)
             << " contains invalid integer values: " << ex.what() << endl;
        exit(2);
      } catch (out_of_range const &ex) {
        cerr << "+++ The Extended DIMACS literal " << quoted(lit)
             << " contains an integer value that falls out of range: "
             << ex.what() << endl;
        exit(2);
      }
    }
  }
}

ostream &operator<<(ostream &output, const Row &row) {
  // overloading ostream to print a row
  // transforms a tuple (row) to a printable form
  // for (bool bit : row)
  for (size_t i = 0; i < row.size(); ++i)
    // output << to_string(bit); // bit == true ? 1 : 0;
    // output << to_string(row[i]);
    output << row[i] << " ";
  return output;
}

ostream &operator<<(ostream &output, const RowView &row) {
  // overloading ostream to print a row
  // transforms a tuple (row) to a printable form
  // for (bool bit : row)
  for (size_t i = 0; i < row.size(); ++i)
    // output << to_string(bit); // bit == true ? 1 : 0;
    // output << to_string(row[i]);
    output << row[i] << " ";
  return output;
}

template <typename M>
ostream &internal_matrix_display(ostream &output, const M &m) {
  // overloading ostream to print a matrix
  // transforms a matrix to a printable form
  const size_t n = m.num_rows();
  const size_t max_to_show = (display == yPEEK && n >= 5) ? 5 : n;
  for (size_t i = 0; i < max_to_show; ++i) {
    output << "\t";
    for (size_t j = 0; j < m.num_cols(); ++j) {
      output << m[i][j] << " ";
    }
    output << "\n";
  }
  if (display == yPEEK && n >= 5)
    output << "\t...\n" << endl;
  return output;
}

ostream &operator<<(ostream &output, const Matrix &M) {
  return internal_matrix_display<Matrix>(output, M);
}
ostream &operator<<(ostream &output, const MatrixMask &M) {
  return internal_matrix_display<MatrixMask>(output, M);
}
