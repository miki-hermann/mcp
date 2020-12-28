MULTI-CHARACTERIZATION PROBLEM (MCP) v1.03
==========================================

Table of contents
=================

0. Motivation
1. Invocation
   1.1 Sequential version
   1.2 Parallel version with POSIX threads
   1.3 Parallel version with MPI
   1.4 Parallel hybrid version
2. Input file
   2.1 Header section
   2.2 Data section
   2.3 Sample input file
3. Command-line options
   3.1 Sequential version
   3.2 Parallel version with POSIX threads
   3.3 Parallel version with MPI
   3.4 Parallel hybrid version


0. Motivation
=============

Given a set of bit sequences representing positive instances, T, and a
set of bit sequences representing negative instances, F, the task is
to find a formula that is satisfied by the instances in T and
falsified by the ones in F. Each position in the sequences corresponds
to a propositional variable. Various requirements are imposed on the
formula: It may be supposed to be a Horn, dual Horn, or bijunctive
formula, and smaller/minimal formulas are preferable.


1. Invocation
=============

1.1 Sequential version
----------------------

	# mcp-seq <command-line options> << <input> >> <output>

or

	# mcp-seq --input <input> --output <output> <other command-line options>

mcp without command-line options is equivalent to
  mcp-seq --action all \
      	  --closure horn \
      	  --cluster -1 \
      	  --direction begin \
      	  --strategy large \
      	  --cooking welldone \
      	  --setcover yes \
      	  --input STDIN \
      	  --output STDOUT \
      	  --matrix undefined \
      	  --offset 0 \
      	  --print implication

1.2 Parallel version with pthreads
----------------------------------

	# mcp-pthread --input <input file> --output <output file>

MCP version with parallel threads needs to have an output file, since
each thread writes in a different temporary file to avoid
write-crossing, followed at the end of run by copying all temporary
files to the output file. All other options remain unchanged.

1.3 Parallel version with MPI
-----------------------------

You need to have MPI installed first, otherwise this version will not
run.

	# mpirun mcp-mpi --input <input file> --output <output file>

As in the case with pthreads, also this MCP version needs to have an
output file, since each process writes in a different temporary file
to avoid write-crossing, followed at the end of run by copying all
temporary files to the output file. All other options remain
unchanged.

1.4 Parallel hybrid version
---------------------------

This version combines both MPI and POSIX threads.

	# mpirun mcp-hybrid --input <input file> --output <output file>

Also this MCP version needs to have an output file, since each process
writes in a different temporary file to avoid write-crossing, followed
at the end of run by copying all temporary files to the output
file. All other options remain unchanged.

2. Input file
=============

The input file is a text file consisting of a header section and a
data section.

2.1 Header section
------------------

The first line consists of two numbers, <i> and <j>.  The rest of the
header section consists of i+j lines that will be ignored. Typically,
the header conists just of the single line
0 0

2.2 Data section
----------------

For some fixed number k, each line of the data section is a sequence
of k+1 numbers.
The first number in each line is an integer indicating the group.
The remaining k numbers of each line may be either 0 or 1.

2.3 Sample input file
---------------------

0 0
1 0 1 1 1 1 
1 1 0 1 1 1 
2 0 0 1 1 0 
3 1 1 1 0 1 

The header consists of a single line, the data section contains three
groups of bit sequences labeled 1, 2, and 3. Group 1 contains the two
sequences 01111 and 10111, group 2 contains the single sequence 00110,
and group 3 contains the single sequence 11101.


3. Command-line options
=======================

3.1 Sequentila version
----------------------

--action [ one | all | nosection ]
  default: all

  one: take each group in turn as the positive instances T and some
       other group as negative instances F. For a total of n groups,
       this will result in n*(n-1) formulas, each characterising one
       group in relation to some other group.
       After T and F have been fixed, columns are removed as long as
       the problem is still solvable; see the --direction option for
       details.

  all: take each group in turn as the positive instances T and all
       other groups together as negative instances F. For a total of n
       groups, this will result in n formulas, each characterising one
       group in relation to the union of all other groups.
       After T and F have been fixed, columns are removed as long as
       the problem is still solvable; see the --direction option for
       details.

  nosection: like "all", but no columns are removed. The --direction
       option is ignored.



--closure [ horn | dualHorn | bijunctive ]
  default: horn

  Compute Horn, dual Horn, or bijunctive formulas that are satisfied
  by the positive instances, T, and falsified by the negative
  instances, F.
  


--cluster <integer>
  default: -1

  Cluster the matrix columns with epsilon radius of <integer>. If 0 is
  selected, it is equivalent to identifying equal columns. If a
  negative integer is selected, no clustering is performed. A variant
  of DBSCAN without noise and treshold is implemented.



--direction [ begin | end | lowcard | highcard | random | optimum ]
  default: begin

  begin: prefer columns to the left (at the begin) of the matrix.
       Columns are removed from the right, skipping columns whose
       removal would render the problem unsolvable.

  end: prefer columns to the right (at the end) of the matrix.
       Columns are removed from the left, skipping columns whose
       removal would render the problem unsolvable.

  lowcard: prefer columns with a lower Hamming weight (more
       zeros), by starting to remove columns with high Hamming
       weight (many ones).

  highcard: prefer columns with a higher Hamming weight (more
       ones), by starting to remove columns with small Hamming
       weight (many zeros).

  random: try to remove columns in random order.

  optimum: select the minimal number of columns that still yields
       a solvable problem. You probably don't want to wait for
       this to finish, and rather prefer to do something else
       with your life.



--strategy [ exact | large ]
  default: large

  exact: the formulas generated by the program characterize the smallest
         closure containing T
         "--strategy exact" implies "--setcover no" (even if "yes"
         has been specified) 

  large: the formulas generated by the program characterize the largest
         closure containing T that does not yet intersect with F



--cooking [ raw | bleu | medium | welldone ]
  default: welldone

  raw: no redundancy elimination

  bleu: performs unit resolution, by removing all literals
       from clauses that are complementary to some unit clause.

  medium: performs unit resolution (see above) and subsumption.  A
       clause subsumes another one if it is a subset for the latter.

  welldone: performs unit resolution and subsumption (see above).
       Moreover, clauses that are implied by several other ones are
       removed.



--setcover [ yes | no ]
  default: yes for "--strategy large", no otherwise

  yes: a formula is computed from the sets of positive T and negative
       F examples, which are presented in form of sets of tuples, but
       not all are actually needed when the large strategy is applied;
       the formula as a set of clauses is minimized with respect to
       the falsified tuples from the set of negative examplesby means
       of a set cover algorithm
  no:  no set cover is executed



--matrix [ undefined | hide | peek | section | show ]
  default: undefined

  undefined: printing of the input matrices is undefined; their status
  	     will be deduced later from the arity and size of the
  	     input. The output matrices will NOT be printed.
  hide:      the input and output matrices will NOT be printed
  peek:	     the input matrices will be printed, the output matrices
  	     will NOT be printed
  section:   the input matrices will NOT be printed, the output
  	     matrices will be printed
  show:      both the input and output matrices will be printed



--input [ STDIN | <file name> ]
  default: STDIN

  Input with the matrices; if STDIN is chosen, the input must be
  directed from a file by a pipe



--output [ STDOUT | <file name> ]
  default: STDOUT

  Place where the whole output will be written.
  


--print [ clause | implication | mix | DIMACS ]
  default: clause for "--closure bijunctive", mix otherwise

  clause: print each clause as a disjunction of positive and negative
          literals
          Example: (-x0 + x1) * (x0 + -x1)
          This formula consists of the two clauses "not x0 or x1" and
          "x0 or not x1".

  implication: print each clause as an implication
          Example: (x0 -> x1) * (x1 -> x0)
          This formula consists of the two clauses "x0 implies x1" and
          "x1 implies x0"; equivalent to the example above.

  mix:    if there is no negative literal or if there is no positive
  	  literal, then print like clause, otherwise print like
  	  implication

  DIMACS: print the formula in DIMACS format, one clause per line
  	  (implies --offset greater than 0)



--offset <integer>
  default: 1 for "--print DIMACS", 0 otherwise

  Internally, all indices begin with 0. However, when the data is
  displayed in an Excel sheet, the variables may begin in a column
  different from 0. To identify the same variables in an Excel sheet
  and the output of this software, you can define an offset. E.g., the
  offset 1 will shift the variable indices by 1 and therefore first
  variable will have the index 1.

  Negative offsets are converted to 0.

3.2 Parallel version with POSIX threads
---------------------------------------

All parameters for the sequential version are also valid for the
parallel version with POSIX threads. In addition, the parallel version
with POSIX threads has the following parameters:

--chunk <integer>
  default: 4000

Pthreads have been included into C++11, therefore we use it. Inside
the software we manipulate large matrices. This treatment can be
executed in parallel by chunks of treated matrices. Each chunk
contains a certain amount of rows.

3.3 Parallel version with MPI
-----------------------------

All parameters for the sequential version are also valid for the
parallel version with MPI. In addition, the parallel version with MPI
has the following parameters:

--fit [ yes | no ]
  default: no

  For efficiency reasons, we can require to run the parallel software
  with the number of processes equal to the number of groups. This is
  done by the command

	# mpirun -np X mcp-mpi --fit yes ...

  where X is the number of processes.

3.4 Parallel hybrid version
---------------------------

All parameters  for the sequential, POSIX thread, and MPI versions are
also valid for the hybrid version.

EOF
