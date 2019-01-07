libstyxe
==================
[![TravisCI][travis-shield]][travis-link]
[![License][license-shield]][license-link]

[travis-shield]: https://travis-ci.org/abbyssoul/libstyxe.png?branch=master
[travis-link]: https://travis-ci.org/abbyssoul/libstyxe
[license-shield]: https://img.shields.io/badge/License-Apache%202.0-blue.svg
[license-link]: https://opensource.org/licenses/Apache-2.0

An implementation of 9P2000 protocol.

It also includes 9P2000.e extension. To learn more about the extension please see http://erlangonxen.org/more/9p2000e

# Using this library

### To create 9P message:
The library is using `Solace::WriteBuffer` / `Solace::ReadBuffer` to read/write byte streams.
Note that this adaptors do not allocate memory. So the user is responsible for creating a
buffer of appropriate size to write the resulting message to.
Note that the size of the target buffer should be no more then negotiated message size for the current session.


```
#include "styxe/9p2000.hpp"

...
Solace::WriteBuffer buffer(...);

// Write TVersion request into the beffer
styxe::Protocol::RequestBuilder(destBuffer)
            .version();
...
// Write TOpen request into the given destination buffer
styxe::Protocol::RequestBuilder(destBuffer)
            .open(42, styxe::Protocol::OpenMode::READ));

```

### Parsing 9P message from a byte buffer:
Parsing of 9P protocol messages differ slightly depending on if you are implementing server - expecting request type messages - or a client - parsing server responses.

### Parsing requests (server side):
```

styxe::Protocol proc(...);
...
proc.parseMessageHeader(buffer)
    .then([&](Protocol::MessageHeader&& header) {
        return proc.parseRequest(header, buffer)
            .then(handleRequest);
    })
    .orElse([](Error&& err) {
        std::cerr << "Error parsing request: " << err.toString() << std::endl;
    });
```
### Parsing response (client side):
```
styxe::Protocol proc(...);
...
proc.parseMessageHeader(buffer)
    .then([&](Protocol::MessageHeader&& header) {
        return proc.parseResponse(header, buffer)
            .then(handleRequest);
    })
    .orElse([](Error&& err) {
        std::cerr << "Error parsing response: " << err.toString() << std::endl;
    });
```

See [examples](docs/examples.md) for other example usage of this library.


## Dependencies
This library depends on [libsolace](https://github.com/abbyssoul/libsolace) for low level data manipulation primitives
such as ByteReader/ByteWriter and Result<> type.
Since it is only a 9P protocol parser - there is dependency on the IO. It is library users responsibility to provide data stream.

### GTest
Note test framework used is *gtest* and it is managed via git modules.
Don't forget to do `git submodule update --init --recursive` on a new checkout to pull sub-module dependencies.


## Contributing changes
The framework is work in progress and contributions are very welcomed.
Please see  [`CONTRIBUTING.md`](CONTRIBUTING.md) for details on how to contribute to
this project.

Please note that in order to maintain code quality a set of static code analysis tools is used as part of the build process.
Thus all contributions must be verified by this tools before PR can be accepted.


# Building

### Build tool dependencies
In order to build this project following tools must be present in the system:
* git (to check out project and it’s external modules, see dependencies section)
* doxygen (for documentation)
* cppcheck (static code analysis, latest version from git is used as part of the 'codecheck' step)
* cpplint (for static code analysis in addition to cppcheck)
* valgrind (for runtime code quality verification)

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


## License
Please see LICENSE file for details


## Authors
Please see AUTHORS file for the list of contributors
