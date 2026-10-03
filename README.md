#  tidal accelerations

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.23111591.svg)](https://doi.org/10.5281/zenodo.23111591)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

This code calculates the tidal accelerations that the objects of a planetary system exert on each other over time, using only a minimal set of physical and orbital parameters. It was developed to estimate the potential for tidal heating, and with it for (cryo-)volcanism and liquid water as a solvent for life, in planetary systems where too little is known for a full tidal heating model (Love numbers, tidal dissipation, interior structure). The solar system, where volcanism and subsurface oceans are known for many objects, serves as the "gold standard" to calibrate the results, which can then be transferred to extrasolar planetary systems such as TRAPPIST-1. Simulation in **C++** (using the header-only linear algebra library **Eigen**), plotting in **Python3**, and a **Makefile** for easy building.

[Klaus Paschek](https://klauspaschek.com) [![ORCID](https://info.orcid.org/wp-content/uploads/2019/11/orcid_16x16.png)](https://orcid.org/0000-0003-2603-4236), [Kirchhoff-Institute for Physics](https://www.kip.uni-heidelberg.de/), Heidelberg University, Heidelberg, Germany
(now at the [Max Planck Institute for Astronomy](https://www.mpia.de/institute/staff/113334), Heidelberg, Germany)

Setup and data of the extrasolar planetary system TRAPPIST-1 by Arthur Roßmann, Kirchhoff-Institute for Physics, Heidelberg University.

**Publication based on this code:**

- Paschek, K., Roßmann, A., Hausmann, M., & Hildenbrand, G. (2021). Analysis of Tidal Accelerations in the Solar System and in Extrasolar Planetary Systems. *Applied Sciences*, 11(18), 8624. [doi:10.3390/app11188624](https://doi.org/10.3390/app11188624)

**Related publication:**

- Hildenbrand, G., Paschek, K., Schäfer, M., & Hausmann, M. (2022). Cryovolcanism in the Solar System and beyond: Considerations on Energy Sources, Geological Aspects, and Astrobiological Perspectives. In *Astronomy and Planetary Science - From Cryovolcanism to Black Holes and Galactic Evolution*. IntechOpen. [doi:10.5772/intechopen.105067](https://doi.org/10.5772/intechopen.105067)

> **Version 2.0 corrects the errors of the code used for the paper**, most importantly a wrong conversion from eccentric to true anomaly, with which every object passed through its orbit twice per orbital period. As a consequence, most values of A<sub>total</sub>·ΔA<sub>total</sub>/Δt are about a factor of 2 lower than published, while **the ranking of the objects, and with it the main conclusions of the paper, are unchanged** (see [Changes of the results compared to the paper](#changes-of-the-results-compared-to-the-paper)). To reproduce the results of the paper, use [version 1.0](https://doi.org/10.5281/zenodo.23111592). See [`CHANGELOG.md`](CHANGELOG.md) for all changes.

## Table of Contents
1. [Method](#method)
2. [Repository structure](#repository-structure)
3. [Installation](#installation)
4. [Simulations of the paper](#simulations-of-the-paper)
5. [Changes of the results compared to the paper](#changes-of-the-results-compared-to-the-paper)
6. [Extrasolar planetary systems](#extrasolar-planetary-systems)
7. [Input data](#input-data)
8. [Output](#output)
9. [Plots](#plots)
10. [Citation](#citation)
11. [License](#license)

## Method
**Orbits.** Every object is described by its Keplerian orbital elements (longitude of the ascending node Ω, argument of periapsis ω, mean anomaly M, semi-major axis a, eccentricity e, inclination i) at a reference epoch, together with their linear rates of change, e.g., the mean motion n. At each time step the elements are evolved linearly in time, Kepler's equation is solved for the eccentric anomaly with the Newton–Raphson method (with the mean anomaly reduced to [−π, π] and a start value converging for all eccentricities < 1), and the position in the orbital plane is rotated into heliocentric ecliptic cartesian coordinates. The central star sits fixed at the origin. Moons are propagated relative to their host planet (or host trans-Neptunian object). Their orbits may be given with respect to the ecliptic, the Laplace plane, or the equatorial plane of the host; in the latter two cases the orbit is rotated via the International Celestial Reference Frame (ICRF), whose fundamental plane is the Earth's mean equator of J2000.0, into the ecliptic.

**Tidal acceleration.** The tidal acceleration exerted by a body of mass $M$ at distance $r$ on the point of an object of radius $R$ closest to that body is the difference between the gravitational acceleration at that surface point and at the object's centre:

$$A = \frac{G M}{r^2}\left[\frac{1}{\left(1 - R/r\right)^2} - 1\right] \approx \frac{2 G M R}{r^3}$$

For each object, the tidal acceleration vectors of all other objects are added up to $A_{total}$, so that tidal influences of several objects can enhance or partially cancel each other. The norm of the vector difference of $A_{total}$ between two consecutive time steps, $\Delta A_{total}$, gives a rough estimate of the deformation of the object. The product $A_{total} \cdot \Delta A_{total}/\Delta t \approx A^2 n \approx 4G^2M^2R^2n/r^6$ is comparable between simulations with different time steps Δt and contains the same variables as viscoelastic tidal heating models; its maximum over time is used to classify the objects. This requires a time step that resolves the orbits (the code uses at most about 18° per time step for the objects of interest), as otherwise ΔA<sub>total</sub> saturates at 2·A<sub>total</sub>. To account for uncertain masses (e.g., of asteroids and TNOs), all values are calculated for the nominal (or mean estimated), the minimal, and the maximal mass.

**Relevant objects.** For each object of interest, the code first runs through the full time span to find the 10 perturbers with the largest tidal acceleration and the 10 perturbers with the largest variation of it. In the second run, the contributions of these objects are written out individually, all remaining objects are summed up as `other`.

**Simplified orbits.** For extrasolar systems often only a few orbital parameters are known. The simplified version of the simulation uses only the semi-major axis a and the mean motion n (e, Ω, i, ω, and M<sub>0</sub> are set to zero). As shown in the paper, this changes the results only slightly in the solar system.

## Repository structure
```
src/                  simulation code shared by all systems (header files)
solar_system/         setup of the solar system (main.cpp) and its input data (data/)
exoplanets/           simulation of extrasolar planetary systems given in data files (main.cpp) and the
                      data of TRAPPIST-1 (data/)
plotting/             Python3 scripts for the bar charts and the figures of the paper
plotting/paper_data/  classification of the objects by known volcanism and subsurface oceans (from literature), Figures 5-7
```
The solar system, with its different types of objects and data sources, is set up in `solar_system/main.cpp`. Extrasolar planetary systems with one star are simulated with `exoplanets/planetary_system` from a single data file each, so new systems need no changes of the code.

## Installation
After cloning this repository, the [gcc](https://gcc.gnu.org/) compiler and **make** are needed. At least a _C++11_-standard compatible version of **gcc** is needed. **gcc** is usually preinstalled on Linux or can be installed (including **make**) using the command:
```
sudo apt install build-essential
```
Further, the header-only linear algebra library [Eigen](https://eigen.tuxfamily.org/) is needed:
```
sudo apt install libeigen3-dev
```
If **Eigen** is installed in a different location than `/usr/include/eigen3`, change the variable `EIGENDIR` in the **Makefile** or give it to **make**, e.g.:
```
make EIGENDIR=./eigen-<your_version>/
```
For plotting, one needs **Python3** (usually already preinstalled on Linux) together with the libraries `numpy` and `matplotlib`, e.g., installed using **pip3**:
```
pip3 install -r requirements.txt
```

**Windows:** the code is developed for Linux. On Windows, the easiest way is to use the [Windows Subsystem for Linux (WSL)](https://learn.microsoft.com/windows/wsl/install) with, e.g., Ubuntu, where all commands above work unchanged. Alternatively, **gcc** and **make** are available via [MSYS2](https://www.msys2.org/). The input data are text files with Linux line endings; the CSV reader also accepts files with Windows line endings, e.g., tables exported from Excel.

## Simulations of the paper
Compile the code, run all simulations of the paper, and create its figures by simply typing in the directory of the cloned repository:
```
make
make run
make figures
```
`make run` takes about ten minutes on a current computer and runs:

1. The solar system with full orbital elements (`solar_system/results/full/`), in two parts:
   - from 1950 Jan. 01, 12:00 UT, with a time step of 90 days for 2400 iterations (≈ 590 years), for all asteroids and TNOs with a diameter ≥ 150 km, the comets, the moons of TNOs, the terrestrial planets, and Pluto (files `delta_t_90d_...`),
   - from 1999 May 05, 12:00 UT, for 600 days, for all moons with a radius ≥ 75 km, the terrestrial planets, Pluto, Eris, and Haumea (files `delta_t_0_0125d_...`). The time step of 0.0125 days resolves the orbits of the fastest moons (Metis: orbital period of 0.295 days, 15° per time step; the paper used 0.25 days, which did not resolve the orbits of several inner moons); every 20th time step is written to the output files.
2. The same with simplified orbital elements (`solar_system/results/simplified/`).
3. TRAPPIST-1 with the setup of the paper: from epoch J2000.0 with a time step of 0.05 days for 1800 iterations (90 days) (`exoplanets/results/trappist-1/`).

`make figures` creates Figures 1-10 and Table 1 of the paper with the current results in `figures/`. The classification of the objects in Figures 5-7 by what is known about their volcanism and subsurface oceans is based on the scientific literature cited in the paper (see [`plotting/paper_data/README.md`](plotting/paper_data/README.md)). The script also compares all values of {A<sub>total</sub>·ΔA<sub>total</sub>/Δt}<sub>max</sub> with the published ones (`figures/comparison_with_paper.csv`). The dashed line in Figures 6 and 7, which separates the regions of mainly tidal and mainly radioactive heating, was determined empirically and drawn by hand in the paper; here it is drawn at the same position, shifted by the factor of 2 by which most values are lower than in the paper, and should be reevaluated with the current results.

The simulations of the solar system can also be run individually, from their directory:
```
cd solar_system
./solar_system                 # full orbital elements
./solar_system --simplified    # only semi-major axis and mean motion
```

## Changes of the results compared to the paper
The corrections of version 2.0 (see [`CHANGELOG.md`](CHANGELOG.md)) change the values of {A<sub>total</sub>·ΔA<sub>total</sub>/Δt}<sub>max</sub>, but **not the main conclusions of the paper**. All values below are from `figures/comparison_with_paper.csv`, created by `make figures`.

**Systematic factor of about 2.** With the wrong conversion from eccentric to true anomaly, every object passed through its orbit twice per orbital period, i.e., with twice the angular velocity, so A<sub>total</sub>·ΔA<sub>total</sub>/Δt was about a factor of 2 too high. With the correct orbits, almost all values are lower by this factor (median ratio of current to published values 0.52; e.g., (J1) Io 2.43 → 1.41 × 10<sup>-4</sup>, Europa 6.31 → 3.32 × 10<sup>-6</sup>, Titan 1.11 → 0.57 × 10<sup>-8</sup>, TRAPPIST-1b 9.36 → 4.74 × 10<sup>-3</sup> m<sup>2</sup>/(s<sup>4</sup> d)). As this factor is the same for nearly all objects, the empirical thresholds derived from them (such as the boundary between mainly tidal and mainly radioactive heating in Figures 6 and 7) move down by the same factor, while the relative positions of the objects are preserved.

**The "gold standard" still holds.** The approach of the paper is to use the objects of the solar system with known (cryo-)volcanism and subsurface oceans as a reference, and to rate objects not yet considered in the solar system, as well as extrasolar planets, against them. This relies on the relative order of the objects, which is almost unchanged:
- **Figure 5, objects with known volcanism/subsurface ocean:** of the 27 objects, only Enceladus and Tethys swap places (ranks 3 and 4; their values differ by only 10 %, 9.3 and 8.4 × 10<sup>-6</sup>). All other objects keep their rank. Venus (× 2.2) and Mars (× 2.5) increase, but keep their ranks: for Venus, the paper showed only the maximum of the 90-day run instead of the maximum of both runs, and for Mars, the orbit of Phobos is now resolved in time (see below).
- **Figure 5, new candidates for tidal based volcanism:** the inner moons of Jupiter, Saturn, Uranus, and Neptune move faster than the time step of 0.25 days used for the paper could resolve (118-305° per time step), so their ΔA<sub>total</sub> was underestimated. With the time step of 0.0125 days used now, their values increase: Amalthea × 48 (rank 7 → 3, now 1.64 × 10<sup>-4</sup> m<sup>2</sup>/(s<sup>4</sup> d), slightly above (J1) Io), Larissa × 4.7 (rank 3 → 4), Galatea × 3.7, Despina × 2.4 (still the highest value of all objects of the solar system). Proteus (× 0.71) drops from rank 4 to 7, and Nereid and Hi'iaka swap places (ranks 10 and 11). These moons are therefore even stronger candidates than in the paper. Only Sycorax, an irregular moon of Uranus at the bottom of the ranking, drops strongly (× 0.004), probably because its orbit is highly eccentric and inclined (retrograde), so that the corrected orbital motion changes its distances to Uranus strongly; with 4.4 × 10<sup>-22</sup> m<sup>2</sup>/(s<sup>4</sup> d) it is far from being tidally active in either case.
- **Table 1, TRAPPIST-1 compared with (J1) Io and Ganymede:** the planets of TRAPPIST-1, Io, and Ganymede all change by about the same factor (0.50-0.58), so the order is exactly as in the paper: TRAPPIST-1b and c above Io, TRAPPIST-1d, e, and f between Io and Ganymede, and TRAPPIST-1g and h below Ganymede. The comparison of TRAPPIST-1 with the solar system therefore holds unchanged.
- **Figure 6:** apart from the changes above, only objects with values of about 10<sup>-20</sup> m<sup>2</sup>/(s<sup>4</sup> d), far below all objects with known volcanism, change their order (the asteroids (1) Ceres, (2) Pallas, (3) Juno, (4) Vesta, and (7) Iris, and the comets 1P/Halley and Hale-Bopp, see below).
- **Figure 7, simplified orbits:** the results with the simplified orbits (only semi-major axis and mean motion) still differ only slightly from those with the full orbits, as stated in the paper, except for co-orbital moons: with all mean anomalies at epoch set to zero, they have (almost) the same position. In the paper, this enhanced the values of Tethys and Dione (by their Trojan moons) and Janus (by Epimetheus). Overlapping bodies are now excluded (see [Output](#output)), so Tethys and Dione agree with the full orbits, while Janus gets the highest value of all objects (4.2 × 10<sup>-3</sup> m<sup>2</sup>/(s<sup>4</sup> d)) from a close approach to Epimetheus, an artefact of the simplified orbits (marked in Figure 7).

**Encounters, e.g., the comet Hale-Bopp.** In the paper, Figure 4 is discussed as a close flyby of Hale-Bopp at Jupiter; these peaks of the tidal acceleration (around the years 2454 and 2461) are artefacts of the wrong orbit. With the correct orbit, the simulation reproduces the real close approach of Hale-Bopp to Jupiter on 1996 April 5 (0.77 au) as well as its perihelion on 1997 April 1 (0.91 au), so Figure 4 now shows this time window. During the encounter, the tidal acceleration exerted by Jupiter reaches about 15 % of that exerted by the Sun, and the maximum of A<sub>total</sub>·ΔA<sub>total</sub>/Δt is reached at perihelion, dominated by the Sun. The encounter with Jupiter is therefore much less significant than in the paper. Still, close encounters of objects on eccentric orbits with massive bodies can in principle lead to strong short-term changes of the tidal acceleration, and thus at least to temporary tidal heating events.

**Conclusion.** The corrections lower the values by about a factor of 2, but the relative order of the objects, on which the classification of the paper is based, is preserved for the objects with known volcanism and subsurface oceans, and the comparison of TRAPPIST-1 with (J1) Io and Ganymede is unchanged. The fast inner moons, now resolved in time, become even stronger candidates for tidal based volcanism. **The main conclusions of the paper are therefore unchanged**, while the absolute values and empirical thresholds given in the paper should be taken from version 2.0, i.e., about a factor of 2 lower.

## Extrasolar planetary systems
Each extrasolar planetary system is given by one data file in `exoplanets/data/` with the star and the planets (see [`exoplanets/data/README.md`](exoplanets/data/README.md) for the format and the sources). Simulate a system with, e.g.:
```
cd exoplanets
mkdir -p results/trappist-1
./planetary_system data/trappist-1.csv
```
By default, the time step is 1/20 of the shortest orbital period and the simulated time span 10 times the longest orbital period; both can be set with `--time-step <days>` and `--duration <days>`, and `--simplified` uses the simplified orbital elements. `make run-exoplanets` simulates all systems in `exoplanets/data/`. Further systems can be added as data files in the same format.

## Input data
The data of the objects are given as **csv** files with the column names in the first row, in `solar_system/data/` and `exoplanets/data/`. See the README files there for the sources, units, and how the masses of objects without known mass were estimated.

## Output
For each object of interest, one **csv** file named `<prefix>_tidal_acc_<designation>_<name>.csv` is written, where the prefix gives the time step (e.g., `delta_t_0_0125d`). Each row is one written time step (for long simulations only every n-th time step is written, while ΔA<sub>total</sub> always refers to one time step), and all values are absolute values of the tidal acceleration in m/s<sup>2</sup>:

| Column | Content |
| ------ | ------ |
| `julian_date` | time in julian date |
| `<designation>_<name>`, `min_mass`, `max_mass` | tidal acceleration exerted by each relevant object for its nominal, minimal, and maximal mass |
| `other`, `min_mass`, `max_mass` | summed tidal acceleration exerted by all remaining objects |
| `total`, `min_mass`, `max_mass` | total tidal acceleration A<sub>total</sub> exerted by all objects |
| `total_change`, `min_mass`, `max_mass` | change ΔA<sub>total</sub> of the total tidal acceleration since the previous time step |

In addition, `<prefix>_maxima.csv` gives for each object its radius and {A<sub>total</sub>·ΔA<sub>total</sub>/Δt}<sub>max</sub> in m<sup>2</sup>/(s<sup>4</sup> d) for the nominal, minimal, and maximal mass, determined from all time steps, and the julian date of the maximum.

If two bodies overlap (distance smaller than the sum of their radii), the tidal acceleration is not defined and set to zero, and a warning is given. This happens in the simplified version of the solar system for the co-orbital moons Janus and Epimetheus, which have almost the same orbit and, with all mean anomalies at epoch set to zero, the same position at J2000.0.

## Plots
The **Python3** script `plotting/plot_tidal_acc.py` creates bar charts as in Figures 1-4 and 8-10 of the paper for a result file or all files in a directory:
```
python3 plotting/plot_tidal_acc.py solar_system/results/full/delta_t_0_0125d_tidal_acc_J_3_Ganymede.csv plots/
```
The time span is split into 15 parts (option `--split`), each plotted as a **png** file `plot_<csv_filename>_<number>.png`. Each plot shows the tidal accelerations of the individual perturbers on a logarithmic scale (top), A<sub>total</sub>, ΔA<sub>total</sub>, and A<sub>total</sub>·ΔA<sub>total</sub>/Δt (bottom). Every bar consists of three sub-bars for the minimal, mean, and maximal mass (from left to right).

`make plots` creates the bar charts for all objects of all simulations in `plots/` (about 5000 plots at 300 dpi, which takes one to two hours on a computer with 12 cores).

## Citation
If you use this code, please cite it using the DOI provided by Zenodo: [doi:10.5281/zenodo.23111591](https://doi.org/10.5281/zenodo.23111591) (all versions; version 1.0 with the state of the paper: [doi:10.5281/zenodo.23111592](https://doi.org/10.5281/zenodo.23111592); version 2.0 with the corrected code: [doi:10.5281/zenodo.23122749](https://doi.org/10.5281/zenodo.23122749)), and the paper describing the method: [Paschek et al. (2021)](https://doi.org/10.3390/app11188624). Citation metadata is also provided in [`CITATION.cff`](CITATION.cff) (see "Cite this repository" on GitHub).

## License
This code is licensed under the [MIT License](LICENSE).
