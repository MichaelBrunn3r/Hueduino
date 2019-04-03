#include "Group.h"

namespace Hueduino {
    ActionStrBuilder& ActionStrBuilder::on(bool on) {
        if(hasAtLeastOneArg) action += ',';
        hasAtLeastOneArg = true;

        action += "\"on\":";
        action += on ? "true" : "false";
        return *this;
    }

    ActionStrBuilder& ActionStrBuilder::bri(Light::bri_type bri) {
        if(hasAtLeastOneArg) action += ',';
        hasAtLeastOneArg = true;

        action += "\"bri\":";
        action += String(bri);
        return *this;
    }

    ActionStrBuilder& ActionStrBuilder::hue(Light::hue_type hue) {
        if(hasAtLeastOneArg) action += ',';
        hasAtLeastOneArg = true;

        action += "\"hue\":";
        action += String(hue);
        return *this;
    }

    ActionStrBuilder& ActionStrBuilder::sat(Light::sat_type sat) {
        if(hasAtLeastOneArg) action += ',';
        hasAtLeastOneArg = true;

        action += "\"sat\":";
        action += String(sat);
        return *this;
    }

    ActionStrBuilder& ActionStrBuilder::xy(Light::xy_type x, Light::xy_type y) {
        if(hasAtLeastOneArg) action += ',';
        hasAtLeastOneArg = true;

        action += "\"xy\":[";
        action += String(x);
        action += ",";
        action += String(y);
        action += "]";
        return *this;
    }   

    ActionStrBuilder& ActionStrBuilder::ct(Light::ct_type ct) {
        if(hasAtLeastOneArg) action += ',';
        hasAtLeastOneArg = true;

        action += "\"ct\":";
        action += String(ct);
        return *this;
    }

    ActionStrBuilder& ActionStrBuilder::alert(Light::Alert alert) {
        if(hasAtLeastOneArg) action += ',';
        hasAtLeastOneArg = true;

        action += "\"alert\":\"";
        action += Light::alertToStr(alert);
        action += "\"";
        return *this;
    }

    ActionStrBuilder& ActionStrBuilder::effect(Light::Effect effect) {
        if(hasAtLeastOneArg) action += ',';
        hasAtLeastOneArg = true;

        action += "\"effect\":\"";
        action += Light::effectToStr(effect);
        action += "\"";
        return *this;
    }

    ActionStrBuilder& ActionStrBuilder::transitiontime(uint16_t transitiontime) {
        if(hasAtLeastOneArg) action += ',';
        hasAtLeastOneArg = true;

        action += "\"transitiontime\":";
        action += String(transitiontime);
        return *this;
    }

    ActionStrBuilder& ActionStrBuilder::bri_inc(short bri_inc) {
        if(hasAtLeastOneArg) action += ',';
        hasAtLeastOneArg = true;

        action += "\"bri_inc\":";
        action += String(bri_inc);
        return *this;
    }

    ActionStrBuilder& ActionStrBuilder::sat_inc(short sat_inc) {
        if(hasAtLeastOneArg) action += ',';
        hasAtLeastOneArg = true;

        action += "\"sat_inc\":";
        action += String(sat_inc);
        return *this;
    }

    ActionStrBuilder& ActionStrBuilder::hue_inc(short hue_inc) {
        if(hasAtLeastOneArg) action += ',';
        hasAtLeastOneArg = true;

        action += "\"hue_inc\":";
        action += String(hue_inc);
        return *this;
    }

    ActionStrBuilder& ActionStrBuilder::ct_inc(short ct_inc) {
        if(hasAtLeastOneArg) action += ',';
        hasAtLeastOneArg = true;

        action += "\"ct_inc\":";
        action += String(ct_inc);
        return *this;
    }

    ActionStrBuilder& ActionStrBuilder::xy_inc(float xy_inc, unsigned short decimalPlaces) {
        if(hasAtLeastOneArg) action += ',';
        hasAtLeastOneArg = true;

        action += "\"xy_inc\":";
        action += String(xy_inc, decimalPlaces);
        return *this;
    }

    ActionStrBuilder& ActionStrBuilder::scene(const char* scene) {
        if(hasAtLeastOneArg) action += ',';
        hasAtLeastOneArg = true;

        action += "\"scene\":\"";
        action += scene;
        action += "\"";
        return *this;
    }

    ActionStrBuilder::operator String&&() {
        action += "}";
        return std::move(action); // notice the move
    }
} // Hueduino
