# CITS3003 Project

Graphics and Animations Project

## Project Documentation

This project has been built using Ubuntu 22.4 and the included standard CMake compilers.

## Cmake

The following are commands for cmake

### Generating and Building Files

Please be sure to ONLY build the debug profile when developing. The release profile is not necessary until submission.

#### Generate Debug Build Files

```bash
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
```

#### Generate Release Build Files

```bash
cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
```

#### Build Debug Profile

```bash
cmake --build cmake-build-debug
```

#### Build Release Profile

```bash
cmake --build cmake-build-release
```

#### Run Latest Build

```bash
./cits3003_project
```
