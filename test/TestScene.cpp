#include "toString.hpp"
#include "catch.hpp"

#include <vector>
#include <iostream>
#include <utility>
#include <cstring>
#include <sstream>
#include <cmath>

#define private public
#define protected public
#include <Scene.h>
#undef protected
#undef private

using namespace Hueduino;

TEST_CASE("::operator== && ::operator!=") {
	std::vector<std::tuple<Scene, Scene, bool>> tests {
        // All attributes match
        {{"abc", "s1", {}, Scene::Type::_DEFAULT}, {"abc", "s1", {}, Scene::Type::_DEFAULT}, true},

        // Different attributes don't equal
        {{"abc", "s1", {}, Scene::Type::_DEFAULT}, {"def", "s1", {}, Scene::Type::_DEFAULT}, false},
        {{"abc", "s1", {}, Scene::Type::_DEFAULT}, {"abc", "s2", {}, Scene::Type::_DEFAULT}, false},
        {{"abc", "s1", {}, Scene::Type::_DEFAULT}, {"abc", "s1", {1}, Scene::Type::_DEFAULT}, false},
        {{"abc", "s1", {}, Scene::Type::GROUPSCENE}, {"abc", "s1", {}, Scene::Type::LIGHTSCENE}, false},

        // Ignore groupId if type is not GroupScene
        {{"abc", "s1", {}, Scene::Type::LIGHTSCENE, 1}, {"abc", "s1", {}, Scene::Type::LIGHTSCENE, 2}, true},
        {{"abc", "s1", {}, Scene::Type::GROUPSCENE, 1}, {"abc", "s1", {}, Scene::Type::GROUPSCENE, 2}, false},
	};

	for(int testIdx=0; testIdx<tests.size(); testIdx++) {
		Scene scene = std::get<0>(tests.at(testIdx));
		Scene other = std::get<1>(tests.at(testIdx));
		bool areEqual = std::get<2>(tests.at(testIdx));

		if(areEqual) {
			REQUIRE(scene == other);
			REQUIRE_FALSE(scene != other);
		} else {
			REQUIRE(scene != other);
			REQUIRE_FALSE(scene == other);
		}
	}
}

TEST_CASE("::typeToStr && ::strToType") {
    SECTION("valid type/string") {
        std::vector<std::tuple<Scene::Type, const char*>> tests {
            {Scene::Type::LIGHTSCENE, "LightScene"},
            {Scene::Type::GROUPSCENE, "GroupScene"},
            {Scene::Type::_DEFAULT, "LightScene"},
        };

        for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            Scene::Type type = std::get<0>(tests.at(testIdx));
            const char* expectedStr = std::get<1>(tests.at(testIdx));

            CAPTURE(testIdx, type);

            const char* str = Scene::typeToStr(type);
            CHECK_THAT(str, Catch::Matchers::Equals(expectedStr));

            int type_idx = Scene::strToType(expectedStr);
            REQUIRE(type_idx >= 0);
            REQUIRE(static_cast<Scene::Type>(type_idx) == type);
        }
    }

    SECTION("invalid ::strToType argument") {
        std::vector<std::tuple<const char*>> tests {
			{0},
			{""},
			{"Light"},
			{"LightScen"},
			{"LightSceneButLonger"},
			{"LightScens"},
		};

        for(int testIdx=0; testIdx<tests.size(); testIdx++) {
			const char* str = std::get<0>(tests.at(testIdx));

			CAPTURE(testIdx, str);

			int type_idx = Scene::strToType(str);
			REQUIRE(type_idx == -1);
		}
    }
}