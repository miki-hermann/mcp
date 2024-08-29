# MULTI-CLASSIFICATION PROBLEM (MCP), v1.04
				   
## Table of contents

* [Brief Description](#brief-description)
* [Compilation and Installation](#compilation-and-installation)
* [Examples](#examples)
* [UCI Examples](#uci-examples)
* [kaggle Examples](#kaggle-examples)



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

    mcp-guess    (guessing the structure of the input dataset),
    mcp-overview (overview of concept values and their percentage in dataset)
    mcp-sample   (choosing a representative sample of dataset determined by confidence interval, error bound, or cardinality)
    mcp-clean    (cleaning the dataset from outliers)
    mcp-split    (splitting a dataset into a learning and checking part)
    mcp-trans    (binarization of input dataset)
    mcp-uniq     (elimination of doubled data)

The sequel modules are

    mcp-check   (checking the accuracy of the produced formula)
    mcp-predict (prediction of values in test dataset)
    mcp-chk2tst (transforming check files to test files)
    mcp-mat2csv  (transforming a matrix file to CSV file: spaces -> commas)

Additionally, the distribution contains the following files:

    README.md (this document),
    LICENCE   (GNU General Public Licence v.3, under which this software is distributed),
    Makefile  (compilation and installation of the MCP system)

## Compilation and Installation
* [Outline of the distribution](#outline-of-the-distribution)
* [Compilation](#compilation)
* [Invocation](#invocation)

### Outline of the distribution

The subdirectories of this distribution are:
 - *src*   containing the sources of the MCP system,
 - *man*   containing the manual pages,
 - *paper* containing the PDF document `mcp.pdf` with a detailed description of the MCP system
 - *bin*   is necessary for compilation,
 - *uci*   containing several examples from the UCI Machine Learning
         Repository, treated by the MCP system, and
 - *kaggle* containing example(s) from the kaggle machine learning repository, treated by the MCP system

### Compilation

A  C++  compiler  satisfying  the   C++20  revision  is  necessary  to
successfully  compile the  MCP system,  mainly for  the use  of `auto`
types and POSIX threads.  Only  the standard and **`boost`** libraries
are used,  therefore there is  no need  to install any  additional C++
libraries.  However, you  will need to install the  `boost` library if
ou do  not have it  yet , since the  system uses the  *dynamic bitset*
structure.  The g++ GNU Project compiler is used in the `Makefile`. If
you have a different compiler,  please modify the `Makefile` according
to your installation.

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

## Examples

All  examples  are  equipped   with  a  `Makefile`,  facilitating  the
execution of  the MCP system on  them. Some of these  examples use the
parallel modules  `mcp-mpi` or `mcp-hybrid`.   If you did  not install
them,  please  replace  the  commands  `mpirun  mcp-mpi`  and  `mpirun
mcp-hybrid` by `mcp-pthread`.

## UCI Examples

The  uci subdirectory  contains the  following examples  from the  [UCI
Machine Learning Repository](http://archive.ics.uci.edu/ml/), prepared
to be treated by the MCP system. These examples are

 - **abalone**                 (identifying abalone with 27 rings),
 - **accent**		       (speaker accents recognition),
 - **balance-scale**           (identifying psychological experiments balancing a scale),
 - **balloons**                (a toy example, where specific formulas are required to be produced),
 - **banknote**		       (recognizing genuine and forged banknotes),
 - **bean**		       (identifying grains of 7 different dry beans),
 - **breast-cancer-wisconsin** (identifying benign and malignant breast cancer cases in Wisconsin),
 - **car**                     (identifying very good cars),
 - **divorce**		       (divorce prediction),
 - **forest-fire**             (predicting forest fires in July, August, and September),
 - **iris**                    (identifying three types of iris flowers),
 - **monk**		       (the MONK's problems),
 - **mushroom**                (identifying edible and poisonous mushrooms),
 - **nursery**		       (evaluation of applications for nursery schools),
 - **optdigits**	       (optical recognition of handwritten digits),
 - **rice**		       (identifying two rice varieties: cammeo and osmancik),
 - **shuttle**		       (shuttle example),
 - **vote**                    (identifying democrats and republicans in the House of
                                Representatives according to the 1984 US Congressional Voting Records).

## kaggle Examples

The  kaggle  subdirectory contains  the  following  examples from  the
[kaggle Machine Learning Repository](https://www.kaggle.com/),
prepared to be treated by the MCP system. These examples are

 - **credit card fraud**                 (identifying credit card fraud in 2023).

EOF
