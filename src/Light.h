#pragma once

#include <Internals/Types.h>

namespace Hueduino {
    class Light {
        public:
            enum class Alert : byte {
                NONE,
                SELECT,
                LSELECT
            };
            static const char* alertToStr(Alert alert);

            enum class Effect : byte {
                NONE,
                COLORLOOP
            };
            static const char* effectToStr(Effect effect);

            typedef uint8_t bri_type;
            typedef uint16_t hue_type;
            typedef uint8_t sat_type;
            typedef float xy_type;
            typedef uint16_t ct_type;

            typedef unsigned int id_type;
    };
}