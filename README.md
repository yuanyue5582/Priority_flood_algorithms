## Project Overview

This project implements multiple depression-filling algorithms for Digital Elevation Models (DEMs) used in hydrological analysis. The following algorithms are included:

- **Wang & Liu (2006)** – `FillDEM_Wang`
- **Barnes et al. (2014)** – `FillDEM_Barnes`
- **Wei et al. (2019)** – `fillDEM` (in `fillDEM_Wei.cpp`)
- **Zhou et al. variants**:
  - One‑pass implementation – `FillDEM_Zhou_OnePass`
  - Two‑pass implementation – `FillDEM_Zhou_TwoPass`
  - Direct implementation – `FillDEM_Zhou_Direct`
- **Planchon & Darboux (2002) (P&D)** – `FillDEM_PD` (newly added)

All algorithms work with **floating‑point GeoTIFF** DEMs and rely on the **GDAL library** for raster I/O.

## Dependencies

- **GDAL** (version ≥ 1.9.1 is recommended; tested with 3.7.0)
- C++11 standard library (iostream, fstream, queue, algorithm, chrono, etc.)

## Compilation

1.Using Visual Studio 2022 (Windows)

- Create a new empty C++ console project.
- Add all `.cpp` and `.h` files to the project.
- Configure project properties:
  - **VC++ Directories** → **Include Directories**: add GDAL's `include` path.
  - **VC++ Directories** → **Library Directories**: add GDAL's `lib` path.
  - **Linker** → **Input** → **Additional Dependencies**: add `gdal_i.lib` (or the appropriate library name).
- Build as Release/x64.

2.Using WSL Ubuntu
- Run: wsl --install --web-download -d Ubuntu-24.04
- restart the computer
- Install dependencies in WSL Ubuntu:
 - sudo apt update
 - sudo apt install -y build-essential cmake cmake-gui git
 - sudo apt install -y libgdal-dev gdal-bin
 - gdalinfo --version
 - pkg-config --modversion gdal
 - sudo apt install -y libtiff-dev libgeotiff-dev
- Prepare project files:Create project directory (assuming the project is named DEM_Filling)
 - cd ~
 - mkdir DEM_Filling
 - cd DEM_Filling
- Copy all your .cpp and .h 、CMakeLists.txt files into this directory
- File paths need to be modified to Linux path format：std::string filename = "/mnt/d/GIS_Data/aktin.tif";std::string outputFilename = "/mnt/d/GIS_Data/pp1.tif";
- Configure the CMake project
  - mkdir build
  - cd build
  - cmake ..
  - make -j$(nproc)
  - ls -la bin/
  - cd bin
  - ./DEM_Filling


## Running the Program

The program reads a DEM file, fills depressions using the selected algorithm, and writes the result to a new GeoTIFF file. The input file path, output file path, and algorithm choice are set directly in `main.cpp`.

### Parameters in `main.cpp`

- `filename` : path to the input DEM (GeoTIFF, float32).
- `outputFilename` : path for the output filled DEM.
- `m` : integer selecting the algorithm:
  - `1` – Zhou one‑pass
  - `2` – Wang & Liu (2006)
  - `3` – Barnes et al. (2014)
  - `4` – Zhou two‑pass
  - `5` – Wei et al. (2019)
  - `6` – Planchon & Darboux (2002) (P&D)
  - any other value – Zhou direct

Example:

```cpp
std::string filename = "D:\\GIS_Data\\aktin1.tif";
std::string outputFilename = "D:\\GIS_Data\\dem_filled.tif";
int m = 3;   // use Barnes algorithm
```

After compilation, run the executable. Progress messages are printed to the console.

### Output

The output is a GeoTIFF file containing the depression‑filled DEM. Statistics (minimum, maximum, mean, standard deviation) are calculated and stored as metadata. No‑data value is set to `-9999.0`.

## File Descriptions

| File                         | Description                                                                                  |
|------------------------------|----------------------------------------------------------------------------------------------|
| `dem.h` / `dem.cpp`          | `CDEM` class – manages DEM memory, basic operations (get/set value, no‑data checks).         |
| `Node.h`                     | `Node` structure – stores row, column, and elevation, used in priority queues and queues.    |
| `utils.h` / `utils.cpp`      | Utility functions: GeoTIFF I/O, statistics, neighbour indexing, flag management (`Flag`).    |
| `FillDEM_Barnes.cpp`         | Implementation of the Barnes et al. (2014) algorithm.                                         |
| `FillDEM_Wang.cpp`           | Implementation of the Wang & Liu (2006) algorithm.                                           |
| `fillDEM_Wei.cpp`            | Implementation of the Wei et al. (2019) algorithm (function `fillDEM`).                      |
| `FillDEM_Zhou_OnePass.cpp`   | One‑pass variant of the Zhou algorithm.                                                      |
| `FillDEM_Zhou-Direct.cpp`    | Direct variant of the Zhou algorithm.                                                        |
| `FillDEM_Zhou-TwoPass.cpp`   | Two‑pass variant of the Zhou algorithm.                                                      |
| `FillDEM_PD.cpp`             | Implementation of the Planchon & Darboux (2002) algorithm (newly added).                     |
| `main.cpp`                   | Program entry point – selects algorithm based on variable `m` and calls the corresponding function. |
| `README.md`                  | This documentation file.                                                                     |

## Tested Environment

- **Operating System**: Windows 11 Pro‌ (64‑bit)
- **Compiler**: Microsoft Visual Studio 2022 (version 17.x)
- **GDAL version**: 3.7.0

## Notes

- The P&D algorithm is now included (see `FillDEM_PD.cpp`). It follows the iterative approach described in Planchon & Darboux (2002).
- The original `README.md` mentioned a P&D algorithm that was previously missing; this has been corrected.
- For large DEMs, the priority‑queue based algorithms (Wang, Barnes, Zhou variants) are generally more efficient than the iterative P&D method.
- If you encounter issues with GDAL linking, ensure that the GDAL development package is properly installed and that the compiler can find its headers and libraries.
