libstyxe
==================
[![TravisCI][travis-shield]][travis-link]
[![License][license-shield]][license-link]

[travis-shield]: https://travis-ci.org/abbyssoul/libstyxe.png?branch=master
[travis-link]: https://travis-ci.org/abbyssoul/libstyxe
[license-shield]: https://img.shields.io/badge/License-Apache%202.0-blue.svg
[license-link]: https://opensource.org/licenses/Apache-2.0

An implementation of 9P2000 protocol.

It also includes 9P2000.e extention. To learn more about the extention please see http://erlangonxen.org/more/9p2000e


## Dependencies
Please note that this library depends on [libsolace](https://github.com/abbyssoul/libsolace).


## Contributing changes
The framework is work in progress and contributions are very welcomed.
Please see  [`CONTRIBUTING.md`](CONTRIBUTING.md) for details on how to contribute to
this project.

Please note that in order to maintain code quality a set of static code analysis tools is used as part of the build process.
Thus all contributions must be verifyed by this tools before PR can be accepted.


# Building

### Build tool dependencies
In order to build this project following tools must be present in the system:
* git (to check out project and it’s external modules, see dependencies section)
* doxygen (for documentation)
* cppcheck (static code analysis, latest version from git is used as part of the 'codecheck' step)
* cpplint (for static code analysis in addition to cppcheck)
* valgrind (for runtime code quality verification)

This project is using C++14 features extensively. The minimal tested/required version of gcc is gcc-4.9.
CI is using clang-5 and gcc-7.
To install build tools on Debian based Linux distribution:
```shell
sudo apt-get update -qq
sudo apt-get install git doxygen python-pip valgrind ggcov
sudo pip install cpplint
```

The library has one external dependency: [libsolace](https://github.com/abbyssoul/libsolace).
Please make sure it is installed and avaliable in the build path before building the project.

## Building the project
```shell
# In the project check-out directory:
# To build debug version with sanitizer enabled (recommended for development)
./configure --enable-debug --enable-sanitizer

# To build the library it self
make

# To build and run unit tests:
make test

# To run valgrind on test suit:
# Please note – doesn’t work with ./configure --enable-sanitize option
make verify

# To build API documentation using doxygen:
make doc
```

To install locally for testing:
```shell
make --prefix=/user/home/<username>/test/lib install
```
To install system wide (as root):
```shell
make install
```
To run code quality check before submission of a patch:
```shell
# Verify code quality before submission
make codecheck
```


## License
Please see LICENSE file for details


## Authors
Please see AUTHORS file for the list of contributors

