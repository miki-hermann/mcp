# MULTI-CLASSIFICATION PROJECT (MCP), v1.05
				   
## Table of contents

* [General instructions](#general-instructions)
* [Brief Description](#brief-description)
* [Compilation and Installation](#compilation-and-installation)
* [Examples](#examples)
* [UCI Examples](#uci-examples)
* [kaggle Examples](#kaggle-examples)

## General instructions

Before downloading the MCP software, you need to install the `git` and
`git-lfs` packages. Under RHEL, Fedora, and CentOS based distributions
run the command
``` bash
sudo dnf install git git-lfs
```
Under Debian and Ubuntu based distributions, run the command

``` bash
sudo apt-get install git git-lfs
```
Then you can `cd` to the directory where you wish the MCP system to be
installed and run the command

``` bash
git clone https://github.com/miki-hermann/mcp.git
```

## Brief Description

This  is   the  MCP  system   designed  to  transform   datasets  into
propositional formulas. It consists of  several modules. The main part
is the MCP core. In addition to  the core there are prequel and sequel
supporting modules.

MCP core has two variants:

    mcp-seq     (sequential)
    mcp-pthread (parallel using POSIX threads)

The prequel modules are

    mcp-guess    (guessing the structure of the input dataset),
    mcp-overview (overview of concept values and their percentage in dataset)
    mcp-trans    (transformation of input dataset)
    mcp-sample   (choosing a representative sample of dataset determined by confidence interval, error bound, or cardinality)
    mcp-clean    (cleaning the dataset from outliers)
    mcp-split    (splitting a dataset into a learning and checking part)
    mcp-uniq     (elimination of doubled data)

The sequel modules are

    mcp-check   (checking the accuracy of the produced formula)
    mcp-predict (prediction of values in test dataset)
    mcp-chk2tst (transforming check files to test files)
    mcp-compare (comparison of prediction wrt existing concept values)
    mcp-mat2csv (transforming a matrix file to CSV file: spaces -> commas)

Additionally, the distribution contains the following files:

    README.md (this document),
    LICENCE   (GNU General Public Licence v.3, under which this software is distributed),
    Makefile  (compilation and installation of the MCP system)

## Compilation and Installation
* [Outline of the distribution](#outline-of-the-distribution)
* [Prerequisites for compilation](#prerequisites-for-compilation)
* [Compilation](#compilation)

### Outline of the distribution

The subdirectories of this distribution are:
 - *src-noarch*  sources of MCP system for all variants,
 - *src-seine*   sources of MCP system for Boolean variant using only C++ standard library,
 - *src-danube*  sources of MCP system for Boolean variant using C++ standard  and boost libraries,
 - *src-mekong*  sources of MCP system for many-valued variant using only C++ standard library,
 - *man*         manual pages of MCP commands,
 - *paper*       PDF document `mcp.pdf` with a detailed description of the MCP system
 - *bin*         sudirectory containing the binaries (necessary for compilation),
 - *uci*         examples from the *UCI Machine Learning Repository*, treated by the MCP system, and
 - *kaggle*      examples from the *kaggle* machine learning repository, treated by the MCP system

The sources in *src-noarch* are independent of the used version. The
sources in *src-seine* and *src-danube* are almost the same, only the
rows in the former are coded by `vector<bool>` whereas in the latter
by `dynamic_bitset`. This means that the structures in the *seine*
version are eight times longer than in the *danube* version.  The
sources of *src-mekong* are quite different from the previous two.

All modules exist either in the *noarch* version or for all three
versions. From outside the modules for different version have the same
appearance.

### Prerequisites for compilation

Several modules are interconnected and call themselves depending on
the chosen version. To ensure compatibility between them and also to
avoid potential threats from hackers by supplying a virus named as one
of the modules, the whole interdependence of the modules is guaranteed
by means of SHA3 encoding. To be able to use it you need to run the
following command(s) from your shell: in Fedora, RHEL, and CentOS
based distributions, the commands are
```bash
sudo dnf install openssl openssl-devel
sudo dnf install cryptopp cryptopp-devel
```
On Debian and Ubuntu based distributions, you need to install `libssl`
instead of `openssl`:
```bash
sudo apt-get install libssl libssl-dev
sudo get-apt install libcrypto libcrypto-dev
```

### Compilation

* [Invocation](#invocation)
* [Changing compiler variables in Makefile](#changing-compiler-variables-in-Makefile)
* [Partial compilation](#partial-compilation)

A C++ compiler satisfying the C++17  revision or newer is necessary to
successfully  compile the  MCP system,  mainly for  the use  of `auto`
types,  POSIX  threads,  and  for sampling.   Only  the  standard  and
**`boost`** libraries are used, therefore  there is no need to install
any additional  C++ libraries.  You  will need to install  the `boost`
library  for  the `*danube*`  variant,  since  this variant  uses  the
`dynamic bitset` structure.

### Invocation

To compile and install the MCP system, write the command
```Makefile
make
```
in the root directory of this distribution. The system then

   - compiles the sources,
   - links the binaries together using SHA3,
   - installs the binaries in the *EXECUTABLES* directory `/usr/local/bin`,
   - determines which variant (*seine* or *danube* or *mekong*) will be the
     default version of the MCP system

The command
```Makefile
make man
```
installs the manual pages in the directories
`/usr/local/share/man/man1` and `/usr/local/share/man/man5`,

You need to have superuser privileges to execute these commands.

There are three cleaning commands in the Makefile, graded by their
force. To clean the intermediate results of compilation (`*.o` files)
and the binaries in the local `bin` directory, type

``` Makefile
make clean
```
To erase also the switching module together with the previous
cleaning, type

``` Makefile
make scratch
```
To erase all MCP system binaries from the *EXECUTABLES* directory,
together with the previous scratching, type

``` Makefile
make eliminate
```
You need to have superuser privileges for the last command.

You are suggested to clean up after the compilation and installation
process with the command `make scratch`.

### Changing compiler variables in Makefile

The sources of the MCP system are compiled by default with the GNU C++
compiler `g++` satisfying  the C++23 revision.  If  your installed C++
compiler has  a different  identifier, set the  variable *GXX*  in the
`make` command, e.g.
```Makefile
make GXX="cxx"
```
when your C++ compiler identifier is `c++`. If you need to downgrade
the version of your C++ compiler, use the variable *GXX_VERSION* in
the `make` command, e.g.
```Makefile
make GXX="-std=c++17"
```
when you need to downgrade to  version C++17. C++ compiler of an older
version than C++17 will not be able to compile the sources.

If you do not have superuser privileges or you do not need them, you
can suppress the call to the `sudo` command by setting the variable
*SUDO* with
```Makefile
make SUDO=""
```
If you  you want  to or  you must  store the  binaries in  a different
directory that `/usr/local/bin`, you can do it by setting the variable
*EXECUTABLES*, e.g.,
```Makefile
make EXECUTABLES="~/bin"
```
if you want to store the executables in the subdirectory `bin` under
your home directory.

If your manual pages reside somewhere else than in the directory
`/usr/local/share/man`, you can force to store them by the variable
*MANPAGES*, e.g.
```Makefile
make man MANPAGES="~/man"
```
if you want to keep them in the subdirectory `man` under your home
directory.

### Partial compilation

You do not need to install the whole MCP system, but you can choose
the variants which you wish to install. The following step-by-step
procedure explains the precedure to follow. The installation must be
performed in the described order.

The first variant to install is *noarch*. You are strongly advised
**not** to skip this part, otherwise you will loose all prequel
modules except `mcp-trans`, as well as the sequel modules `mcp-chk2tst`
and `mcp-mat2csv`. You install the *noarch* modules with the command

``` bash
make noarch
```

The second variant to install is  *seine*. All modules in this variant
are equivalent to those in  the *danube* variant.  The only difference
is that the row structure is coded by `vector<bool>`. You can skip the
installation  of  this  variant  if you  have  installed  the  `boost`
library,   since  the   *danube*   version   equivalent  modules   are
faster. However, you  are strongly advised to install at  least one of
the variants  *seine* and *danube*.   You install the  *seine* modules
with the command

``` bash
make seine
```

The third variant to install is *danube*. All modules in this variant
are equivalent to those in the *seine* variant. The only difference is
that the row structure is coded by `dynamic_bitset` from the `boost`
library.  You can skip the installation of this variant if you did not
install the `boost` library, but the *seine* version equivalent
modules are slower. However, you are strongly advised to install at
least one of the variants *seine* and *danube*.  You install the
*danube* modules with the command

``` bash
make danube
```

The fourth variant to install is *mekong*. All modules have the same
semantics as those in the *seine* and *danube* variants, but the
implementation and the results are different. You can skip the
installation of this variant if you do not wish to produce formulas in
many-valued logics and you prefer to stick to the classical Boolean
ones. You install the *mekong* modules with the command

``` bash
make mekong
```

Now you must execute a command which prepares the switching and
interconnection between different variants of the same modules. It is
performed with the command

``` bash
make switch
```

Once  your  chosen  combination  of versions  *seine*,  *danube*,  and
*mekong* has  been compiled, and  the switching was prepared  as well,
you need to install them by the command

``` bash
make install
```
so that the following commands can find them in the execution path.

Finally, you need to generate the interconnection between modules,
ensured by SHA3 encoding, and perform the final installation with the
command

``` bash
make generate-and-install
```

You are suggested to clean up after the compilation and installation
process with the command `make scratch`.

## Examples

All  examples  are  equipped   with  a  `Makefile`,  facilitating  the
execution of  the MCP system on  them. If you want to run these
examples or your own examples, you must set the emvironment variable
*MCP_VERSION* to `seine`, `danube`, or `mekong`, depending of the
version you want to use. It also allows you to easily switch between
the versions. For instance,
```csh
setenv MCP_VERSION danube
```
forces the use of the *danube* version of of MCP when you use the
`[t]csh` shell. With `bash`, this will be written as
```bash
export MCP_VERSION=danube
```
If you want now to switch to the *mekong* version, just write
```csh
setenv MCP_VERSION mekong
```
or
```bash
export MCP_VERSION=mekong
```
depeing on your shell.

If  you do  not specify  the *MCP_VERSION*  variable, MCP  system will
launch  the version  declared as  default during  installation, but  a
warning will be issued.

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

 - **credit card fraud**       (identifying credit card fraud in 2023).

EOF
