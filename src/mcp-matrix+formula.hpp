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
 *      File:    mcp-matrix+formula.hpp                                   *
 *                                                                        *
 *      Copyright (c) 2019 - 2023                                         *
 *                                                                        *
 * Data structures for row, matrices, literals, clauses, and formula.     *
 *                                                                        *
 * This software has been created within the ACCA Project.                *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <boost/dynamic_bitset.hpp>

#define GLOBAL_VERSION "1.04d-danube-"

using namespace std;

//------------------------------------------------------------------------------

extern string version;
extern const string print_strg[];
extern const string display_strg[];

// typedef deque<bool> Row;
typedef boost::dynamic_bitset<> Row;
typedef deque<Row> Matrix;
typedef map<string, Matrix> Group_of_Matrix;
extern Group_of_Matrix group_of_matrix;
extern vector<string> grps;

enum Action    {aONE    = 0, aALL    = 1, aNOSECT      = 2, aSELECTED  = 3};
enum Print     {pVOID   = 0, pCLAUSE = 1, pIMPL        = 2, pMIX       = 3, pDIMACS   = 4};
enum Display   {yUNDEF  = 0, yHIDE   = 1, yPEEK        = 2, ySECTION   = 3, ySHOW     = 4};

// enum Literal {lneg = -1, lnone = 0, lpos = 1};
// typedef deque<Literal> Clause;
typedef char Literal;
#define lneg  '<'
#define lnone '='
#define lpos  '>'
typedef string Clause;
typedef deque<Clause> Formula;

extern string varid;
extern bool varswitch;
extern vector<string> varnames;
enum NAME {nOWN = 0, nPOSITIVE = 1, nNEGATIVE = 2};

extern const int SENTINEL;
extern const double RSNTNL;
extern const int MTXLIMIT;

extern Action action;
extern string selected;
extern string suffix;
extern int arity;
extern int offset;
extern Print print;
extern Display display;

//------------------------------------------------------------------------------

void read_matrix (Group_of_Matrix &matrix);
void print_matrix (const Group_of_Matrix &matrix);
void read_formula (vector<int> &names, Formula &formula);
// vector<string> split (const string &strg, char delimiter);
vector<string> split (string strg, string delimiters);
string formula2dimacs (const vector<int> &names, const Formula &formula);
string formula2string (const vector<int> &names, const Formula &formula);
string formula2latex (const vector<int> &names, const Formula &formula);
bool sat_clause (const Row &tuple, const Clause &clause);
bool sat_formula (const Row &tuple, const Formula &formula);
ostream& operator<< (ostream &output, const Row &row);
ostream& operator<< (ostream &output, const Matrix &M);
void push_front (Row &row1, const bool b);
void push_front (Row &row1, const Row &row2);
bool front (const Row &row);
bool back (const Row &row);
void pop_front (Row &row);
