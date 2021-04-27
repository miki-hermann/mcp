# MULTI-CHARACTERIZATION PROBLEM (MCP), v1.04
				   
## Table of contents

* [Brief Description](#brief-description)
* [Compilation and Installation](#compilation-and-installation)
* [UCI Examples](#uci-examples)



## Brief Description

This  is  the   MCP  system  designed  to  transform   Big  Data  into
propositional formulas. It consists of  several modules. The main part
is the MCP core. In addition to  the core there are prequel and sequel
supporting modules.

MCP core has four variants:

    mcp-seq     (sequential),
    mcp-mpi     (parallel using the Message Passing Interface),
    mcp-pthread (parallel using POSIX threads), and
    mcp-hybrid  (parallel using a combination of MPI and POSIX threads).

The prequel modules are

    mcp-guess (for guessing the structure of the input data),
    mcp-trans (for binarization of input data), and
    mcp-split (for splitting binarized data into a learning and checking part)

The sequel modules are

    mcp-check (for checking the accuracy of the produced formula)

Additionally, the distribution contains the following files:

    README.md (this document),
    LICENCE   (GNU General Public Licence v.3, under which this software is distributed),
    Makefile  (for the compilation and installation of the MCP system)

## Compilation and Installation
* [Outline of the distribution](#outline-of-the-distribution)
* [Compilation](#compilation)
* [Invocation](#invocation)

### Outline of the distribution

The subdirectories of this distribution are:
 - *src*   containing the sources of the MCP system,
 - *man*   containing the manual pages,
 - *paper* containing the PDF document `mcp-sat.pdf` with a detailed description of the MCP system
 - *bin*   is necessary for compilation, and
 - *uci*   containing several examples from the UCI Machine Learning
         Repository, treated by the MCP system.

### Compilation

A C++ compiler satisfying at least  the C++17 revision is necessary to
successfully  compile the  MCP system,  mainly for  the use  of `auto`
types and POSIX threads. Only  the standard library is used, therefore
there is no need to install any additional C++ libraries.  The g++ GNU
Project compiler  is used  in the  `Makefile`. If  you have  a different
compiler, please modify the `Makefile` according to your installation.

You need to  have installed the Message Passing Interface  (MPI) to be
able to compile the modules `mcp-mpi` and `mcp-hybrid`.  However, this
is not  an absolute necessity.  The MCP system functions  even without
this two variants of the MCP core.

### Invocation

To compile and install the MCP system, write the command
```Makefile
   make
```
in the root directory of this unpacked tarball. The system then

   - compiles the sources,
   - places the binaries in the root directory of the installation,
   - installs the manual pages in the directories
     `/usr/local/share/man/man1` and `/usr/local/share/man/man5`,
   - installs the binaries in the directory `/usr/local/bin`.

You need to have superuser privileges to execute the last two parts.

If you do  not have MPI installed, compile and  install the MCP system
(without the modules `mcp-mpi` and `mcp-hybrid`) by the command
```Makefile
    make no-mpi
```

## UCI Examples

The  uci subdirectory  contains the  following examples  from the  [UCI
Machine Learning Repository](http://archive.ics.uci.edu/ml/), prepared
to be treated by the MCP system. These examples are

 - **abalone**                 (identifying abalone with 27 rings),
 - **balance-scale**           (identifying psychological experiments balancing a scale),
 - **balloons**                (a toy example, where specific formulas are required to be produced),
 - **breast-cancer-wisconsin** (identifying benign and malignant breast cancer cases in Wisconsin),
 - **car**                     (identifying very good cars),
 - **forest-fire**             (predicting forest fires in July, August, and September),
 - **iris**                    (identifying three types of iris flowers),
 - **mushroom**                (identifying edible and poisonous mushrooms), and
 - **vote**                    (identifying democrats and republicans in the House of
                                Representatives according to the 1984 US Congressional Voting Records).

All  examples  are  equipped   with  a  `Makefile`,  facilitating  the
execution of  the MCP system on  them. Some of these  examples use the
parallel modules  `mcp-mpi` or `mcp-hybrid`.   If you did  not install
them,  please  replace  the  commands  `mpirun  mcp-mpi`  and  `mpirun
mcp-hybrid` by `mcp-pthread`.

EOF
