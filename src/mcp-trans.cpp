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
 *      File:    mcp-trans.cpp                                            *
 *                                                                        *
 *      Copyright (c) 2019 - 2023                                         *
 *                                                                        *
 * Given a meta-description of a data file, this software generates the   *
 * Boolean matrix input for mcp-seq, mcp-mpi, mcp-pthread, mcp-hybrid,    *
 * and mcp-predict.                                                       *
 *                                                                        *
 * This software has been created within the ACCA Project.                *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <vector>
#include <unordered_map>
#include <climits>
#include "mcp-matrix+formula.hpp"

using namespace std;

enum Index {LOCAL = 0, GLOBAL = 1};
enum Token {
  ERROR,
  // symbols
  EQUAL,	// =
  COLON,	// :
  SCOL,		// ;
  LPAR,		// (
  RPAR,		// )
  LBRA,		// [
  RBRA,		// ]
  PLUS,		// +
  MINUS,	// -
  QMARK,	// ?
  EXMARK,	// !
  AT,		// @
  DOLLAR,	// $
  PERCENT,	// %
  AND,		// &
  OR,		// |
  STAR,		// *
  USCORE,	// _
  LESS,		// <
  GREATER,	// >
  COMMA,	// ,
  DOT,		// .
  SLASH,	// /
  BSLASH,	// \ //
  TILDA,	// ~
  CARET,	// ^
  // entities
  STRING,
  NUM,
  FLOAT,
  SCIENTIFIC,
  // reserved words
  CONCEPT,
  PIVOT,
  BOOL,
  ENUM,
  UP,
  DOWN,
  INT,
  DISJOINT,
  OVERLAP,
  SPAN,
  WARP,
  CHECKPOINTS,
  STEP,
  DATE,
  DNUM,
  DVAL,
  TIME,
  TNUM,
  TVAL,
  WEEK,
  MONTH,
  YEAR,
  JAN,
  FEB,
  MAR,
  APR,
  MAY,
  JUN,
  JUL,
  AUG,
  SEP,
  OCT,
  NOV,
  DEC,
  MON,
  TUE,
  WED,
  THU,
  FRI,
  SAT,
  SUN
};
enum Token_Type {
  GENERAL_T = 0,
  DATE_T    = 1,
  TIME_T    = 2,
  WEEK_T    = 3,
  MONTH_T   = 4,
  YEAR_T    = 5
};
const map<Token,string> token_string = {
  {ERROR,	"ERROR"},
  //symbols
  {EQUAL,	"EQUAL"},
  {COLON,	"COLON"},
  {SCOL,	"SCOL"},
  {LPAR,	"LPAR"},
  {RPAR,	"RPAR"},
  {LBRA,	"LBRA"},
  {RBRA,	"RBRA"},
  {PLUS,	"PLUS"},
  {MINUS,	"MINUS"},
  {QMARK,	"QMARK"},
  {EXMARK,	"EXMARK"},
  {AT,		"AT"},
  {DOLLAR,	"DOLLAR"},
  {PERCENT,	"PERCENT"},
  {AND,		"AND"},
  {OR,		"OR"},
  {STAR,	"STAR"},
  {USCORE,	"USCORE"},
  {LESS,	"LESS"},
  {GREATER,	"GREATER"},
  {COMMA,	"COMMA"},
  {DOT,		"DOT"},
  {SLASH,	"SLASH"},
  {BSLASH,	"BSLASH"},
  {TILDA,	"TILDA"},
  {CARET,	"CARET"},
  //entities
  {STRING,	"STRING"},
  {NUM,		"NUM"},
  {FLOAT,	"FLOAT"},
  {SCIENTIFIC,	"SCIENTIFIC"},
  // reserved words
  {CONCEPT,	"CONCEPT"},
  {PIVOT,	"PIVOT"},
  {BOOL,	"BOOL"},
  {ENUM,	"ENUM"},
  {UP,		"UP"},
  {DOWN,	"DOWN"},
  {INT,		"INT"},
  {DISJOINT,	"DISJOINT"},
  {OVERLAP,	"OVERLAP"},
  {SPAN,	"SPAN"},
  {WARP,	"WARP"},
  {CHECKPOINTS,	"CHECKPOINTS"},
  {STEP,	"STEP"},
  {DATE,	"DATE"},
  {DNUM,	"DNUM"},
  {DVAL,	"DVAL"},
  {TIME,	"TIME"},
  {TNUM,	"TNUM"},
  {TVAL,	"TVAL"},
  {WEEK,	"WEEK"},
  {MONTH,	"MONTH"},
  {YEAR,	"YEAR"},
  {JAN,		"JAN"},
  {FEB,		"FEB"},
  {MAR,		"MAR"},
  {APR,		"APR"},
  {MAY,		"MAY"},
  {JUN,		"JUN"},
  {JUL,		"JUL"},
  {AUG,		"AUG"},
  {SEP,		"SEP"},
  {OCT,		"OCT"},
  {NOV,		"NOV"},
  {DEC,		"DEC"},
  {MON,		"MON"},
  {TUE,		"TUE"},
  {WED,		"WED"},
  {THU,		"THU"},
  {FRI,		"FRI"},
  {SAT,		"SAT"},
  {SUN,		"SUN"}
};

string msrc;				// meta source
int lineno = 0;				// line number

string yytext;				// result of yylex
bool anything   = false;
bool errorflag  = false;
int qmarkcount  = 0;
int linecount   = 0;
int dropcount   = 0;

// #define SENTINEL -1
#define STDIN    "STDIN"
#define STDOUT   "STDOUT"
#define NOSTRING " #=:;?[]"
#define DIGITS   "0123456789"

// int offset = 0;
Index idx  = LOCAL;

// for robust extension with --robust flag
const set<string> empty_string_set {};
bool robust     = false;		// generate robust extensions
int robustcount = 0;			// number of lines generated by robust extension
vector<vector<string>> incomplete;	// incomplete lines with '?' in chunks
vector<vector<int>> inc_index;		// indices of '?' in incomplete line
vector<set<string>> robust_set;		// all values appearing in coordinates
vector<vector<string>> robust_vect;	// all values appearing in coordinates
// for treating values outside intervals
enum Drop {NODROP = 0, DROP = 1, SILENT = 2};
Drop drop       = NODROP;

string input    = STDIN;
string output   = STDOUT;
string metaput  = "";
string pivotput = "";

ifstream infile;
ifstream metafile;
ofstream outfile;
ofstream pvtfile;
streambuf *backup;

Token_Type t_type = GENERAL_T;
set<string> symtab;
string desc;
vector<string> description = {" "};
int orig_column;
int concept = SENTINEL;
bool IDpresent = true;
int pivot = SENTINEL;
bool PVTpresent = false;
unordered_map<int, Token> type;
vector<int> target = {SENTINEL};
vector<vector<string>> args;

//------------------------------------------------------------------------------

void read_arg (int argc, char *argv[]) {	// reads the input parameters
  int argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "-i"
	|| arg == "--input") {
      input = argv[++argument];
    } else if (arg == "-o"
	       || arg == "--output") {
      output = argv[++argument];
    } else if (arg == "-m"
	       || arg == "--meta") {
      metaput = argv[++argument];
    } else if (arg == "-r"
	       || arg == "--robust") {
      string rpar = argv[++argument];
      if (rpar == "yes"
	  || rpar == "y"
	  || rpar == "1")
	robust = true;
      else if (rpar == "no"
	       || rpar == "n"
	       || rpar == "0")
	robust = false;
      else {
	cerr << "+++ argument error: " << arg << " " << rpar << endl;
	exit(1);
      }
    } else if (arg == "--drop") {
      string dpar = argv[++argument];
      if (dpar == "n"
	  || dpar == "no")
	drop = NODROP;
      else if (dpar == "y"
	       || dpar == "yes")
	drop = DROP;
      else if (dpar == "s"
	       || dpar == "silent")
	drop = SILENT;
      else {
	cerr << "+++ argument error: " << arg << " " << dpar << endl;
	exit(1);
      }
    } else if (arg == "--pivot"
	       || arg == "--pvt") {
      pivotput = argv[++argument];
    } else if (arg == "--index") {
      string idxpar = argv[++argument];
      if (idxpar == "l"
	  || idxpar == "loc"
	  || idxpar == "local")
	idx = LOCAL;
      else if (idxpar == "g"
	       || idxpar == "glob"
	       || idxpar == "global")
	idx = GLOBAL;
      else {
	cerr << "+++ argument error: " << arg << " " << idxpar << endl;
	exit(1);
      }
    } else if (arg == "--offset") {
      offset = stoi(argv[++argument]);
    } else if (arg == "--concept") {
      string idpar = argv[++argument];
      if (idpar == "yes"
	  || idpar == "y"
	  || idpar == "1")
	IDpresent = true;
      else if (idpar == "no"
	       || idpar == "n"
	       || idpar == "0")
	IDpresent = false;
      else {
	cerr << "+++ argument error: " << arg << " " << idpar << endl;
	exit(1);
      }
    } else {
      cerr << "+++ argument error: " << arg << endl;
      exit(1);
    }
    ++argument;
  }

  if (robust && !pivotput.empty()) {
    cerr << "+++ pivot incompatible with robust" << endl
	 << "+++ robust extensions eliminated" << endl;
    robust = false;
  }

  if (input != STDIN && output == STDOUT) {
    string::size_type pos = input.rfind('.');
    output = (pos == string::npos ? input : input.substr(0, pos)) + ".mat";
  }
  if (metaput.empty()) {
    cerr << "+++ argument error: no metafile" << endl;
    exit(1);
  }
}

void read_meta () {
  metafile.open(metaput);
  if (!metafile.is_open()) {
    cerr << "+++ Cannot open meta file " << metaput << endl;
    exit(1);
  }

  string line;
  while (getline(metafile, line)) {
    lineno++;
    string::size_type comidx = line.find('#');
    if (comidx != string::npos)
      line.erase(comidx);
    int len = 0;
    while (isspace(line[len]))
      len++;
    if (len > 0)
      line.erase(0, len);
    len = line.size()-1;
    while (isspace(line[len]))
      len--;
    if (len < line.size()-1)
      line.erase(len+1);

    bool space = false;
    string line1;
    for (int i = 0; i < line.size(); ++i)
      if (space && !isspace(line[i])) {
	line1 = line1 + " " + line[i];
	space = false;
      } else if (isspace(line[i]))
	space = true;
      else
	line1 += line[i];
    line = line1;

    if (line.size() > 0)
      msrc += " #" + to_string(lineno) + "# " + line;
  }
  metafile.close();
}

Token yylex () {
  auto nospace = msrc.find_first_not_of(" \t",0);
  msrc.erase(0, nospace);

  if (msrc.substr(0,1) == "#") {
    lineno = 0;
    int i = 1;
    while (isdigit(msrc[i]))
      lineno = 10*lineno + msrc[i++] - '0';
    msrc.erase(0, i+1);
    nospace = msrc.find_first_not_of(" \t");
    msrc.erase(0, nospace);
  }

  Token token = ERROR;
  if (anything && msrc[0] == ']') {
    anything = false;
    msrc.erase(0,1);
    token = RBRA;
  } else if  (anything && msrc[0] == ':') {
    msrc.erase(0,1);
    token = COLON;
  } else if (anything && !isspace(msrc[0])) {
    auto noany = msrc.find_first_of(" ]:");
    yytext = msrc.substr(0, noany);
    msrc.erase(0, noany);
    token = STRING;
  } else if (msrc[0] == '=') {
    msrc.erase(0,1);
    token = EQUAL;
  } else if (msrc[0] == ':') {
    msrc.erase(0,1);
    token = COLON;
  } else if (msrc[0] == ';') {
    anything = false;
    msrc.erase(0,1);
    t_type = GENERAL_T;
    token = SCOL;
  // } else if (msrc[0] == '(') {
  //   msrc.erase(0,1);
  //   token = LPAR;
  // } else if (msrc[0] == ')') {
  //   msrc.erase(0,1);
  //   token = RPAR;
  } else if (msrc[0] == '[') {
    anything = true;
    msrc.erase(0,1);
    token = LBRA;
  } else if (msrc[0] == ']') {
    anything = false;
    msrc.erase(0,1);
    token = RBRA;
  } else if (msrc[0] == '+') {
    msrc.erase(0,1);
    token = PLUS;
  } else if (msrc[0] == '-') {
    msrc.erase(0,1);
    token = MINUS;
  } else if (msrc[0] == '?') {
    msrc.erase(0,1);
    token = QMARK;
  // } else if (msrc[0] == '!') {
  //   msrc.erase(0,1);
  //   token = EXMARK;
  // } else if (msrc[0] == '@') {
  //   msrc.erase(0,1);
  //   token = AT;
  } else if (msrc[0] == '$') {
    msrc.erase(0,1);
    token = DOLLAR;
  // } else if (msrc[0] == '%') {
  //   msrc.erase(0,1);
  //   token = PERCENT;
  // } else if (msrc[0] == '&') {
  //   msrc.erase(0,1);
  //   token = AND;
  // } else if (msrc[0] == '|') {
  //   msrc.erase(0,1);
  //   token = OR;
  // } else if (msrc[0] == '*') {
  //   msrc.erase(0,1);
  //   token = STAR;
  // } else if (msrc[0] == '_') {
  //   msrc.erase(0,1);
  //   token = USCORE;
  // } else if (msrc[0] == '<') {
  //   msrc.erase(0,1);
  //   token = LESS;
  // } else if (msrc[0] == '>') {
  //   msrc.erase(0,1);
  //   token = GREATER;
  // } else if (msrc[0] == ',') {
  //   msrc.erase(0,1);
  //   token = COMMA;
  // } else if (msrc[0] == '.') {
  //   msrc.erase(0,1);
  //   token = DOT;
  // } else if (msrc[0] == '/') {
  //   msrc.erase(0,1);
  //   token = SLASH;
  // } else if (msrc[0] == '\\') {
  //   msrc.erase(0,1);
  //   token = BSLASH;
  // } else if (msrc[0] == '~') {
  //   msrc.erase(0,1);
  //   token = TILDA;
  } else if (msrc[0] == '^') {
    msrc.erase(0,1);
    token = CARET;
  } else if (t_type == GENERAL_T && msrc.substr(0, 7) == "concept" && !isalnum(msrc[7])) {
    msrc.erase(0,7);
    token = CONCEPT;
  } else if (t_type == GENERAL_T && msrc.substr(0, 5) == "pivot" && !isalnum(msrc[5])) {
    msrc.erase(0,5);
    token = PIVOT;
  } else if (t_type == GENERAL_T && msrc.substr(0, 4) == "bool" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    token = BOOL;
  } else if (t_type == GENERAL_T && msrc.substr(0, 4) == "enum" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    token = ENUM;
  } else if (t_type == GENERAL_T && msrc.substr(0, 2) == "up" &&  !isalnum(msrc[2])) {
    msrc.erase(0,2);
    token = UP;
  } else if (t_type == GENERAL_T && msrc.substr(0, 4) == "down" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    token = DOWN;
  } else if (t_type == GENERAL_T && msrc.substr(0, 3) == "int" &&  !isalnum(msrc[3])) {
    msrc.erase(0,3);
    token = INT;
  } else if (t_type == GENERAL_T && msrc.substr(0, 2) == "dj" &&  !isalnum(msrc[2])) {
    msrc.erase(0,2);
    token = DISJOINT;
  } else if (t_type == GENERAL_T && msrc.substr(0, 8) == "disjoint" &&  !isalnum(msrc[8])) {
    msrc.erase(0,8);
    token = DISJOINT;
  } else if (t_type == GENERAL_T && msrc.substr(0, 4) == "over" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    token = OVERLAP;
  } else if (t_type == GENERAL_T && msrc.substr(0, 7) == "overlap" &&  !isalnum(msrc[7])) {
    msrc.erase(0,7);
    token = OVERLAP;
  } else if (t_type == GENERAL_T && msrc.substr(0, 4) == "span" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    token = SPAN;
  } else if (t_type == GENERAL_T && msrc.substr(0, 4) == "warp" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    token = WARP;
  } else if (t_type == GENERAL_T && msrc.substr(0, 2) == "cp" &&  !isalnum(msrc[2])) {
    msrc.erase(0,2);
    token = CHECKPOINTS;
  } else if (t_type == GENERAL_T && msrc.substr(0, 11) == "checkpoints" &&  !isalnum(msrc[11])) {
    msrc.erase(0,11);
    token = CHECKPOINTS;
  } else if (msrc.substr(0, 4) == "step" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    token = STEP;
  } else if (t_type == GENERAL_T && msrc.substr(0, 4) == "date" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    t_type = DATE_T;
    token = DATE;
  } else if (t_type == GENERAL_T && msrc.substr(0, 4) == "time" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    t_type = TIME_T;
    token = TIME;
  } else if (t_type == GENERAL_T && msrc.substr(0, 4) == "week" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    t_type = WEEK_T;
    token = WEEK;
  } else if (t_type == GENERAL_T && msrc.substr(0, 5) == "month" &&  !isalnum(msrc[5])) {
    msrc.erase(0,5);
    t_type = MONTH_T;
    token = MONTH;
  } else if (t_type == GENERAL_T && msrc.substr(0, 4) == "year" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    t_type = YEAR_T;
    token = YEAR;
  } else if (msrc[0] == '.') {			// float
    auto nodigit = msrc.find_first_not_of(DIGITS, 1);
    yytext = msrc.substr(0, nodigit);
    msrc.erase(0, nodigit);
    token = FLOAT;
  } else if (isdigit(msrc[0])) {		// int or float
    auto nodigit = msrc.find_first_not_of(DIGITS, 0);
    if (msrc[nodigit] != '.') {
      yytext = msrc.substr(0, nodigit);
      msrc.erase(0, nodigit);
      token = NUM;
    } else {
      yytext = msrc.substr(0, nodigit+1);
      msrc.erase(0, nodigit+1);
      nodigit = msrc.find_first_not_of(DIGITS, 0);
      yytext = yytext + msrc.substr(0, nodigit);
      msrc.erase(0, nodigit);
      token = FLOAT;
    }
  } else if (!isspace(msrc[0])) {		// string
    auto nostring = msrc.find_first_of(NOSTRING, 0);
    yytext = msrc.substr(0, nostring);
    msrc.erase(0, nostring);
    token = STRING;
  } else {
    cerr << "+++ unexpected EOF" << endl;
    exit(1);
  }

  return token;
}

void error (const string &message) {
  errorflag = true;
  cerr << "+++ error on line " << lineno << ": " << message << endl;
}

void flush (const string &strg, const bool include) {
  auto noflush = msrc.find_first_of(strg);
  msrc.erase(0, noflush);
  if (include)
    msrc.erase(0, 1);
}

void specification () {
  Token token;
  vector<string> row_args;
  int minus;
  Token spec = yylex();
  if (spec == CONCEPT) {
    if (concept != SENTINEL) {
      error("double concept");
      flush(token_string.at(SCOL), true);
      return;
    }
    concept = orig_column;
    description[0] = desc;
    return;
  } else if (spec == PIVOT) {
    if (pivot != SENTINEL) {
      error("double pivot");
      flush(token_string.at(SCOL), true);
      return;
    }
    pivot = orig_column;
    if (input != STDIN && pivotput.empty()) {
      string::size_type pos = input.rfind('.');
      pivotput = (pos == string::npos ? input : input.substr(0, pos)) + ".pvt";
    }
    return;
  } else {
    description.push_back(desc);
  }

  switch (spec) {
  case BOOL:
  case ENUM:
  case UP:
  case DOWN:
  case INT:
  case DISJOINT:
  case OVERLAP:
  case SPAN:
  case WARP:
  case CHECKPOINTS:
    type[orig_column] = spec;
    target.push_back(orig_column);
    break;
  default:
    error("wrong start of specification");
    flush(token_string.at(SCOL), true);
    return;
  }

  switch (spec) {
  case BOOL:
  case ENUM:
  case UP:
  case DOWN:
    token = yylex();
    if (token != LBRA)
      error("missing [");
    break;
  }

  int number_of_arguments = 0;
  long double minimum, maximum;
  long double pred, succ;
  switch (spec) {
  case BOOL:
  case ENUM:
  case UP:
  case DOWN:
    while (true) {
      token = yylex();
      if (token == RBRA)
	break;
      else if (token == SCOL) {
	error("missing ]");
	msrc = ";" + msrc;
	break;
      } else if (msrc.empty()) {
	error("unexpected EOF");
	exit(1);
      }
      minus = 1;
      if (token == MINUS) {
	minus = -1;
	token = yylex();
      } else if (token == PLUS)
	token = yylex();
      if (token == NUM)
	row_args.push_back(to_string(minus * stoi(yytext)));
      else if (token == FLOAT)
	row_args.push_back(to_string(minus * stold(yytext)));
      else if (minus == -1) {
	error("after - must follow num of float");
	flush(token_string.at(SCOL), false);
	break;
      } else if (token == STRING)
	row_args.push_back(yytext);
      else if (token == QMARK)
	cerr << "+++ question mark on line " << lineno << " ignored" << endl;
      number_of_arguments++;
    }
    if (spec == BOOL && number_of_arguments != 2)
      error("bool must have 2 arguments");
    else if (number_of_arguments == 0)
      error("no arguments in " + token_string.at(spec));
    break;
  case INT:
    for (int i = 0; i <= 1; ++i) {
      token = yylex();
      minus = 1;
      if (token == MINUS) {
	minus = -1;
	token = yylex();
      } else if (token == PLUS)
	token = yylex();
      if (token == NUM)
	row_args.push_back(to_string(minus * stoi(yytext)));
      else {
	error(yytext + " must be an integer");
	flush(token_string.at(SCOL), false);
	break;
      }
    }
    if (stoi(row_args[0]) > stoi(row_args[1]))
      error("first argument must be smaller than or equal to the second");
    break;
  case DISJOINT:
  case OVERLAP:
    token = yylex();
    if (token == NUM)
      row_args.push_back(yytext);
    else {
      error("ćardinality must be a positive integer");
      flush(token_string.at(SCOL), true);
      return;
    }
    token = yylex();
    minus = 1;
    if (token == MINUS) {
      minus = -1;
      token = yylex();
    } else if (token == PLUS)
      token = yylex();
    if (token == NUM) {
      int x = minus * stoi(yytext);
      row_args.push_back(to_string(x));
      minimum = 1.0 * x;
    } else if (token == FLOAT) {
      minimum = minus * stold(yytext);
      row_args.push_back(to_string(minimum));
    } else {
      error("minimum must be a number");
      flush(token_string.at(SCOL), true);
      return;
    }
    token = yylex();
    minus = 1;
    if (token == MINUS) {
      minus = -1;
      token = yylex();
    } else if (token == PLUS)
      token = yylex();
    if (token == NUM) {
      int x = minus * stoi(yytext);
      row_args.push_back(to_string(x));
      maximum = 1.0 * x;
    } else if (token == FLOAT) {
      maximum = minus * stold(yytext);
      row_args.push_back(to_string(maximum));
    } else {
      error("maximum must be a number");
      flush(token_string.at(SCOL), true);
      return;
    }
    if (minimum >= maximum) {
      error("minimum must be smaller than maximum in " + token_string.at(spec));
    }
    if (spec == OVERLAP) {
      token = yylex();
      if (token == NUM || token == FLOAT)
	row_args.push_back(yytext);
      else {
	error("overlap must be a positive number");
	flush(token_string.at(SCOL), true);
	return;
      }
    }
    break;
  case SPAN:
  case WARP:
    token = yylex();
    if (token == NUM || token == FLOAT)
      row_args.push_back(yytext);
    else {
      error("length must be a positive number");
      flush(token_string.at(SCOL), true);
      return;
    }
    token = yylex();
    minus = 1;
    if (token == MINUS) {
      minus = -1;
      token = yylex();
    } else if (token == PLUS)
      token = yylex();
    if (token == NUM) {
      int x = minus * stoi(yytext);
      row_args.push_back(to_string(x));
      minimum = 1.0 * x;
    } else if (token == FLOAT) {
	minimum = minus * stold(yytext);
	row_args.push_back(to_string(minimum));
    }
    else {
      error("minimum must be a number");
      flush(token_string.at(SCOL), true);
      return;
    }
    token = yylex();
    minus = 1;
    if (token == MINUS) {
      minus = -1;
      token = yylex();
    } else if (token == PLUS)
      token = yylex();
    if (token == NUM) {
      int x = minus * stoi(yytext);
      row_args.push_back(to_string(x));
      maximum = 1.0 * x;
    } else if (token == FLOAT) {
      maximum = minus * stold(yytext);
      row_args.push_back(to_string(maximum));
    } else {
      error("maximum must be a number");
      flush(token_string.at(SCOL), true);
      return;
    }
    if (minimum >= maximum) {
      error("minimum must be smaller than maximum in " + token_string.at(spec));
    }
    if (spec == WARP) {
      token = yylex();
      if (token == NUM || token == FLOAT)
	row_args.push_back(yytext);
      else {
	error("overlap must be a positive number");
	flush(token_string.at(SCOL), true);
	return;
      }
    }
    break;
  case CHECKPOINTS:
    token = yylex();
    if (token == CARET) {
      row_args.push_back(token_string.at(CARET));
      token = yylex();
    }
    pred = 1.0 * LLONG_MIN;
    while (token != SCOL && token != DOLLAR && !msrc.empty()) {
      minus = 1;
      if (token == MINUS) {
	minus = -1;
	token = yylex();
      } else if (token == PLUS)
	token = yylex();

      if (token == NUM) {
	int x = minus * stoi(yytext);
	row_args.push_back(to_string(x));
	succ = 1.0 * x;
      } else if (token == FLOAT) {
	succ = minus * stold(yytext);
	row_args.push_back(to_string(succ));
      } else if (minus == -1) {
	error("after - or + must follow num of float");
	flush(token_string.at(SCOL), false);
	return;
      } else if (token == QMARK)
	cerr << "+++ question mark on line " << lineno << " ignored" << endl;
      if (succ <= pred) {
	cerr << "*** pred = " << pred << ", succ = " << succ << endl;
	error("consecutive checkpoints must have increasing values");
	flush(token_string.at(SCOL), false);
	return;
      }
      pred = succ;
      number_of_arguments++;
      token = yylex();
    }
    if (number_of_arguments == 0)
      error("no arguments in CHECKPOINTS");
    else if (number_of_arguments == 1
	     && row_args[0] != token_string.at(CARET)
	     && token != DOLLAR)
      error("singleton in CHECKPOINTS");
    if (msrc.empty()) {
	error("unexpected EOF");
	exit(1);
    } else if (token == DOLLAR)
      row_args.push_back(token_string.at(DOLLAR));
    else
      msrc = ";" + msrc;
    break;
  default:
    cerr << "+++ I should not be here +++" << endl;
    exit(1);
  }
  args.push_back(row_args);
}

void attribute_line () {
  Token token = yylex();
  if (token == STRING)
    desc = yytext;
  else {
    error("attribute description must start with a string");
    flush(token_string.at(EQUAL) + token_string.at(COLON) + token_string.at(SCOL),
	  false);
  }
  auto res_ins = symtab.insert(desc);
  if (!res_ins.second)
    error("attribute " + desc + " already exists");

  token = yylex();
  if (token != EQUAL) {
    error("missing =");
    flush(token_string.at(NUM), false);
  }

  token = yylex();
  if (token == NUM)
    orig_column = stoi(yytext);
  else {
    error("column number missing");
    flush(token_string.at(COLON), false);
  }

  token = yylex();
  if (token != COLON) {
    error("missing :");
    flush(token_string.at(SCOL), false);
  }

  specification();

  token = yylex();
  if (token != SCOL) {
    error("missing ; on previous line?");
    flush(token_string.at(SCOL), true);
  }
}

void program () {
  vector<string> dummy;
  dummy.push_back(" ");
  args.push_back(dummy);	// reserved for concept
  
  while (msrc.size() > 0)
    attribute_line();
  if (concept == SENTINEL && IDpresent)
    error("missing concept");
  else if (concept != SENTINEL && !IDpresent)
    error("superfluous concept");
  if (pivot == SENTINEL && PVTpresent)
    error("missing pivot");
}

int position (const string &item, const vector<string> &list) {
  int i = list.size()-1;
  while (i >= 0 && list[i] != item)
    i--;
  return i;
}

void IO_open () {
  if (input != STDIN) {
    infile.open(input);
    if (infile.is_open()) {
      cin.rdbuf(infile.rdbuf());
    } else {
      cerr << "+++ Cannot open input file " << input << endl;
      exit(1);
    }
  }
  
  if (output != STDOUT) {
    outfile.open(output);
    if (outfile.is_open()) {
      backup = cout.rdbuf();
      cout.rdbuf(outfile.rdbuf());
    } else {
      cerr << "+++ Cannot open output file " << output << endl;
      exit(1);
    }
  }

  if (! pivotput.empty()) {
    pvtfile.open(pivotput);
    PVTpresent = true;
    if (! pvtfile.is_open()) {
      cerr << "+++ Cannot open pivot file " << pivotput << endl;
      exit(1);
    }
  }
}

void IO_close () {
  if (input != STDIN)
    infile.close();
  if (output != STDOUT) {
    outfile.close();
    cout.rdbuf(backup);
  }
  if (! pivotput.empty())
    pvtfile.close();
}

void header () {
  cout << "1 0" << endl;
  int item_length;
  int varnum = 0;

  for (int tgt = 1; tgt < target.size(); ++tgt) {
    if (idx == LOCAL)
      varnum = 0;
    int ocl = target[tgt];
    switch (type[ocl]) {
    case BOOL:
      item_length = 1;
      break;
    case ENUM:
    case UP:
    case DOWN:
      item_length = args[tgt].size();
      break;
    case CHECKPOINTS:
      item_length = args[tgt].size()-1;
      break;
    case INT:
      item_length = stoi(args[tgt][1]) - stoi(args[tgt][0]) + 1;
      break;
    case DISJOINT:
    case OVERLAP:
      item_length = stoi(args[tgt][0]);
      break;
    case SPAN:
    case WARP:
      long double ratio = (stold(args[tgt][2]) - stold(args[tgt][1])) / stold(args[tgt][0]);
      item_length = ratio;
      item_length += (ratio - item_length > 0) ? 1 : 0;
      break;
    }
    for (int i = 1; i <= item_length; ++i) {
      cout << description[tgt] << "_"
	   << (type[ocl] == BOOL
	       || type[ocl] == ENUM
	       || type[ocl] == UP
	       || type[ocl] == DOWN
	       || type[ocl] == INT
	       ? item_length - i : i - 1)
	      + varnum + offset;

      long double min, max, ilngt;
      long double over = 0.0;
      // positive case
      cout << ":";
      switch (type[ocl]) {
      case BOOL:
	cout << description[tgt] << "==" << args[tgt][1];
	// cout << description[tgt] << "==" << args[tgt][0];
	break;
      case ENUM:
	cout << description[tgt] << "==" << args[tgt][item_length - i];
	break;
      case UP:
	cout << description[tgt] << ">=" << args[tgt][item_length - i];
	break;
      case DOWN:
	cout << description[tgt] << "<=" << args[tgt][item_length - i];
	break;
      case INT:
	cout << description[tgt] << "==" << stoi(args[tgt][1]) - i + 1;
	break;
      case DISJOINT:
      case OVERLAP:
      case SPAN:
      case WARP:
	min = stold(args[tgt][1]);
	max = stold(args[tgt][2]);
	ilngt = type[ocl] <= OVERLAP
	  ? (max - min) / stoi(args[tgt][0])
	  : stold(args[tgt][0]);
	if (type[ocl] == OVERLAP || type[ocl] == WARP)
	  over = stold(args[tgt][3]);
	cout << min + ilngt * (i-1) - over/2
	     << "<="
	     << description[tgt]
	     << "<"
	     << min + ilngt * i + over/2;
	break;
      case CHECKPOINTS:
	if (args[tgt][i-1] == token_string.at(CARET))
	  cout << description[tgt]
	     << "<"
	     << stold(args[tgt][i]);
	else if (args[tgt][i] == token_string.at(DOLLAR))
	  cout << description[tgt]
	       << ">="
	       << stold(args[tgt][i-1]);
	else
	  cout << stold(args[tgt][i-1])
	       << "<="
	       << description[tgt]
	       << "<"
	       << stold(args[tgt][i]);
	break;
      default:
	cerr << "+++ positive header: you should not be here +++" << endl;
	exit(1);
      }

      // negative case
      cout << ":";
      switch (type[ocl]) {
      case BOOL:
	cout << description[tgt] << "==" << args[tgt][0];
	// cout << description[tgt] << "==" << args[tgt][1];
	break;
      case ENUM:
	cout << description[tgt] << "!=" << args[tgt][item_length - i];
	break;
      case UP:
	cout << description[tgt] << "<" << args[tgt][item_length - i];
	break;
      case DOWN:
	cout << description[tgt] << ">" << args[tgt][item_length - i];
	break;
      case INT:
	cout << description[tgt] << "!=" << stoi(args[tgt][1]) - i + 1;
	break;
      case DISJOINT:
      case OVERLAP:
      case SPAN:
      case WARP:
	min = stold(args[tgt][1]);
	max = stold(args[tgt][2]);
	ilngt = type[ocl] <= OVERLAP
	  ? (max - min) / stoi(args[tgt][0])
	  : stold(args[tgt][0]);
	if (type[ocl] == OVERLAP || type[ocl] == WARP)
	  over = stold(args[tgt][3]);
	cout << description[tgt]
	     << "<"
	     << min + ilngt * (i-1) - over/2
	     << "||"
	     << description[tgt]
	     << ">="
	     << min + ilngt * i + over/2;
	break;
      case CHECKPOINTS:
	if (args[tgt][i-1] == token_string.at(CARET))
	  cout << description[tgt]
	     << ">="
	     << stold(args[tgt][i]);
	else if (args[tgt][i] == token_string.at(DOLLAR))
	  cout << description[tgt]
	       << "<"
	       << stold(args[tgt][i-1]);
	else
	  cout << description[tgt]
	       << "<"
	       << stold(args[tgt][i-1])
	       << "||"
	       << description[tgt]
	       << ">="
	       << stold(args[tgt][i]);
	break;
      default:
	cerr << "+++ negative header: you should not be here +++" << endl;
	exit(1);
      }

      cout << " ";
    }
    varnum += item_length;
  }
  cout << endl;
}

bool is_int (const string &s) {
  if (s.empty())
    return false;
  int start = s[0] == '-' || s[0] == '+' ? 1 : 0;
  for (int i = start; i < s.size(); ++i)
    if (! isdigit(s[i]))
      return false;
  return true;
}

bool is_float (const string &s) {
  if(s.empty())
    return false;
  int start = s[0] == '-' || s[0] == '+' ? 1 : 0;
  for (int i = start; i < s.size(); ++i)
    if (! isdigit(s[i]) && s[i] != '.')
      return false;
  return true;
}

void chunkline(const vector<string> &chunk) {
  linecount++;
  if (IDpresent)
    cout << chunk[concept];
  if (PVTpresent)
    pvtfile << chunk[pivot] << endl;
  int mypos;
  bool noflush = false;
  for (int tgt = 1; tgt < target.size(); ++tgt) {
    // if (tgt == pivot)
    // 	continue;
    int ocl = target[tgt];
    int imin, imax;
    long double min, max;
    long double over;
    bool out_of_bounds;
    switch (type[ocl]) {
    case BOOL:
      mypos = position(chunk[ocl], args[tgt]);
      if (mypos == SENTINEL) {
	noflush = true;
	dropcount++;
	switch (drop) {
	case NODROP:
	  error(chunk[ocl]
		+
		" not in bool specification on coordinate " + to_string(ocl));
	  break;
	case DROP:
	  cerr << "+++ "
	       << chunk[ocl]
	       << " not in bool specification on coordinate "
	       << to_string(ocl)
	       << endl;
	  break;
	case SILENT:
	  break;
	}
      } else
	cout << ' ' << mypos;
      break;
    case ENUM:
      mypos = position(chunk[ocl], args[tgt]);
      if (mypos == SENTINEL) {
	noflush = true;
	dropcount++;
	switch (drop) {
	case NODROP:
	  error(chunk[ocl]
		+
		" not in enum specification on coordinate " + to_string(ocl));
	  break;
	case DROP:
	  cerr << "+++ "
	       << chunk[ocl]
	       << " not in enum specification on coordinate "
	       << to_string(ocl)
	       << endl;
	  break;
	case SILENT:
	  break;
	}
      } else
	for (int j = 0; j < args[tgt].size(); ++j)
	  cout << (args[tgt].size() - 1 - j == mypos ? " 1" : " 0");
      break;
    case UP:
      mypos = position(chunk[ocl], args[tgt]);
      if (mypos == SENTINEL) {
	noflush = true;
	dropcount++;
	switch (drop) {
	case NODROP:
	  error(chunk[ocl]
		+
		" not in up specification on coordinate " + to_string(ocl));
	  break;
	case DROP:
	  cerr << "+++ "
	       << chunk[ocl]
	       << " not in up specification on coordinate "
	       << to_string(ocl)
	       << endl;
	  break;
	case SILENT:
	  break;
	}
      } else {
	for (int j = mypos+1; j < args[tgt].size(); ++j)
	  cout << " 0";
	for (int j = 0; j <= mypos; ++j)
	  cout << " 1";
      }
      break;
    case DOWN:
      mypos = position(chunk[ocl], args[tgt]);
      if (mypos == SENTINEL) {
	noflush = true;
	dropcount++;
	switch (drop) {
	case NODROP:
	  error(chunk[ocl]
		+
		" not in down specification on coordinate " + to_string(ocl));
	  break;
	case DROP:
	  cerr << "+++ "
	       << chunk[ocl]
	       << " not in down specification on coordinate "
	       << to_string(ocl)
	       << endl;
	  break;
	case SILENT:
	  break;
	}
      } else {
	for (int j = 0; j < mypos; ++j)
	  cout << " 0";
	for (int j = mypos; j < args[tgt].size(); ++j)
	  cout << " 1";
      }
      break;
    case INT:
      imin = stoi(args[tgt][0]);
      imax = stoi(args[tgt][1]);

      // cerr << "*** imin = " << imin << ", imax = " << imax << endl;

      if (is_int(chunk[ocl])) {
	int ivalue = stoi(chunk[ocl]);
	if (ivalue < imin || ivalue > imax) {
	  noflush = true;
	  dropcount++;
	  switch (drop) {
	  case NODROP:
	    error(chunk[ocl]
		  +
		  " out of bounds " + args[tgt][0] + ".." + args[tgt][1]
		  + " on coordinate " + to_string(ocl));
	    break;
	  case DROP:
	    cerr << "+++ "
		 << chunk[ocl] << " out of bounds " << args[tgt][0] << ".." << args[tgt][1]
		 << " on coordinate " << to_string(ocl) << " dropped"
		 << endl;
	    break;
	  case SILENT:
	    break;
	  }
	}
	for (int j = imax; j >= imin; --j)
	  cout << (j == ivalue ? " 1" : " 0");
      } else
	error(chunk[ocl]
	      +
	      " not an integer on coordinate " + to_string(ocl));
      break;
    case DISJOINT:
    case OVERLAP:
    case SPAN:
    case WARP:
      min = stold(args[tgt][1]);
      max = stold(args[tgt][2]);
      int icard;
      long double ilngt;
      over = 0.0;
      if (type[ocl] == DISJOINT || type[ocl] == OVERLAP) {
	icard = stoi(args[tgt][0]);
	ilngt = (max - min) / icard;
      } else if (type[ocl] == SPAN || type[ocl] == WARP) {
	ilngt = stold(args[tgt][0]);
	long double ratio = (max - min) / ilngt;
	icard = ratio;
	icard += ratio - icard > 0 ? 1 : 0;
      }
      if (type[ocl] == OVERLAP || type[ocl] == WARP)
	over = stold(args[tgt][3]);

      if (! is_int(chunk[ocl]) && ! is_float(chunk[ocl])) {
	noflush = true;
	dropcount++;
	switch (drop) {
	case NODROP:
	  error(chunk[ocl]
		+
		" is not a number on coordinate " + to_string(ocl));
	  break;
	case DROP:
	  cerr << "+++ "
	       << chunk[ocl]
	       << " is not a number on coordinate "
	       << to_string(ocl)
	       << endl;
	  break;
	case SILENT:
	  break;
	}
      } else if (stold(chunk[ocl]) < min - over / 2
		 ||
		 stold(chunk[ocl]) >= max + over / 2) {
	noflush = true;
	dropcount++;
	switch (drop) {
	case NODROP:
	  error(chunk[ocl] +
		" out of bounds " +
		to_string(min - over/2) +
		".." +
		to_string(max + over/2) +
		" on coordinate " + to_string(ocl));
	  break;
	case DROP:
	  cerr << "+++ "
	       << chunk[ocl]
	       << " out of bounds "
	       << to_string(min - over/2)
	       << ".."
	       << to_string(max + over/2)
	       << " on coordinate " << to_string(ocl)
	       << " dropped" << endl;
	  break;
	case SILENT:
	  break;
	}
      }
      else
	for (int j = 1; j <= icard; ++j)
	  cout << (stold(chunk[ocl]) >= min + ilngt * (j-1) - over/2
		   &&
		   stold(chunk[ocl]) <  min + ilngt * j + over/2
		   ? " 1" : " 0");
      break;
    case CHECKPOINTS:
      // cerr << "*** CHECKPOINTS in chunkline not implemented yet" << endl;
      // for (int kk = 0; kk < args[tgt].size(); ++kk)
      // 	cerr << "\targs[tgt][" << kk << "] = " << args[tgt][kk] << endl;
      // exit(1);
      icard = args[tgt].size()-1;
      out_of_bounds = args[tgt][0] != token_string.at(CARET)
	&& (stold(chunk[ocl]) < stold(args[tgt][0]))
	|| args[tgt][icard] != token_string.at(DOLLAR)
	&& (stold(chunk[ocl]) >= stold(args[tgt][icard]))
	? true : false;
      if (out_of_bounds) {
	noflush = true;
	dropcount++;
	switch (drop) {
	case NODROP:
	  error(chunk[ocl]
		+ " out of checkpoint bounds on coordinate "
		+ to_string(ocl));
	  break;
	case DROP:
	  cerr << "+++ "
	       << chunk[ocl]
	       << " out of checkpoint bounds "
	       << " on coordinate " << to_string(ocl)
	       << " dropped" << endl;
	  break;
	case SILENT:
	  break;
	}
      }
      else
	for (int j = 1; j <= icard; ++j)
	  cout << (
		   j == 1
		   && args[tgt][0] == token_string.at(CARET)
		   && stold(chunk[ocl]) < stold(args[tgt][1])
		   ||
		   j == icard
		   && args[tgt][icard] == token_string.at(DOLLAR)
		   && stold(chunk[ocl]) >= stold(args[tgt][icard-1])
		   ||
		   // j >= 1 &&
		   j < icard
		   && stold(args[tgt][j-1]) <= stold(chunk[ocl])
		   && stold(chunk[ocl]) < stold(args[tgt][j])
		   ? " 1" : " 0");
      break;
    default:
      cerr << "+++ chunkline: you should not be here +++" << endl;
      exit(1);
    }
  }
  if (noflush)
    cout.clear();
  else
    cout << endl;
}

void fill_robust (vector<string> &new_chunk, const vector<int> &idx, int k) {
  if (k == idx.size()) {
    robustcount++;
    chunkline(new_chunk);
  }
  else
    for (string rb: robust_vect[idx[k]]) {
      new_chunk[idx[k]] = rb;
      fill_robust(new_chunk, idx, k+1);
    }
}

void matrix () {
  lineno = 0;
  string line1;

  if (robust)
    for (int i = 0; i < target.size(); ++i)
      // robust_set.push_back(set<string>{});
      robust_set.push_back(empty_string_set);

  while(getline(cin, line1)) {
    lineno++;
    if (line1.empty())
      continue;

    auto nospace = line1.find_first_not_of(" \t");
    line1.erase(0, nospace);
    nospace = line1.find_last_not_of(" \t");
    line1.erase(nospace+1);
    if (line1.empty())
      continue;

    string line2;
    bool is_string = false;
    for (int i = 0; i < line1.size(); ++i) {
      char chr = line1[i];
      if (chr == '"') {
	is_string = ! is_string;
      } else if (is_string && chr == ' ')
	line2 = "_";
      else if (is_string && chr == '?')
	line2 = "<>";
      else if (is_string && (chr == ',' || chr == ';'))
	line2 = ".";
      else
	line2 += chr;
    }

    bool has_qmark = line2.find('?') != string::npos;
    if (has_qmark) {
      qmarkcount++;
      if (!robust)
	continue;
    }
    
    string line;
    for (int i = 0; i < line2.size(); ++i)
      line +=
	(line2[i] == '"' || line2[i] == ',' || line2[i] == ';' ? ' ' : line2[i]);
    line += ' ';

    vector<string> chunk = split(line, ' ');
    if (chunk.size() < target.size()) {
      error(to_string(target.size())
	    +
	    " elements required, but only "
	    +
	    to_string(chunk.size())
	    +
	    " present");
      return;
    }

    if (has_qmark && robust) {
      incomplete.push_back(chunk);
      vector<int> tmp;
      for (int i = 0; i < chunk.size(); ++i)
	if (chunk[i] == "?")
	  tmp.push_back(i);
      inc_index.push_back(tmp);
    }

    if (robust)
      for (int i = 1; i < target.size(); ++i) {
	int ocl = target[i];
	if (chunk[ocl] != "?")
	  robust_set[ocl].insert(chunk[ocl]);
      }

    if (!has_qmark)
      chunkline(chunk);
    if (errorflag)
      return;
  }

  if (qmarkcount > 0 && robust) {
    // robust extensions must be generated here

    for (set<string> rs : robust_set) {	// convert robust_set to robust_vect
      vector<string> tmp(rs.begin(), rs.end());
      robust_vect.push_back(tmp);
    }

    for (int j = 0; j < incomplete.size(); ++j)
      // each incomplete[j] is a defective line
      fill_robust(incomplete[j], inc_index[j], 0);

    cerr << "+++ " <<  qmarkcount
	 << " lines with missing values '?' generated robust extensions"
	 << endl;
    cerr << "+++ " << robustcount
	 << " lines produced by robust extensions"
	 << endl;
  } else if (qmarkcount > 0 && !robust)
    cerr << "+++ " << qmarkcount
	 << " lines skipped due to missing values represented by '?'"
	 << endl;
  if (dropcount > 0)
    cerr << "+++ " << dropcount
	 << (dropcount == 1 ? " line" : " lines")
	 << " dropped "
	 << endl;
  cerr << "+++ " << linecount
       << (linecount == 1 ? " line" : " lines")
       << " produced "
       << endl;
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  read_arg(argc, argv);
  read_meta();
  program();
  if (errorflag) {
    cerr << "+++ errors in meta file " << metaput << endl;
    exit(1);
  } else
    cerr << "+++ meta file " << metaput << " OK, start processing input file "
	 << input << endl;
  IO_open();
  header();
  matrix();
  IO_close();
  if (errorflag) {
    if (output != STDOUT) {
      remove(output.c_str());
      cerr << "+++ output file " << output << " deleted" << endl;
    }
    if (PVTpresent) {
      remove(pivotput.c_str());
      cerr << "+++ pivot file " << pivotput << " deleted" << endl;
    }
    cerr << "+++ runtime errors in data file " << input << endl;
  } else {
    if (output != STDOUT)
      cerr << "+++ output file " << output << " generated" << endl;
    if (PVTpresent)
      cerr << "+++ pivot file " << pivotput << " generated" << endl;
    cerr << "+++ transformation successful" << endl;
  }
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
