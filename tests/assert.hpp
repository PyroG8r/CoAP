#pragma once

#include <cstdlib>
#include <iostream>

#define EXPECT(CONDITION, MESSAGE)                                                   \
    do {                                                                            \
        if (!(CONDITION)) {                                                         \
            std::cerr << "Assertion failed: " << (MESSAGE) << " at " << __FILE__   \
                      << ":" << __LINE__ << "\n";                                 \
            std::exit(EXIT_FAILURE);                                                \
        }                                                                           \
    } while (0)
