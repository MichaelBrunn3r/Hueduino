#include "Bridge.h"

namespace Hueduino {
    Bridge::Bridge(const char* ip, const char* apiKey, bool https) {
        if(https) base_url += "https://";
        else base_url += "http://";
        base_url += (String)ip + "/api/" + apiKey;
    }

    GroupStream Bridge::getGroups(HTTPClient& http, WiFiClient& client) {
        return GroupStream(http, client, base_url.c_str());
    }

    SceneStream Bridge::getScenes(HTTPClient& http, WiFiClient& client) {
        return SceneStream(http, client, base_url.c_str());
    }
} // Hueduino