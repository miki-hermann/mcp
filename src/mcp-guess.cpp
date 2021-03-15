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
 *      File:    mcp-guess.cpp                                            *
 *                                                                        *
 *      Copyright (c) 2019 - 2021                                         *
 *                                                                        *
 * Guess a skeleton of a meta file from a (CSV) data file                 *
 *                                                                        *
 * This software has been created within the ACCA Project.                *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <algorithm>

#define STDIN    "STDIN"
#define STDOUT   "STDOUT"

using namespace std;

const float  ENUM_RATIO = 0.1;
const int    ENUM_MAX   = 200;
const string QMARK      = "?";

enum item {UNDEF = 0, INT = 1, FLOAT = 2, STRING = 3};
const string item_name[] = {"undef", "int", "float", "string"};

vector<vector<string>> trdata, data;
vector<item> type;
int row_length = 0;
int row_count  = 0;
vector<int> flength;
bool errorflag = false;
bool qflag     = false;

string input   = STDIN;
string output  = STDOUT;
ifstream infile;
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

vector<vector<string>> transpose (const vector<vector<string>> &trd) {
  vector<vector<string>> d;
  for (int column = 0; column < trd[0].size(); ++column) {
    vector<string> temp;
    for (int row = 0; row < trd.size(); ++row)
      temp.push_back(trd[row][column]);
    d.push_back(temp);
  }
  return d;
}

int main (int argc, char **argv)
{
  regex empty_pattern("^[ \t]+$", regex::egrep);
  regex comma_scolon("[,;]", regex::egrep);
  string line;

  read_arg(argc, argv);
  IO_open();
  
  while (getline(cin, line)) {
    row_count++;
    if (line.empty() || regex_match(line, empty_pattern))
      continue;

    // elimination of quotes, replacements of spaces, commas, and semicolons
    string line1;
    bool in_string = false;
    for (int i = 0; i < line.size(); ++i)
      if (line[i] == '"') {
	in_string = ! in_string;
	line1 += " ";
      } else if (in_string && (line[i] == ' ' || line[i] == '\t'))
	line1 += "_";
      else if (in_string && line[i] == '?')
	line1 += "<>";
      else if (in_string && (line[i] == ',' || line[i] == ';'))
	line1 += ".";
      else
	line1 = line1 + line[i];

    // elimination of commas and semicolons outside quotes
    string line2 = regex_replace(line1, comma_scolon, " ");

    // chop line into string pieces
    istringstream instring(line2);
    vector<string> row;
    string strg;
    while (instring >> strg)
      row.push_back(strg);
    trdata.push_back(row);

    if (row_length == 0)
      row_length = row.size();
    else if (row_length != row.size()) {
      errorflag = true;
      cout << "+++ item count discrepancy on line " << row_count << endl;
      cout << "+++ row length = " << row_length
	   << ", row size = " << row.size() << endl;
    }
  }

  if (errorflag) {
    cout << "+++ errors in data file" << endl;
    exit(1);
  }
  
  data = transpose(trdata);
  regex int_pattern("^-?[0-9]+$", regex::egrep);
  regex float_pattern("^-?[0-9]*\\.([0-9]+)$", regex::egrep);
  regex efloat_pattern("^-?[0-9]+\\.([0-9]+)e(\\-|\\+)[0-9]+$", regex::egrep);
  smatch result;
  for (int col = 0; col < data.size(); ++col) {
    type.push_back(UNDEF);
    flength.push_back(0);
    for (int row = 0; row < data[col].size(); ++row) {
      switch (type[col]) {
      case INT:
	if (regex_match(data[col][row], result, float_pattern)
	    ||
	    regex_match(data[col][row], result, efloat_pattern)) {
	  flength[col] = max(flength[col], int(result.str(1).length()));
	  type[col] = FLOAT;
	} else if (data[col][row] != QMARK
		   &&
		   !regex_match(data[col][row], int_pattern))
	  type[col] = STRING;
	else if (data[col][row] == QMARK)
	  qflag = true;
	break;
      case  FLOAT:
	if (data[col][row] != QMARK
	    &&
	    !regex_match(data[col][row], result, int_pattern)
	    &&
	    !regex_match(data[col][row], result, float_pattern)
	    &&
	    !regex_match(data[col][row], result, efloat_pattern)
	    )
	  type[col] = STRING;
	else if (data[col][row] == QMARK)
	  qflag = true;
	break;
      case  UNDEF:
	if (regex_match(data[col][row], result, int_pattern))
	  type[col] = INT;
	else if (regex_match(data[col][row], result, float_pattern)
		 ||
		 regex_match(data[col][row], result, efloat_pattern)) {
	  flength[col] = max(flength[col], int(result.str(1).length()));
	  type[col] = FLOAT;
	} else if(data[col][row] != QMARK)
	  type[col] = STRING;
	else if (data[col][row] == QMARK)
	  qflag = true;
	break;
      }
    }
  }

  string fmt = "= %" + to_string(to_string(data.size()).length()) + "d: ";
  for (int col = 0; col < data.size(); ++col) {
    vector<string> row = data[col];
    vector<int> irow;
    vector<float> frow;

    int wide = to_string(data.size()).length();
    cout << "id" << left << setw(wide) << col << right;
    cout << " = " << setw(wide) << col << ": ";
    
    switch (type[col]) {
    case INT:
      for (auto r : row)
	if (r != QMARK)
	  irow.push_back(stoi(r));
      sort(irow.begin(), irow.end());
      row.clear();
      for (auto ir : irow)
	row.push_back(to_string(ir));
      irow.clear();
      break;
    case FLOAT:
      for (auto r : row)
	if (r != QMARK)
	  frow.push_back(stof(r));
      sort(frow.begin(), frow.end());
      row.clear();
      for (auto fr : frow)
	row.push_back(to_string(fr));
      frow.clear();
      break;
    case STRING:
      sort(row.begin(), row.end());
      break;
    }
    auto ref = unique(row.begin(), row.end());
    row.resize(distance(row.begin(), ref));

    int rsz = row.size();
    int rsz1 = rsz - 1;

    bool is_enum = rsz <= ENUM_MAX
      ||
      rsz <= (row_count * ENUM_RATIO);
    if (rsz == 2)
      cout << "bool ";
    else if (is_enum && type[col] != FLOAT)
      cout << "enum " << item_name[type[col]] << " ";
    else
      cout << item_name[type[col]] << " ";
    if (is_enum && type[col] != FLOAT) {
      cout << "[";
      for (int i = 0; i < rsz1; ++i)
	cout << row[i] << " ";
      cout << row[rsz-1] << "]";
    } else if (type[col] == FLOAT) {
      float r0f   = stof(row[0]);
      float rsz1f = stof(row[rsz1]);
      cout << showpoint;
      cout << setprecision(flength[col] + to_string((int)r0f).length());
      cout << r0f << " ";
      cout << setprecision(flength[col] + to_string((int)rsz1f).length());
      cout << rsz1f
	   << noshowpoint;
    } else
      cout << row[0] << " " << row[rsz1];
    cout << ";\t# card " << rsz;
    bool is_cons = type[col] == INT;
    if (is_cons)
      for (int i = 1; i < rsz; ++i)
	if (stoi(row[i]) != stoi(row[i-1])+1) {
	  is_cons = false;
	  break;
	}
    if (is_cons)
      cout << " consecutive";
    cout << endl;
  }
  if (qflag)
    cout << "# missing values represented by '?' encountered" << endl;

  IO_close();
}
