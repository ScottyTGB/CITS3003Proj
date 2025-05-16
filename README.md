# CITS3003 Project

Graphics and Animations Project

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

## Project Documentation

TBD
