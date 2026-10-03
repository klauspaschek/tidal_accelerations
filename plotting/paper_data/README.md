# Classification of the objects for Figures 5-7

These files contain the classification of the objects shown in Figures 5-7 of [Paschek et al. (2021)](https://doi.org/10.3390/app11188624) by what is known about their volcanism and subsurface oceans, together with their radii and the values of {A<sub>total</sub>·ΔA<sub>total</sub>/Δt}<sub>max</sub> as published in the paper. The classification is based on a review of the scientific literature, see references [3,14–34] of the paper:

| Column | Classification |
| ------ | ------ |
| `siliceous` | siliceous volcanism known |
| `former siliceous` | at least former siliceous volcanism known |
| `cryo` | non-siliceous cryovolcanism/subsurface ocean known or strongly assumed |
| `former cryo` | at least former non-siliceous cryovolcanism/subsurface ocean known or strongly assumed |
| `uncertain cryo` | potential candidates for a reevaluation of tidal based volcanism |
| (none) | new candidates for tidal based volcanism (Figure 5) or no volcanism known |

| File | Figure | Content |
| ------ | ------ | ------ |
| `fig5_volcanism_ranking.csv` | 5 | objects in the order of the paper: first the 27 objects with known volcanism/subsurface ocean, then the new candidates (within both groups, the figure is sorted by the current values) |
| `fig6_objects_full.csv` | 6 | objects of the simulation with full orbital elements and the planets of TRAPPIST-1, with radius `R` (km) and eccentricity `e`; the column `class` gives the type of object (in German) |
| `fig7_objects_simplified.csv` | 7 | the same for the simulation with simplified orbital elements |

`paper_figures.py` uses these files only for the classification, the radii, and the selection of objects; the values of {A<sub>total</sub>·ΔA<sub>total</sub>/Δt}<sub>max</sub> are calculated from the simulation results and compared to the published ones given here.

An extended classification of the volcanism in the solar system, including the respective references, was compiled later in Table 1 (and Figure 1, which also includes TRAPPIST-1) of Hildenbrand, G., Paschek, K., Schäfer, M., & Hausmann, M. (2022), Cryovolcanism in the Solar System and beyond: Considerations on Energy Sources, Geological Aspects, and Astrobiological Perspectives, IntechOpen, [doi:10.5772/intechopen.105067](https://doi.org/10.5772/intechopen.105067). It is more recent and therefore not identical to the classification used in the paper and in these files.
