# The code is released under GPL v2
#
#
#    Makefile for creating the APPL library and related executables.
#
#
#    make
#	Build all the targets.
#
#    make clean
#	Remove all the generated binaries.
#
#    make install
#	Install all the targets.
#
#    make depend
# 	Generate source file dependencies.
#
#    make parser
#	Create the POMDP input file parser.
#
#

# ----------------------------------------------------------------------
# Customizations
# ----------------------------------------------------------------------

#
# root directory of the package
#
ROOT	      = ..

#
# destination directory
#
DESTDIR	      = $(ROOT)

#
# name of dependency file
#
DEPFILE       = Makefile.dep

#
# detecting OS
#
uname_O := $(shell sh -c 'uname -o 2>/dev/null || echo not')

ifeq ($(uname_O),Cygwin)
	CYGWIN_CFLAGS = -D_CYGWIN
	CYGWIN_INCDIR = -I./miniposix/
endif


# ----------------------------------------------------------------------
# Compiler
# ----------------------------------------------------------------------

CC            = gcc

CFLAGS        = -g -w -O3 $(INCDIR) -msse2  -mfpmath=sse $(CYGWIN_CFLAGS)  #-DDEBUG_LOG_ON

CXX           = g++

CXXFLAGS      = $(CFLAGS)

INCDIR	      = -I./MathLib -I./Algorithms -I./Algorithms/HSVI -I./Algorithms/SARSOP -I./Models/MOMDP/ -I./Models/MOMDP/CoLoc/ -I./OfflineSolver/ -I./Bounds/ -I./Core/ -I./Parser/Cassandra/ -I./Parser/Cassandra/include -I./Parser/ -I./Parser/POMDPX/ -I./Utils/ -I./Simulator/ -I./Evaluator/ -I./Controller/ $(CYGWIN_INCDIR)


# ----------------------------------------------------------------------
# Linker
# ----------------------------------------------------------------------

LINKER	      = g++

LDFLAGS	      = -L.

LIBS	      = -lappl


# ----------------------------------------------------------------------
# Archiver
# ----------------------------------------------------------------------

ARCHIVER      = ar

ARFLAGS	      = -ruv


# ----------------------------------------------------------------------
# Others
# ----------------------------------------------------------------------

PRINT	      = lpr


# ----------------------------------------------------------------------
# Files
# ----------------------------------------------------------------------

TARGETS	      = $(TARGET_APPL) $(TARGET_OFSOL) $(TARGET_SIM) $(TARGET_EVA) $(TARGET_GRAPH) $(TARGET_CONVERT) $(TARGET_CTRL)

HDRS	      = $(HDRS_APPL) $(HDRS_OFSOL) $(HDRS_SIM) $(HDRS_EVA) $(HDRS_GRAPH) $(HDRS_CONVERT) $(HDRS_CTRL)

SRCS	      = $(SRCS_APPL) $(SRCS_OFSOL) $(SRCS_SIM) $(SRCS_EVA) $(SRCS_GRAPH) $(SRCS_CONVERT) $(HDRS_CTRL)

OBJS	      = $(OBJS_APPL) $(OBJS_OFSOL) $(OBJS_SIM) $(OBJS_EVA) $(OBJS_GRAPH) $(OBJS_CONVERT) $(OBJS_CTRL)

#
# source file directories
#

#
# target: evaluate
#
TARGET_EVA	      = pomdpeval

HDRS_EVA	      =

SRCS_EVA	      = ./Evaluator/Evaluator.cpp


OBJS_EVA	      = $(SRCS_EVA:.cpp=.o) # C++ object targets


#
# target: simulate
#
TARGET_SIM	      = pomdpsim

HDRS_SIM	      =

SRCS_SIM	      = ./Simulator/Simulator.cpp


OBJS_SIM	      = $(SRCS_SIM:.cpp=.o) # C++ object targets


#
# target: offline solver
#
TARGET_OFSOL	      = pomdpsol

HDRS_OFSOL	      =

SRCS_OFSOL	      = ./OfflineSolver/solver.cpp

OBJS_OFSOL	      = $(SRCS_OFSOL:.cpp=.o) # C++ object targets

#
# target: policy graph plotter
#
TARGET_GRAPH	      = polgraph

HDRS_GRAPH      =

SRCS_GRAPH	      = ./PolicyGraph/PolicyGraph.cpp

OBJS_GRAPH	      = $(SRCS_GRAPH:.cpp=.o) # C++ object targets

#
# target: pomdp to pomdpx convertor
#
TARGET_CONVERT	      = pomdpconvert

HDRS_CONVERT      =

SRCS_CONVERT	      = ./PomdpConvertor/convertor.cpp

OBJS_CONVERT	      = $(SRCS_CONVERT:.cpp=.o) # C++ object targets

#
# target: library
#
TARGET_APPL	      = libappl.a

HDRS_APPL	      = 	./PolicyGraph/PolicyGraphGenerator.h \
					./Evaluator/EvaluationEngine.h \
					./Evaluator/EvaluatorBeliefTreeNodeTuple.h \
					./Evaluator/EvaluatorSampleEngine.h \
					./Simulator/SimulationEngine.h \
					./MathLib/DenseVector.cpp \
					./MathLib/DenseVector.h \
					./MathLib/MathLib.cpp \
					./MathLib/MathLib.h \
					./MathLib/SparseMatrix.cpp \
					./MathLib/SparseMatrix.h \
					./MathLib/SparseVector.cpp \
					./MathLib/SparseVector.h \
					./Algorithms/PointBasedAlgorithm.h \
					./Algorithms/SARSOP/AlphaPlaneMaxMeta.h \
					./Algorithms/SARSOP/BinManager.cpp \
					./Algorithms/SARSOP/BinManager.h \
					./Algorithms/SARSOP/BinManagerSet.cpp \
					./Algorithms/SARSOP/BinManagerSet.h \
					./Algorithms/SARSOP/Prune.h \
					./Algorithms/SARSOP/Sample.cpp \
					./Algorithms/SARSOP/Sample.h \
					./Algorithms/SARSOP/SampleBP.cpp \
					./Algorithms/SARSOP/SampleBP.h \
					./Algorithms/SARSOP/SARSOP.cpp \
					./Algorithms/SARSOP/SARSOP.h \
					./Algorithms/SARSOP/SARSOPPrune.cpp \
					./Algorithms/SARSOP/SARSOPPrune.h \
					./miniposix/getopt.h \
					./miniposix/getopt_int.h \
					$(wildcard ./Models/MOMDP/*.h) \
					./OfflineSolver/GlobalResource.h \
					./OfflineSolver/solverUtils.h \
					./Bounds/BlindLBInitializer.h \
					./Bounds/AlphaPlane.h \
					./Bounds/AlphaPlanePool.h \
					./Bounds/AlphaPlanePoolSet.h \
					./Bounds/AlphaVectorPolicy.h \
					./Bounds/BackupAlphaPlaneMOMDP.h \
					./Bounds/BackupAlphaPlaneMOMDPLite.h \
					./Bounds/BackupBeliefValuePairMOMDP.h \
					./Bounds/BackupBeliefValuePairMOMDPLite.h \
					./Bounds/BeliefValuePair.h \
					./Bounds/BeliefValuePairPool.h \
					./Bounds/BeliefValuePairPoolSet.h \
					./Bounds/FastInfUBInitializer.h \
					./Bounds/FullObsUBInitializer.h \
					./Bounds/PruneAlphaPlane.cpp \
					./Bounds/PruneAlphaPlane.h \
					./Bounds/PruneBeliefValuePair.h \
					./Bounds/xml_parse_lib.h \
					./Evaluator/EvaluatorBeliefTreeNodeTuple.h \
					./Core/FacmodelStructs.h \
					./Core/Actions.h \
					./Core/Backup.h \
					./Core/Belief.h \
					./Core/BeliefCache.h \
					./Core/Policy.h \
					./Core/BeliefCacheRow.h \
					./Core/BeliefException.h \
					./Core/BeliefForest.h \
					./Core/BeliefTransition.h \
					./Core/BeliefTreeNode.h \
					./Core/BeliefWithState.h \
					./Core/VariableContainer.h \
					./Core/Bound.h \
					./Core/BoundSet.h \
					./Core/Const.h \
					./Core/IndexedTuple.h \
					./Core/MObject.h \
					./Core/MObjectManager.h \
					./Core/MObjectUser.h \
					./Core/Observations.h \
					./Core/States.h \
					./Core/SymbolSet.h \
					./Core/SymbolSetIterator.h \
					./Core/Tuple.h \
					./Core/UniqueBeliefHeap.h \
					./Parser/Cassandra/decision-tree.h \
					./Parser/Cassandra/imm-reward.h \
					./Parser/Cassandra/include/pomdp_spec.tab.hh \
					./Parser/Cassandra/MDP.h \
					./Parser/Cassandra/mdpCassandra.h \
					./Parser/Cassandra/Parser.h \
					./Parser/Cassandra/parse_constant.h \
					./Parser/Cassandra/parse_err.h \
					./Parser/Cassandra/parse_hash.h \
					./Parser/Cassandra/POMDP.h \
					./Parser/Cassandra/pomdpCassandraWrapper.h \
					./Parser/Cassandra/sparse-matrix.h \
					./Parser/ParserSelector.h \
					./Parser/POMDPX/FactoredPomdp.h \
					./Parser/POMDPX/Function.h \
					./Parser/POMDPX/ObsAct.h \
					./Parser/POMDPX/POMDPLayer.h \
					./Parser/POMDPX/PreCEntry.h \
					./Parser/POMDPX/PreCMatrix.h \
					./Parser/POMDPX/SparseEntry.h \
					./Parser/POMDPX/SparseTable.h \
					./Parser/POMDPX/State.h \
					./Parser/POMDPX/StateObsAct.h \
					./Parser/POMDPX/tinystr.h \
					./Parser/POMDPX/tinyxml.h \
					./Parser/POMDPX/UniqueIndex.h \
					./Utils/CPMemUtils.h \
					./Utils/CPTimer.h \
					./Utils/Cache.h \
					./Utils/InfoLog.h \
					./Utils/md5.h \
					./Utils/md5wrapper.h \
					./Utils/StatsCollector.h \
					./Utils/SimulationRewardCollector.h \
					./Parser/Cassandra/include/pomdp_spec.tab.hh \
					./Controller/ControllerInterface.h \
					./Controller/Controller.h


SRCS_APPL	      = 	./PolicyGraph/PolicyGraph.cpp \
					./PolicyGraph/PolicyGraphGenerator.cpp \
					./Evaluator/EvaluationEngine.cpp \
					./Evaluator/EvaluatorSampleEngine.cpp \
					./Simulator/SimulationEngine.cpp \
					./MathLib/DenseVector.cpp \
					./MathLib/MathLib.cpp \
					./MathLib/SparseMatrix.cpp \
					./MathLib/SparseVector.cpp \
					./Algorithms/SARSOP/BinManager.cpp \
					./Algorithms/SARSOP/BinManagerSet.cpp \
					./Algorithms/SARSOP/Sample.cpp \
					./Algorithms/SARSOP/SampleBP.cpp \
					./Algorithms/SARSOP/SARSOP.cpp \
					./Algorithms/SARSOP/SARSOPPrune.cpp \
					$(wildcard ./Models/MOMDP/*.cpp) \
				./OfflineSolver/GlobalResource.cpp \
					./OfflineSolver/solverUtils.cpp \
					./Bounds/AlphaPlane.cpp \
					./Bounds/AlphaPlanePool.cpp \
					./Bounds/AlphaPlanePoolSet.cpp \
					./Bounds/AlphaVectorPolicy.cpp \
					./Bounds/BackupAlphaPlaneMOMDP.cpp \
					./Bounds/BackupAlphaPlaneMOMDPLite.cpp \
					./Bounds/BackupBeliefValuePairMOMDP.cpp \
					./Bounds/BackupBeliefValuePairMOMDPLite.cpp \
					./Bounds/BeliefValuePair.cpp \
					./Bounds/BeliefValuePairPool.cpp \
					./Bounds/BeliefValuePairPoolSet.cpp \
					./Bounds/BlindLBInitializer.cpp \
					./Bounds/FastInfUBInitializer.cpp \
					./Bounds/FullObsUBInitializer.cpp \
					./Bounds/PruneAlphaPlane.cpp \
					./Bounds/PruneBeliefValuePair.cpp \
					./Bounds/xml_parse_lib.c \
					./Core/Actions.cpp \
				./Core/VariableContainer.cpp \
					./Core/Belief.cpp \
					./Core/BeliefCache.cpp \
					./Core/BeliefException.cpp \
					./Core/BeliefForest.cpp \
					./Core/BeliefTreeNode.cpp \
					./Core/BeliefWithState.cpp \
					./Core/MObject.cpp \
					./Core/Observations.cpp \
					./Core/States.cpp \
					./Core/UniqueBeliefHeap.cpp \
					./Parser/Cassandra/Parser.cpp \
					./Parser/Cassandra/POMDP.cpp \
					./Parser/Cassandra/pomdpCassandraWrapper.cpp \
					./Parser/ParserSelector.cpp \
					./Parser/POMDPX/FactoredPomdp.cpp \
					./Parser/POMDPX/Function.cpp \
					./Parser/POMDPX/ObsAct.cpp \
					./Parser/POMDPX/PreCEntry.cpp \
					./Parser/POMDPX/PreCMatrix.cpp \
					./Parser/POMDPX/SparseEntry.cpp \
					./Parser/POMDPX/SparseTable.cpp \
					./Parser/POMDPX/State.cpp \
					./Parser/POMDPX/StateObsAct.cpp \
					./Parser/POMDPX/tinystr.cpp \
					./Parser/POMDPX/tinyxml.cpp \
					./Parser/POMDPX/tinyxmlerror.cpp \
					./Parser/POMDPX/tinyxmlparser.cpp \
					./Utils/InfoLog.cpp \
					./Utils/md5.cpp \
					./Utils/md5wrapper.cpp \
					./Utils/CPMemUtils.cpp \
					./Utils/StatsCollector.cpp \
					./Utils/SimulationRewardCollector.cpp \
					./Parser/Cassandra/include/pomdp_spec.tab.cc \
					./Parser/Cassandra/include/pomdp_spec.yy.cc \
					./miniposix/getopt.c \
					./miniposix/getopt1.c \
					./miniposix/getopt_init.c \
					./Parser/Cassandra/decision-tree.c \
					./Parser/Cassandra/imm-reward.c \
					./Parser/Cassandra/mdpCassandra.c \
					./Parser/Cassandra/parse_err.c \
					./Parser/Cassandra/parse_hash.c \
					./Parser/Cassandra/sparse-matrix.c \
					./Controller/Controller.cpp




OBJS_APPL_C	      = $(SRCS_APPL:.c=.o) 	# C object targets
OBJS_APPL_CC	  = $(OBJS_APPL_C:.cc=.o) 	# C++ object targets
OBJS_APPL	      = $(OBJS_APPL_CC:.cpp=.o)  # C++ object targets

#
# target: controller
#
TARGET_CTRL		= testControllerTiger
HDRS_CTRL		=
SRCS_CTRL		= ./Controller/testControllerTiger.cpp
OBJS_CTRL		= $(SRCS_CTRL:.cpp=.o) # C++ object targets


# ----------------------------------------------------------------------
# Targets
# ----------------------------------------------------------------------

all:		$(TARGETS)

$(TARGET_APPL): $(OBJS_APPL)
		$(ARCHIVER) $(ARFLAGS) $(TARGET_APPL) $(OBJS_APPL)
		ranlib $(TARGET_APPL)

$(TARGET_OFSOL): $(OBJS_OFSOL) $(TARGET_APPL)
		 $(LINKER) $(LDFLAGS) $(OBJS_OFSOL) $(LIBS)  -o $(TARGET_OFSOL)

$(TARGET_SIM):	$(OBJS_SIM) $(TARGET_APPL)
		$(LINKER) $(LDFLAGS) $(OBJS_SIM) $(LIBS)  -o $(TARGET_SIM)

$(TARGET_EVA):	$(OBJS_EVA) $(TARGET_APPL)
		$(LINKER) $(LDFLAGS) $(OBJS_EVA) $(LIBS)  -o $(TARGET_EVA)

$(TARGET_GRAPH):$(OBJS_GRAPH) $(TARGET_APPL)
		$(LINKER) $(LDFLAGS) $(OBJS_GRAPH) $(LIBS)  -o $(TARGET_GRAPH)

$(TARGET_CONVERT):$(OBJS_CONVERT) $(TARGET_APPL)
		$(LINKER) $(LDFLAGS) $(OBJS_CONVERT) $(LIBS)  -o $(TARGET_CONVERT)

$(TARGET_CTRL):$(OBJS_CTRL) $(TARGET_APPL)
		$(LINKER) $(LDFLAGS) $(OBJS_CTRL) $(LIBS)  -o $(TARGET_CTRL)

clean:;		rm -f $(OBJS)
		rm -f $(TARGETS)


install:	$(TARGETS)
		[ -d ../bin ] || mkdir ../bin
		[ -d ../lib ] || mkdir ../lib
		mv $(TARGET_OFSOL) $(DESTDIR)/bin/$(TARGET_OFSOL)
		mv $(TARGET_APPL) $(DESTDIR)/lib/$(TARGET_APPL)

depend:;	g++ -MM $(INCDIR) $(HDRS) $(SRCS) > $(DEPFILE)

#depend:;	makedepend -Y $(INCDIR) $(SRCS)

.PHONY:		parser
parser:;	cd $(ParserDIR) && $(MAKE)

echo:;		echo $(TARGETS);

#update:	$(DEST)/$(PROGRAM)

#print:;	$(PRINT) $(HDRS) $(SRCS)


include $(DEPFILE)

# DO NOT DELETE
