#ifndef COUNOSH_COUNOSCORE_RPCMBSTRING_H
#define COUNOSH_COUNOSCORE_RPCMBSTRING_H

#include <stdint.h>
#include <string>

namespace mastercore
{
/** Replaces invalid UTF-8 characters or character sequences with question marks. */
std::string SanitizeInvalidUTF8(const std::string& s);
}

#endif // COUNOSH_COUNOSCORE_RPCMBSTRING_H
