target = figrid
OPT = -O3 -flto

ifeq '$(findstring sh,$(SHELL))' 'sh'
# UNIX, MSYS2 or Cygwin
RM = rm -f
else
# Windows Cmd
RM = del /Q
endif

CXXFLAGS = $(OPT)
LDFLAGS = $(CXXFLAGS)

objects = recording.o tree.o rule.o rule_original.o session.o tui.o main.o

$(target): $(objects)
	$(CXX) $(objects) $(LDFLAGS) -o $@

.PHONY: clean
clean:
	-$(RM) *.o $(target)
