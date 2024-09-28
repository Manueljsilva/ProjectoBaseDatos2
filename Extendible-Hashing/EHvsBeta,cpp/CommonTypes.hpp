#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <string>

// Define the missing types based on assumed appropriate data types.
using position_t = long;        // Assuming a type suitable for file positions.
using bucketSize_t = size_t;    // Assuming size_t for bucket sizes.
using filename_t = std::string; // Assuming filename is represented as a string.
using depth_t = int;            // Assuming an integer type for depth.
using suffix_t = unsigned int;  // Assuming an unsigned integer for suffixes.

#endif // COMMON_TYPES_H
