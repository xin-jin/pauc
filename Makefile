CXX = clang++

# Define CXX compile flags
CXXFLAGS += -std=c++14 -O3 -funroll-loops -W -Wall -Wextra #-Wfatal-errors
LDFLAGS +=
LDLIBS +=
INCLUDES +=

EXEC += pauc

##################
# The following part of the makefile defines generic rules; it can be used to
# build any executable just by changing the definitions above.
#
#   $^: the name of the prereqs of the rule
#   $<: the name of the first prereq of the rule
#   $@: the name of the target of the rule
##################

# 'make' - default rule
all: $(EXEC)

# Default rule for creating an exec of $(EXEC) from a .o file
$(EXEC): % : %.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Default rule for creating a .o file from a .cpp file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEPSFLAGS) -c -o $@ $<

# 'make clean' - deletes all .o files, exec, and dependency files
clean:
	-$(RM) $(EXEC)
	$(RM) -r $(DEPSDIR)
	find . -name "*.o" -delete

# Define rules that do not actually generate the corresponding file
.PHONY: clean all

