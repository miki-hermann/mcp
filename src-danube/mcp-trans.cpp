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
 *      File:    mcp-trans.cpp                                            *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Given a meta-description of a data file, this software generates the   *
 * Boolean matrix input for mcp-seq, mcp-mpi, mcp-pthread, mcp-hybrid,    *
 * and mcp-predict.                                                       *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <unordered_set>
#include <vector>
#include <map>
#include <unordered_map>
#include <climits>
#include <filesystem>
#include "mcp-matrix+formula.hpp"

using namespace std;

bool debug = false;

enum Index {LOCAL = 0, GLOBAL = 1};
enum Token {
  ERROR,
  // used symbols ... must agree with symbol_tab below
  EQUAL,	// =
  COLON,	// :
  SCOL,		// ;
  LBRA,		// [
  RBRA,		// ]
  PLUS,		// +
  MINUS,	// -
  QMARK,	// ?
  DOLLAR,	// $
  CARET,	// ^
  LPAR,		// (
  RPAR,		// )
  // limit of used symbols
  // unused symbols
  EXMARK,	// !
  AT,		// @
  PERCENT,	// %
  AND,		// &
  OR,		// |
  STAR,		// *
  LESS,		// <
  GREATER,	// >
  COMMA,	// ,
  SLASH,	// /
  TILDA,	// ~
  DOT,		// .
  USCORE,	// _
  BSLASH,	// \ //
  // limit of unused symbols
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
  INF,			// minimum or maximum infinity
  // not implemented yet
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
string symbol_tab = " =:;[]+-?$^()";	// must agree with Token
const unordered_map<string, Token> keywords = {
  {"concept", CONCEPT},
  {"pivot", PIVOT},
  {"bool", BOOL},
  {"enum", ENUM},
  {"up", UP},
  {"down", DOWN},
  {"int", INT},
  {"dj", DISJOINT},
  {"disjoint", DISJOINT},
  {"over", OVERLAP},
  {"overlap", OVERLAP},
  {"span", SPAN},
  {"warp", WARP},
  {"cp", CHECKPOINTS},
  {"checkpoints", CHECKPOINTS},
  {"inf", INF},
  {"INF", INF},
  // not implemented yet
  {"step", STEP},
  {"date", DATE},
  {"time", TIME},
  {"week", WEEK},
  {"month", MONTH},
  {"year", YEAR}
};
const unordered_map<Token,string> token_string = {
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
  {LPAR,	"LPAR"},
  {RPAR,	"RPAR"},
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
  {INF,         "INF"},
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
size_t qmarkcount  = 0;			// count of lines with question marks
size_t linecount   = 0;			// count of lines
size_t dropcount   = 0;			// count of dropped lines in input

// #define SENTINEL -1
#define STDIN    "STDIN"
#define STDOUT   "STDOUT"
#define NOSTRING " \t#=:;?[]"
#define DIGITS   "0123456789"
#define SPACE " \t"

// int offset = 0;
Index idx  = LOCAL;

// for robust extension with --robust flag
const unordered_set<string> empty_string_set {};
bool robust     = false;		// generate robust extensions
size_t robustcount = 0;			// number of lines generated by robust extension
vector<vector<string>> incomplete;	// incomplete lines with '?' in chunks
vector<vector<size_t>> inc_index;	// indices of '?' in incomplete line
map<size_t, set<string>> robust_set;	// all values appearing in coordinates

// for treating values outside intervals
enum Drop {NODROP = 0, DROP = 1, SILENT = 2};
Drop drop       = NODROP;

string input     = STDIN;
string output    = STDOUT;
string metaput   = "";
string headerput = "";
string pivotput  = "";
string precput   = "";

ifstream infile;
ifstream metafile;
ofstream outfile;
ofstream headerfile;
ofstream precfile;
ofstream pvtfile;
streambuf *backup;

Token_Type t_type = GENERAL_T;
unordered_set<string> symtab;
string desc;
vector<string> description = {" "};
size_t orig_column;
// concept is a keyword in c++20
int cncpt = SENTINEL;
bool IDpresent = true;
int pivot = SENTINEL;
bool PVTpresent = false;
unordered_map<size_t, Token> type;
vector<int> target;			// = {SENTINEL};
set<size_t> attribute_coords;
vector<vector<string>> args;
map<size_t, size_t> precedence;

//------------------------------------------------------------------------------

void read_arg (int argc, char *argv[]) {	// reads the input parameters
  int argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "-i"
	|| arg == "--input") {
      if (argument < argc-1) {
	input = argv[++argument];
      } else
	cerr << "+++ no input file selected" << endl;
    } else if (arg == "-o"
	       || arg == "--output") {
      if (argument < argc-1) {
	output = argv[++argument];
      } else
	cerr << "+++ no output file selected, revert to default" << endl;
    } else if (arg == "-m"
	       || arg == "--meta") {
      if (argument < argc-1) {
	metaput = argv[++argument];
      } else
	cerr << "+++ no meta file selected" << endl;
    } else if (arg == "--hdr"
	       || arg == "--header") {
      if (argument < argc-1) {
	headerput = argv[++argument];
      } else
	cerr << "+++ no header file selected, revert to default" << endl;
    } else if (arg == "-p"
	       || arg == "--prec"
	       || arg == "--precedence") {
      if (argument < argc-1) {
	precput = argv[++argument];
      } else
	cerr << "+++ no precedence file selected, revert to default" << endl;
    } else if (arg == "-r"
	       || arg == "--robust") {
      if (argument < argc-1) {
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
      } else
	cerr << "+++ no robust option selected, revert to default" << endl;
    } else if (arg == "--drop") {
      if (argument < argc-1) {
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
      } else
	cerr << "+++ no drop option selected, revert to default" << endl;
    } else if (arg == "--pivot"
	       || arg == "--pvt") {
      if (argument < argc-1) {
	pivotput = argv[++argument];
      } else
	cerr << "+++ no pivot file selected, revert to default" << endl;
    } else if (arg == "--index") {
      if (argument < argc-1) {
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
      } else
	cerr << "+++ no index option selected, revert to default" << endl;
    } else if (arg == "--offset") {
      if (argument < argc-1) {
	offset = stoi(argv[++argument]);
      } else
	cerr << "+++ no offset selected, revert to default" << endl;
    } else if (arg == "--concept"
	       || arg == "-c") {
      if (argument < argc-1) {
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
      } else
	cerr << "+++ no concept option selected, revert to default" << endl;
    } else if (arg == "--debug") {
      debug = true;
    } else {
      cerr << "+++ argument error: " << arg << endl;
      exit(1);
    }
    ++argument;
  }

  if (IDpresent)
    target.push_back(SENTINEL);

  if (robust && !pivotput.empty()) {
    cerr << "+++ pivot incompatible with robust" << endl
	 << "+++ robust extensions eliminated" << endl;
    robust = false;
  }

  if (input != STDIN && output == STDOUT) {
    string::size_type pos = input.rfind('.');
    output = (pos == string::npos ? input : input.substr(0, pos)) + ".mat";
  }
  
  if (output != STDOUT && headerput.empty()) {
    string::size_type pos = output.rfind('.');
    headerput = (pos == string::npos ? output : output.substr(0, pos)) + ".hdr";
  }
  
  if (output != STDOUT && precput.empty()) {
    string::size_type pos = output.rfind('.');
    precput = (pos == string::npos ? output : output.substr(0, pos)) + ".prc";
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
    if (line.empty())
      continue;    
    string::size_type nospace = line.find_first_not_of(" \t");
    line.erase(0, nospace);
    nospace = line.find_last_not_of(" \t\n\v\f\r");
    line.erase(nospace+1);
    if (line.empty())
      continue;

    bool space = false;
    string line1;
    for (size_t i = 0; i < line.size(); ++i)
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

Token symbol (const char ch) {
  symbol_tab[0] = ch;
  size_t found = symbol_tab.find_last_of(ch);
  return (Token) found;
}

Token yylex () {
  auto nospace = msrc.find_first_not_of(SPACE,0);
  msrc.erase(0, nospace);

  if (msrc.substr(0,1) == "#") {
    lineno = 0;
    size_t i = 1;
    while (isdigit(msrc[i]))
      lineno = 10*lineno + msrc[i++] - '0';
    msrc.erase(0, i+1);
    nospace = msrc.find_first_not_of(SPACE);
    msrc.erase(0, nospace);
  }

  Token token = symbol(msrc[0]);
  if (anything && (token == RBRA || token == COLON)) {
    anything = false;
    msrc.erase(0,1);
  // } else if  (anything & token == COLON) {
  //   anything = false;
  //   msrc.erase(0,1);
  } else if (anything && !isspace(msrc[0])) {
    auto noany = msrc.find_first_of(" ]:");
    yytext = msrc.substr(0, noany);
    msrc.erase(0, noany);
    token = STRING;
  } else if (isalpha(msrc[0])) {
    size_t nostring = msrc.find_first_of(NOSTRING);
    yytext = msrc.substr(0,nostring);
    msrc.erase(0,nostring);
    if (t_type == GENERAL_T && keywords.count(yytext) > 0
	||
	yytext == "STEP")
      token = keywords.at(yytext);
    else
      token = STRING;
    switch (token) {	// date time week month year
    case DATE:
      t_type = DATE_T;
      break;
    case TIME:
      t_type = TIME_T;
      break;
    case WEEK:
      t_type = WEEK_T;
      break;
    case MONTH:
      t_type = MONTH_T;
      break;
    case YEAR:
      t_type = YEAR_T;
      break;
    }
  } else if (msrc[0] == '.') {			// float
    auto nodigit = msrc.find_first_not_of(DIGITS, 1);
    yytext = msrc.substr(0, nodigit);
    msrc.erase(0, nodigit);
    token = FLOAT;
    // SCIENTIFIC
    if (msrc[0] == 'e' || msrc[0] == 'E') {
      size_t expstart = msrc.length() > 1 &&
	(msrc[1] == '+' || msrc[1] == '-')
	? 2 : 1;
      nodigit = msrc.find_first_not_of(DIGITS, expstart);
      yytext += msrc.substr(0, nodigit);
      msrc.erase(0, nodigit);
      token = SCIENTIFIC;
    }
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
    // SCIENTIFIC
    if (msrc[0] == 'e' || msrc[0] == 'E') {
      size_t expstart = msrc.length() > 1 &&
	(msrc[1] == '+' || msrc[1] == '-')
	? 2 : 1;
      nodigit = msrc.find_first_not_of(DIGITS, expstart);
      yytext += msrc.substr(0, nodigit);
      msrc.erase(0, nodigit);
      token = SCIENTIFIC;
    }
  } else if (token != ERROR) {			// symbol
    msrc.erase(0,1);
    if (token == RBRA || token == SCOL)
      anything = false;
    else if (token == LBRA)
      anything = true;
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

// catch PLUS or MINUS
size_t catch_pm (Token &token) {
  int minus = 1;
  if (token == MINUS) {
    minus = -1;
    token = yylex();
  } else if (token == PLUS)
    token = yylex();
  return minus;
}

void specification () {
  Token token;
  vector<string> row_args;
  int minus;
  Token spec = yylex();
  if (spec == CONCEPT) {
    if (cncpt != SENTINEL) {
      error("double concept");
      flush(token_string.at(SCOL), true);
      return;
    }
    if (pivot != SENTINEL) {
      error("both concept and pivot");
      flush(token_string.at(SCOL), true);
      return;
    }
    cncpt = orig_column;
    description[0] = desc;
    return;
  } else if (spec == PIVOT) {
    if (pivot != SENTINEL) {
      error("double pivot");
      flush(token_string.at(SCOL), true);
      return;
    }
    if (cncpt != SENTINEL) {
      error("both concept and pivot");
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
  case STEP:
  case DATE:
  case DNUM:
  case DVAL:
  case TIME:
  case TNUM:
  case TVAL:
  case WEEK:
  case MONTH:
  case YEAR:
  case JAN:
  case FEB:
  case MAR:
  case APR:
  case MAY:
  case JUN:
  case JUL:
  case AUG:
  case SEP:
  case OCT:
  case NOV:
  case DEC:
  case MON:
  case TUE:
  case WED:
  case THU:
  case FRI:
  case SAT:
  case SUN:
    error(token_string.at(spec) + ": extended syntax not implemented yet");
    flush(token_string.at(SCOL), true);
    return;
  default:
    error("wrong start of attribute specification");
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
    token = yylex();
    while (token != RBRA) {
      if (token == SCOL) {
	error("missing ]");
	msrc = ";" + msrc;
	break;
      } else if (msrc.empty()) {
	error("unexpected EOF");
	exit(1);
      }
      minus = catch_pm(token);
      if (token == NUM)
	row_args.push_back(to_string(minus * stoi(yytext)));
      else if (token == FLOAT || token == SCIENTIFIC)
	row_args.push_back(to_string(minus * stold(yytext)));
      else if (minus == -1) {
	error("after - must follow unsigned number, float, or scientific");
	flush(token_string.at(SCOL), false);
	break;
      } else if (token == STRING)
	row_args.push_back(yytext);
      else if (token == QMARK)
	cerr << "+++ question mark on line " << lineno << " ignored" << endl;
      number_of_arguments++;
      token = yylex();
    }
    if (spec == BOOL && number_of_arguments != 2)
      error("bool must have 2 arguments");
    else if (number_of_arguments == 0)
      error("no arguments in " + token_string.at(spec));
    break;
  case INT:
    for (size_t i = 0; i <= 1; ++i) {
      token = yylex();
      minus = catch_pm(token);
      if (token == NUM)
	row_args.push_back(to_string(minus * stol(yytext)));
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
      error("ćardinality must be an unsigned positive integer");
      flush(token_string.at(SCOL), true);
      return;
    }
    token = yylex();
    minus = catch_pm(token);
    if (token == NUM) {
      long x = minus * stol(yytext);
      row_args.push_back(to_string(x));
      minimum = 1.0 * x;
    } else if (token == FLOAT || token == SCIENTIFIC) {
      minimum = minus * stold(yytext);
      row_args.push_back(to_string(minimum));
    } else {
      error("minimum must be a number");
      flush(token_string.at(SCOL), true);
      return;
    }
    token = yylex();
    minus = catch_pm(token);
    if (token == NUM) {
      long x = minus * stol(yytext);
      row_args.push_back(to_string(x));
      maximum = 1.0 * x;
    } else if (token == FLOAT || token == SCIENTIFIC) {
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
      if (token == NUM || token == FLOAT || token == SCIENTIFIC)
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
    if (token == NUM || token == FLOAT || token == SCIENTIFIC)
      row_args.push_back(yytext);
    else {
      error("length must be an unsigned positive number");
      flush(token_string.at(SCOL), true);
      return;
    }
    token = yylex();
    minus = catch_pm(token);
    if (token == NUM) {
      long x = minus * stol(yytext);
      row_args.push_back(to_string(x));
      minimum = 1.0 * x;
    } else if (token == FLOAT || token == SCIENTIFIC) {
	minimum = minus * stold(yytext);
	row_args.push_back(to_string(minimum));
    }
    else {
      error("minimum must be a number");
      flush(token_string.at(SCOL), true);
      return;
    }
    token = yylex();
    minus = catch_pm(token);
    if (token == NUM) {
      long x = minus * stol(yytext);
      row_args.push_back(to_string(x));
      maximum = 1.0 * x;
    } else if (token == FLOAT || token == SCIENTIFIC) {
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
      if (token == NUM || token == FLOAT || token == SCIENTIFIC)
	row_args.push_back(yytext);
      else {
	error("overlap must be an unsigned positive number");
	flush(token_string.at(SCOL), true);
	return;
      }
    }
    break;
  case CHECKPOINTS:
    pred = 1.0 * LLONG_MIN;

    token = yylex();
    if (token == MINUS) {
      token = yylex();
      if (token == INF) {
	row_args.push_back(token_string.at(CARET));
	succ = pred + 1.0;
      } else if (token == NUM) {
	long x = -1 * stol(yytext);
	row_args.push_back(to_string(x));
	succ = 1.0 * x;
      } else if (token == FLOAT || token == SCIENTIFIC) {
	succ = -1 * stold(yytext);
	row_args.push_back(to_string(succ));
      } else {
	error("after - must follow num of float, scientific, or inf");
	flush(token_string.at(SCOL), false);
	return;
      }
      if (succ <= pred) {
	error("consecutive checkpoints must have increasing values");
	flush(token_string.at(SCOL), false);
	return;
      }
      pred = succ;
      if (token != INF)		// not sure about this
	number_of_arguments++;
      token = yylex();
    } else if (token == CARET) {
      row_args.push_back(token_string.at(CARET));
      token = yylex();
    }
    
    while (token != SCOL
	   && token != DOLLAR
	   && !msrc.empty()) {
      minus = catch_pm(token);

      if (token == INF) {
	if (minus == -1) {
	  error("minimal infinity allowed only at first position");
	  flush(token_string.at(SCOL), false);
	  return;
	}
	token = DOLLAR;
	break;
      } else if (token == NUM) {
	size_t x = minus * stoll(yytext);
	row_args.push_back(to_string(x));
	succ = 1.0 * x;
      } else if (token == FLOAT || token == SCIENTIFIC) {
	succ = minus * stold(yytext);
	row_args.push_back(to_string(succ));
      } else if (minus == -1) {
	error("after - or + must follow num, float, or scientific");
	flush(token_string.at(SCOL), false);
	return;
      } else if (token == QMARK)
	cerr << "+++ question mark on line " << lineno << " ignored" << endl;
      if (succ <= pred) {
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
    flush(token_string.at(EQUAL)
	  + token_string.at(COLON)
	  + token_string.at(SCOL),
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
  if (token == NUM) {
    orig_column = stoul(yytext);
    attribute_coords.insert(orig_column);
  } else {
    error("column number missing");
    flush(token_string.at(COLON) + token_string.at(LPAR), false);
  }

  token = yylex();
  if (token == LPAR) {
    token = yylex();
    if (token != NUM) {
      error("precedence weight must be a positive integer");
      flush(token_string.at(COLON) + token_string.at(RPAR), false);
    } else if (precedence.count(orig_column) > 0) {
      error("precedence for column "
	    + to_string(orig_column)
	    + " exists already");
    } else
      precedence[orig_column] = stoul(yytext);
    token = yylex();
    if (token != RPAR) {
      error("missing )");
      flush(token_string.at(COLON), false);
    } else
      token = yylex();
  }
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
  if (cncpt == SENTINEL && IDpresent)
    error("missing concept");
  else if (cncpt != SENTINEL && !IDpresent)
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

  if (! headerput.empty()) {
    headerfile.open(headerput);
    if (! headerfile.is_open()) {
      cerr << "+++ Cannot open header file " << headerput << endl;
      exit(1);
    }
  }

  if (! precput.empty()) {
    if (filesystem::exists(precput)) {
      cerr << "+++ precedence file " << precput << " exists" << endl
	   << "... no new precedence file generated" << endl;
      precput.clear();
    } else {
      precfile.open(precput);
      if (! precfile.is_open()) {
	cerr << "+++ Cannot open precedence file " << precput << endl;
	exit(1);
      }
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

void header2matrix () {
  headerfile.close();
  // cout.rdbuf(backup);
  
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
}

void IO_close () {
  if (input != STDIN)
    infile.close();
  if (output != STDOUT) {
    outfile.close();
    cout.rdbuf(backup);
  }
  if (! precput.empty())
    precfile.close();
  if (! pivotput.empty())
    pvtfile.close();
}

void header () {
  size_t item_length;
  size_t varnum = 0;
  size_t precount = 0;

  for (size_t tgt = 1; tgt < target.size(); ++tgt) {
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
      item_length = stol(args[tgt][0]);
      break;
    case SPAN:
    case WARP:
      long double ratio = (stold(args[tgt][2]) - stold(args[tgt][1])) / stold(args[tgt][0]);
      item_length = ratio;
      item_length += (ratio - item_length > 0) ? 1 : 0;
      break;
    }
    for (int i = 1; i <= item_length; ++i) {
      headerfile << description[tgt] << "_"
	   << (type[ocl] == BOOL
	       || type[ocl] == ENUM
	       || type[ocl] == UP
	       || type[ocl] == DOWN
	       || type[ocl] == INT
	       ? item_length - i : i - 1)
	      + varnum + offset;
      
      if (! precput.empty())
	precfile << precount++ << " "
		 << (precedence.count(ocl) > 0
		     ? precedence.at(ocl)
		     : 50)
		 << endl;

      long double min, max, ilngt;
      long double over = 0.0;
      // positive case
      headerfile << ":";
      switch (type[ocl]) {
      case BOOL:
	headerfile << description[tgt] << "==" << args[tgt][1];
	// headerfile << description[tgt] << "==" << args[tgt][0];
	break;
      case ENUM:
	headerfile << description[tgt] << "==" << args[tgt][item_length - i];
	break;
      case UP:
	headerfile << description[tgt] << ">=" << args[tgt][item_length - i];
	break;
      case DOWN:
	headerfile << description[tgt] << "<=" << args[tgt][item_length - i];
	break;
      case INT:
	headerfile << description[tgt] << "==" << stoi(args[tgt][1]) - i + 1;
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
	headerfile << min + ilngt * (i-1) - over/2
	     << "<="
	     << description[tgt]
	     << "<"
	     << min + ilngt * i + over/2;
	break;
      case CHECKPOINTS:
	if (args[tgt][i-1] == token_string.at(CARET))
	  headerfile << description[tgt]
	     << "<"
	     << stold(args[tgt][i]);
	else if (args[tgt][i] == token_string.at(DOLLAR))
	  headerfile << description[tgt]
	       << ">="
	       << stold(args[tgt][i-1]);
	else
	  headerfile << stold(args[tgt][i-1])
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
      headerfile << ":";
      switch (type[ocl]) {
      case BOOL:
	headerfile << description[tgt] << "==" << args[tgt][0];
	// headerfile << description[tgt] << "==" << args[tgt][1];
	break;
      case ENUM:
	headerfile << description[tgt] << "!=" << args[tgt][item_length - i];
	break;
      case UP:
	headerfile << description[tgt] << "<" << args[tgt][item_length - i];
	break;
      case DOWN:
	headerfile << description[tgt] << ">" << args[tgt][item_length - i];
	break;
      case INT:
	headerfile << description[tgt] << "!=" << stoi(args[tgt][1]) - i + 1;
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
	headerfile << description[tgt]
	     << "<"
	     << min + ilngt * (i-1) - over/2
	     << "||"
	     << description[tgt]
	     << ">="
	     << min + ilngt * i + over/2;
	break;
      case CHECKPOINTS:
	if (args[tgt][i-1] == token_string.at(CARET))
	  headerfile << description[tgt]
	     << ">="
	     << stold(args[tgt][i]);
	else if (args[tgt][i] == token_string.at(DOLLAR))
	  headerfile << description[tgt]
	       << "<"
	       << stold(args[tgt][i-1]);
	else
	  headerfile << description[tgt]
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

      // headerfile << " ";
      headerfile << endl;
    }
    varnum += item_length;
  }
  // headerfile << endl;
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

void chunkline (const vector<string> &chunk) {
  linecount++;
  if (IDpresent)
    cout << chunk[cncpt];
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
		 << chunk[ocl] << " out of bounds "
		 << args[tgt][0] << ".." << args[tgt][1]
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
	for (int j = 1; j <= icard; ++j) {
	  cout << (
		   j == 1
		   && args[tgt][0] == token_string.at(CARET)
		   && stold(chunk[ocl]) < stold(args[tgt][1])
		   ||
		   j == icard
		   && args[tgt][icard] == token_string.at(DOLLAR)
		   && stold(chunk[ocl]) >= stold(args[tgt][icard-1])
		   ||
		   j > 1 &&
		   j < icard
		   && stold(args[tgt][j-1]) <= stold(chunk[ocl])
		   && stold(chunk[ocl]) < stold(args[tgt][j])
		   ? " 1" : " 0");
	}
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

void fill_robust (vector<string> &new_chunk,
		  const vector<size_t> &idx,
		  const size_t k) {
  if (k == idx.size()) {
    robustcount++;
    chunkline(new_chunk);
  } else
    for (const string &rb: robust_set.at(idx[k])) {
      new_chunk[idx[k]] = rb;
      fill_robust(new_chunk, idx, k+1);
    }
}

void matrix () {
  lineno = 0;
  string line;

  while(getline(cin, line)) {
    lineno++;
    if (line.empty() || ! clear_line(lineno, line))
      continue;
    uncomma_line(line);
    const vector<string> chunk = split(line, SPACE);
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
    bool has_qmark = false;
    vector<size_t> qmarks;
    for (size_t i = 0; i < chunk.size(); ++i)
      if (chunk[i] == "?" && attribute_coords.count(i) > 0) {
	// only coordinates which have been explicitly written in
	// meta-file (active coordinates) are kept
	has_qmark = true;
	qmarks.push_back(i);
	// only lines with '?' in active coordinates are kept,
	// otherwise qmark is ignored
      }
    qmarkcount += has_qmark;
    if (has_qmark && !robust)
      continue;
    else if (has_qmark && robust) {
	inc_index.push_back(qmarks);
	incomplete.push_back(chunk);
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

    for (size_t j = 0; j < incomplete.size(); ++j)
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
  version += "trans";
  cerr << "+++ version = " << version << endl;

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
  header2matrix();
  matrix();
  IO_close();
  if (errorflag) {
    if (!headerput.empty()) {
      remove(headerput.c_str());
      cerr << "+++ header file " << headerput << " deleted" << endl;
    }
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
    if (output != STDOUT) {
      cerr << "+++ header file " << headerput << " generated" << endl;
      cerr << "+++ output file " << output << " generated" << endl;
    }
    if (PVTpresent)
      cerr << "+++ pivot file " << pivotput << " generated" << endl;
    cerr << "+++ transformation successful" << endl;
  }
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
