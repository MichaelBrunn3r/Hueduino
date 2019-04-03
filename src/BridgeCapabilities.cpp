#include "Bridge.h"
#include <JsonParser.h>

namespace Hueduino {
    Bridge::Capabilities Bridge::Capabilities::get(HTTPClient& http, WiFiClient& client, const char* base_url) {
        String url = base_url;
        url += "/capabilities";
        http.begin(client, (char*)url.c_str());
        http.GET();

        Capabilities cap;

        JStream::JsonParser parser = JStream::JsonParser(client);
        parser.enterObj();

        if(!parser.find("lights/available")) return cap;
        unsigned int lights_available = parser.parseInt();

        if(!parser.findKey("total")) return cap;
        cap.max_lights = parser.parseInt();
        cap.num_lights = cap.max_lights - lights_available;

        parser.exitCollection();
        
        if(!parser.find("sensors/available")) return cap;
        unsigned int sensors_available = parser.parseInt();

        if(!parser.findKey("total")) return cap;
        cap.max_sensors = parser.parseInt();
        cap.num_sensors = cap.max_sensors - sensors_available;

        parser.exitCollection();

        if(!parser.find("groups/available")) return cap;
        cap.max_groups = parser.parseInt();

        parser.exitCollection();

        if(!parser.find("scenes/available")) return cap;
        unsigned int scenes_available = parser.parseInt();

        if(!parser.findKey("total")) return cap;
        cap.max_scenes = parser.parseInt();
        cap.num_scenes = cap.max_scenes - scenes_available;

        parser.exitCollection(2);

        return cap;
    }
} // Hueduino
