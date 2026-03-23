## PlanetViewer: Planet Observation Planner

Simulate the disk views of Mercury, Venus, Mars, Jupiter and Saturn from the Earth.

<p>
  <img width="1584" height="1024" alt="Image" src="https://github.com/user-attachments/assets/2a6e73d8-5266-47ef-b558-0e6b8c59927c" /> <img width="1585" height="1023" alt="Image" src="https://github.com/user-attachments/assets/bb4a63e7-0b86-453b-b78b-a9f7f2f962cf" /> <img width="1586" height="1023" alt="Image" src="https://github.com/user-attachments/assets/936d9142-c696-45f7-bca5-71a4c0a941bb" />
</p>

#### For Mac users
Compiled for Apple Silicon and MacOS > 11.0  
Download from [Google Drive](https://drive.google.com/file/d/1zmyvnSbiS8W9kSEtoAxOJdEL8b5NWmsa/view?usp=sharing "Planet View")  
and allow to excute by `xattr -cr PlanetViewer.app`

### Range of calculation date
Jan 1, 1970 to Dec 31, 2120

### Observation sites:
  - IRTF, Mauna Kea, Hawai
  - T60, Haleakala, Hawai
  - NAOJ, Miatka, Tokyo
  - Tohoku U., Sendai, Miyagi
  - R-CCS, Kobe, Hyogo
  - ALMA, Atacama, Chile
  - Subaru, Mauna Kea, Hawai
  - Sendai Observatory, Sendai, Miyagi

### Observation Targets
  - Mercury
  - Venus
  - Mars
  - Jupiter
  - Saturn
  - Jupiter's Galilean Moons

### Ver 1.0

### Ver 2.0
  - Fixed crash when simulation time is out of range
  - Added Jupiter's Galilean Moons
  - Added Saturn and its rings
  - Added Altitude-time graph
  - Added Martian Solar Longitude, Ls
  - Added Sub-Solar and Sub-Earth longitude/latitude

### Ver 2.1
  - Displaying Planets considering flattening

Kernel list is specified in `kernels.tm`. 
Download them from `https://naif.jpl.nasa.gov/pub/naif/generic_kernels/`.

Make `spice_kernel` directory and put these kernels into this directory.

### This software uses SPICE toolkit and kernels
- Acton, C.H.; "Ancillary Data Services of NASA's Navigation and Ancillary Information Facility;" Planetary and Space Science, Vol. 44, No. 1, pp. 65-70, 1996.
DOI 10.1016/0032-0633(95)00107-7
- Charles Acton, Nathaniel Bachman, Boris Semenov, Edward Wright; A look toward the future in the handling of space science mission geometry; Planetary and Space Science (2017);
DOI 10.1016/j.pss.2017.02.013

### This software uses imGUI
