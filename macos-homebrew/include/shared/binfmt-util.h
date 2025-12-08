/* SPDX-License-Identifier: LGPL-2.1-or-later */
#pragma once

#ifndef HAS_FEATURE_MEMORY_SANITIZER
#define HAS_FEATURE_MEMORY_SANITIZER 0
#endif

#include "log.h"

int disable_binfmt(void);
