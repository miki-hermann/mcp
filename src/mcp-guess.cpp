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
 *      File:    mcp-guess.cpp                                            *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Guess a skeleton of a meta file from a (CSV) data file                 *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <algorithm>
#include "mcp-matrix+formula.hpp"

#define STDIN    "STDIN"
#define STDOUT   "STDOUT"

using namespace std;

// double  ENUM_RATIO = 0.01;
int    ENUM_MAX   = 20;		// 50 80 200 20
const string QMARK      = "?";

enum item {UNDEF = 0, INT = 1, FLOAT = 2, STRING = 3};
const string item_name[] = {"undef", "int", "float", "string"};

vector<vector<string>> trdata, mydata;
vector<item> type;
size_t row_length = 0;
size_t row_count  = 0;
vector<size_t> flength;
bool errorflag = false;
bool qflag     = false;
string id      = "id";
vector<string> id_names;

string input   = STDIN;
string name    = "";
string output  = STDOUT;
ifstream infile;
ifstream namefile;
ofstream outfile;
streambuf *backup;

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
    } else if (arg == "-n"
	       || arg == "--names") {
      name = argv[++argument];
    } else if (arg == "-e"
	       || arg == "--enum") {
      ENUM_MAX = stoi(argv[++argument]);
    // } else if (arg == "-r"
    // 	       || arg == "--ratio") {
    //   ENUM_RATIO = stof(argv[++argument]);
    } else {
      cerr << "+++ argument error: " << arg << endl;
      exit(1);
    }
    ++argument;
  }
  if (input != STDIN && output == STDOUT) {
    string::size_type pos = input.rfind('.');
    output = (pos == string::npos ? input : input.substr(0, pos)) + ".txt";
  }
}

void adjust () {
  // if (ENUM_RATIO < 0.0 || ENUM_RATIO > 1.0) {
  //   cerr << "*** ENUM_RATIO reset to 0.01" << endl;
  //   ENUM_RATIO = 0.01;
  // }

  if (ENUM_MAX < 3 || ENUM_MAX > 500) {
    cerr << "*** ENUM_MAX reset to 20" << endl;
    ENUM_MAX = 20;
  }
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

  if (!name.empty()) {
    namefile.open(name);
    if (!namefile.is_open()) {
      cerr << "+++ Cannot open name file " << name << endl;
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
  if (!name.empty())
    namefile.close();
}

vector<vector<string>> transpose (const vector<vector<string>> &trd) {
  vector<vector<string>> d;
  for (size_t column = 0; column < trd[0].size(); ++column) {
    vector<string> temp;
    for (size_t row = 0; row < trd.size(); ++row)
      temp.push_back(trd[row][column]);
    d.push_back(temp);
  }
  return d;
}

// drop trailing 0's from floats
string notrail0 (string fnum) {
  while (fnum.back() == '0')
    fnum = fnum.substr(0, fnum.length()-1);
  if (fnum.back() == '.')
    fnum = fnum.substr(0, fnum.length()-1);
  return fnum;
}

int main (int argc, char **argv)
{
  regex empty_pattern("^[ \t]+$", regex::egrep);
  regex comma_scolon("[,;]", regex::egrep);
  string line;

  read_arg(argc, argv);
  adjust();
  IO_open();

  while (getline(cin, line)) {
    row_count++;

    // elimination of quotes, replacements of spaces, commas, and semicolons
    clear_line(row_count, line);
    if (line.empty())
      continue;
    // elimination of commas and semicolons outside quotes
    uncomma_line(line);
    // chop line into string pieces
    vector<string> row = split(line, " \t");
    trdata.push_back(row);

    if (row_length == 0)
      row_length = row.size();
    else if (row_length != row.size()) {
      errorflag = true;
      cerr << "+++ item count discrepancy on line " << row_count << endl;
      cerr << "+++ row length = " << row_length
	   << ", row size = " << row.size() << endl;
    }
  }

  if (row_count == 0) {
    cerr << "+++ input file " << input << " is empty" << endl
	 << "... is it correct?" << endl;
    exit(2);
  }

  if (errorflag) {
    cerr << "+++ errors in data file" << endl;
    IO_close();
    if (output != STDOUT) {
      cerr << "*** no output file generated" << endl;
      remove(output.c_str());
    }
    exit(1);
  }

  mydata = transpose(trdata);
  regex int_pattern("^-?[0-9]+$", regex::egrep);
  regex float_pattern("^-?[0-9]*\\.([0-9]+)$", regex::egrep);
  regex efloat_pattern("^-?[0-9]+\\.([0-9]+)[e|E](\\-|\\+)[0-9]+$", regex::egrep);
  smatch result;
  for (size_t col = 0; col < mydata.size(); ++col) {
    type.push_back(UNDEF);
    flength.push_back(0);
    for (size_t row = 0; row < mydata[col].size(); ++row) {
      switch (type[col]) {
      case INT:
	if (regex_match(mydata[col][row], result, float_pattern)
	    ||
	    regex_match(mydata[col][row], result, efloat_pattern)) {
	  flength[col] = max(flength[col], result.str(1).length());
	  type[col] = FLOAT;
	} else if (mydata[col][row] != QMARK
		   &&
		   !regex_match(mydata[col][row], int_pattern))
	  type[col] = STRING;
	else if (mydata[col][row] == QMARK)
	  qflag = true;
	break;
      case  FLOAT:
	if (mydata[col][row] != QMARK
	    &&
	    !regex_match(mydata[col][row], result, int_pattern)
	    &&
	    !regex_match(mydata[col][row], result, float_pattern)
	    &&
	    !regex_match(mydata[col][row], result, efloat_pattern)
	    )
	  type[col] = STRING;
	else if (mydata[col][row] == QMARK)
	  qflag = true;
	break;
      case  UNDEF:
	if (regex_match(mydata[col][row], result, int_pattern))
	  type[col] = INT;
	else if (regex_match(mydata[col][row], result, float_pattern)
		 ||
		 regex_match(mydata[col][row], result, efloat_pattern)) {
	  flength[col] = max(flength[col], result.str(1).length());
	  type[col] = FLOAT;
	} else if(mydata[col][row] != QMARK)
	  type[col] = STRING;
	else if (mydata[col][row] == QMARK)
	  qflag = true;
	break;
      }
    }
  }

  size_t id_length = 0;
  size_t name_count = 0;
  if (!name.empty())
    while (getline(namefile, line)) {
      clear_line(++name_count, line);
      if (line.empty())
	continue;
      id_names.push_back(line);
      id_length = max(id_length, line.length());
    }
  else
    id_length = to_string(mydata.size()).length();
  size_t wide = to_string(mydata.size()).length();

  if (!name.empty() && id_names.size() != mydata.size()) {
    cerr << "*** names versus data discrepancy: data.size";
    cerr << "(" << mydata.size() << ")";
    cerr << " != names.size";
    cerr << "(" << id_names.size() << ")";
    cerr << endl;
    IO_close();
    if (output != STDOUT) {
      cerr << "*** no output file generated" << endl;
      remove(output.c_str());
    }
    exit(1);
  }

  const string fmt = "= %" + to_string(to_string(mydata.size()).length()) + "d: ";
  cout << "# This file has been produced by mcp-guess" << endl
       << "# It is NOT a valid transformation meta-file for mcp-trans" << endl
       << "# It must be edited following the meta-file syntax before use" << endl
       << endl;
  for (size_t col = 0; col < mydata.size(); ++col) {
    vector<string> row = mydata[col];

    if (!name.empty())
      cout << left << setw(id_length) << id_names[col] << right;
    else
      cout << id << left << setw(wide) << col << right;
    cout << " = " << setw(wide) << col << ": ";
    
    switch (type[col]) {
    case INT: {
      vector<int> irow;
      for (const string &r : row)
	if (r != QMARK)
	  irow.push_back(stoi(r));
      sort(irow.begin(), irow.end());
      row.clear();
      for (const int &ir : irow)
	row.push_back(to_string(ir));
      // irow.clear();
    } break;
    case FLOAT: {
      vector<double> frow;
      for (const string &r : row)
	if (r != QMARK)
	  frow.push_back(stof(r));
      sort(frow.begin(), frow.end());
      row.clear();
      for (const double &fr : frow)
	row.push_back(to_string(fr));
      // frow.clear();
    } break;
    case STRING: {
      auto it = row.begin();
      while (it != row.end())
	if (*it == QMARK) {
	  it = row.erase(it);
	  qflag = true;
	} else
	  ++it;
      sort(row.begin(), row.end());
    } break;
    }
    auto ref = unique(row.begin(), row.end());
    row.resize(distance(row.begin(), ref));

    const size_t rsz = row.size();
    const long rsz1 = rsz - 1;

    bool is_enum = rsz <= ENUM_MAX
      || type[col] == STRING
      // ||
      // rsz <= (row_count * ENUM_RATIO)
      ;
    if (rsz == 2)
      cout << "bool ";
    // else if (is_enum && type[col] != FLOAT)
    else if (is_enum)
      cout << "enum " << item_name[type[col]] << " ";
    else
      cout << item_name[type[col]] << " ";
    // if (is_enum && type[col] != FLOAT) {
    if (is_enum) {
      cout << "[";
      for (size_t i = 0; i < rsz1; ++i)
	cout << (type[col] != FLOAT ? row[i] : notrail0(row[i])) << " ";
      cout << (type[col] != FLOAT ? row[rsz-1] : notrail0(row[rsz-1])) <<  "]";
    } else if (!is_enum && type[col] == FLOAT) {
      const double r0f   = stod(row[0]);
      const double rsz1f = stod(row[rsz1]);
      cout << showpoint;
      cout << setprecision(flength[col] + to_string(r0f).length());
      cout << r0f << " ";
      cout << setprecision(flength[col] + to_string(rsz1f).length());
      cout << rsz1f
	   << noshowpoint;
    } else
      cout << row[0] << " " << row[rsz1];
    cout << ";\t# card " << rsz;
    bool is_cons = type[col] == INT;
    if (is_cons)
      for (size_t i = 1; i < rsz; ++i)
	if (stoi(row[i]) != stoi(row[i-1])+1) {
	  is_cons = false;
	  break;
	}
    if (is_cons)
      cout << " consecutive";
    cout << endl;
  }
  if (qflag) {
    cout << "# missing values represented by '?' encountered" << endl;
    cerr << "*** missing values represented by '?' encountered" << endl;
  }

  IO_close();
}
