/*
 *   Unicode types for native system APIs (generic narrow/UTF-8 version)
 *
 */

#pragma once

#include <string>

using UNICODE_STRING = std::string;
using UNICODE_LITERAL = const char *;
#define _UNICODE(str) str
