#include "toString.hpp"
#include "catch.hpp"

#include <vector>
#include <iostream>
#include <utility>
#include <cstring>
#include <sstream>
#include <cmath>

#include <ESP8266HTTPClient.h>

#define private public
#define protected public
#include <Group.h>
#include <Bridge.h>
#include <JsonParser.h>
#undef protected
#undef private

using namespace Hueduino;

TEST_CASE("::end") {
    String ip = "192.0.0.1";
    String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";

    const char* json = "{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}";
    WiFiClient client;
    HTTPClient http;
    Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
    http.addGETResponse(bridge.base_url + "/groups", json);
    bridge.requestGroups(http, client);
    GroupStream stream = bridge.parseGroupStream(client);
    http.end();

    REQUIRE(stream.next() == nullptr);
    REQUIRE(stream.mEnded == true);
}

TEST_CASE("::next", "[next]") {
    SECTION("another group exists") {
        std::vector<std::tuple<const char*,Group,const char*>> tests {
            {"{ \"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}", {1, "g1", {}, {}, Group::Type::LIGHTGROUP} , "}"},

            {"{ \"1\": {\"name\":\"g1\",\"lights\":[\"1\", \"2\"],\"sensors\":[],\"type\":\"LightGroup\"}}", {1, "g1", {1,2}, {}, Group::Type::LIGHTGROUP} , "}"},
            {"{ \"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[\"1\", \"2\"],\"type\":\"LightGroup\"}}", {1, "g1", {}, {1,2}, Group::Type::LIGHTGROUP} , "}"},

            // Skip invalid groups
            {"{ \"1\":, \"2\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}", {2, "g1", {}, {}, Group::Type::LIGHTGROUP}, "}"},
            {"{ \"1\": {}, \"2\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}", {2, "g1", {}, {}, Group::Type::LIGHTGROUP}, "}"},

            // Skip invalid values
            {"{ \"1\", \"2\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}", {2, "g1", {}, {}, Group::Type::LIGHTGROUP}, "}"},
            {"{ [[[]]], \"2\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}", {2, "g1", {}, {}, Group::Type::LIGHTGROUP}, "}"},
            {"{ {{{}}}, \"2\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}", {2, "g1", {}, {}, Group::Type::LIGHTGROUP}, "}"},

            // Skip group with type 'Room' but without class
            {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Room\"},\
                \"2\": {\"name\":\"g2\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}", {2, "g2", {}, {}, Group::Type::LIGHTGROUP} , "}"},

            // Only allow valid light/sensor IDs
            {"{ \"1\": {\"name\":\"g1\",\"lights\":[-1,-2,\"astring\",0,1],\"sensors\": [-1,-2,\"astring\",0,1],\"type\":\"LightGroup\"}}", 
                {1, "g1", {0,1}, {0,1}, Group::Type::LIGHTGROUP}, "}"},
        };

        String ip = "192.0.0.1";
        String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";
        
        for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            Group expectedGroup = std::get<1>(tests.at(testIdx));
            const char* json_after_exec = std::get<2>(tests.at(testIdx));

            CAPTURE(testIdx, json);
            
            WiFiClient client;
            HTTPClient http;
            Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
            http.addGETResponse(bridge.base_url + "/groups", json);
            bridge.requestGroups(http, client);
            GroupStream stream = bridge.parseGroupStream(client);

            // Compare ::next
            std::unique_ptr<Group> group = std::unique_ptr<Group>(stream.next()); 
            REQUIRE(group != nullptr);
            REQUIRE(*group == expectedGroup);

            CHECK_THAT(client.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
            http.end();
        }
    }

    SECTION("no more valid groups exists") {
        std::vector<std::tuple<const char*,Group*,const char*>> tests {
            // Empty Json
            {"", nullptr, ""},
            {"[]", nullptr, "[]"},
            {"{}", nullptr, "}"},

            // Skip invalid groups
            {"{ \"1\":, \"1\":, \"1\":}", nullptr, "}"},
            {"{ \"1\": {}}", nullptr, "}"},

            // Skip values
            {"{ \"1\"}", nullptr, "}"},
            {"{ 123}", nullptr, "}"},
            {"{ 123, 123, 123, 123}", nullptr, "}"},
            {"{ [[[]]]}", nullptr, "}"},
            {"{ {{{}}}}", nullptr, "}"},

            // Skip groups with missing attributes
            {"{ \"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[]}}", nullptr, "}"},
            {"{ \"1\": {\"name\":\"g1\",\"lights\":[],\"type\":\"LightGroup\"}}", nullptr, "}"},
            {"{ \"1\": {\"name\":\"g1\",\"sensors\":[],\"type\":\"LightGroup\"}}", nullptr, "}"},
            {"{ \"1\": {\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}", nullptr, "}"},

            // Skip groups with invalid type
            {"{ \"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"AType\"}}", nullptr, "}"},
            {"{ \"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":}}", nullptr, "}"},
            {"{ \"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":[]}}", nullptr, "}"},

            // Skip groups with invalid sensors
            {"{ \"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":,\"type\":\"LightGroup\"}}", nullptr, "}"},
            {"{ \"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\": \"astring\",\"type\":\"LightGroup\"}}", nullptr, "}"},
            {"{ \"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\": [-1,-2,-3],\"type\":\"LightGroup\"}}", new Group(1, "g1", {}, {}, Group::Type::LIGHTGROUP), "}"},
            {"{ \"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\": [\"astring\"],\"type\":\"LightGroup\"}}", new Group(1, "g1", {}, {}, Group::Type::LIGHTGROUP), "}"},

            // Skip groups with invalid lights
            {"{ \"1\": {\"name\":\"g1\",\"lights\":,\"sensors\":[],\"type\":\"LightGroup\"}}", nullptr, "}"},
            {"{ \"1\": {\"name\":\"g1\",\"lights\": \"astring\",\"sensors\":[],\"type\":\"LightGroup\"}}", nullptr, "}"},
            {"{ \"1\": {\"name\":\"g1\",\"lights\": [-1,-2,-3],\"sensors\":[],\"type\":\"LightGroup\"}}", new Group(1, "g1", {}, {}, Group::Type::LIGHTGROUP), "}"},
            {"{ \"1\": {\"name\":\"g1\",\"lights\": [\"astring\"],\"sensors\":[],\"type\":\"LightGroup\"}}", new Group(1, "g1", {}, {}, Group::Type::LIGHTGROUP), "}"},

            {"{ \"1\": {\"name\": 123,\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}", nullptr, "}"},
            {"{ \"1\": {\"name\": [],\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}", nullptr, "}"},
        };

        String ip = "192.0.0.1";
        String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";
        
        for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            std::unique_ptr<Group> expectedGroup = std::unique_ptr<Group>(std::get<1>(tests.at(testIdx)));
            const char* json_after_exec = std::get<2>(tests.at(testIdx));

            CAPTURE(testIdx, json);
            WiFiClient client;
            HTTPClient http;
            Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
            http.addGETResponse(bridge.base_url + "/groups", json);
            bridge.requestGroups(http, client);
            GroupStream stream = bridge.parseGroupStream(client);

            if(expectedGroup == nullptr) {
                REQUIRE(stream.next() == nullptr);
            } else {
                // Compare ::next
                std::unique_ptr<Group> group = std::unique_ptr<Group>(stream.next()); 
                REQUIRE(group != nullptr);
                REQUIRE(*group == *expectedGroup);
            }
            
            CHECK_THAT(client.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
            http.end();
        }
    }
}

TEST_CASE("::collect") {
    std::vector<std::tuple<const char*,std::vector<Group>,const char*>> tests {
        {"", {}, ""},
        {"{}", {}, "}"},
        {"[]", {}, "[]"},

        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}", {{1, "g1", {}, {}, Group::Type::LIGHTGROUP}}, "}"},
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"},\
            \"2\": {\"name\":\"g2\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"},\
            \"3\": {\"name\":\"g3\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}", 
            {{1, "g1", {}, {}, Group::Type::LIGHTGROUP},{2, "g2", {}, {}, Group::Type::LIGHTGROUP},{3, "g3", {}, {}, Group::Type::LIGHTGROUP}}, "}"},

        // Skip invalid groups
        {"{\"1\": {\"name\":\"invalid\",\"lights\":[],\"sensors\":[],\"type\":\"Lol\"}}", {}, "}"},
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"},\
            \"2\": {\"name\":\"invalid\",\"lights\":[],\"sensors\":[],\"type\":\"Lol\"},\
            \"3\": {\"name\":\"g3\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}", 
            {{1, "g1", {}, {}, Group::Type::LIGHTGROUP},{3, "g3", {}, {}, Group::Type::LIGHTGROUP}}, "}"},
    };

    String ip = "192.0.0.1";
    String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";

    for(int testIdx=0; testIdx<tests.size(); testIdx++) {
        const char* json = std::get<0>(tests.at(testIdx));
        std::vector<Group> expectedGroups = std::get<1>(tests.at(testIdx));
        const char* json_after_exec = std::get<2>(tests.at(testIdx));

        CAPTURE(testIdx, json);
        
        WiFiClient client;
        HTTPClient http;
        Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
        http.addGETResponse(bridge.base_url + "/groups", json);
        bridge.requestGroups(http, client);
        GroupStream stream = bridge.parseGroupStream(client);

        std::vector<std::unique_ptr<Group>> groups = stream.collect();

        REQUIRE(groups.size() == expectedGroups.size());
        for(int i=0; i<groups.size(); i++) {
            REQUIRE(groups.at(i) != nullptr);
            REQUIRE(*groups.at(i) == expectedGroups.at(i));
        }

        // Check if stream ended
        REQUIRE(stream.next() == nullptr);
        REQUIRE(stream.mEnded == true);

        CHECK_THAT(client.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        http.end();
    };
}

TEST_CASE("::filterIDs") {
    std::vector<std::tuple<const char*,unsigned int,unsigned int,std::vector<Group>,const char*>> tests {
        {"", 0,1, {}, ""},
        {"{}", 0,1, {}, "}"},
        {"[]", 0,1, {}, "[]"},

        // Get range
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"2\": {\"name\":\"g2\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"3\": {\"name\":\"g3\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"}}", 0,2,
             {{1, "g1", {}, {}, Group::Type::ZONE},{2, "g2", {}, {}, Group::Type::ZONE}}, "}"},
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"2\": {\"name\":\"g2\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"3\": {\"name\":\"g3\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"}}", 2,3,
            {{2, "g2", {}, {}, Group::Type::ZONE},{3, "g3", {}, {}, Group::Type::ZONE}}, "}"},

        // Get id
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"2\": {\"name\":\"g2\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"3\": {\"name\":\"g3\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"}}", 2,2,
            {{2, "g2", {}, {}, Group::Type::ZONE}}, "}"},

        // Min > Max
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"2\": {\"name\":\"g2\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"3\": {\"name\":\"g3\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"}}", 2,1, {}, "}"},
    };

    String ip = "192.0.0.1";
    String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";

    for(int testIdx=0; testIdx<tests.size(); testIdx++) {
        const char* json = std::get<0>(tests.at(testIdx));
        unsigned int min_id = std::get<1>(tests.at(testIdx));
        unsigned int max_id = std::get<2>(tests.at(testIdx));
        std::vector<Group> expectedGroups = std::get<3>(tests.at(testIdx));
        const char* json_after_exec = std::get<4>(tests.at(testIdx));

        CAPTURE(testIdx, json);

        WiFiClient client;
        HTTPClient http;
        Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
        http.addGETResponse(bridge.base_url + "/groups", json);
        bridge.requestGroups(http, client);
        GroupStream stream = bridge.parseGroupStream(client);

        std::vector<std::unique_ptr<Group>> groups = stream.filterIDs(min_id,max_id).collect();

        REQUIRE(groups.size() == expectedGroups.size());
        for(int i=0; i<groups.size(); i++) {
            REQUIRE(groups.at(i) != nullptr);
            REQUIRE(*groups.at(i) == expectedGroups.at(i));
        }

        CHECK_THAT(client.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        http.end();
    }
}

TEST_CASE("::filterNames") {
    std::vector<std::tuple<const char*,std::vector<const char*>,std::vector<Group>,const char*>> tests {
        {"", {}, {}, ""},
        {"{}", {}, {}, "}"},
        
        // Filter no names 
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"}}", {}, {{1, "g1", {}, {}, Group::Type::ZONE}}, "}"},

        // Filter one name
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"}}", {"g1"}, {{1, "g1", {}, {}, Group::Type::ZONE}}, "}"},
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"2\": {\"name\":\"g2\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"3\": {\"name\":\"g3\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"}}", 
            {"g2"}, {{2, "g2", {}, {}, Group::Type::ZONE}}, "}"},

        // Filter multiple names
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"}}", {"g1","g2","g3"}, {{1, "g1", {}, {}, Group::Type::ZONE}}, "}"},
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"2\": {\"name\":\"g2\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"3\": {\"name\":\"g3\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"}}", 
            {"g2","g3"}, {{2, "g2", {}, {}, Group::Type::ZONE},{3, "g3", {}, {}, Group::Type::ZONE}}, "}"},

        // Filter names not in json
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"2\": {\"name\":\"g2\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"3\": {\"name\":\"g3\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"}}", 
            {"group1","group2","group3"}, {}, "}"},
    };

    String ip = "192.0.0.1";
    String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";

    for(int testIdx=0; testIdx<tests.size(); testIdx++) {
        const char* json = std::get<0>(tests.at(testIdx));
        std::vector<const char*> searchedNames = std::get<1>(tests.at(testIdx));
        std::vector<Group> expectedGroups = std::get<2>(tests.at(testIdx));
        const char* json_after_exec = std::get<3>(tests.at(testIdx));

        CAPTURE(testIdx, json, searchedNames);

        WiFiClient client;
        HTTPClient http;
        Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
        http.addGETResponse(bridge.base_url + "/groups", json);
        bridge.requestGroups(http, client);
        GroupStream stream = bridge.parseGroupStream(client);

        std::vector<std::unique_ptr<Group>> groups = stream.filterNames(searchedNames).collect();

        REQUIRE(groups.size() == expectedGroups.size());
        for(int i=0; i<groups.size(); i++) {
            REQUIRE(groups.at(i) != nullptr);
            REQUIRE(*groups.at(i) == expectedGroups.at(i));
        }

        CHECK_THAT(client.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        http.end();
    }
}

TEST_CASE("::filterTypes") {
    std::vector<std::tuple<const char*,std::set<Group::Type>,std::vector<Group>,const char*>> tests {
        {"", {}, {}, ""},
        {"{}", {}, {}, "}"},

        // Filter no types 
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"}}", {}, {{1, "g1", {}, {}, Group::Type::ZONE}}, "}"},

        // Filter one type
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Entertainment\"}}", {Group::Type::ENTERTAINMENT}, {{1, "g1", {}, {}, Group::Type::ENTERTAINMENT}}, "}"},
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"2\": {\"name\":\"g2\",\"lights\":[],\"sensors\":[],\"type\":\"Entertainment\"},\
            \"3\": {\"name\":\"g3\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"}}", 
            {Group::Type::ENTERTAINMENT}, {{2, "g2", {}, {}, Group::Type::ENTERTAINMENT}}, "}"},

        // Filter multiple types
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"}}", 
            {Group::Type::ENTERTAINMENT, Group::Type::ZONE, Group::Type::LIGHTGROUP}, {{1, "g1", {}, {}, Group::Type::ZONE}}, "}"},
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Entertainment\"},\
            \"2\": {\"name\":\"g2\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"3\": {\"name\":\"g3\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}", 
            {Group::Type::ZONE, Group::Type::LIGHTGROUP}, {{2, "g2", {}, {}, Group::Type::ZONE},{3, "g3", {}, {}, Group::Type::LIGHTGROUP}}, "}"},

        // Filter types not in json
        {"{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Entertainment\"},\
            \"2\": {\"name\":\"g2\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"3\": {\"name\":\"g3\",\"lights\":[],\"sensors\":[],\"type\":\"LightGroup\"}}", 
            {Group::Type::ROOM, Group::Type::LIGHTSOURCE, Group::Type::LUMINAIRE}, {}, "}"},
    };

    String ip = "192.0.0.1";
    String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";

    for(int testIdx=0; testIdx<tests.size(); testIdx++) {
        const char* json = std::get<0>(tests.at(testIdx));
        std::set<Group::Type> searchedTypes = std::get<1>(tests.at(testIdx));
        std::vector<Group> expectedGroups = std::get<2>(tests.at(testIdx));
        const char* json_after_exec = std::get<3>(tests.at(testIdx));

        CAPTURE(testIdx, json);

        WiFiClient client;
        HTTPClient http;
        Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
        http.addGETResponse(bridge.base_url + "/groups", json);
        bridge.requestGroups(http, client);
        GroupStream stream = bridge.parseGroupStream(client);

        std::vector<std::unique_ptr<Group>> groups = stream.filterTypes(searchedTypes).collect();

        REQUIRE(groups.size() == expectedGroups.size());
        for(int i=0; i<groups.size(); i++) {
            REQUIRE(groups.at(i) != nullptr);
            REQUIRE(*groups.at(i) == expectedGroups.at(i));
        }

        CHECK_THAT(client.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        http.end();
    }
}

TEST_CASE("::collect speed test", "[!hide][time_collect][!benchmark]") {
    String ip = "192.0.0.1";
    String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";

    String json = "{\"1\": {\"name\":\"g1\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"},\
            \"2\": {\"name\":\"g2\",\"lights\":[],\"sensors\":[],\"type\":\"Entertainment\"},\
            \"3\": {\"name\":\"g3\",\"lights\":[],\"sensors\":[],\"type\":\"Zone\"}}";

    WiFiClient client;
    HTTPClient http;
    Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
    http.addGETResponse(bridge.base_url + "/groups", json);
    auto stream = bridge.parseGroupStream(client);

    BENCHMARK("collect scenes") {
        stream.collect();
    }

    

    http.end();
}