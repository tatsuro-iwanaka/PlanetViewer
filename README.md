## PlanetViewer: Planet Observation Planner.

Simulate the disk views of Mercury, Venus, Mars and Jupiter from the Earth.

Range of calculation date: Jan 1, 1970 to Dec 31, 2120

Observation sites:
  - IRTF, Mauna Kea, Hawai
  - T60, Haleakala, Hawai
  - NAOJ, Miatka, Tokyo
  - Tohoku U., Sendai, Miyagi
  - R-CCS, Kobe, Hyogo
  - ALMA, Atacama, Chile
  - Subaru, Mauna Kea, Hawai
  - Sendai Observatory, Sendai, Miyagi

Kernel list is specified in `kernels.tm`. 
Download them from `https://naif.jpl.nasa.gov/pub/naif/generic_kernels/`.

Make `spice_kernel` directory and put these kernels into this directory.

<img width="1589" height="1059" alt="Image" src="https://private-user-images.githubusercontent.com/136674384/562075944-6345194a-6595-4951-a4ca-23c6211c3051.png?jwt=eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3NzMzMDA2NTAsIm5iZiI6MTc3MzMwMDM1MCwicGF0aCI6Ii8xMzY2NzQzODQvNTYyMDc1OTQ0LTYzNDUxOTRhLTY1OTUtNDk1MS1hNGNhLTIzYzYyMTFjMzA1MS5wbmc_WC1BbXotQWxnb3JpdGhtPUFXUzQtSE1BQy1TSEEyNTYmWC1BbXotQ3JlZGVudGlhbD1BS0lBVkNPRFlMU0E1M1BRSzRaQSUyRjIwMjYwMzEyJTJGdXMtZWFzdC0xJTJGczMlMkZhd3M0X3JlcXVlc3QmWC1BbXotRGF0ZT0yMDI2MDMxMlQwNzI1NTBaJlgtQW16LUV4cGlyZXM9MzAwJlgtQW16LVNpZ25hdHVyZT1kMDVjMzEzOTEzNGNkYTM3NzBlN2NjYWMxYWNjMDFmNDk5YjBhNTQwOThiNzJjN2ZjNjJjZmFkNTljN2UwNDA2JlgtQW16LVNpZ25lZEhlYWRlcnM9aG9zdCJ9.OlD7rD7P7eHjdw2yaO3_xVdZ6SOM0XAktBIq4FpzX1g" />

### This software uses SPICE toolkit and kernels
- Acton, C.H.; "Ancillary Data Services of NASA's Navigation and Ancillary Information Facility;" Planetary and Space Science, Vol. 44, No. 1, pp. 65-70, 1996.
DOI 10.1016/0032-0633(95)00107-7
- Charles Acton, Nathaniel Bachman, Boris Semenov, Edward Wright; A look toward the future in the handling of space science mission geometry; Planetary and Space Science (2017);
DOI 10.1016/j.pss.2017.02.013

### This software uses imGUI
