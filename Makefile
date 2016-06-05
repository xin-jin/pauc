CXX = clang++

# Define CXX compile flags
# CXXFLAGS += -std=c++14 -O3 -funroll-loops -W -Wall -Wextra #-Wfatal-errors
CXXFLAGS = -g $(WARNINGS) -O0 -std=c++14 $(DEPS) $(DEFINES) $(INCLUDES)
LDFLAGS += -pthread
LDLIBS +=
INCLUDES +=

EXEC += pauc

WARNINGS = -Wall -pedantic
DEPS = -MMD -MF $(@:.o=.d)
DEFINES = -D_GLIBCXX_USE_NANOSLEEP -D_GLIBCXX_USE_SCHED_YIELD

TP_LIB_SRC = thread-pool.cpp
TP_LIB_OBJ = $(patsubst %.cpp,%.o,$(patsubst %.S,%.o,$(TP_LIB_SRC)))
TP_LIB_DEP = $(patsubst %.o,%.d,$(TP_LIB_OBJ))
TP_LIB = thread-pool.a

PROGS = pauc
PROGS_SRC = $(PROGS:%=%.cpp)
PROGS_OBJ = $(patsubst %.cpp,%.o,$(patsubst %.S,%.o,$(PROGS_SRC)))
PROGS_DEP = $(patsubst %.o,%.d,$(PROGS_OBJ))


##################
# The following part of the makefile defines generic rules; it can be used to
# build any executable just by changing the definitions above.
#
#   $^: the name of the prereqs of the rule
#   $<: the name of the first prereq of the rule
#   $@: the name of the target of the rule
##################

# # Default rule for creating an exec of $(EXEC) from a .o file
# $(EXEC): %:%.o $(TP_LIB)
# 	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

default: $(PROGS)

$(PROGS): %:%.o $(TP_LIB)
	$(CXX) $^ $(LDFLAGS) -o $@

$(TP_LIB): $(TP_LIB_OBJ)
	rm -f $@
	ar r $@ $^
	ranlib $@

# # Default rule for creating a .o file from a .cpp file
# %.o: %.cpp
# 	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEPSFLAGS) -c -o $@ $<

# 'make clean' - deletes all .o files, exec, and dependency files
clean:
	-$(RM) $(EXEC)
	$(RM) -r $(DEPSDIR)
	rm -f $(TP_LIB) $(TP_LIB_DEP) $(TP_LIB_OBJ)	
	find . -name "*.o" -delete
	find . -name "*.out" -delete
	find . -name "*.in" -delete

# Define rules that do not actually generate the corresponding file
.PHONY: clean all

-include $(TP_LIB_DEP) $(PROGS_DEP)
