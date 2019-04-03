#include "Light.h"

namespace Hueduino {
    const char* Light::alertToStr(Alert alert) {
        switch (alert) {
            case Alert::NONE: return "none";
            case Alert::SELECT: return "select";
            case Alert::LSELECT: return "lselect";
        }
    }

    const char* Light::effectToStr(Effect effect) {
        switch (effect) {
            case Effect::NONE: return "none";
            case Effect::COLORLOOP: return "colorloop";
        }
    }
} // Hueduino