# Building MLIO

* [Runtime Requirements](#Runtime-Requirements)
* [Build Requirements](#Build-Requirements)
* [Installing System Dependencies](#Installing-System-Dependencies)
* [Building the Library](#Building-the-Library)
* [Build Options](#Build-Options)
* [Installing the Library](#Installing-the-Library)
* [Building the Python Package](#Building-the-Python-Package)
* [Building the Apache Arrow Integration](#Building-the-Apache-Arrow-Integration)
* [Development Guidelines](#Development-Guidelines)
    * [Compiler Warnings](#Compiler-Warnings)
    * [Sanitizers](#Sanitizers)
    * [Static Analyzer](#Static-Analyzer)
    * [Code Style and Formatting](#Code-Style-and-Formatting)

## Runtime Requirements
* On Linux libc 2.12 or higher
* Intel TBB 2019.0 or higher
* Python 3.6 or higher
* AWS C++ SDK 1.7 or higher (optional)
* Apache Arrow 14.0.1 (optional)

## Build Requirements
* A C++17-enabled compiler. On Linux gcc 7.0 or higher; on macOS clang 6 or higher should be sufficient.
* CMake 3.13 or higher
* Ninja build
* Doxygen 1.8 or higher (optional)

## Installing System Dependencies
The build instructions for Linux are based on Ubuntu 18.04. In case you are using a different distribution, please perform the equivalent steps.

Official Ubuntu APT repositories do not contain an up-to-date version of CMake required to build MLIO. [Follow the instructions](https://apt.kitware.com) on Kitware to add a repository that contains the latest version of CMake.

On Ubuntu 18.04 you can install the build dependencies with:
```bash
$ sudo -- sh -c 'apt update && apt install -qy build-essential autoconf libtool cmake ninja-build doxygen git python3-dev python3-distutils python3-pip zlib1g-dev libssl-dev libcurl4-openssl-dev'
```

On macOS you can use Homebrew:
```bash
$ brew update && brew install automake libtool llvm cmake ninja doxygen git
```

## Building the Library
Run the following commands to pull and build the MLIO C++ library.

### Linux build commands

```bash
$ git clone https://github.com/awslabs/ml-io.git mlio
$ cd mlio
$ build-tools/build-dependency build/third-party all
$ mkdir build/release
$ cd build/release
$ cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH="$(pwd)/../third-party" ../..
$ cmake --build .
```

### Mac build commands
The Clang fork that is distributed as part of Xcode is not compatible with MLIO and causes segmentation faults when used with certain C++17 language features. It is recommended to use the vanilla LLVM that is installed via Homebrew; this version is usually found in ```/usr/local/opt/llvm/bin/clang``` after installation.

```bash
$ export CC=/usr/local/opt/llvm/bin/clang
$ export CXX=/usr/local/opt/llvm/bin/clang++
$ git clone https://github.com/awslabs/ml-io.git mlio
$ cd mlio
$ build-tools/build-dependency build/third-party all
$ mkdir build/release
$ cd build/release
$ cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH="$(pwd)/../third-party" ../..
$ cmake --build .
```

## Build Options
| Name                               | Description                                                          | Default |
|------------------------------------|----------------------------------------------------------------------|---------|
| MLIO_INCLUDE_LIB                   | Generates build target 'mlio' for the runtime library                | ON      |
| MLIO_INCLUDE_CONTRIB               | Generates build target 'mlio-contrib' for the contrib library        | OFF     |
| MLIO_INCLUDE_PYTHON_EXTENSION      | Generates build target 'mlio-py' for the Python C extension          | OFF     |
| MLIO_INCLUDE_ARROW_INTEGRATION     | Generates build target 'mlio-arrow' for the Apache Arrow integration | OFF     |
| MLIO_INCLUDE_TESTS                 | Generates build target 'mlio-test' for the tests                     | ON      |
| MLIO_INCLUDE_DOC                   | Generates build target 'mlio-doc' for the documentation              | OFF     |
| MLIO_BUILD_S3                      | Builds with Amazon S3 support                                        | OFF     |
| MLIO_BUILD_IMAGE_READER            | Builds with image reader support                                     | OFF     |
| MLIO_BUILD_FOR_NATIVE_ARCHITECTURE | Builds for the processor type of the compiling machine               | OFF     |
| MLIO_TREAT_WARNINGS_AS_ERRORS      | Treats compilation warnings as errors                                | OFF     |
| MLIO_ENABLE_LTO                    | Enables link time optimization                                       | ON      |
| MLIO_ENABLE_ASAN                   | Enables address sanitizer                                            | OFF     |
| MLIO_ENABLE_UBSAN                  | Enables undefined behavior sanitizer                                 | OFF     |
| MLIO_ENABLE_TSAN                   | Enables thread sanitizer                                             | OFF     |
| MLIO_USE_CLANG_TIDY                | Uses clang-tidy as static analyzer (supported only with clang)       | OFF     |
| MLIO_USE_IWYU                      | Uses include-what-you-use (supported only with clang)                | OFF     |

To specify a different value for one of the options listed above, you can call cmake like:

```bash
$ cmake -Doption_name=[ON|OFF] ...
$ cmake --build .
```

In case you want to have a debug build replace `-DCMAKE_BUILD_TYPE=RelWithDebInfo` with `-DCMAKE_BUILD_TYPE=Debug`.

Using [Ninja](https://ninja-build.org) instead of make usually results in faster builds. To use it pass `-GNinja` to cmake:

```bash
$ cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH="$(pwd)/../third-party" ../..
$ cmake --build .
```

## Installing the Library
Once you have successfully built the project, you can install it via cmake:

```bash
$ sudo cmake --build . --target install
```

By default cmake will install MLIO system wide. If you want to have it installed under a specific directory (or if you don't have sudo access), you can tell cmake the desired install location:

```bash
$ cmake -DCMAKE_INSTALL_PREFIX=/custom/path ../..
$ cmake --build . --target install
```

## Building the Python Package
MLIO Python package uses an extension module that needs to be build first. Pass the `MLIO_INCLUDE_PYTHON_EXTENSION` option to cmake:

```bash
$ cmake -DMLIO_INCLUDE_PYTHON_EXTENSION=ON ../..
$ cmake --build . --target mlio-py
```

The command above will build the extension module for the system-provided Python. In case you want to build the extension module against a specific Python version you have two options:
* Either run cmake under a Python virtual environment (or Conda environment) with the desired Python version.
* or explicitly specify the path to the Python interpreter in cmake:

```bash
$ cmake -DMLIO_INCLUDE_PYTHON_EXTENSION=ON -DPYTHON_EXECUTABLE=/path/to/python ../..
$ cmake --build . --target mlio-py
```

Once you have the extension module in place, you can easily generate a distribution via `setup.py`:

```bash
$ cd <repo_root>/src/mlio-py
$ python setup.py bdist_wheel
```

Note that the generated wheel won't contain the MLIO runtime library or any of its dependencies though.

If you want to do development in Python, an easy way to avoid repeated installs everytime you edit a Python file is to have an editable (a.k.a. develop mode) install:

```bash
$ cd src/mlio-py
$ pip install -e .
```

With this mode changes made in Python files will be immediately reflected when the `mlio` package gets imported.

## Building the Apache Arrow Integration
Please refer to Arrow's official install instructions [here](https://arrow.apache.org/install/) first. As Arrow does not guarantee API compatibility (yet) you have to make sure that your environment has the right version. As of today MLIO works with Arrow v14.0.1. Once you have it installed, turn on the `MLIO_INCLUDE_ARROW_INTEGRATION` flag as follows:

```bash
$ conda install pyarrow=14.0.1
$ cmake DMLIO_INCLUDE_PYTHON_EXTENSION=ON -DMLIO_INCLUDE_ARROW_INTEGRATION=ON ../..
$ cmake --build . --target mlio-py
$ cmake --build . --target mlio-arrow
```

> Arrow integration requires building the Python C extension as well. This might change in the future.

## Development Guidelines
If you wish to contribute to the C++ codebase, there are a couple steps you have to perform before sending your PR.

### Compiler Warnings
Your code must build successfully with the `-DMLIO_TREAT_WARNINGS_AS_ERRORS=ON` cmake option. Make sure to test your changes with both gcc and clang and, if possible, on both Linux and macOS.

Explicitly suppressing compiler warnings via pragma directives is not allowed.

### Sanitizers
All your unit and/or integration tests must pass successfully when you build your code with address (ASan) and undefined behavior (USan) sanitizers enabled. 

### Static Analyzer
Before submitting a PR you must analyze your code with `clang-tidy`. Any false positives reported by clang-tidy (which is rare) must be suppressed with a `NOLINT` inline comment and must include an explanation why it is suppressed.

### Code Style and Formatting
MLIO has its own style guidelines formally defined in the .clang-format file at the root of the project. Before submitting a PR you must format your code using `clang-format`.
