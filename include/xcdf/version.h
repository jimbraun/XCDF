#ifndef XCDF_VERSION_H
#define XCDF_VERSION_H
/**
 * Header defining the API for getting version information for libxcdf
 */

#include <string>

namespace xcdf {
/**
 * Get the full version string (as output by git describe)
 */
std::string get_version();

/**
 * Get the full hash of the latest commit
 */
std::string get_git_hash();

/**
 * Get the major version
 */
int get_major_version();

/**
 * Get the minor version
 */
int get_minor_version();

/**
 * Get the patch version
 */
int get_patch_version();
}  // namespace xcdf

#endif  // XCDF_VERSION_H