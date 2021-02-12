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
 *	Version: common for all                                           *
 *      File:    mcp-common.hpp                                           *
 *                                                                        *
 *      Copyright (c) 2019 - 2021                                         *
 *                                                                        *
 * Given several  Boolean matrices  representing the  presence/absence of *
 * attributes in  observations, this software generates  Horn, dual Horn, *
 * or bijunctive formulas from positive and negative examples represented *
 * by these matrices.                                                     *
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
#include "mcp-matrix+formula.hpp"

// #define GLOBAL_VERSION "1.04-c++-"

using namespace std;

//--------------------------------------------------------------------------------------------------

extern string version;
extern bool debug;

// extern string varid;
// extern bool varswitch;
// extern vector<string> varnames;
// enum NAME {nOWN = 0, nPOSITIVE = 1, nNEGATIVE = 2};

extern map<Row, int> pred;		// predecessor function for Zanuttini's algorithm
extern map<Row, int> succ;		// successor function for Zanuttini's algorithm
extern map<Row, vector<int>> sim;	// sim table for Zanuttini's algorithm

// enum Action    {aONE    = 0, aALL    = 1, aNOSECT      = 2};
enum Closure   {clHORN  = 0, clDHORN = 1, clBIJUNCTIVE = 2, clAFFINE   = 3, clCNF = 4};
enum Cooking   {ckRAW   = 0, ckBLEU  = 1, ckMEDIUM     = 2, ckWELLDONE = 3};
enum Direction {dBEGIN  = 0, dEND    = 1, dOPT         = 2, dRAND      = 3, dLOWCARD  = 4, dHIGHCARD = 5};
// enum Print     {pVOID   = 0, pCLAUSE = 1, pIMPL        = 2, pMIX       = 3, pDIMACS   = 4};
enum Strategy  {sLARGE  = 0, sEXACT  = 1};
// enum Display   {yUNDEF  = 0, yHIDE   = 1, yPEEK        = 2, ySECTION   = 3, ySHOW     = 4};
enum Arch      {archSEQ = 0, archMPI = 1, archPTHREAD  = 2, archHYBRID = 3};

extern const int SENTINEL;
extern const string STDIN;
extern const string STDOUT;
// extern const int MTXLIMIT;
extern const int CLUSTERLIMIT;

// extern Action action;
extern Closure closure;
extern Cooking cooking;
extern Direction direction;
// extern Print print;
extern bool setcover;
extern Strategy strategy;
// extern Display display;
extern string input;
extern string output;
extern bool disjoint;
// extern int arity;
extern int cluster;
// extern int offset;
extern string tpath;		// directory where the temporary files will be stored
extern bool np_fit;
extern int chunkLIMIT;		// heavily hardware dependent; must be optimized
extern Arch arch;
extern string latex;		// file to store latex output

extern ifstream infile;
extern ofstream outfile;
extern ofstream latexfile;
extern string formula_output;

extern const string action_strg[];
extern const string closure_strg[];
extern const string cooking_strg[];
extern const string direction_strg[];
extern const string pcl_strg[];
extern const string strategy_strg[];
// extern const string print_strg[];
// extern const string display_strg[];
extern const string arch_strg[];

//--------------------------------------------------------------------------------------------------

void read_arg (int argc, char *argv[]);
Matrix transpose (Matrix &batch);
int hamming_distance (const Row &u, const Row &v);
void clustering(Matrix &batch);
bool operator>= (const Row &a, const Row &b);
ostream& operator<< (ostream &output, const Row &row);
ostream& operator<< (ostream &output, const Matrix &M);
Matrix ObsGeq (const Row &a, const Matrix &M);
bool inadmissible (const Matrix &T, const Matrix &F);
int hamming_weight (const Row &row);
Row minsect (const Matrix &T, const Matrix &F);
bool satisfied_by (const Clause &clause, const Matrix &T);
Matrix restrict (const Row &sect, const Matrix &A);
Matrix HornClosure (const Matrix &M);
Row minHorn (const Matrix &M);
// void sort_formula (Formula &formula, int low, int high);
// Formula unitres (const Formula &formula);
// Formula binres (const Formula &formula);
// Formula subsumption (Formula formula);
// Formula redundant (Formula formula);
Formula primality (const Formula &phi, const Matrix &M);
Formula SetCover (const Matrix &Universe, const Formula &SubSets);
void cook (Formula &formula);
Formula learnHornExact (Matrix T);
Formula learnCNFlarge (const Matrix &F);
Formula learnCNFexact (Matrix T);
void write_formula (const string &suffix1, const string &suffix2,
		    const vector<int> &names, const Formula &formula);
void write_formula (const string &suffix,
		    const vector<int> &names, const Formula &formula);
Matrix polswap_matrix (const Matrix &A);
Formula polswap_formula (const Formula &formula);
string time2string (int seconds);

//==================================================================================================
