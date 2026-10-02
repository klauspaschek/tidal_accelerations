# Input data: TRAPPIST-1

Data of the seven planets of TRAPPIST-1 as used for [Paschek et al. (2021)](https://doi.org/10.3390/app11188624), compiled by Arthur Roßmann in 2018:

- masses, semi-major axes, and eccentricities: Grimm et al. (2018), *A&A* 613, A68, [doi:10.1051/0004-6361/201732233](https://doi.org/10.1051/0004-6361/201732233)
- mean motions and radii: Delrez et al. (2018), *MNRAS* 475, 3577, [doi:10.1093/mnras/sty051](https://doi.org/10.1093/mnras/sty051)
- inclinations: Gillon et al. (2017), *Nature* 542, 456, [doi:10.1038/nature21360](https://doi.org/10.1038/nature21360), and Delrez et al. (2018)

The star (0.089 solar masses, 0.121 solar radii, from Van Grootel et al. (2018), *ApJ* 853, 30, [doi:10.3847/1538-4357/aaa023](https://doi.org/10.3847/1538-4357/aaa023)) is set in `../main.cpp`.

| Column | Content | Unit |
| ------ | ------ | ------ |
| `pdes`, `Planet` | designation and name | |
| `radius` | radius | km |
| `O`, `w`, `M` | longitude of the ascending node, argument of periapsis, mean anomaly at epoch J2000.0 (unknown, set to zero) | radians |
| `n` | mean motion | radians/day |
| `a` | semi-major axis | au |
| `e` | eccentricity | |
| `i` | inclination | radians |
| `mean_mass`, `min_mass`, `max_mass` | mass and its lower/upper limit from the uncertainties | kg |

The file is identical to the one used for the paper (created on Windows), only the line endings were converted to Linux (LF).
