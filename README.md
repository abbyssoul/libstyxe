# libstyxe [![C++ standard][c++-standard-shield]][c++-standard-link] [![License][license-shield]][license-link]
---
[![TravisCI][travis-shield]][travis-link]
[![Codecov][codecov-shield]][codecov-link]
[![Coverity][coverity-shield]][coverity-link]
[![Coverage Status][coveralls-shield]][coveralls-link]
[![LGTM][LGTM-shield]][LGTM-link]


[c++-standard-shield]: https://img.shields.io/badge/c%2B%2B-14/17/20-blue.svg
[c++-standard-link]: https://en.wikipedia.org/wiki/C%2B%2B#Standardization
[license-shield]: https://img.shields.io/badge/License-Apache%202.0-blue.svg
[license-link]: https://opensource.org/licenses/Apache-2.0
[travis-shield]: https://travis-ci.org/abbyssoul/libstyxe.png?branch=master
[travis-link]: https://travis-ci.org/abbyssoul/libstyxe
[codecov-shield]: https://codecov.io/gh/abbyssoul/libstyxe/branch/master/graph/badge.svg
[codecov-link]: https://codecov.io/gh/abbyssoul/libstyxe
[coverity-shield]: https://scan.coverity.com/projects/18800/badge.svg
[coverity-link]: https://scan.coverity.com/projects/abbyssoul-libstyxe
[coveralls-shield]: https://coveralls.io/repos/github/abbyssoul/libstyxe/badge.svg?branch=master
[coveralls-link]: https://coveralls.io/github/abbyssoul/libstyxe?branch=master
[LGTM-shield]: https://img.shields.io/lgtm/grade/cpp/github/abbyssoul/libstyxe.svg
[LGTM-link]: https://lgtm.com/projects/g/abbyssoul/libstyxe/alerts/


A _library_ for parsing 9P2000 protocol messages.
> library: a collection of types, functions, classes, etc. implementing a set of facilities (abstractions) meant to be potentially used as part of more that one program. From [Cpp Code guidelines gloassay](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#glossary)

It also includes 9P2000.e extension. To learn more about the extension please see http://erlangonxen.org/more/9p2000e

# Using this library

### To create 9P message:
The library is using `Solace::ByteWriter` / `Solace::ByteReader` to read/write byte streams.
Note that this adaptors do not allocate memory. So the user is responsible for creating a
buffer of appropriate size to write the resulting message to.
Note that the size of the target buffer should be no more then negotiated message size for the current session.


```C++
#include <styxe/styxe.hpp>

...
Solace::ByteWriter byteWriter{...};

// Write TVersion request message into the output buffer
styxe::RequestWriter requestWriter{byteWriter, 1};
requestWriter << Request::Version{parser.maxMessageSize(), _9P2000U::kProtocolVersion}
...

// Write TOpen request into the given destination buffer
styxe::RequestWriter requestWriter{buffer, 1};
requestWriter << Open{42, styxe::OpenMode::READ)};
```

### Parsing 9P message from a byte buffer:
Parsing of 9P protocol messages differ slightly depending on if you are implementing server - expecting request type messages - or a client - parsing server responses.

### Parsing requests (server side):
```C++
styxe::Parser parser{...};
...
Solace::ByteReader byteReader{...};
auto maybeHeader = parser.parseMessageHeader{byteReader};
if (!maybeHeader) {
    LOG() << "Failed to parse message header";
    return maybeHeader.getError();
}

auto maybeMessage = parser.parseRequest(*maybeHeader, byteReader);
if (!maybeMessage) {
    LOG() << "Failed to parse message";
    return maybeMessage.getError();
}

handleRequest(*maybeMessage);
...
```

Alternatively you can prefer fluent interface:
```c++
styxe::Parser parser{...};
...
Solace::ByteReader byteReader{...};
parser.parseMessageHeader(byteReader)
    .then([&](styxe::MessageHeader header) {
        return parser.parseRequest(header, byteReader)
            .then(handleRequest);
    })
    .orElse([](Error&& err) {
        std::cerr << "Error parsing request: " << err << std::endl;
    });
```

### Client side: parsing responses from a server
```c++
styxe::Parser parser{...};
...
parser.parseMessageHeader(byteReader)
    .then([&](styxe::MessageHeader header) {
        return parser.parseResponse(header, byteReader)
            .then(handleRequest);
    })
    .orElse([](Error&& err) {
        std::cerr << "Error parsing response: " << err << std::endl;
    });
```

See [examples](docs/examples.md) for other example usage of this library.

# Using the library from your project.
This library needs to be installed on your system in order to be used. There are a few ways this can be done:
 - You can install the pre-built version via [Conan](https://conan.io/) package manager. (Recommended)
 - You can build it from sources and install it locally.
 - You can install a pre-built version via your system package manager such as deb/apt if it is available in your system repository.

## Consuming library with Conan
The library is available via [Conan](https://conan.io/) package manager. Add this to your project `conanfile.txt`:
```
[requires]
libstyxe/0.6
```

Please check the latest available [binary version][conan-central-latest].


## Dependencies
This library depends on [libsolace](https://github.com/abbyssoul/libsolace) for low level data manipulation primitives
such as ByteReader/ByteWriter and Result<> type.
Since it is only a 9P protocol parser - there is dependency on the IO. It is library users responsibility to provide data stream.

### GTest
Note test framework used is *gtest* and it is managed via git modules.
Don't forget to do `git submodule update --init --recursive` on a new checkout to pull sub-module dependencies.



# Building

### Build tool dependencies
In order to build this project following tools must be present in the system:
* git (to check out project and it’s external modules, see dependencies section)
* cmake - user for build script generation
* ninja (opional, used by default)
* doxygen (opional, for documentation generation)
* cppcheck (opional, but recommended for static code analysis, latest version from git is used as part of the 'codecheck' step)
* cpplint (opional, for static code analysis in addition to cppcheck)
* valgrind (opional, for runtime code quality verification)

This project is using C++17 features extensively. The minimal tested/required version of gcc is gcc-7.
[CI](https://travis-ci.org/abbyssoul/libstyxe) is using clang-6 and gcc-7.
To install build tools on Debian based Linux distribution:
```shell
sudo apt-get update -qq
sudo apt-get install git doxygen python-pip valgrind ggcov
sudo pip install cpplint
```

The library has one external dependency: [libsolace](https://github.com/abbyssoul/libsolace)  which is managed via conan.
Please make sure [conan is installed](https://docs.conan.io/en/latest/installation.html) on your system if you want to build this project.

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
# Note: `valgrind` doesn’t work with ./configure --enable-sanitize option
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


## Contributing changes
The framework is work in progress and contributions are very welcomed.
Please see  [`CONTRIBUTING.md`](CONTRIBUTING.md) for details on how to contribute to
this project.

Please note that in order to maintain code quality a set of static code analysis tools is used as part of the build process.
Thus all contributions must be verified by this tools before PR can be accepted.


## License
The library available under Apache License 2.0
Please see [`LICENSE`](LICENSE) for details.


## Authors
Please see [`AUTHORS`](AUTHORS) file for the list of contributors.
