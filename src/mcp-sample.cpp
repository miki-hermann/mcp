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
 *      File:    mcp-sample.cpp                                           *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
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
#include <csignal>
#include "mcp-matrix+formula.hpp"	// for split
#include "mcp-tally.hpp"

using namespace std;

#define STDIN  "STDIN"
#define STDOUT "STDOUT"

string input  = STDIN;
string output = STDOUT;
ifstream infile;
ofstream outfile;
ofstream routfile;
string tpath = "/tmp/mcp-tmp-";

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

size_t number_of_lines = 0;
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

// test if sample size is correct and print out override information
inline void try_sample_size (const string &what) {
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
	 << " overrides " << what << endl;
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
    try_sample_size("confidence interval");
  } else if (! error_bound.empty() && ! sample_card.empty()) {
    try_sample_size("error bound");
  } else if (! error_bound.empty()) {
    try {
      conf = 2.0 * string2double(error_bound);
    } catch (invalid_argument err) {
      cerr << "+++ " << error_bound
	     << " is not a valid error bound"
	     << endl;
      exit(2);
    }
  } else if (! confidence.empty()) {
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
    cerr << "+++ Neither confidence interval, nor error bound, nore cardinality specified" << endl;
    exit(2);
  }

  if ((conf < 0.001 || conf > 0.2) && population == proportional) {
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

// clear eof flags and seek to the beginning of input file
inline void reset_input () {
  cin.clear();
  infile.clear();
  cin.seekg(0);
  infile.seekg(0);
}

void first_pass () {
  string line;
  size_t lineno = 0;
  bool warning = false;
  
  while (getline(cin, line)) {
    lineno++;
    if (line.empty())
      continue;
    number_of_lines++;
    if (! concept_column.empty()) {
      vector<string> chunks = split(line, ", \t");
      if (ccol >= chunks.size()) {
	cerr << "++ concept column (" << ccol
	     << ") beyond item size (" << chunks.size()
	     << ") on input line "
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
}

void second_pass () {
  string line;
  vector<string> data_lines, sample_lines;

  reset_input();
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
  static uniform_int_distribution<int> uni_dist(1, number_of_lines);
  static default_random_engine dre(rd());

  // generate the random numbers of chosen input lines and sort them
  vector<size_t> rand_nums;
  while (rand_nums.size() < sample_size) {
    size_t rnd = uni_dist(dre);
    auto it = find(rand_nums.cbegin(), rand_nums.cend(), rnd);
    if (it == rand_nums.cend())
      rand_nums.push_back(rnd);
  }
  sort(rand_nums.begin(), rand_nums.end());

  // search the sample lines in the big file
  string line;
  size_t linenum = 0;
  size_t pointer = 0;
  reset_input();
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

void erase_tmp () {
  const string temp_prefix = tpath + "mcp-tmp-";
  const string basename = "rm -f " + temp_prefix;
  const string erase_out  = basename + "*.out";
  const string erase_txt  = basename + "*.txt";
  system(erase_out.c_str());
  system(erase_txt.c_str());
}

// terminal handler: we erase the temporary files in case of a crash
void crash (int signal) {
  erase_tmp();
  cerr << endl << "\t*** Segmentation fault ***" << endl << endl;
  exit(signal);
}

// terminal handler: we erase the temporary files in case of an interrupt
void interrupt (int signal) {
  erase_tmp();
  cerr << endl << "\t*** Interrupt ***" << endl << endl;
  exit(signal);
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  read_arg(argc, argv);
  adjust_and_open();
  // if (sample_size == 0)
  //   sample_size = sample_cardinality(prop, conf);
  signal(SIGSEGV, crash);
  signal(SIGINT, interrupt);

  if (population == absolute) {
    if (big || ! concept_column.empty())
      first_pass();
    if (big)
      big_pass();
    else
      second_pass();
  } else if (population == proportional) {
    tally(ccol);
    const time_t start_time = time(nullptr);
    const string basename = tpath + to_string(start_time);
    const string routname = basename + ".out";

    size_t real_size = 0;
    for (const auto &val : percentage) {
      // size_t section = (0.01 * val.second + 0.005 / percentage.size()) * sample_size;
      size_t section = (0.01 * val.second) * sample_size;
      cerr << "+++ section for " << val.first << " = " << section << endl;
      if (section == 0) {
	cerr << "+++ " + val.first + " section skipped" << endl;
	continue;
      }
      real_size += section;

      reset_input();
      routfile.open(routname);
      if (! routfile.is_open()) {
	cerr << "+++ Cannot open rout file " << routname << endl;
	exit(2);
      }

      string line;
      size_t lineno = 0;
      while (getline(cin, line)) {
	if (line.empty())
	  continue;
	clear_line(++lineno, line);
	const vector<string> chunks = split(line, " \t");
	if (ccol >= chunks.size()) {
	  cerr << "+++ concept column out of range on line "
	       << lineno
	       << endl;
	  exit(2);
	}
	if (chunks[ccol] == val.first)
	  routfile << line << endl;
      }
      routfile.close();

      // cerr << "+++ recursive call to mcp-sample for " << val.first << endl;
      string rec_sample
	= "mcp-sample -i " + routname
	// + (big ? " --big" : "")
	+ " --big"
	+ " -pp abs -q -# " + to_string(section)
	+ " -o " + output;
      system(rec_sample.c_str());
      remove(routname.c_str());
    }
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
