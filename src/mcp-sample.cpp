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
 *      File:    mcp-sample.cpp                                           *
 *                                                                        *
 *      Copyright (c) 2019 - 2024                                         *
 *                                                                        *
 * Output a sample from an input file.                                    *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include "mcp-matrix+formula.hpp"	// for split
#include "mcp-tally.hpp"

using namespace std;

#define STDIN  "STDIN"
#define STDOUT "STDOUT"

string input  = STDIN;
string output = STDOUT;
ifstream infile;
ofstream outfile;

string confidence = "2.5%";
double conf;
string error_bound;
string sample_card;
string proportion;
double prop;
string concept_column;
size_t ccol;		// concept column value
bool quiet = false;
bool big = false;

size_t n_of_lines = 0;
size_t sample_size = 0;

enum Population {proportional = 0, absolute = 1};
Population population = proportional;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// reads the input parameters
void read_arg (int argc, char *argv[]) {
  int argument = 1;
  while (argument < argc) {
    string arg = argv[argument];
    if (arg == "--input"
	|| arg == "-i") {
      input = argv[++argument];
    } else if (arg == "--output"
	       || arg == "-o") {
      output = argv[++argument];
    } else if (arg == "--population"
	       || arg == "--pop"
	       || arg == "-pp"
	       || arg == "-p") {
      arg = argv[++argument];
      if (arg == "proportional"
	  || arg == "prop"
	  || arg == "pp"
	  || arg == "p"
	  || arg == "P")
	population = proportional;
      else if (arg == "absolute"
	       || arg == "abs"
	       || arg == "a"
	       || arg == "A")
	population = absolute;
      else
	cerr << "+++ unknown population option " << arg << endl;
    } else if (arg == "--quiet"
	       || arg == "-q") {
      quiet = true;
    } else if (arg == "--confidence"
	       || arg == "--interval"
	       || arg == "-W"
	       || arg == "-w") {
      confidence = argv[++argument];
    } else if (arg == "--bound"
	       || arg == "--error"
	       || arg == "-B"
	       || arg == "-b") {
      error_bound = argv[++argument];
    } else if (arg == "--cardinality"
	       || arg == "--card"
	       || arg == "-#") {
      sample_card = argv[++argument];
    } else if (arg == "--proportion"
	       || arg == "--prop") {
      proportion = argv[++argument];
    } else if (arg == "--concept"
	       || arg == "-c") {
      concept_column = argv[++argument];
    } else if (arg == "--big"
	       || arg == "--BIG") {
      big = true;
    } else
      cerr << "+++ unknown option " << arg << endl;
    ++argument;
  }
}

inline double string2double (const string &s) {
  return (s.back() == '%' ? stod(s.substr(0, s.length()-1)) * 0.01 : stod(s));
}

size_t sample_cardinality (const double &prop, const double &conf) {
  double val = (16.0 * prop * (1.0 - prop)) / (conf * conf);
  size_t sz = val;
  if (val > sz)
    sz++;
  return sz;
}

// adjusts input parameters and open files
void adjust_and_open () {
  // input and output
  if (input != STDIN) {
    infile.open(input);
    if (infile.is_open())
      cin.rdbuf(infile.rdbuf());
    else {
      cerr << "+++ Cannot open input file " << input << endl;
      exit(2);
    }
  }

  if (output == STDOUT && input != STDIN) {
    auto pos = input.rfind('.');
    output = pos == string::npos
      ? input + "-sample.csv"
      : input.substr(0, pos) + "-sample" + input.substr(pos);
  }

  if (output != STDOUT) {
    if (quiet)
      outfile.open(output, ios::out | ios::app);
    else
      outfile.open(output, ios::out | ios::trunc);
    if (outfile.is_open())
      cout.rdbuf(outfile.rdbuf());
    else {
      cerr << "+++ Cannot open output file " << output << endl;
      exit(2);
    }
  }

  // coliding concept column
  if (!proportion.empty() && (!concept_column.empty())) {
    cerr << "+++ Both proportion and concept specified" << endl;
    exit(2);
  }

  if (concept_column.empty() && population == proportional) {
    cerr << "+++ Proportional population without concept column" << endl;
    exit(2);
  }

  if (! confidence.empty() && ! error_bound.empty()) {
    cerr << "+++ Both confidence interval and error bound specified" << endl;
    exit(2);
  } else if (! confidence.empty() && ! sample_card.empty()) {
    try {
      sample_size = stoul(sample_card);
    } catch (invalid_argument err) {
      cerr << "+++ " << sample_card
	     << " is not a valid sample size"
	     << endl;
	exit(2);
    }
    if (!quiet)
      cerr << "+++ Sample cardinality " << sample_size
	   << " overrides confidence interval" << endl;
  } else if (! error_bound.empty() && ! sample_card.empty()) {
    try {    
      sample_size = stoul(sample_card);
    } catch (invalid_argument err) {
      cerr << "+++ " << sample_card
	     << " is not a valid sample size"
	     << endl;
      exit(2);
    }
    if (!quiet)
      cerr << "+++ Sample cardinality " << sample_size
	   << " overrides error bound" << endl;
  } else if (!error_bound.empty()) {
    try {
      conf = 2.0 * string2double(error_bound);
    } catch (invalid_argument err) {
      cerr << "+++ " << error_bound
	     << " is not a valid error bound"
	     << endl;
      exit(2);
    }
  } else if (!confidence.empty()) {
    try {
      conf = string2double(confidence);
    } catch (invalid_argument err) {
      cerr << "+++ " << confidence
	     << " is not a valid confidence interval"
	     << endl;
      exit(2);
    }
  } else if (! sample_card.empty()) {
    try {
      sample_size = stoul(sample_card);
    } catch (invalid_argument err) {
      cerr << "+++ " << sample_card
	     << " is not a valid sample size"
	     << endl;
      exit(2);
    }
  } else {
    cerr << "+++ No confidence interval and no error bound and no cardinality specified" << endl;
    exit(2);
  }

  if ((conf < 0.001 || conf > 0.2) && sample_size == 0) {
    cerr << "+++ Confidence interval reset to 2.5%" << endl;
    conf = 0.025;
  }

  if (concept_column.empty())
    try {
      if (proportion.empty())
	prop = 0.5;
      else if (proportion.back() == '%')
	prop = stod(proportion.substr(0, proportion.length()-1)) / 100.0;
      else
	prop = stod(proportion);
    } catch (invalid_argument err) {
      cerr << "+++ '" << proportion
	   << "' is not a valid proportion"
	   << endl;
      exit(2);
    }
  else {
    prop = 0.5;
    try {
      ccol = stoul(concept_column);
    } catch (invalid_argument err) {
      cerr << "+++ '" << concept_column
	     << "' after --concept is not a valid column number"
	     << endl;
	exit(2);
    }
  }
  if (sample_size == 0)
    sample_size = sample_cardinality(prop, conf);
}

void cleanup () {
  if (output != STDOUT)
    outfile.close();
  if (input != STDIN)
    infile.close();
}

void fst_pass () {
  string line;
  size_t lineno = 0;
  bool warning = false;
  
  while (getline(cin, line)) {
    lineno++;
    if (line.empty())
      continue;
    n_of_lines++;
    if (!concept_column.empty()) {
      vector<string> chunks = split(line, ", \t");
      if (ccol >= chunks.size()) {
	cerr << "++ concept column beyond item size on input line "
	     << lineno
	     << endl;
	warning = true;
      }
    }
  }
  if (warning) {
    cerr << "+++ STOP because of input errors" << endl;
    exit(2);
  }

  // clear eof flags and seek to the beginning of input file
  cin.clear();
  infile.clear();
  cin.seekg(0);
  infile.seekg(0);
}

void snd_pass () {
  string line;
  vector<string> data_lines, sample_lines;

  while (getline(cin, line))
    data_lines.push_back(line);
  sample(data_lines.cbegin(), data_lines.cend(), back_inserter(sample_lines),
	 sample_size,
	 mt19937 {random_device{}()});

  for (const string &sline : sample_lines)
    cout << sline << endl;
}

void big_pass () {
  // install the random device
  random_device rd;
  static uniform_int_distribution<int> uni_dist(1, n_of_lines);
  static default_random_engine dre(rd());

  // generate the random numbers of chosen input lines and sort them
  vector<size_t> rand_nums;
  while (rand_nums.size() < sample_size) {
    size_t rnd = uni_dist(dre);
    auto it = find(rand_nums.cbegin(), rand_nums.cend(), rnd);
    if (it == cend(rand_nums))
      rand_nums.push_back(rnd);
  }
  sort(rand_nums.begin(), rand_nums.end());

  // search the sample lines in the big file
  string line;
  size_t linenum = 0;
  size_t pointer = 0;
  while (getline(cin, line)) {
    if (line.empty())
      continue;
    linenum++;
    if (linenum == rand_nums[pointer]) {
      cout << line << endl;
      pointer++;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  read_arg(argc, argv);
  adjust_and_open();
  // if (sample_size == 0)
  //   sample_size = sample_cardinality(prop, conf);
  if (population == absolute) {
    if (big || !concept_column.empty())
      fst_pass();
    if (big)
      big_pass();
    else
      snd_pass();
  } else if (population == proportional) {
    tally(ccol);
    const time_t start_time = time(nullptr);
    const string basename = "/tmp/mcp-tmp-"+ to_string(start_time);
    const string metaname = basename + ".meta";
    const string routname = basename + ".out";

    size_t real_size = 0;
    for (const auto &val:percentage) {
      // size_t section = (0.01 * val.second + 0.005 / percentage.size()) * sample_size;
      size_t section = (0.01 * val.second) * sample_size;
      cerr << "+++ section for " << val.first << " = " << section << endl;
      real_size += section;
      string metacmd = concept_column + " : string = " + val.first + ";";

      ofstream metafile;
      metafile.open(metaname);
      metafile << metacmd << endl;
      metafile.close();

      cerr << "+++ call to mcp-clean for " << val.first << endl;
      string clean_cmd
	= "mcp-clean -i " + input
	+ " -m " + metaname
	+ " -o " + routname;
      system(clean_cmd.c_str());

      cerr << "+++ recursive call to mcp-sample for " << val.first << endl;
      string rec_sample
	= "mcp-sample -i " + routname
	+ (big ? " --big" : "")
	+ " -pp abs -q -# " + to_string(section)
	+ " -o " + output;
      system(rec_sample.c_str());
    }
    remove(metaname.c_str());
    remove(routname.c_str());
    sample_size = real_size;
  }
  cleanup();
  if (!quiet) {
    cerr << "+++ sample size = " << sample_size << endl
	 << "+++ sample written on " << output << endl;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
