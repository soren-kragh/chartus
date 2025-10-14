# Makefile for Chartus

MAKEFLAGS += -j

CXX       := g++
CXXFLAGS  := -std=c++17 -Wall -Wextra -Wfatal-errors -O3
SRC_DIRS  := src svg
INCLUDES  := $(addprefix -I, $(SRC_DIRS))
INCS      := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))
SRCS      := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
BUILD_DIR := build
OBJS      := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
TARGET    := ./chartus
SCRIPT    := bin/svg2png
PREFIX    ?= /usr/local
BINDIR    := $(PREFIX)/bin

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking $(notdir $(TARGET))..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

$(BUILD_DIR)/%.o: %.cpp $(INCS)
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

examples: $(TARGET)
	@for i in 1 2 3 4 5 6 7 8 9; do \
	  echo "Generating example $${i}..."; \
	  ${TARGET} -e$${i} >e$${i}.txt; \
	  ${TARGET} e$${i}.txt >e$${i}.svg; \
	  ### ${TARGET} -e$${i} | ${TARGET} >e$${i}.svg; \
	done
	@cat `ls -1 e?.svg` | cksum

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
	rm -rf $(BUILD_DIR) $(TARGET)
	rm -f e?.txt e?.svg

.PHONY: all examples install uninstall clean
