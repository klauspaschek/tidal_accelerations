# directory containing the Eigen headers (the one that contains the folder 'Eigen/')
EIGENDIR = /usr/include/eigen3

CXX      = g++
CXXFLAGS = -std=c++11 -O3 -I$(EIGENDIR) -Isrc
PYTHON   = python3

HEADERS = $(wildcard src/*.h)

.PHONY: all run run-solar-system run-trappist-1 figures plots clean clean-results

# compile the simulations
all: solar_system/solar_system trappist-1/trappist-1

solar_system/solar_system: solar_system/main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $< -o $@

trappist-1/trappist-1: trappist-1/main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $< -o $@

# run all simulations of the paper
run: run-solar-system run-trappist-1

run-solar-system: solar_system/solar_system
	mkdir -p solar_system/results/full solar_system/results/simplified
	cd solar_system && ./solar_system && ./solar_system --simplified

run-trappist-1: trappist-1/trappist-1
	mkdir -p trappist-1/results
	cd trappist-1 && ./trappist-1

# figures 1-10 and table 1 of the paper (needs the results of 'make run')
figures:
	$(PYTHON) plotting/paper_figures.py

# bar charts for all objects and all time windows (takes long)
plots:
	$(PYTHON) plotting/plot_tidal_acc.py solar_system/results/full       plots/solar_system/full
	$(PYTHON) plotting/plot_tidal_acc.py solar_system/results/simplified plots/solar_system/simplified
	$(PYTHON) plotting/plot_tidal_acc.py trappist-1/results              plots/trappist-1 --split 9

clean:
	rm -f solar_system/solar_system trappist-1/trappist-1

clean-results:
	rm -rf solar_system/results trappist-1/results figures plots
