#include "styxe/styxe.hpp"
#include <cstdlib>  // EXIT_SUCCESS/EXIT_FAILURE


int main() {
    return (styxe::headerSize() > 0 &&
            styxe::headerSize() < styxe::kMaxMessageSize)
                ? EXIT_SUCCESS
                : EXIT_FAILURE;
}
