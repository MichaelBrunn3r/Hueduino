#pragma once

#include <Arduino.h>

namespace Hueduino {
    namespace Internals {
        int searchStr(const char* str, const char* arr[], long arr_len);
    } // Internals
} // Hueduino
