#  tidal accelerations

[![DOI](https://zenodo.org/badge/DOI/<your_zenodo_doi>.svg)](https://doi.org/<your_zenodo_doi>)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

This code calculates the tidal accelerations that the objects of a planetary system exert on each other over time, using only a minimal set of physical and orbital parameters. It was developed to estimate the potential for tidal heating, and with it for (cryo-)volcanism and liquid water as a solvent for life, in planetary systems where too little is known for a full tidal heating model (Love numbers, tidal dissipation, interior structure). The solar system, where volcanism and subsurface oceans are known for many objects, serves as the "gold standard" to calibrate the results, which can then be transferred to extrasolar planetary systems such as TRAPPIST-1. Simulation in **C++** (using the header-only linear algebra library **Eigen**), plotting in **Python3**, and a **Makefile** for easy building.

[Klaus Paschek](https://klauspaschek.com) [![ORCID](https://info.orcid.org/wp-content/uploads/2019/11/orcid_16x16.png)](https://orcid.org/0000-0003-2603-4236), [Kirchhoff-Institute for Physics](https://www.kip.uni-heidelberg.de/), Heidelberg University, Heidelberg, Germany  
(now at the [Max Planck Institute for Astronomy](https://www.mpia.de/institute/staff/113334), Heidelberg, Germany)

Setup and data of the extrasolar planetary system TRAPPIST-1 by Arthur Roßmann, Kirchhoff-Institute for Physics, Heidelberg University.

**Publication based on this code:**

- Paschek, K., Roßmann, A., Hausmann, M., & Hildenbrand, G. (2021). Analysis of Tidal Accelerations in the Solar System and in Extrasolar Planetary Systems. *Applied Sciences*, 11(18), 8624. [doi:10.3390/app11188624](https://doi.org/10.3390/app11188624)

**Related publication:**

- Hildenbrand, G., Paschek, K., Schäfer, M., & Hausmann, M. (2022). Cryovolcanism in the Solar System and beyond: Considerations on Energy Sources, Geological Aspects, and Astrobiological Perspectives. In *Astronomy and Planetary Science - From Cryovolcanism to Black Holes and Galactic Evolution*. IntechOpen. [doi:10.5772/intechopen.105067](https://doi.org/10.5772/intechopen.105067)

> **Version 1.0 reproduces the paper, including its known errors.** Most importantly, the conversion from eccentric to true anomaly is wrong, so every object passes through its orbit twice per orbital period. See [`CHANGELOG.md`](CHANGELOG.md) for all known errors; they are fixed in the next version.

## Table of Contents
1. [Method](#method)
2. [Repository structure](#repository-structure)
3. [Installation](#installation)
4. [Reproducing the paper](#reproducing-the-paper)
5. [Input data](#input-data)
6. [Output](#output)
7. [Plots](#plots)
8. [Citation](#citation)
9. [License](#license)

## Method
**Orbits.** Every object is described by its Keplerian orbital elements (longitude of the ascending node Ω, argument of periapsis ω, mean anomaly M, semi-major axis a, eccentricity e, inclination i) at a reference epoch, together with their linear rates of change, e.g., the mean motion n. At each time step the elements are evolved linearly in time, Kepler's equation is solved for the eccentric anomaly with the Newton–Raphson method, and the position in the orbital plane is rotated into heliocentric ecliptic cartesian coordinates. The central star sits fixed at the origin. Moons are propagated relative to their host planet (or host trans-Neptunian object). Their orbits may be given with respect to the ecliptic, the Laplace plane, or the equatorial plane of the host; in the latter two cases the orbit is rotated via the International Celestial Reference Frame (ICRF), whose fundamental plane is the Earth's mean equator of J2000.0, into the ecliptic.

**Tidal acceleration.** The tidal acceleration exerted by a body of mass $M$ at distance $r$ on the point of an object of radius $R$ closest to that body is the difference between the gravitational acceleration at that surface point and at the object's centre:

$$A = \frac{G M}{r^2}\left[\frac{1}{\left(1 - R/r\right)^2} - 1\right] \approx \frac{2 G M R}{r^3}$$

For each object, the tidal acceleration vectors of all other objects are added up to $A_{total}$, so that tidal influences of several objects can enhance or partially cancel each other. The norm of the vector difference of $A_{total}$ between two consecutive time steps, $\Delta A_{total}$, gives a rough estimate of the deformation of the object. The product $A_{total} \cdot \Delta A_{total}/\Delta t \approx A^2 n \approx 4G^2M^2R^2n/r^6$ is comparable between simulations with different time steps Δt and contains the same variables as viscoelastic tidal heating models; its maximum over time is used to classify the objects. To account for uncertain masses (e.g., of asteroids and TNOs), all values are calculated for the nominal (or mean estimated), the minimal, and the maximal mass.

**Relevant objects.** For each object of interest, the code first runs through the full time span to find the 10 perturbers with the largest tidal acceleration and the 10 perturbers with the largest variation of it. In the second run, the contributions of these objects are written out individually, all remaining objects are summed up as `other`.

**Simplified orbits.** For extrasolar systems often only a few orbital parameters are known. The simplified version of the simulation uses only the semi-major axis a and the mean motion n (e, Ω, i, ω, and M<sub>0</sub> are set to zero). As shown in the paper, this changes the results only slightly in the solar system.

## Repository structure
```
src/                  simulation code shared by all systems (header files)
solar_system/         setup of the solar system (main.cpp) and its input data (data/)
trappist-1/           setup of TRAPPIST-1 (main.cpp) and its input data (data/)
plotting/             Python3 scripts for the bar charts and the figures of the paper
plotting/paper_data/  classification of the objects by known volcanism and subsurface oceans (from literature), Figures 5-7
```
Each system is a self-contained example: `main.cpp` sets up the objects from the data files and the time steps. To simulate another planetary system, copy `trappist-1/` and adapt the data file and the star in `main.cpp`.

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
For plotting, one needs **Python3** (usually already preinstalled on Linux) together with the libraries `numpy`, `matplotlib`, and `adjustText`, e.g., installed using **pip3**:
```
pip3 install -r requirements.txt
```

**Windows:** the code is developed for Linux. On Windows, the easiest way is to use the [Windows Subsystem for Linux (WSL)](https://learn.microsoft.com/windows/wsl/install) with, e.g., Ubuntu, where all commands above work unchanged. Alternatively, **gcc** and **make** are available via [MSYS2](https://www.msys2.org/). The input data are text files with Linux line endings; the CSV reader also accepts files with Windows line endings, e.g., tables exported from Excel.

## Reproducing the paper
Compile the code, run all simulations of the paper, and create its figures by simply typing in the directory of the cloned repository:
```
make
make run
make figures
```
`make run` takes about two minutes on a current computer and runs:

1. The solar system with full orbital elements (`solar_system/results/full/`), in two parts:
   - from 1950 Jan. 01, 12:00 UT, with a time step of 90 days for 2400 iterations (≈ 590 years), for all asteroids and TNOs with a diameter ≥ 150 km, the comets, the moons of TNOs, the terrestrial planets, and Pluto (files `delta_t_90d_...`),
   - from 1999 May 05, 12:00 UT, with a time step of 0.25 days for 2400 iterations (600 days), for all moons with a radius ≥ 75 km, the terrestrial planets, Pluto, Eris, and Haumea (files `delta_t_0_25d_...`).
2. The same with simplified orbital elements (`solar_system/results/simplified/`).
3. TRAPPIST-1 from epoch J2000.0 with a time step of 0.05 days for 1800 iterations (90 days) (`trappist-1/results/`).

`make figures` creates Figures 1-10 and Table 1 of the paper in `figures/`. The classification of the objects in Figures 5-7 by what is known about their volcanism and subsurface oceans is based on the scientific literature cited in the paper (see [`plotting/paper_data/README.md`](plotting/paper_data/README.md)). The script also compares all reproduced values of {A<sub>total</sub>·ΔA<sub>total</sub>/Δt}<sub>max</sub> with the published ones (`figures/reproduction_check.csv`). The dashed line in Figures 6 and 7, which separates the regions of mainly tidal and mainly radioactive heating, was determined empirically and drawn by hand in the paper; here it is drawn at the same position.

The simulations can also be run individually, from their directory:
```
cd solar_system
./solar_system                 # full orbital elements
./solar_system --simplified    # only semi-major axis and mean motion
```

## Input data
The data of the objects are given as **csv** files with the column names in the first row, in `solar_system/data/` and `trappist-1/data/`. See the README files there for the sources, units, and how the masses of objects without known mass were estimated.

## Output
For each object of interest, one **csv** file named `<prefix>_tidal_acc_<designation>_<name>.csv` is written, where the prefix gives the time step (e.g., `delta_t_0_25d`). Each row is one time step, and all values are absolute values of the tidal acceleration in m/s<sup>2</sup>:

| Column | Content |
| ------ | ------ |
| `julian_date` | time in julian date |
| `<designation>_<name>`, `min_mass`, `max_mass` | tidal acceleration exerted by each relevant object for its nominal, minimal, and maximal mass |
| `other`, `min_mass`, `max_mass` | summed tidal acceleration exerted by all remaining objects |
| `total`, `min_mass`, `max_mass` | total tidal acceleration A<sub>total</sub> exerted by all objects |
| `total_change`, `min_mass`, `max_mass` | change ΔA<sub>total</sub> of the total tidal acceleration since the previous time step |

## Plots
The **Python3** script `plotting/plot_tidal_acc.py` creates bar charts as in Figures 1-4 and 8-10 of the paper for a result file or all files in a directory:
```
python3 plotting/plot_tidal_acc.py solar_system/results/full/delta_t_0_25d_tidal_acc_J_3_Ganymede.csv plots/
```
The time span is split into 15 parts (option `--split`), each plotted as a **png** file `plot_<csv_filename>_<number>.png`. Each plot shows the tidal accelerations of the individual perturbers on a logarithmic scale (top), A<sub>total</sub>, ΔA<sub>total</sub>, and A<sub>total</sub>·ΔA<sub>total</sub>/Δt (bottom). Every bar consists of three sub-bars for the minimal, mean, and maximal mass (from left to right).

`make plots` creates the bar charts for all objects of all simulations in `plots/` (about 5000 plots at 300 dpi, which takes one to two hours on a computer with 12 cores).

## Citation
If you use this code, please cite it using the DOI provided by Zenodo: [doi:<your_zenodo_doi>](https://doi.org/<your_zenodo_doi>), and the paper describing the method: [Paschek et al. (2021)](https://doi.org/10.3390/app11188624). Citation metadata is also provided in [`CITATION.cff`](CITATION.cff) (see "Cite this repository" on GitHub).

## License
This code is licensed under the [MIT License](LICENSE).
