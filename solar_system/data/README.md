# Input data: solar system

Data as used for [Paschek et al. (2021)](https://doi.org/10.3390/app11188624), collected in 2017/2018. The content is identical to the files used for the paper, only the line endings were converted to Linux (LF) (see [Notes](#notes)).

| File | Objects | Header lines | Source |
| ------ | ------ | ------ | ------ |
| `planets_orbital_elements_physical.csv` | 8 planets and Pluto | 1 (units) | JPL [Solar System Bodies](https://ssd.jpl.nasa.gov/): [approximate positions of the major planets](https://ssd.jpl.nasa.gov/planets/approx_pos.html) (epoch J2000.0, rates per century) and planetary physical parameters |
| `moons_orbital_elements_physical.csv` | moons of the planets and of Pluto | 1 (units) | JPL [Solar System Bodies](https://ssd.jpl.nasa.gov/): [planetary satellite mean elements](https://ssd.jpl.nasa.gov/sats/elem/) and physical parameters |
| `asteroids_r_greater_75km_or_GM_given_with_mass_with_guessed_mass.csv` | asteroids with radius > 75 km or known mass | 0 | [JPL Small-Body Database Search Engine](https://ssd.jpl.nasa.gov/tools/sbdb_query.html) |
| `comets_with_mass_with_guessed_mass.csv` | comets (only the 16 comets with a known or estimated mass are used) | 0 | [JPL Small-Body Database Search Engine](https://ssd.jpl.nasa.gov/tools/sbdb_query.html) |
| `TNOs_supplemented_with_guessed_mass.csv` | trans-Neptunian objects | 0 | JPL Small-Body Database, supplemented by the publications listed in Table A1 of the paper |
| `moons_TNOs_v2.csv` | moons of Eris and Haumea | 1 (units) | Table A1 of the paper |

The mass and radius of the Sun (1.9884 × 10<sup>30</sup> kg, 696342 km) are taken from The Astronomical Almanac for the Year 2012 and are set in `../main.cpp`.

**Estimated ("guessed") masses:** for asteroids, comets, and TNOs without a known mass, the mean density of the objects with known mass and radius is calculated (assuming a spherical shape). The mean density gives the mean mass estimate (`*_mean_guess`) from the radius, and the mean density minus/plus its standard deviation gives the minimal/maximal mass estimate (`*_min_guess`, `*_max_guess`). If a mass is known, it is used as mean, minimal, and maximal mass.

**Units:** angles in degrees, mean motion `n` in degrees/day, semi-major axis `a` in au (moons: km), masses in kg, radii and diameters in km, epochs in julian date; the precession periods `Pnode` and `Pw` of the moons are given in years.

## Notes

**Epochs of the moons of TNOs.** Due to the CSV reader used for the paper, the epochs of Dysnomia, Namaka, and Hi'iaka in `moons_TNOs_v2.csv` were read without their last digit (245154 and 245461 instead of 2451545 and 2454615). Version 1.0 reproduces this with the values as effectively read; since version 2.0, the correct values are given and used.

**Moons with a precession period of zero.** The moons Nix, Kerberos, and S/2004 N 1 have a precession period (`Pw`, `Pnode`) of `0.000`, meaning no precession. Version 1.0 interpreted this as an infinite rate of precession, so that these moons did not contribute to the results; since version 2.0 they are treated as not precessing.
