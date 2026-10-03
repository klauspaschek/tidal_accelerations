# directory containing the Eigen headers (the one that contains the folder 'Eigen/')
EIGENDIR = /usr/include/eigen3

CXX      = g++
CXXFLAGS = -std=c++11 -O3 -I$(EIGENDIR) -Isrc
PYTHON   = python3

HEADERS   = $(wildcard src/*.h)
EXOPLANET_SYSTEMS = $(basename $(notdir $(wildcard exoplanets/data/*.csv)))

.PHONY: all run run-solar-system run-trappist-1 run-exoplanets figures plots clean clean-results

# compile the simulations
all: solar_system/solar_system exoplanets/planetary_system

solar_system/solar_system: solar_system/main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $< -o $@

exoplanets/planetary_system: exoplanets/main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $< -o $@

# run all simulations of the paper
run: run-solar-system run-trappist-1

run-solar-system: solar_system/solar_system
	mkdir -p solar_system/results/full solar_system/results/simplified
	cd solar_system && ./solar_system && ./solar_system --simplified

# TRAPPIST-1 with the setup of the paper (time step of 0.05 days for 90 days)
run-trappist-1: exoplanets/planetary_system
	mkdir -p exoplanets/results/trappist-1
	cd exoplanets && ./planetary_system data/trappist-1.csv --time-step 0.05 --duration 90

# all extrasolar planetary systems in exoplanets/data with automatic time step and duration
run-exoplanets: exoplanets/planetary_system
	for system in $(EXOPLANET_SYSTEMS); do \
		mkdir -p exoplanets/results/$$system && (cd exoplanets && ./planetary_system data/$$system.csv) || exit 1; \
	done

# figures 1-10 and table 1 of the paper (needs the results of 'make run')
figures:
	$(PYTHON) plotting/paper_figures.py

# bar charts for all objects and all time windows (takes long)
plots:
	$(PYTHON) plotting/plot_tidal_acc.py solar_system/results/full       plots/solar_system/full
	$(PYTHON) plotting/plot_tidal_acc.py solar_system/results/simplified plots/solar_system/simplified
	for system in $$(ls exoplanets/results); do \
		$(PYTHON) plotting/plot_tidal_acc.py exoplanets/results/$$system plots/exoplanets/$$system --split 9 || exit 1; \
	done

clean:
	rm -f solar_system/solar_system exoplanets/planetary_system

clean-results:
	rm -rf solar_system/results exoplanets/results figures plots
