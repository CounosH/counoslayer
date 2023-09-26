#include <counoscore/version.h>

#include <clientversion.h>
#include <tinyformat.h>

#include <string>

#ifdef HAVE_BUILD_INFO
#include <obj/build.h>
#endif

#ifdef COUNOSCORE_VERSION_STATUS
#    define COUNOSCORE_VERSION_SUFFIX STRINGIZE(COUNOSCORE_VERSION_STATUS)
#else
#    define COUNOSCORE_VERSION_SUFFIX ""
#endif

//! Name of client reported in the user aagent message.
const std::string COUNOS_CLIENT_NAME("Counos");

//! Returns formatted Counos Core version, e.g. "1.2.0" or "1.3.4.1"
const std::string CounosCoreVersion()
{
    if (COUNOSCORE_VERSION_BUILD) {
        return strprintf("%d.%d.%d.%d",
                COUNOSCORE_VERSION_MAJOR,
                COUNOSCORE_VERSION_MINOR,
                COUNOSCORE_VERSION_PATCH,
                COUNOSCORE_VERSION_BUILD);
    } else {
        return strprintf("%d.%d.%d",
                COUNOSCORE_VERSION_MAJOR,
                COUNOSCORE_VERSION_MINOR,
                COUNOSCORE_VERSION_PATCH);
    }
}

//! Returns formatted CounosH Core version, e.g. "0.10", "0.9.3"
const std::string CounosHCoreVersion()
{
    if (CLIENT_VERSION_BUILD) {
        return strprintf("%d.%d.%d.%d",
                CLIENT_VERSION_MAJOR,
                CLIENT_VERSION_MINOR,
                CLIENT_VERSION_REVISION,
                CLIENT_VERSION_BUILD);
    } else {
        return strprintf("%d.%d.%d",
                CLIENT_VERSION_MAJOR,
                CLIENT_VERSION_MINOR,
                CLIENT_VERSION_REVISION);
    }
}
