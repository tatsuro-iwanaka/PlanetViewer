## PlanetViewer: Planet Observation Planner.

Simulate the disk views of Mercury, Venus, Mars and Jupiter from the Earth.

Range of calculation date: Jan 1, 1970 to Dec 31, 2120

Observation sites:
  - IRTF, Mauna Kea
  - T60, Haleakala
  - NAOJ, Miatka
  - Tohoku U., Sendai
  - R-CCS, Kobe
  - ALMA, Chile

Kernel list is specified in `kernels.tm`. 
Download them from `https://naif.jpl.nasa.gov/pub/naif/generic_kernels/`.

Make `spice_kernel` directory and put these kernels into this directory.

<img width="1589" height="1059" alt="Image" src="https://github.com/user-attachments/assets/c1767e16-6c9c-4532-879c-d8c9565c783c" />

### This software uses SPICE toolkit and kernels
- Acton, C.H.; "Ancillary Data Services of NASA's Navigation and Ancillary Information Facility;" Planetary and Space Science, Vol. 44, No. 1, pp. 65-70, 1996.
DOI 10.1016/0032-0633(95)00107-7
- Charles Acton, Nathaniel Bachman, Boris Semenov, Edward Wright; A look toward the future in the handling of space science mission geometry; Planetary and Space Science (2017);
DOI 10.1016/j.pss.2017.02.013
