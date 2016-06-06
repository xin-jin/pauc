TARGET   = pauc

CXX       = clang++
# compiling flags here
CXXFLAGS  = -O3 -std=c++14 -Wall -I.

LINKER   = $(CXX) -o
# linking flags here
LFLAGS   = -Wall -I. -lm -pthread

# change these to set the proper directories where each files shoould be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
INCDIR   = include

SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
INCLUDES := $(wildcard $(INCDIR)/*.hpp)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
rm       = rm -f

# useful for debugging and profiling
ifeq ($(PROFILE), 1)
	CXXFLAGS += -g -O0
endif

# use g++ instead
ifeq ($(GCC), 1)
	CXX = g++
endif


$(BINDIR)/$(TARGET): $(OBJECTS)	
	    @$(LINKER) $@ $(LFLAGS) $(OBJECTS)
	    @echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	    @$(CXX) $(CXXFLAGS) -c $< -o $@
	    @echo "Compiled "$<" successfully!"

.PHONEY: clean
clean:
		@$(rm) $(OBJECTS)
		@$(rm) $(BINDIR)/$(TARGET)	
	    @echo "Cleanup complete!"
