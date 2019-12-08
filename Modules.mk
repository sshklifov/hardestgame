PREFIX ?= ../..
INCLUDEDIR=$(PREFIX)/include
LIBDIR=$(PREFIX)/lib

OPT=-Wall -Wextra -fmax-errors=1 -O2 -DNDEBUG
MACROS:=-I$(INCLUDEDIR)

headerFiles:=$(wildcard *.h)
sourceFiles:=$(wildcard *.cpp)
files:=$(basename $(sourceFiles))
objectFiles:=$(addsuffix .o,$(files))

outputHeaderFiles:=$(addprefix $(INCLUDEDIR)/,$(headerFiles))
outputObjectFiles:=$(addprefix $(LIBDIR)/,$(objectFiles))

.PHONY: default
default: headers objs

.PHONY: headers
headers: $(outputHeaderFiles) ;

.PHONY: objs
objs: depend $(outputObjectFiles)

$(outputObjectFiles): $(LIBDIR)/%.o: %.cpp
	mkdir -p $(@D)
	g++ -c ${OPT} ${MACROS} $< -o $@

$(outputHeaderFiles): $(INCLUDEDIR)/%.h: %.h
	mkdir -p $(@D)
	cp $< $@

.PHONY: clean
clean:
	rm -f $(outputHeaderFiles) $(outputObjectFiles)
	rmdir $(INCLUDEDIR) &>/dev/null || true
	rmdir $(LIBDIR) &>/dev/null || true
	rm -f Makefile.bak

depend: $(sourceFiles)
	makedepend -I$(INCLUDEDIR) -p$(LIBDIR)/ -- $(OPT) $^ 2>/dev/null
