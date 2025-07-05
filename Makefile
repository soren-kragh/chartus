# Makefile for Chartus

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O3 -Wfatal-errors
SRC_DIR  := src
SVG_DIR  := svg
INCLUDES := -I$(SRC_DIR) -I$(SVG_DIR)
SRCS     := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SVG_DIR)/*.cpp)
INCS     := $(wildcard $(SRC_DIR)/*.h) $(wildcard $(SVG_DIR)/*.h)
TARGET   := chartus
SCRIPT   := bin/svg2png
PREFIX   ?= /usr/local
BINDIR   := $(PREFIX)/bin

all: $(TARGET)

$(TARGET): $(SRCS) $(INCS)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(TARGET)

install: $(TARGET) $(SCRIPT)
	install -d $(BINDIR)
	install -m755 $(TARGET) $(BINDIR)/$(notdir $(TARGET))
	install -m755 $(SCRIPT) $(BINDIR)/$(notdir $(SCRIPT))
	@command -v rsvg-convert >/dev/null || \
		{ echo "Warning: 'rsvg-convert' not found; svg2png will not work until you install it."; }

uninstall:
	rm -f $(BINDIR)/$(notdir $(TARGET))
	rm -f $(BINDIR)/$(notdir $(SCRIPT))

clean:
	rm -f $(TARGET)

.PHONY: all clean install uninstall
