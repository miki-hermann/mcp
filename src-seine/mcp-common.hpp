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
 *	Version: common for all                                           *
 *      File:    mcp-common.hpp                                           *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Given several  Boolean matrices  representing the  presence/absence of *
 * attributes in  observations, this software generates  Horn, dual Horn, *
 * or bijunctive formulas from positive and negative examples represented *
 * by these matrices.                                                     *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <deque>
#include <map>
#include "mcp-matrix+formula.hpp"

using namespace std;

//--------------------------------------------------------------------------------------------------

// extern string version;
extern bool debug;

enum Parameter : size_t {parERROR = 0,
			 parINPUT = 1,
			 parOUTPUT = 2,
			 parHEADER = 3,
			 parFORMULA = 4,
			 parTPATH = 5,
			 parLATEX = 6,
			 parWEIGHTS = 7,
			 //---
			 parACTION = 8,
			 parNOSECTION = 9,
			 parDIRECTION = 10,
			 parCLOSURE = 11,
			 parPRINT = 12,
			 parSTRATEGY = 13,
			 parSETCOVER = 14,
			 parCOOKING = 15,
			 parMATRIX = 16,
			 parOFFSET = 17,
			 parCHUNK = 18,
			 parDEBUG = 19};

enum Closure : char  {clHORN  = 0,
		      clDHORN = 1,
		      clBIJUNCTIVE = 2,
		      clAFFINE   = 3,
		      clCNF = 4};

enum Cooking : char  {ckRAW   = 0,
		      ckBLEU  = 1,
		      ckMEDIUM     = 2,
		      ckWELLDONE = 3};

enum Direction : char {dBEGIN  = 0,
		       dEND    = 1,
		       dOPT         = 2,
		       dRAND      = 3,
		       dLOWSCORE  = 4,
		       dHIGHSCORE = 5,
		       dPREC = 6};

extern const string STDIN;
extern const string STDOUT;
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
extern string headerput;
extern string weights;
extern bool disjoint;
extern int cluster;
extern string tpath;		// directory where the temporary files will be stored
extern bool np_fit;
extern unsigned chunkLIMIT;	// heavily hardware dependent; must be optimized
extern Arch arch;
extern string latex;		// file to store latex output

extern ifstream infile;
extern ifstream headerfile;
extern ifstream precfile;
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
Matrix transpose (const Matrix &batch);
int hamming_distance (const Row &u, const Row &v);
void clustering(Matrix &batch);
Row Min (const Row &a, const Row &b);
bool operator>= (const Row &a, const Row &b);
// ostream& operator<< (ostream &output, const Row &row);
// ostream& operator<< (ostream &output, const Matrix &M);
// Matrix ObsGeq (const Row &a, const Matrix &M);
unique_ptr<Row> ObsGeq (const Row &a, const Matrix &M);
bool inadmissible (const Matrix &T, const Matrix &F);
size_t hamming_weight (const Row &row);
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
		    const vector<size_t> &names, const Formula &formula);
void write_formula (const string &suffix,
		    const vector<size_t> &names, const Formula &formula);
void polswap_matrix (Matrix &A);
Formula polswap_formula (const Formula &formula);
string time2string (size_t milliseconds);

//==================================================================================================
