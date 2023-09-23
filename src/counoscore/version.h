#ifndef COUNOSH_COUNOSCORE_VERSION_H
#define COUNOSH_COUNOSCORE_VERSION_H

#if defined(HAVE_CONFIG_H)
#include <config/counosh-config.h>
#else

//
// Counos Core version information are also to be defined in configure.ac.
//
// During the configuration, this information are used for other places.
//

// Increase with every consensus affecting change
#define COUNOSCORE_VERSION_MAJOR       0

// Increase with every non-consensus affecting feature
#define COUNOSCORE_VERSION_MINOR       10

// Increase with every patch, which is not a feature or consensus affecting
#define COUNOSCORE_VERSION_PATCH       0

// Non-public build number/revision (usually zero)
#define COUNOSCORE_VERSION_BUILD       0

#endif // HAVE_CONFIG_H

#if !defined(WINDRES_PREPROC)

//
// *-res.rc includes this file, but it cannot cope with real c++ code.
// WINDRES_PREPROC is defined to indicate that its pre-processor is running.
// Anything other than a define should be guarded below:
//

#include <string>

//! Counos Core client version
static const int COUNOSCORE_VERSION =
                    +100000000000 * COUNOSCORE_VERSION_MAJOR
                    +    10000000 * COUNOSCORE_VERSION_MINOR
                    +        1000 * COUNOSCORE_VERSION_PATCH
                    +           1 * COUNOSCORE_VERSION_BUILD;

static const int COUNOS_USERAGENT_VERSION =
                           1000000 * COUNOSCORE_VERSION_MAJOR
                         +   10000 * COUNOSCORE_VERSION_MINOR
                         +     100 * COUNOSCORE_VERSION_PATCH
                         +       1 * COUNOSCORE_VERSION_BUILD;

extern const std::string COUNOS_CLIENT_NAME;

//! Returns formatted Counos Core version, e.g. "1.2.0"
const std::string CounosCoreVersion();

//! Returns formatted CounosH Core version, e.g. "0.10", "0.9.3"
const std::string CounosHCoreVersion();


#endif // WINDRES_PREPROC

#endif // COUNOSH_COUNOSCORE_VERSION_H
