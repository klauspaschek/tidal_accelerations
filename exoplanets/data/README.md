# Input data: extrasolar planetary systems

One **csv** file per planetary system with one star, read by `../planetary_system`. The first line gives the column names, the second line the units; then one line per object. Empty entries are allowed for the columns marked as optional.

| Column | Content | Unit | |
| ------ | ------ | ------ | ------ |
| `type` | `star` (exactly one) or `planet` | | |
| `name` | name of the object | | |
| `mass` | mass (mean value) | M<sub>⊕</sub> (star: M<sub>☉</sub>) | |
| `mass_min`, `mass_max` | minimal and maximal mass, e.g., from the uncertainties | M<sub>⊕</sub> | optional, default: `mass` |
| `radius` | radius | R<sub>⊕</sub> (star: R<sub>☉</sub>) | optional for the star |
| `period` | orbital period | days | optional if `a` is given |
| `a` | semi-major axis | au | optional if `period` is given |
| `e` | eccentricity | | optional, default: 0 |
| `i` | inclination | deg | optional, default: 0 |
| `omega` | argument of periapsis ω | deg | optional, default: 0 |
| `Omega` | longitude of the ascending node Ω | deg | optional, default: 0 |
| `M0` | mean anomaly at epoch J2000.0 | deg | optional, default: 0 |
| `reference` | sources of the values and notes | | optional |

If only one of `period` and `a` is given, the other is calculated from Kepler's third law. The constants used are M<sub>⊕</sub> = 5.9722 × 10<sup>24</sup> kg, M<sub>☉</sub> = 1.9884 × 10<sup>30</sup> kg, R<sub>⊕</sub> = 6378.137 km (equatorial), and R<sub>☉</sub> = 695700 km.

**Orientation of the orbits.** For transiting planets, the inclination is measured against the plane of the sky, which is used as reference plane. The longitude of the ascending node Ω is usually unknown and set to zero for all planets, i.e., all orbits are assumed to share the same line of nodes, so that the mutual inclinations are the differences of the inclinations. The mean anomalies at J2000.0 determine the relative orbital phases of the planets; if they are not known, all planets start in conjunction (M<sub>0</sub> = 0, as in the paper).

## Systems

| File | System | Source |
| ------ | ------ | ------ |
| `trappist-1.csv` | TRAPPIST-1 | data used for [Paschek et al. (2021)](https://doi.org/10.3390/app11188624), compiled by Arthur Roßmann: star from Van Grootel et al. (2018), *ApJ* 853, 30, [doi:10.3847/1538-4357/aaa023](https://doi.org/10.3847/1538-4357/aaa023); masses, semi-major axes, and eccentricities from Grimm et al. (2018), *A&A* 613, A68, [doi:10.1051/0004-6361/201732233](https://doi.org/10.1051/0004-6361/201732233); mean motions (here converted to orbital periods) and radii from Delrez et al. (2018), *MNRAS* 475, 3577, [doi:10.1093/mnras/sty051](https://doi.org/10.1093/mnras/sty051); inclinations from Gillon et al. (2017), *Nature* 542, 456, [doi:10.1038/nature21360](https://doi.org/10.1038/nature21360), and Delrez et al. (2018). ω, Ω, and M<sub>0</sub> are unknown and set to zero. |

## New systems

Further systems can be added as data files in the format described above; please document the sources of the values in the column `reference`.
