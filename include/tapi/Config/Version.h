//===- tapi/Config/Version.h - TAPI Version Number --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines version macros and version-related utility functions for
/// tapi.
///
//===----------------------------------------------------------------------===//

#ifndef TAPI_CONFIG_VERSION_H
#define TAPI_CONFIG_VERSION_H

#include "tapi/Defines.h"
#include <string>

TAPI_NAMESPACE_INTERNAL_BEGIN

/// \brief Retrieves a string representing the full tapi name and version.
std::string getTAPIFullVersion();

TAPI_NAMESPACE_INTERNAL_END

#endif // TAPI_CONFIG_VERSION_H
