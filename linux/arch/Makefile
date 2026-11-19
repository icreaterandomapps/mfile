CXX ?= g++
CXXFLAGS ?= -std=c++11 -O2 -Wall -Wextra -Wno-unused-parameter
PKG_CONFIG ?= pkg-config
GTK_CFLAGS := $(shell $(PKG_CONFIG) --cflags gtk+-3.0 gio-2.0)
GTK_LIBS := $(shell $(PKG_CONFIG) --libs gtk+-3.0 gio-2.0)

TARGET := mfile
SRC := mfile.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(GTK_CFLAGS) $(GTK_LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
