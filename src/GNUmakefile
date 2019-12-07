PREFIX ?= ..
LIBDIR=$(PREFIX)/lib
SYSTEM_LIBS=glfw3
OPT=-O2 -DNDEBUG -ldl -lpthread
objectFiles:=$(wildcard $(LIBDIR)/*.o)

EXECUTABLE=dr

$(EXECUTABLE): $(objectFiles)
	g++ $(objectFiles) $(OPT) `pkg-config --cflags --libs ${SYSTEM_LIBS}` -o $(EXECUTABLE)
