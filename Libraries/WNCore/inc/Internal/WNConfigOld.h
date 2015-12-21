// Copyright (c) 2015, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#pragma once

#ifndef __WN_CORE_INTERNAL_CONFIG_OLD_H__
#define __WN_CORE_INTERNAL_CONFIG_OLD_H__

#ifndef __WN_CORE_CONFIG_OLD_H__
    #error "Internal/WNConfig.h should never be included directly."
#endif

#ifndef _WN_CORE_CONFIG_DISABLE_EXTENSIONS
    #include "WNCore/inc/WNBase.h"

    #define __WN_CORE_EXTENSIONS_ENABLED

    #ifdef _WN_X86
        #include "WNCore/inc/Internal/x86/WNConfig.h"
    #elif defined _WN_ARM
        #include "WNCore/inc/Internal/ARM/WNConfig.h"
    #endif
#endif

#endif // __WN_CORE_INTERNAL_CONFIG_OLD_H__