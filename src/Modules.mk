PREFIX ?= ../..
INCLUDEDIR=$(PREFIX)/include
LIBDIR=$(PREFIX)/lib

OPT=-Wall -Wextra -fmax-errors=1 -O2 -DNDEBUG
MACROS:=-I$(INCLUDEDIR)

headerFiles:=$(wildcard *.h)
inlineFiles:=$(wildcard *.inl)

sourceFiles:=$(wildcard *.cpp)
files:=$(basename $(sourceFiles))
objectFiles:=$(addsuffix .o,$(files))

outputHeaderFiles:=$(addprefix $(INCLUDEDIR)/,$(headerFiles))
outputInlineFiles:=$(addprefix $(INCLUDEDIR)/,$(inlineFiles))
outputObjectFiles:=$(addprefix $(LIBDIR)/,$(objectFiles))

.PHONY: default
default: headers objs

.PHONY: headers
headers: $(outputHeaderFiles) $(outputInlineFiles) ;

.PHONY: objs
objs: $(outputObjectFiles)

$(outputObjectFiles): $(LIBDIR)/%.o: %.cpp
	mkdir -p $(@D)
	g++ -c ${OPT} ${MACROS} $< -o $@

$(outputHeaderFiles): $(INCLUDEDIR)/%.h: %.h
	mkdir -p $(@D)
	cp $< $@

$(outputInlineFiles): $(INCLUDEDIR)/%.inl: %.inl
	mkdir -p $(@D)
	cp $< $@

.PHONY: clean
clean:
	rm -f $(outputHeaderFiles) $(outputInlineFiles) $(outputObjectFiles)
	rmdir --ignore-fail-on-non-empty $(INCLUDEDIR)
	rmdir --ignore-fail-on-non-empty $(LIBDIR)
