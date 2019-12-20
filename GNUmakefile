PREFIX ?= ..
LIBDIR=$(PREFIX)/lib
objectFiles:=$(wildcard $(LIBDIR)/*.o)

SYSTEM_LIBS=glfw3
OPT=-ggdb -ldl -lpthread
#OPT=-O2 -DNDEBUG -ldl -lpthread
EXECUTABLE=hard

$(EXECUTABLE): $(objectFiles)
	g++ $(objectFiles) $(OPT) `pkg-config --cflags --libs ${SYSTEM_LIBS}` -o $(EXECUTABLE)

.PHONY: clean
clean:
	rm -f $(EXECUTABLE)
