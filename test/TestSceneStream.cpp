#include "toString.hpp"
#include "catch.hpp"

#include <vector>
#include <iostream>
#include <utility>
#include <cstring>
#include <sstream>
#include <cmath>
#include <chrono>

#include <ESP8266HTTPClient.h>

#define private public
#define protected public
#include <Scene.h>
#include <Bridge.h>
#include <JsonParser.h>
#undef protected
#undef private

using namespace Hueduino;

TEST_CASE("::next") {
    SECTION("another scene exists") {

        struct Example { 
            String str = "\"007\": {\"name\": \"ex1\", \"type\": \"LightScene\", \"lights\": [1]}"; 
            Scene scene = Scene("007", "ex1", {1}, Scene::Type::LIGHTSCENE);
        } ex1;

        std::vector<std::tuple<String,Scene,const char*>> tests {
            {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", {"abc", "s1", {1}, Scene::Type::GROUPSCENE, 1}, "}"},

            // Skip json elements that are no scenes
            {"{ 123," + ex1.str + "}", ex1.scene, "}"},
            {"{ \"abc\"," + ex1.str + "}", ex1.scene, "}"},
            {"{ \"abc\":," + ex1.str + "}", ex1.scene, "}"},
            {"{ \"abc\":{}," + ex1.str + "}", ex1.scene, "}"},
            {"{ [[[]]]," + ex1.str + "}", ex1.scene, "}"},
            {"{ {{{}}}," + ex1.str + "}", ex1.scene, "}"},
            
            // invalid name
            {"{ \"abc\": {\"type\": \"LightScene\", \"lights\": [1]}," + ex1.str + "}", ex1.scene, "}"}, // missing
            {"{ \"abc\": {\"name\": , \"type\": \"LightScene\", \"lights\": [1]}," + ex1.str + "}", ex1.scene, "}"}, // empty
            {"{ \"abc\": {\"name\": 123, \"type\": \"LightScene\", \"lights\": [1]}," + ex1.str + "}", ex1.scene, "}"},
            {"{ \"abc\": {\"name\": [], \"type\": \"LightScene\", \"lights\": [1]}," + ex1.str + "}", ex1.scene, "}"},

            // invalid type
            {"{ \"abc\": {\"name\": \"s1\", \"lights\": [1]}," + ex1.str + "}", ex1.scene, "}"}, // missing
            {"{ \"abc\": {\"name\": \"s1\", \"type\": , \"lights\": [1]}," + ex1.str + "}", ex1.scene, "}"}, // empty
            {"{ \"abc\": {\"name\": \"s1\", \"type\": \"Invalid\", \"lights\": [1]}," + ex1.str + "}", ex1.scene, "}"},
            {"{ \"abc\": {\"name\": \"s1\", \"type\": 123, \"lights\": [1]}," + ex1.str + "}", ex1.scene, "}"},
            {"{ \"abc\": {\"name\": \"s1\", \"type\": [], \"lights\": [1]}," + ex1.str + "}", ex1.scene, "}"},
            
            // invalid light
            {"{ \"abc\": {\"name\": \"s1\", \"type\": \"LightScene\"}," + ex1.str + "}", ex1.scene, "}"}, // missing
            {"{ \"abc\": {\"name\": \"s1\", \"type\": \"LightScene\", \"lights\": }," + ex1.str + "}", ex1.scene, "}"}, // empty
            {"{ \"abc\": {\"name\": \"s1\", \"type\": \"LightScene\", \"lights\": \"astring\"}," + ex1.str + "}", ex1.scene, "}"},
            {"{ \"abc\": {\"name\": \"s1\", \"type\": \"LightScene\", \"lights\": 123}," + ex1.str + "}", ex1.scene, "}"},
            {"{ \"abc\": {\"name\": \"s1\", \"type\": \"LightScene\", \"lights\": {}}," + ex1.str + "}", ex1.scene, "}"},

            // invalid group
            {"{ \"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"lights\": [1]}," + ex1.str + "}", ex1.scene, "}"}, // missing
            {"{ \"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": , \"lights\": [1]}," + ex1.str + "}", ex1.scene, "}"}, // empty
            {"{ \"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": 1, \"lights\": [1]}," + ex1.str + "}", ex1.scene, "}"}, // not a string
            {"{ \"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"astring\", \"lights\": [1]}," + ex1.str + "}", ex1.scene, "}"}, // not a number

            // Don't parse invalid light IDs
            {"{\"abc\": {\"name\": \"s1\", \"type\": \"LightScene\", \"lights\": [-1,-2,3,4]}}", {"abc", "s1", {3,4}, Scene::Type::LIGHTSCENE}, "}"},

            // Scenes must have at least one valid light id
            {"{ \"abc\": {\"name\": \"s1\", \"type\": \"LightScene\", \"lights\": []}," + ex1.str + "}", ex1.scene, "}"}, // no ids
            {"{ \"abc\": {\"name\": \"s1\", \"type\": \"LightScene\", \"lights\": [-1,-2]}," + ex1.str + "}", ex1.scene, "}"}, // only invalid ids

            // Scenes of type Type::GroupScene must have a valid group argument
            {"{\"abc\": {\"name\": \"s1\", \"type\": \"LightScene\", \"lights\": [1]}}", {"abc", "s1", {1}, Scene::Type::LIGHTSCENE, 3}, "}"}, // LightScenes don't need group arg
            {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"lights\": [1]}," + ex1.str + "}", ex1.scene, "}"}, // missing group arg
            {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"3\", \"lights\": [1]}}", {"abc", "s1", {1}, Scene::Type::GROUPSCENE, 3}, "}"},
        };

        String ip = "192.0.0.1";
        String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";

        for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx)).c_str();
            Scene expectedScene = std::get<1>(tests.at(testIdx));
            const char* json_after_exec = std::get<2>(tests.at(testIdx));

            CAPTURE(testIdx, json);

            WiFiClient client;
            HTTPClient http;
            Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
            http.addGETResponse(bridge.base_url + "/scenes", json);
            SceneStream stream = bridge.getScenes(http, client);

            std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(stream.next());
            REQUIRE(scene != nullptr);
            REQUIRE(*scene == expectedScene);

            CHECK_THAT(client.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
            http.end();
        }
    }

    SECTION("No more scenes exist") {
        std::vector<std::tuple<const char*,const char*>> tests {
            // Empty Json
            {"", ""},
            {"[]", "[]"},
            {"{}", "}"},

            // Skip json elements that are no scenes
            {"{ 123 }", "}"},
            {"{ \"abc\" }", "}"},
            {"{ \"abc\": }", "}"},
            {"{ \"abc\":{} }", "}"},
            {"{ [[[]]] }", "}"},
            {"{ {{{}}} }", "}"},

            // Skip values
            {"{ \"1\" }", "}"},
            {"{ 123 }", "}"},
            {"{ 123, 123, 123, 123 }", "}"},
            {"{ [[[]]] }", "}"},
            {"{ {{{}}} }", "}"},
        };

        String ip = "192.0.0.1";
        String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";
        
        for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            const char* json_after_exec = std::get<1>(tests.at(testIdx));

            CAPTURE(testIdx, json);

            WiFiClient client;
            HTTPClient http;
            Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
            http.addGETResponse(bridge.base_url + "/scenes", json);
            SceneStream stream = bridge.getScenes(http, client);

            std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(stream.next());
            REQUIRE(scene.get() == nullptr);
            
            CHECK_THAT(client.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
            http.end();
        }
    }
}

TEST_CASE("::collect") {
    std::vector<std::tuple<const char*,std::vector<Scene>,const char*>> tests {
        {"", {}, ""},
        {"[]", {}, "[]"},
        {"{}", {}, "}"},

        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 1}}, "}"},
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", 
           {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 1}, {"def", "s2", {1}, Scene::Type::GROUPSCENE, 1}, {"ghi", "s3", {1}, Scene::Type::GROUPSCENE, 1}}, "}"},

        // Skip invalid scenes
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"Invalid\", \"group\": \"1\", \"lights\": [1]}}", {}, "}"},
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"Invalid\", \"group\": \"1\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", 
           {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 1}, {"ghi", "s3", {1}, Scene::Type::GROUPSCENE, 1}}, "}"},
    };

    String ip = "192.0.0.1";
    String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";

    for(int testIdx=0; testIdx<tests.size(); testIdx++) {
        const char* json = std::get<0>(tests.at(testIdx));
        std::vector<Scene> expectedScene = std::get<1>(tests.at(testIdx));
        const char* json_after_exec = std::get<2>(tests.at(testIdx));

        CAPTURE(testIdx, json);

        WiFiClient client;
        HTTPClient http;
        Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
        http.addGETResponse(bridge.base_url + "/scenes", json);
        SceneStream stream = bridge.getScenes(http, client);

        std::vector<std::unique_ptr<Scene>> scenes = stream.collect();

        REQUIRE(scenes.size() == expectedScene.size());
        for(int i=0; i<scenes.size(); i++) {
            REQUIRE(scenes.at(i) != nullptr);
            REQUIRE(*scenes.at(i) == expectedScene.at(i));
        }

        // Check if stream ended
        REQUIRE(stream.next() == nullptr);
        REQUIRE(stream.mReachedEOF == true);

        CHECK_THAT(client.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        http.end();
    }
}

TEST_CASE("Filter names") {
    std::vector<std::tuple<const char*,std::vector<const char*>, std::vector<Scene>,const char*>> tests {
        {"", {}, {}, ""},
        {"[]", {}, {}, "[]"},
        {"{}", {}, {}, "}"},

        // Filter no names
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", 
           {}, {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 1}, {"def", "s2", {1}, Scene::Type::GROUPSCENE, 1}, {"ghi", "s3", {1}, Scene::Type::GROUPSCENE, 1}}, "}"},

        // Filter one name
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", {"s1"}, {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 1}}, "}"},
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", {"s2"}, {{"def", "s2", {1}, Scene::Type::GROUPSCENE, 1}}, "}"},

        // Filter multiple names
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", {"s1", "s2", "s3"}, {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 1}}, "}"},
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", 
            {"s1", "s3"}, {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 1}, {"ghi", "s3", {1}, Scene::Type::GROUPSCENE, 1}}, "}"},

        // Filter names not in response
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", {"scene1", "scene3"}, {}, "}"},
    };

    String ip = "192.0.0.1";
    String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";

    for(int testIdx=0; testIdx<tests.size(); testIdx++) {
        const char* json = std::get<0>(tests.at(testIdx));
        std::vector<const char*> searchedNames = std::get<1>(tests.at(testIdx));
        std::vector<Scene> expectedScene = std::get<2>(tests.at(testIdx));
        const char* json_after_exec = std::get<3>(tests.at(testIdx));

        CAPTURE(testIdx, json, searchedNames);

        WiFiClient client;
        HTTPClient http;
        Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
        http.addGETResponse(bridge.base_url + "/scenes", json);
        auto scenes = bridge.getScenes(http, client).filterNames(searchedNames).collect();

        REQUIRE(scenes.size() == expectedScene.size());
        for(int i=0; i<scenes.size(); i++) {
            REQUIRE(scenes.at(i) != nullptr);
            REQUIRE(*scenes.at(i) == expectedScene.at(i));
        }

        CHECK_THAT(client.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        http.end();
    }
}

TEST_CASE("Filter type") {
    std::vector<std::tuple<const char*,Scene::Type, std::vector<Scene>,const char*>> tests {
        {"", Scene::Type::_DEFAULT, {}, ""},
        {"[]", Scene::Type::_DEFAULT, {}, "[]"},
        {"{}", Scene::Type::_DEFAULT, {}, "}"},

        // Filter Type::LightScene
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"LightScene\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"LightScene\", \"lights\": [1]}}", 
           Scene::Type::LIGHTSCENE, {{"abc", "s1", {1}, Scene::Type::LIGHTSCENE}, {"ghi", "s3", {1}, Scene::Type::LIGHTSCENE}}, "}"},

        // Filter Type::GroupScene
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"LightScene\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", 
           Scene::Type::GROUPSCENE, {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 1}, {"ghi", "s3", {1}, Scene::Type::GROUPSCENE, 1}}, "}"},
    };

    String ip = "192.0.0.1";
    String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";

    for(int testIdx=0; testIdx<tests.size(); testIdx++) {
        auto json = std::get<0>(tests.at(testIdx));
        auto searchedType = std::get<1>(tests.at(testIdx));
        auto expectedScene = std::get<2>(tests.at(testIdx));
        auto json_after_exec = std::get<3>(tests.at(testIdx));

        CAPTURE(testIdx, json, searchedType);

        WiFiClient client;
        HTTPClient http;
        Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
        http.addGETResponse(bridge.base_url + "/scenes", json);
        auto scenes = bridge.getScenes(http, client).filterType(searchedType).collect();

        REQUIRE(scenes.size() == expectedScene.size());
        for(int i=0; i<scenes.size(); i++) {
            REQUIRE(scenes.at(i) != nullptr);
            REQUIRE(*scenes.at(i) == expectedScene.at(i));
        }

        CHECK_THAT(client.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        http.end();
    }
}

TEST_CASE("Filter group IDs") {
    std::vector<std::tuple<const char*,std::vector<Group::id_type>, std::vector<Scene>,const char*>> tests {
        {"", {}, {}, ""},
        {"[]", {}, {}, "[]"},
        {"{}", {}, {}, "}"},

        // Filter no IDs
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", 
           {}, {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 1}, {"def", "s2", {1}, Scene::Type::GROUPSCENE, 1}, {"ghi", "s3", {1}, Scene::Type::GROUPSCENE, 1}}, "}"},

        // Filter one ID
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"7\", \"lights\": [1]}}", {7}, {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 7}}, "}"},
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"7\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", {7}, {{"def", "s2", {1}, Scene::Type::GROUPSCENE, 7}}, "}"},

        // Filter multiple IDs
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"7\", \"lights\": [1]}}", {7,8,9}, {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 7}}, "}"},
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"7\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"8\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"9\", \"lights\": [1]}}", 
            {7,8,9}, {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 7}, {"def", "s2", {1}, Scene::Type::GROUPSCENE, 8}, {"ghi", "s3", {1}, Scene::Type::GROUPSCENE, 9}}, "}"},

        // Filter IDs not in response
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"2\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"3\", \"lights\": [1]}}", {7,8,9}, {}, "}"},

        // Don't filter scenes not of type Type::GROUPSCENE
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"LightScene\", \"group\": \"7\", \"lights\": [1]}}", {7}, {}, "}"},
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"7\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"LightScene\", \"group\": \"7\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"7\", \"lights\": [1]}}", 
           {7}, {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 7}, {"ghi", "s3", {1}, Scene::Type::GROUPSCENE, 7}}, "}"},
    };

    String ip = "192.0.0.1";
    String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";

    for(int testIdx=0; testIdx<tests.size(); testIdx++) {
        auto json = std::get<0>(tests.at(testIdx));
        auto searchedGroupIDs = std::get<1>(tests.at(testIdx));
        auto expectedScene = std::get<2>(tests.at(testIdx));
        auto json_after_exec = std::get<3>(tests.at(testIdx));

        CAPTURE(testIdx, json, searchedGroupIDs);

        WiFiClient client;
        HTTPClient http;
        Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
        http.addGETResponse(bridge.base_url + "/scenes", json);
        auto scenes = bridge.getScenes(http, client).filterGroups(searchedGroupIDs).collect();

        REQUIRE(scenes.size() == expectedScene.size());
        for(int i=0; i<scenes.size(); i++) {
            REQUIRE(scenes.at(i) != nullptr);
            REQUIRE(*scenes.at(i) == expectedScene.at(i));
        }

        CHECK_THAT(client.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        http.end();
    }
}

TEST_CASE("Filter IDs") {
    std::vector<std::tuple<const char*,std::vector<const char*>, std::vector<Scene>,const char*>> tests {
        {"", {}, {}, ""},
        {"[]", {}, {}, "[]"},
        {"{}", {}, {}, "}"},

        // Filter no IDs
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", 
           {}, {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 1}, {"def", "s2", {1}, Scene::Type::GROUPSCENE, 1}, {"ghi", "s3", {1}, Scene::Type::GROUPSCENE, 1}}, "}"},

        // Filter one ID
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", {"abc"}, {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 1}}, "}"},
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", {"def"}, {{"def", "s2", {1}, Scene::Type::GROUPSCENE, 1}}, "}"},

        // Filter multiple IDs
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", {"abc", "def", "ghi"}, {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 1}}, "}"},
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", 
            {"abc", "ghi"}, {{"abc", "s1", {1}, Scene::Type::GROUPSCENE, 1}, {"ghi", "s3", {1}, Scene::Type::GROUPSCENE, 1}}, "}"},

        // Filter IDs not in response
        {"{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}", {"uvw", "xyz"}, {}, "}"},
    };

    String ip = "192.0.0.1";
    String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";

    for(int testIdx=0; testIdx<tests.size(); testIdx++) {
        auto json = std::get<0>(tests.at(testIdx));
        auto searchedIDs = std::get<1>(tests.at(testIdx));
        auto expectedScene = std::get<2>(tests.at(testIdx));
        auto json_after_exec = std::get<3>(tests.at(testIdx));

        CAPTURE(testIdx, json, searchedIDs);

        WiFiClient client;
        HTTPClient http;
        Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
        http.addGETResponse(bridge.base_url + "/scenes", json);
        auto scenes = bridge.getScenes(http,client).filterIDs(searchedIDs).collect();

        REQUIRE(scenes.size() == expectedScene.size());
        for(int i=0; i<scenes.size(); i++) {
            REQUIRE(scenes.at(i) != nullptr);
            REQUIRE(*scenes.at(i) == expectedScene.at(i));
        }

        CHECK_THAT(client.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        http.end();
    }
}

TEST_CASE("::next speed test", "[!hide][time_next][!benchmark]") {
    String ip = "192.0.0.1";
    String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";

    String json = "{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}";

    WiFiClient client;
    HTTPClient http;
    Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
    http.addGETResponse(bridge.base_url + "/scenes", json);
    auto stream = bridge.getScenes(http,client);

    BENCHMARK("iter over scenes") {
        while(stream.next() != nullptr) continue;
    }

    http.end();
}

TEST_CASE("::collect speed test", "[!hide][time_collect][!benchmark]") {
    String ip = "192.0.0.1";
    String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";

    String json = "{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}";

    WiFiClient client;
    HTTPClient http;
    Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
    http.addGETResponse(bridge.base_url + "/scenes", json);
    auto stream = bridge.getScenes(http,client);

    BENCHMARK("collect scenes") {
        stream.collect();
    }

    http.end();
}

TEST_CASE("stream speed test", "[!hide][time_stream][!benchmark]") {
    String ip = "192.0.0.1";
    String apiKey = "cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4";

    String json = "{\"abc\": {\"name\": \"s1\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"def\": {\"name\": \"s2\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}, \
            \"ghi\": {\"name\": \"s3\", \"type\": \"GroupScene\", \"group\": \"1\", \"lights\": [1]}}";

    WiFiClient client;
    HTTPClient http;
    Bridge bridge = Bridge(ip.c_str(), apiKey.c_str());
    http.addGETResponse(bridge.base_url + "/scenes", json);
    http.begin(client, "http://192.0.0.1/api/cf5zoJOtiuFtHpUlPZuna495CDhLHMauQ82FEzx4/scenes");
    http.GET();

    BENCHMARK("read char stream") {
        while(client.available() > 0) client.read();
    }

    http.end();
}