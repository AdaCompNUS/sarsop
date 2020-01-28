# SARSOP
Efficient Point-Based POMDP Planning by Approximating

Approximate POMDP Planning (APPL) Toolkit

APPL is a C++ implementation of the SARSOP algorithm <sup>1</sup>, using the factored MOMDP representation <sup>2</sup>. It takes as input a POMDP model in the POMDP or POMDPX file format and produces a policy file. It also contains a simple simulator for evaluating the quality of the computed policy. 

[1] [**SARSOP: Efficient point-based POMDP planning by approximating optimally reachable belief spaces.**](http://www.roboticsproceedings.org/rss04/p9.pdf) *H. Kurniawati, D. Hsu, and W.S. Lee*. In Proc. Robotics: Science and Systems, 2008.

[2] [**POMDPs for robotic tasks with mixed observability.**](http://www.roboticsproceedings.org/rss05/p26.pdf) *S.C.W. Ong, S.W. Png, D. Hsu, and W.S. Lee*. In Proc. Robotics: Science and Systems, 2009.

[Copyright (c) 2008-2010 by National University of Singapore.](http://motion.comp.nus.edu.sg/projects/pomdp/pomdp.html)


## Table of Contents

* [Requirements](#requirements)
* [Quick Start](#quick-start)
* [Documentation](#documentation)
* [Package Contents](#package-contents)
* [Acknowledgments](#acknowledgements)
* [Bugs and Suggestions](#bugs-and-suggestions)
* [Release Notes](#release-notes)


## Requirements

Tested Operating systems: 

* Linux 
* Mac OS X (unofficial) (see a fork tested on OSX 10.15.2 [here](https://github.com/personalrobotics/appl))
* Windows (unofficial)



Tested compilers:

* gcc/g++ 4.0.1 under Linux
* gcc/g++ 4.1.2 under Linux
* gcc/g++ 4.2.3 under Linux
* gcc/g++ 4.3.2 under Linux
* gcc/g++ 4.4.0 under Linux
* gcc/g++ 4.8.1 under Linux
* gcc/g++ 3.4.4 under Windows (Cygwin)
* Microsoft Visual C++ 9 in Visual Studio 2008 with SP1 under Windows


General:
  For gcc/g++ tool chain, GNU make is required for building.


Windows:
The Visual Studio port is experimental. The generated binaries may behave differently from its Linux counterpart.
  For Visual Studio 2010, refer to FAQ at http://bigbird.comp.nus.edu.sg/pmwiki/farm/appl/index.php?n=Main.FAQ for more information

## Quick Start

For Linux or Windows (Cygwin), at the top-level directory, type the commands:

```bash
$ git clone https://github.com/AdaCompNUS/sarsop.git
$ cd sarsop/src
$ make
```

A total of 4 executables are generated in the current directory:  

 * `pomdpsol` computes a policy.  
 * `pomdpsim` is the policy simulator.  
 * `pomdpeval` is the policy evaluator.  
 * `polgraph` outputs a graphical representation of a policy.  
 * `pomdpconvert` converts a POMDP model file to POMDPX file format.  


Please read [doc/Usage.txt](doc/Usage.text) for command-line options.

### Unix

1. Try solving an example problem. Type the command:
```bash
$ ./pomdpsol ../examples/POMDPX/Tiger.pomdpx
```
The generated policy is written to "out.policy" by default.  

&nbsp;
2. Try simulating a policy. Type the command:
```bash
$ ./pomdpsim --simLen 100 --simNum 1000 --policy-file out.policy ../examples/POMDPX/Tiger.pomdpx
```
  The simulation results are written to the console.

&nbsp;
3. Try generating a graphical representation of a policy . Type the command:
```bash
$ ./polgraph --policy-file out.policy --policy-graph tiger.dot ../examples/POMDPX/Tiger.pomdpx
```
   The generated graph is written to "tiger.dot". You can view the output "tiger.dot" using Graphviz (http://www.graphviz.org/).  A pdf file generated from "tiger.dot" is in "../examples/POMDPX/tiger.pdf".

&nbsp;
4. Try converting a POMDP file to POMDPX file format:
```bash
 $ ./pomdpconvert ../examples/POMDP/Tiger.pomdp
```
  The generated POMDPX file is in the same directory as the POMDP model file.  
  
Sample results from the above commands are in "../examples/POMDPX/tiger.log".

### Windows

&nbsp;
For Windows with Microsoft Visual Studio 2008:

* Unpack the zip file
* Open the solution file : src\momdp.sln
* Select from Menu Build -> Build Solution

Generated binaries is in appl\src\release directory, pomdpsol.exe is the Solver, pomdpsim.exe is the Simulator, pomdpeval.exe is the Evaluator, polgraph is the policy graph generator.

&nbsp;
Try solving an example problem:

* Open up a command-line console:
* Click on Start Menu -> Run
* Key in "cmd" and press Enter
* Go to the directory where the generated binaries are
Example (assume the source code is located at c:\appl\src): 
```bash
$ cd c:\appl\src\release
$ pomdpsol.exe ..\..\examples\POMDPX\Tiger.pomdpx
```
The policy will be written to "out.policy".

&nbsp;
Try simulating the generated policy
```bash
$ pomdpsim.exe --simLen 100 --simNum 1000 --policy-file out.policy ..\..\examples\POMDPX\Tiger.pomdpx
```
The simulation results are written to the console.

&nbsp;
Try evaluating the generated policy
```bash
$ pomdpeval.exe --simLen 100 --simNum 1000 --policy-file out.policy ..\..\examples\POMDPX\Tiger.pomdpx
```
The evaluation results are written to the console.

&nbsp;
Try generating a graphical representation of a policy . Type the command:
```bash
$ polgraph.exe --policy-file out.policy --policy-graph tiger.dot ..\..\examples\POMDPX\Tiger.pomdpx
```
&nbsp;
Try converting a POMDP file to POMDPX file format:
```bash
$ pomdpconvert ..\examples\POMDP\Tiger.pomdp
```
The generated POMDPX file is in the same directory as the POMDP model file.


## Documentation

Documentation can be found in the directory "doc". See PACKAGE CONTENTS for a detailed listing.


## Package Contents

```
doc/FAQ.txt                           Frequently asked questions
doc/Install.txt                           Detailed compilation instruction
doc/Usage.txt                             Explanation of command-line options
doc/POMDP/PomdpFileFormat.html            POMDP file format
doc/POMDPX/PomdpxFileFormat.pdf           POMDPX file format
doc/POMDPX/Pomdpx.xsd                     POMDPX XML schema file
license/License                           License
license/Notice                            Attribution notices as required by Apache License 2.0
src/Parser                                POMDP and POMDPX parsers
src/Models                                POMDP model representation
src/Core                                  Core data structures
src/Algorithms                            POMDP algorithms
src/Bounds                                Value functions
src/MathLib                               Math library
src/OfflineSolver                         Offline solver
src/Simulator                             Simulator
src/Evaluator                             Evaluator
src/Utils                                 Cross platform utility code
src/Makefile                              Linux make file
examples/                                 Example POMDP and POMDPX model files
```

Windows specific:
```
src/miniposix                             POSIX compliance code for Windows
src/OfflineSolver/OfflineSolver.vcproj    Visual Studio 2008 Project file for Offline solver
src/Simulator/Simulator.vcproj            Visual Studio 2008 Project file for Simulator
src/Evaluator/Evaluator.vcproj            Visual Studio 2008 Project file for Evaluator
src/PolicyGraph/PolicyGraph.vcproj	  Visual Studio 2008 Project file for Policy graph generator
src/momdp.sln                             Visual Studio 2008 solution
```

## Acknowledgements

Part of the APPL toolkit makes use of code based on an early version of ZMDP by Trey Smith (http://www.cs.cmu.edu/~trey/zmdp/). ZMDP in turn uses code from pomdp-solve by Tony Cassandra (http://www.cassandra.org/pomdp/code/index.shtml). The POMDPX parser uses TinyXML by Lee Thomason (http://www.grinninglizard.com/tinyxml/).


## Bugs and Suggestions
Please use the issue tracker.

## Release Notes

*9-Jun-2014*  
Fixed compilation issues with gcc version >= 4.7  
Various matrix operation optimizations  

*6-Mar-2012*  
Added support for intra-slice dependency between variables in POMDPX model. Only edges fully observable state X to partially observable state Y are allowed.  

*9-Aug-2011, version 0.95*  
IMPORTANT: Fixed a bug that caused pomdpsim to output wrong maximum likelihood unobserved state in log file  
Fixed a bug in parsing POMDPX file with XML comments (We thank Alex Goldhoorn for bug report and patch)  

*28-Apr-2010, version 0.94*  
Fix a bug that terminal states are not correctly recognized in some model.  

*22-Apr-2010, version 0.93*  
Solves MDP specified in POMDPX file format.  Output MDP solution in PolicyX format.  

*12-Mar-2010, version 0.92*  
Fixed a bug in POMDPX file format parser when reward is a function of current state R(s',a).  
Output policy file in XML format.  
Added POMDP to POMDPX converter.  

*24-Nov2009, version 0.91*  
Fixed several bugs in the POMDPX file format parser.  
Cleaned up POMDPX file format parser code and reduced memory usage.  

*04-Sep-2009, version 0.9*  
Substantially restructured the code base.  
Added support for the new factored .pomdpx file format.  
The main POMDP solver now uses the SARSOP algorithm with the MOMDP representation.  
Added policy graph generator.  

*01-Feb-2009, version 0.3*  
Added option for memory limit.  
Reduced memory usage.  
Added policy evaluator.  

*25-Jul-2008, version 0.2*  
Minor bug fixes.  

*01-Jul-2008, version 0.1*  
Initial release.  
