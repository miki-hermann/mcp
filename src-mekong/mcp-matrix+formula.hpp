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
 *     File:    src-mekong/mcp-matrix+formula.hpp                         *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <algorithm>
#include <cstdint>
#include <deque>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "mcp-trans.hpp"

#define GLOBAL_VERSION "1.05d-mekong-"

//------------------------------------------------------------------------------

extern std::string version;
extern const std::string print_strg[];
extern const std::string display_strg[];

// domain value type
using integer = uint16_t;
// maximum value of the domain for each coordinate (cardinality - 1)

// one line in header
struct Headline {
  std::string name;
  Token token;
  integer DMAX;
  std::vector<string> elements;

  Headline() = default;
  ~Headline() = default;

  explicit Headline(std::string s,
		    Token tk,
		    integer dmax,
		    std::vector<string> el)
    : name(s), token(tk), DMAX(dmax), elements(el) {}
};
extern vector<Headline> headlines;

class RowView;

// mask into a row
using Mask = std::vector<bool>;

// data structure representing a matrix row
class Row {
public:
  using container = std::vector<integer>;

private:
  container data;

public:
  inline explicit Row() = default;
  inline explicit Row(size_t size) : data(container(size)) {}

  // constructs a row from rvalue data
  inline explicit Row(container &&data) : data(data) {}

  Row(const Row &) = delete;
  Row(Row &&) = default;
  Row &operator=(Row &&) = default;
  // explicitely clone the row
  inline Row clone() const & { return Row(container(data)); }
  inline Row to_row() const & { return clone(); }

  // get the size of the row
  inline size_t size() const & noexcept { return data.size(); }
  // constant index operation
  inline const integer &operator[](size_t index) const & noexcept {
    return data[index];
  }
  // mutable index operation
  inline integer &operator[](size_t index) & noexcept { return data[index]; }
  // add a value to the end of the row
  inline void push_back(integer value) & { data.push_back(value); }
  // preallocate enough memory for the specified number of values
  inline void reserve(size_t size) & { data.reserve(size); }
  // resize the row to the given size
  inline void resize(size_t size) & { data.resize(size); }

  // set the row to the (element-wise) minimum between itself and other
  void inplace_minimum(const Row &other) &;
  // set the row to the (element-wise) minimum between itself and other
  void inplace_minimum(const RowView &other) &;

  // total alphabetical order on rows.
  // - if self < other, return -1
  // - if self == other, return 0
  // - if self > other, return 1
  inline int total_order(const Row &other) const & {
    size_t i = 0;
    for (; i < size() && i < other.size(); ++i) {
      if (data[i] < other[i])
        return -1;
      if (data[i] > other[i])
        return 1;
    }
    if (i < size())
      return 1;
    if (i < other.size())
      return -1;
    return 0;
  }

  // restricts the row to the given set of columns
  void restrict(const Mask &m);
  // compute the z-score (standard score) of a row
  double zscore() const;

  bool operator>=(const Row &) const;
  bool operator>(const Row &) const;
  bool operator==(const Row &) const;
};

// basic matrix class
class Matrix {
public:
  using container = std::vector<Row>;

private:
  // the actual data contained in the matrix
  container data;

public:
  // empty matrix
  Matrix() = default;
  // constructs a matrix from rvalue data
  inline Matrix(container &&data) : data(std::move(data)) {}

  Matrix(const Matrix &) = delete;
  Matrix(Matrix &&) = default;
  Matrix &operator=(Matrix &&) = default;
  // explicitely clone the matrix
  inline Matrix clone() const & {
    Matrix res;
    res.reserve(num_rows());
    for (size_t i = 0; i < num_rows(); ++i)
      res.add_row(operator[](i).clone());
    return res;
  }

  // checks wether the matrix is empty
  inline bool empty() const noexcept { return data.empty(); }
  // reserves space for the rows
  inline void reserve(size_t size) { data.reserve(size); }
  // returns the number of rows
  inline size_t num_rows() const noexcept { return data.size(); }
  // returns the number of columns
  inline size_t num_cols() const noexcept {
    return data.size() > 0 ? data[0].size() : 0;
  }

  // equivalent to M[row][col]
  inline integer get(size_t row, size_t col) const noexcept {
    return data[row][col];
  }
  // returns a const view into a row
  inline const Row &operator[](size_t index) const { return data[index]; }

  // returns a mutable view into a row
  inline Row &operator[](size_t index) { return data[index]; }

  // add a new row to the matrix
  inline void add_row(Row &&new_row) & { data.push_back(std::move(new_row)); }

  // deletes a row without preserving row order
  void delete_row(size_t index);

  // restricts the matrix to the given set of columns
  void restrict(const Mask &m);
  // sort the matrix according to the total order on rows
  void sort();
  // remove duplicate rows in a sorted matrix. useful after a restriction.
  void remove_duplicates();
  // transpose matrix
  Matrix transpose() const &;

  friend class MatrixMask;
};

// immutable view into a masked row: behaves as if only a subset of columns were
// present
class RowView {
public:
  using permutation = std::vector<size_t>;

private:
  const Row &data;
  const permutation &cols;

public:
  // construct a new masked row from the original data and the column mask
  inline explicit RowView(const Row &data, const permutation &cols)
    : data(data), cols(cols) {}

  // get the size of the masked row
  inline size_t size() const { return cols.size(); }

  // equivalent to R[index] with the mask applied.
  inline integer operator[](size_t index) const { return data[cols[index]]; }

  bool operator>=(const RowView &) const;
  bool operator>=(const Row &) const;
  bool operator>(const RowView &) const;
  bool operator>(const Row &) const;
  bool operator==(const RowView &) const;
  bool operator==(const Row &) const;

  // copies the masked row to a new one.
  Row to_row() const;
};

// immutable view into a masked matrix: behave as if only a subset of columns
// were present
class MatrixMask {
public:
  using permutation = RowView::permutation;

private:
  permutation cols;
  std::reference_wrapper<const Matrix> matrix;

public:
  inline explicit MatrixMask(const Matrix &mat, const Mask &column_mask)
    : cols(std::vector<size_t>()), matrix(mat) {
    if (column_mask.size() != mat.num_cols()) {
      std::cerr
	<< "/!\\ mask size and number of columns are different, aborting!"
	<< std::endl;
      exit(2);
    }
    for (size_t i = 0; i < column_mask.size(); ++i) {
      if (column_mask[i]) {
        cols.push_back(i);
      }
    }
  }
  inline explicit MatrixMask(const Matrix &mat)
    : cols(permutation()), matrix(mat) {
    cols.reserve(mat.num_cols());
    for (size_t i = 0; i < mat.num_cols(); ++i) {
      cols.push_back(i);
    }
  }
  MatrixMask(MatrixMask &other) = default;
  MatrixMask &operator=(MatrixMask &&other) = default;

  // hides a column in the mask without erasing it from the original matrix
  inline void hide_column(size_t index) {
    auto pos = std::find(cols.begin(), cols.end(), index);
    if (pos == cols.end()) {
      std::cerr << "could not find coordinate" << std::endl;
      return;
    }
    cols.erase(pos);
  }

  // checks if the underlying matrix is empty
  inline bool empty() const { return matrix.get().empty(); }
  // returns the number of rows
  inline size_t num_rows() const { return matrix.get().num_rows(); }
  // returns the number of columns
  inline size_t num_cols() const { return cols.size(); }

  // equivalent to M[row][col], with the mask applied
  inline integer get(size_t row, size_t col) const {
    return matrix.get()[row][cols[col]];
  }
  // returns a const view into a masked row
  inline RowView operator[](size_t index) const {
    return RowView(matrix.get()[index], cols);
  }

  // copies the masked matrix to a new one.
  Matrix to_matrix() const &;
};

#define CARDlimit 50

// sign enumeration
// - lneg is `x <= nval`
// - lpos is `x >= pval`
// - lboth is `x <= nval || x >= pval`
// - lnone means no literal.
enum Sign : char { lnone = 0b00, lneg = 0b01, lpos = 0b10, lboth = 0b11 };
// string representations of Sign
extern const std::string sign_string[];

// variable <= nval or variable >= pval (or both)
struct Literal {
  Sign sign;
  integer pval, nval;

  constexpr Literal() noexcept : sign(lnone), pval(0), nval(0) {}
  ~Literal() = default;

  constexpr explicit Literal(Sign s, integer pv, integer nv)
    : sign(s), pval(pv), nval(nv) {}

  static constexpr Literal none() { return Literal(lnone, 0, 0); }
  static constexpr Literal neg(integer nval) { return Literal(lneg, 0, nval); }
  static constexpr Literal pos(integer pval) { return Literal(lpos, pval, 0); }
  static constexpr Literal both(integer pval, integer nval) {
    return Literal(lboth, pval, nval);
  }

  // inverts the literal.
  // `x <= d` becomes `x >= n - d`.
  // `x >= d` becomes `x <= n - d`.
  inline Literal swap(const size_t &index) const noexcept {
    Literal res;
    if (sign & lneg) {
      res.sign = lpos;
      res.pval = headlines[index].DMAX - nval;
    }
    if (sign & lpos) {
      res.sign = Sign(res.sign | lneg);
      res.nval = headlines[index].DMAX - pval;
    }
    return res;
  }

  inline Literal negate(const size_t &index) const noexcept {
    Literal res;
    if (sign & lneg && nval < headlines[index].DMAX) {
      res.sign = lpos;
      res.pval = nval + 1;
    }
    if (sign & lpos && nval > 0) {
      res.sign = Sign(res.sign | lneg);
      res.nval = pval - 1;
    }
    return res;
  }

  constexpr bool operator==(const Literal &other) const {
    return this->sign == other.sign && this->pval == other.pval &&
      this->nval == other.nval;
  }

  constexpr bool operator!=(const Literal &other) const {
    return this->sign != other.sign || this->pval != other.pval ||
      this->nval != other.nval;
  }

  constexpr bool operator<(const Literal &other) const {
    return this->sign < other.sign ||
      (this->sign == other.sign && this->nval < other.nval) ||
      (this->sign == other.sign && this->pval < other.pval);
  }

  // check whether a value satisfies a literal
  constexpr bool sat(integer val) const {
    return (sign & lneg && val <= nval) || (sign & lpos && val >= pval);
  }
};

// clause type, a disjunction of literals.
// the literal at index `i` is related to variable `x_i`
using Clause = std::vector<Literal>;
// formula type, a conjunction of clauses.
using Formula = std::deque<Clause>;

typedef std::map<std::string, Matrix> Group_of_Matrix;
extern Group_of_Matrix group_of_matrix;
extern std::vector<std::string> grps;

enum Action : char { aONE = 0, aALL = 1, aSELECTED = 2 };
// aNOSECT = 2,
// aSELECTED = 3 };
enum Print : char { pVOID = 0, pCLAUSE = 1, pIMPL = 2, pMIX = 3, pDIMACS = 4 };
enum Display : char {
  yUNDEF = 0,
  yHIDE = 1,
  yPEEK = 2,
  ySECTION = 3,
  ySHOW = 4
};

const std::string print_string[] = {"void", "clause", "implication", "mix",
                                    "dimacs"};

extern std::string varid;
extern bool varswitch;
enum NAME : char { nOWN = 0, nPOSITIVE = 1, nNEGATIVE = 2 };

extern const int SENTINEL;
extern const double RSNTNL;
extern const size_t MTXLIMIT;

extern Action action;
extern bool nosection;
extern std::string selected;
extern std::string suffix;
extern size_t arity;
extern size_t offset;
extern Print print_val;
extern Display display;

//------------------------------------------------------------------------------

// read a matrix from a CSV input
void read_matrix(Group_of_Matrix &matrix);
// print a matrix
void print_matrix(const Group_of_Matrix &matrix);
// read a formula from its Extended DIMACS representation
void read_formula(std::vector<size_t> &names, Formula &formula);
// clear line of leading and trailing spaces
bool clear_line (const size_t lineno, std::string &line);
// transform commas and semicolons outside strings to spaces
void uncomma_line (std::string &line);
// split a string along the specified delimiters
std::vector<std::string> split(std::string, const std::string &);
// get the Extended DIMACS representation of a formula
std::string formula2dimacs(const std::vector<size_t> &names,
                           const Formula &formula);
// get the string representation of a clause
std::string clause2string(const std::vector<size_t> &names,
                           const Clause &clause);
// get the string representation of a formula
std::string formula2string(const std::vector<size_t> &names,
                           const Formula &formula);
// get the latex representation of a formula
std::string formula2latex(const std::vector<size_t> &names,
                          const Formula &formula);
// checks that a row satisfies a clause
bool sat_clause(const RowView &tuple, const Clause &clause);
// checks that a row satisfies a formula
bool sat_formula(const RowView &tuple, const Formula &formula);

// checks that a row satisfies a clause
bool sat_clause(const Row &tuple, const Clause &clause);
// checks that a row satisfies a formula
bool sat_formula(const Row &tuple, const Formula &formula);

// checks that all rows in a matrix satisfy a clause
bool sat_clause(const Matrix &matrix, const Clause &clause);
// checks that all rows in a matrix satisfy a formula
bool sat_formula(const Matrix &matrix, const Formula &formula);

// display a row
std::ostream &operator<<(std::ostream &output, const Row &row);
// display a row view
std::ostream &operator<<(std::ostream &output, const RowView &row);
// display a matrix
std::ostream &operator<<(std::ostream &output, const Matrix &M);
// display a masked matrix
std::ostream &operator<<(std::ostream &output, const MatrixMask &M);

// combine two hash values
constexpr size_t hash_combine(size_t a, size_t b) {
  if (sizeof(size_t) >= 8)
    a ^= b + 0x517cc1b727220a95 + (a << 6) + (a >> 2);
  else
    a ^= b + 0x9e3779b9 + (a << 6) + (a >> 2);
  return a;
}

// specialization of hash for Row
template <> class std::hash<Row> {
public:
  size_t operator()(const Row &r) const noexcept {
    size_t res = 0;
    for (size_t i = 0; i < r.size(); ++i) {
      hash_combine(res, std::hash<integer>{}(r[i]));
    }
    return res;
  }
};

// specialization of hash for RowView
template <> class std::hash<RowView> {
public:
  size_t operator()(const RowView &r) const noexcept {
    size_t res = 0;
    for (size_t i = 0; i < r.size(); ++i) {
      hash_combine(res, std::hash<integer>{}(r[i]));
    }
    return res;
  }
};

template <> class std::hash<Literal> {
public:
  size_t operator()(const Literal &l) const noexcept {
    size_t res = std::hash<Sign>{}(l.sign);
    if (l.sign & lneg) {
      res = hash_combine(res, std::hash<size_t>{}(l.nval));
    }
    if (l.sign & lpos) {
      res = hash_combine(res, std::hash<size_t>{}(l.pval));
    }
    return res;
  }
};

template <> class std::hash<Clause> {
public:
  size_t operator()(const Clause &c) const noexcept {
    size_t res = 0;
    for (auto l : c) {
      res = hash_combine(res, std::hash<Literal>{}(l));
    }
    return res;
  }
};

inline bool operator==(const Clause &a, const Clause &b) {
  if (a.size() != b.size())
    return false;
  size_t i = 0;
  while (i < a.size() && a[i] == b[i])
    ++i;
  return i == a.size();
}
