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
 *      File:    mcp-trans.cpp                                            *
 *                                                                        *
 *      Copyright (c) 2019 - 2021                                         *
 *                                                                        *
 * Given a meta-description of a data file, this software generates the   *
 * Boolean matrix input for mcp-seq, mcp-mpi, mcp-pthread, and mcp-hybrid.*
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

using namespace std;

enum Index {LOCAL = 0, GLOBAL = 1};
enum Token {ERROR  =  0,
	    EQUAL  =  1,
	    COLON  =  2,
	    SCOL   =  3,
	    LBRA   =  4,
	    RBRA   =  5,
	    MINUS  =  6,
	    QMARK  =  7,
	    STRING =  8,
	    NUM    =  9,
	    FLOAT  = 10,
	    IDENT  = 11,
	    BOOL   = 12,
	    ENUM   = 13,
	    UP     = 14,
	    DOWN   = 15,
	    INT    = 16,
	    DJ     = 17,
	    OVER   = 18,
	    SPAN   = 19,
	    WARP   = 20};
const string token_string[] = {"ERROR",
			       "EQUAL",
			       "COLON",
			       "SCOL",
			       "LBRA",
			       "RBRA",
			       "MINUS",
			       "QMARK",
			       "STRING",
			       "NUM",
			       "FLOAT",
			       "IDENT",
			       "BOOL",
			       "ENUM",
			       "UP",
			       "DOWN",
			       "INT",
			       "DJ",
			       "OVER",
			       "SPAN",
			       "WARP"};

string msrc;				// meta source
int lineno = 0;				// line number

string yytext;				// result of yylex
bool anything  = false;
bool errorflag = false;
int qmarkcount = 0;
int linecount  = 0;

#define SENTINEL -1
#define STDIN    "STDIN"
#define STDOUT   "STDOUT"
#define NOSTRING " #=:;?[]"
#define DIGITS   "0123456789"

int offset = 0;
Index idx  = LOCAL;

string input   = STDIN;
string output  = STDOUT;
string metaput = " ";

ifstream infile;
ifstream metafile;
ofstream outfile;
streambuf *backup;

set<string> symtab;
string desc;
vector<string> description = {" "};
int orig_column;
int ident = SENTINEL;
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
    } else if (arg == "--index") {
      ++argument;
      if (argv[argument] == "l"
	  || argv[argument] == "loc"
	  || argv[argument] == "local")
	idx = LOCAL;
      else if (argv[argument] == "g"
	       || argv[argument] == "glob"
	       || argv[argument] == "global")
	idx = GLOBAL;
      else {
	cerr << "+++ argument error: " << arg << " " << argv[argument] << endl;
	exit(1);
      }
    } else if (arg == "--offset") {
      offset = stoi(argv[++argument]);
    } else {
      cerr << "+++ argument error: " << arg << endl;
      exit(1);
    }
    ++argument;
  }
  if (input != STDIN && output == STDOUT) {
    string::size_type pos = input.rfind('.');
    output = (pos == string::npos ? input : input.substr(0, pos)) + ".mat";
  }
  if (metaput == " ") {
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
    token = SCOL;
  } else if (msrc[0] == '[') {
    anything = true;
    msrc.erase(0,1);
    token = LBRA;
  } else if (msrc[0] == ']') {
    anything = false;
    msrc.erase(0,1);
    token = RBRA;
  } else if (msrc[0] == '-') {
    msrc.erase(0,1);
    token = MINUS;
  } else if (msrc[0] == '?') {
    msrc.erase(0,1);
    token = QMARK;
  } else if (msrc.substr(0, 5) == "ident" && !isalnum(msrc[5])) {
    msrc.erase(0,5);
    token = IDENT;
  } else if (msrc.substr(0, 4) == "bool" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    token = BOOL;
  } else if (msrc.substr(0, 4) == "enum" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    token = ENUM;
  } else if (msrc.substr(0, 2) == "up" &&  !isalnum(msrc[2])) {
    msrc.erase(0,2);
    token = UP;
  } else if (msrc.substr(0, 4) == "down" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    token = DOWN;
  } else if (msrc.substr(0, 3) == "int" &&  !isalnum(msrc[3])) {
    msrc.erase(0,3);
    token = INT;
  } else if (msrc.substr(0, 2) == "dj" &&  !isalnum(msrc[2])) {
    msrc.erase(0,2);
    token = DJ;
  } else if (msrc.substr(0, 4) == "over" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    token = OVER;
  } else if (msrc.substr(0, 4) == "span" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    token = SPAN;
  } else if (msrc.substr(0, 4) == "warp" &&  !isalnum(msrc[4])) {
    msrc.erase(0,4);
    token = WARP;
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
    cerr << "+++ inexpected EOF" << endl;
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
  if (spec == IDENT) {
    ident = orig_column;
    description[0] = desc;
    return;
  } else {
    description.push_back(desc);
  }

  if (spec >= BOOL && spec <= WARP) {
    type[orig_column] = spec;
    target.push_back(orig_column);
  } else {
    error("wrong start of specification");
    flush(token_string[SCOL], true);
    return;
  }

  if (spec >= BOOL && spec <= DOWN) {
    token = yylex();
    if (token != LBRA)
      error("missing [");
  }

  int number_of_arguments = 0;
  if (spec >= BOOL && spec <= DOWN) {
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
      }
      if (token == NUM)
	row_args.push_back(to_string(minus * stoi(yytext)));
      else if (token == FLOAT)
	row_args.push_back(to_string(minus * stof(yytext)));
      else if (minus == -1) {
	error("after - must follow num of float");
	flush(token_string[SCOL], false);
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
      error("no arguments");
  } else if (spec == INT) {
    for (int i = 0; i <= 1; ++i) {
      token = yylex();
      minus = 1;
      if (token == MINUS) {
	minus = -1;
	token = yylex();
      }
      if (token == NUM)
	row_args.push_back(to_string(minus * stoi(yytext)));
      else {
	error(yytext + " must be an integer");
	flush(token_string[SCOL], false);
	break;
      }
    }
    if (stoi(row_args[0]) > stoi(row_args[1]))
      error("first argument must be smaller or equal than the second");
  } else if (spec >= DJ && spec <= OVER) {
    token = yylex();
    if (token == NUM)
      row_args.push_back(yytext);
    else {
      error("ćardinality must be a positive integer");
      flush(token_string[SCOL], true);
      return;
    }
    token = yylex();
    minus = 1;
    if (token == MINUS) {
      minus = -1;
      token = yylex();
    }
    if (token == NUM)
      row_args.push_back(to_string(minus * stoi(yytext)));
    else if (token == FLOAT)
      row_args.push_back(to_string(minus * stof(yytext)));
    else {
      error("minimum must be a number");
      flush(token_string[SCOL], true);
      return;
    }
    token = yylex();
    minus = 1;
    if (token == MINUS) {
      minus = -1;
      token = yylex();
    }
    if (token == NUM)
      row_args.push_back(to_string(minus * stoi(yytext)));
    else if (token == FLOAT)
      row_args.push_back(to_string(minus * stof(yytext)));
    else {
      error("maximum must be a number");
      flush(token_string[SCOL], true);
      return;
    }
    if (spec == OVER) {
      token = yylex();
      if (token == NUM || token == FLOAT)
	row_args.push_back(yytext);
      else {
	error("overlap must be a positive number");
	flush(token_string[SCOL], true);
	return;
      }
    }
  } else if (spec >= SPAN && spec <= WARP) {
    token = yylex();
    if (token == NUM || token == FLOAT)
      row_args.push_back(yytext);
    else {
      error("length must be a positive number");
      flush(token_string[SCOL], true);
      return;
    }
    token = yylex();
    minus = 1;
    if (token == MINUS) {
      minus = -1;
      token = yylex();
    }
    if (token == NUM)
      row_args.push_back(to_string(minus * stoi(yytext)));
    else if (token == FLOAT)
	row_args.push_back(to_string(minus * stof(yytext)));
    else {
      error("minimum must be a number");
      flush(token_string[SCOL], true);
      return;
    }
    token = yylex();
    minus = 1;
    if (token == MINUS) {
      minus = -1;
      token = yylex();
    }
    if (token == NUM)
      row_args.push_back(to_string(minus * stoi(yytext)));
    else if (token == FLOAT)
	row_args.push_back(to_string(minus * stof(yytext)));
    else {
      error("maximum must be a number");
      flush(token_string[SCOL], true);
      return;
    }
    if (spec == WARP) {
      token = yylex();
      if (token == NUM || token == FLOAT)
	row_args.push_back(yytext);
      else {
	error("overlap must be a psitive number");
	flush(token_string[SCOL], true);
	return;
      }
    }
  } else {
    cerr << "+++ I should not be here +++" << endl;
    exit(1);
  }
  args.push_back(row_args);
}

void command_line () {
  Token token = yylex();
  if (token == STRING)
    desc = yytext;
  else {
    error("description must start with a string");
    flush(token_string[EQUAL] + token_string[COLON] + token_string[SCOL],
	  false);
  }
  auto res_ins = symtab.insert(desc);
  if (!res_ins.second)
    error(desc + " already exists");

  token = yylex();
  if (token != EQUAL) {
    error("missing =");
    flush(token_string[NUM], false);
  }

  token = yylex();
  if (token == NUM)
    orig_column = stoi(yytext);
  else {
    error("column number missing");
    flush(token_string[COLON], false);
  }

  token = yylex();
  if (token != COLON) {
    error("missing :");
    flush(token_string[SCOL], false);
  }

  specification();

  token = yylex();
  if (token != SCOL) {
    error("missing ; on previous line?");
    flush(token_string[SCOL], true);
  }
}

void program () {
  vector<string> dummy;
  dummy.push_back(" ");
  args.push_back(dummy);	// reserved for ident
  
  while (msrc.size() > 0)
    command_line();
  if (ident == SENTINEL)
    error("no identifier");
}

int position (string &item, vector<string> &list) {
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
}

void IO_close () {
  if (input != STDIN)
    infile.close();
  if (output != STDOUT) {
    outfile.close();
    cout.rdbuf(backup);
  }
}

void header () {
  cout << "1 0" << endl;
  int item_length;
  int varnum = 0;

  for (int tgt = 1; tgt < target.size(); ++tgt) {
    if (idx == LOCAL)
      varnum = 0;
    int ocl = target[tgt];
    if (type[ocl] == BOOL)
      item_length = 1;
    else if (type[ocl] >= ENUM && type[ocl] <= DOWN)
      item_length = args[tgt].size();
    else if (type[ocl] == INT)
      item_length = stoi(args[tgt][1]) - stoi(args[tgt][0]) + 1;
    else if (type[ocl] >= DJ && type[ocl] <= OVER)
      item_length = stoi(args[tgt][0]);
    else if (type[ocl] >= SPAN && type[ocl] <= WARP) {
      float ratio = (stof(args[tgt][2]) - stof(args[tgt][1])) / stof(args[tgt][0]);
      item_length = ratio;
      item_length += (ratio - item_length > 0) ? 1 : 0;
    }
    for (int i = 1; i <= item_length; ++i) {
      cout << description[tgt] << "_"
	   << (type[ocl] >= BOOL && type[ocl] <= INT
	       ? item_length - i
	       : i - 1) + varnum + offset;

      float min, max, ilngt;
      float over = 0.0;
      // positive case
      cout << ":";
      switch (type[ocl]) {
      case BOOL:
	// cout << description[tgt] << "==" << args[tgt][1];
	cout << description[tgt] << "==" << args[tgt][0];
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
      case DJ:
      case OVER:
      case SPAN:
      case WARP:
	min = stof(args[tgt][1]);
	max = stof(args[tgt][2]);
	ilngt = type[ocl] <= OVER
	  ? (max - min) / stoi(args[tgt][0])
	  : stof(args[tgt][0]);
	if (type[ocl] == OVER || type[ocl] == WARP)
	  over = stof(args[tgt][3]);
	cout << min + ilngt * (i-1) - over/2
	     << "<="
	     << description[tgt]
	     << "<"
	     << min + ilngt * i + over/2;
	break;
      }

      // negative case
      cout << ":";
      switch (type[ocl]) {
      case BOOL:
	// cout << description[tgt] << "==" << args[tgt][0];
	cout << description[tgt] << "==" << args[tgt][1];
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
      case DJ:
      case OVER:
      case SPAN:
      case WARP:
	min = stof(args[tgt][1]);
	max = stof(args[tgt][2]);
	ilngt = type[ocl] <= OVER
	  ? (max - min) / stoi(args[tgt][0])
	  : stof(args[tgt][0]);
	if (type[ocl] == OVER || type[ocl] == WARP)
	  over = stof(args[tgt][3]);
	cout << description[tgt]
	     << "<" << min + ilngt * (i-1) - over/2
	     << "||"
	     << description[tgt]
	     << ">="
	     << min + ilngt * i + over/2;
	break;
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
  int start = s[0] == '-' ? 1 : 0;
  for (int i = start; i < s.size(); ++i)
    if (! isdigit(s[i]))
      return false;
  return true;
}

bool is_float (const string &s) {
  if(s.empty())
    return false;
  int start = s[0] == '-' ? 1 : 0;
  for (int i = start; i < s.size(); ++i)
    if (! isdigit(s[i]) && s[i] != '.')
      return false;
  return true;
}

void matrix () {
  lineno = 0;
  string line1;
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
	line2 = line2 + '_';
      else if (is_string && chr == '?')
	line2 = line2 + "<>";
      else if (is_string && (chr == ',' || chr == ';'))
	line2 = line2 + '.';
      else
	line2 = line2 + chr;
    }

    auto has_qmark = line2.find('?');
    if (has_qmark != string::npos) {
      qmarkcount++;
      continue;
    } else
      linecount++;

    string line;
    for (int i = 0; i < line2.size(); ++i)
      line = line
	+
	(line2[i] == '"' || line2[i] == ',' || line2[i] == ';' ? ' ' : line2[i]);
    line = line + ' ';

    vector<string> chunk;
    while (line.size() > 1) {
      auto pos = line.find_first_not_of(' ');
      line.erase(0, pos);
      pos = line.find_first_of(' ');
      chunk.push_back(line.substr(0, pos));
      line.erase(0,pos);
    }

    cout << chunk[ident];
    int mypos;
    for (int tgt = 1; tgt < target.size(); ++tgt) {
      int ocl = target[tgt];
      if (type[ocl] == BOOL) {
	mypos = position(chunk[ocl], args[tgt]);
	if (mypos == SENTINEL)
	  error(chunk[ocl]
		+
		" not in bool specification on coordinate " + to_string(ocl));
	else
	  cout << ' ' << mypos;
      } else if (type[ocl] == ENUM) {
	mypos = position(chunk[ocl], args[tgt]);
	if (mypos == SENTINEL)
	  error(chunk[ocl]
		+
		" not in enum specification on coordinate " + to_string(ocl));
	else
	  for (int j = 0; j < args[tgt].size(); ++j)
	    cout << (args[tgt].size() - 1 - j == mypos ? " 1" : " 0");
      } else if (type[ocl] == UP) {
	mypos = position(chunk[ocl], args[tgt]);
	if (mypos == SENTINEL)
	  error(chunk[ocl]
		+
		" not in up specification on coordinate " + to_string(ocl));
	else {
	  for (int j = mypos+1; j < args[tgt].size(); ++j)
	    cout << " 0";
	  for (int j = 0; j <= mypos; ++j)
	    cout << " 1";
	}
      } else if (type[ocl] == DOWN) {
	mypos = position(chunk[ocl], args[tgt]);
	if (mypos == SENTINEL)
	  error(chunk[ocl]
		+
		" not in down specification on coordinate " + to_string(ocl));
	else {
	  for (int j = 0; j < mypos; ++j)
	    cout << " 0";
	  for (int j = mypos; j < args[tgt].size(); ++j)
	    cout << " 1";
	}
      } else if (type[ocl] == INT) {
	int imin = stoi(args[tgt][0]);
	int imax = stoi(args[tgt][1]);

	// cerr << "*** imin = " << imin << ", imax = " << imax << endl;

	if (is_int(chunk[ocl])) {
	  int value = stoi(chunk[ocl]);
	  if (value < imin || value > imax)
	    error(chunk[ocl]
		  +
		  " out of bounds " + args[tgt][0] + ".." + args[tgt][1]
		  + " on coordinate " + to_string(ocl));
	  else
	    for (int j = imax; j >= imin; --j)
	      cout << (j == value ? " 1" : " 0");
	} else
	  error(chunk[ocl]
		+
		" not an integer on coordinate " + to_string(ocl));
      } else if (type[ocl] >= DJ && type[ocl] <= WARP) {
	float min = stof(args[tgt][1]);
	float max = stof(args[tgt][2]);
	int icard;
	float ilngt;
	float over = 0.0;
	if (type[ocl] <= OVER) {
	  icard = stoi(args[tgt][0]);
	  ilngt = (max - min) / icard;
	} else if (type[ocl] >= SPAN) {
	  ilngt = stof(args[tgt][0]);
	  float ratio = (max - min) / ilngt;
	  icard = ratio;
	  icard += ratio - icard > 0 ? 1 : 0;
	}
	if (type[ocl] == OVER || type[ocl] == WARP)
	  over = stof(args[tgt][3]);

	if (! is_int(chunk[ocl]) && ! is_float(chunk[ocl]))
	  error(chunk[ocl]
		+
		" is not a number on coordinate " + to_string(ocl));
	else if (stof(chunk[ocl]) < min - over / 2
		 ||
		 stof(chunk[ocl]) >= max + over / 2)
	  error(chunk[ocl] +
		" out of bounds " +
		to_string(min - over/2) +
		".." +
		to_string(max + over/2) +
		" on coordinate " + to_string(ocl));
	else
	  for (int j = 1; j <= icard; ++j)
	    cout << (stof(chunk[ocl]) >= min + ilngt * (j-1) - over/2
		     &&
		     stof(chunk[ocl]) <  min + ilngt * j + over/2
		     ? " 1" : " 0");
      } else {
	cerr << "+++ you should not be here +++" << endl;
	exit(1);
      }
    }
    cout << endl;
    if (errorflag)
      return;
  }
  if (qmarkcount > 0)
    cerr << "+++ " << qmarkcount
	 << " lines skipped due to missing values represented by '?'"
	 << endl;
  cerr << "+++ produced " << linecount
       << (linecount == 1 ? " line" : " lines")
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
    cerr << "+++ runtime errors in data file " << input << endl;
  } else {
    if (output != STDOUT)
      cerr << "+++ output file " << output << " generated" << endl;
    cerr << "+++ transformation successful" << endl;
  }
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
