#include "Bridge.h"

namespace Hueduino {
    Bridge::Bridge(const char* ip, const char* apiKey, bool https) {
        if(https) base_url += "https://";
        else base_url += "http://";
        base_url += (String)ip + "/api/" + apiKey;
    }

    bool Bridge::requestGroups(HTTPClient& http, WiFiClient& client) {
        if(!http.begin(client, base_url + "/groups")) return false;
        int httpCode = http.GET();
        return httpCode == HTTP_CODE_OK;
    }

    bool Bridge::requestScenes(HTTPClient& http, WiFiClient& client) {
        if(!http.begin(client, base_url + "/scenes")) return false;
        int httpCode = http.GET();
        return httpCode == HTTP_CODE_OK;
    }

    GroupStream Bridge::parseGroupStream(WiFiClient& client) {
        return GroupStream(client);
    }

    SceneStream Bridge::parseSceneStream(WiFiClient& client) {
        return SceneStream(client);
    }
} // Hueduino