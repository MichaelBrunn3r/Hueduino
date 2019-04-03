#include "toString.hpp"
#include "catch.hpp"

#include <vector>
#include <iostream>
#include <utility>
#include <cstring>
#include <sstream>
#include <cmath>

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <Internals/HueUtils.h>

#define private public
#define protected public
#include <Group.h>
#include <Bridge.h>
#undef protected
#undef private

using namespace Hueduino;

TEST_CASE("Group::operator== && ::operator!=") {
	std::vector<std::tuple<Group, Group, bool>> tests {
		// All attributes match
		{{1, "g1", {}, {}, Group::Type::ENTERTAINMENT, Group::Class::ATTIC}, {1, "g1", {}, {}, Group::Type::ENTERTAINMENT, Group::Class::ATTIC}, true},

		// Different attributes don't equal
		{{1, "g1", {}, {}, Group::Type::ENTERTAINMENT, Group::Class::ATTIC}, {2, "g1", {}, {}, Group::Type::ENTERTAINMENT, Group::Class::ATTIC}, false},
		{{1, "g1", {}, {}, Group::Type::ENTERTAINMENT, Group::Class::ATTIC}, {1, "g2", {}, {}, Group::Type::ENTERTAINMENT, Group::Class::ATTIC}, false},
		{{1, "g1", {}, {}, Group::Type::ENTERTAINMENT, Group::Class::ATTIC}, {1, "g1", {1,2,3}, {}, Group::Type::ENTERTAINMENT, Group::Class::ATTIC}, false},
		{{1, "g1", {}, {}, Group::Type::ENTERTAINMENT, Group::Class::ATTIC}, {1, "g1", {}, {1,2,3}, Group::Type::ENTERTAINMENT, Group::Class::ATTIC}, false},
		{{1, "g1", {}, {}, Group::Type::ENTERTAINMENT, Group::Class::ATTIC}, {1, "g1", {}, {1,2,3}, Group::Type::LIGHTGROUP, Group::Class::ATTIC}, false},

		// Ignore Class if Type is not Room
		{{1, "g1", {}, {}, Group::Type::ENTERTAINMENT, Group::Class::ATTIC}, {1, "g1", {}, {}, Group::Type::ENTERTAINMENT, Group::Class::BALCONY}, true},
		{{1, "g1", {}, {}, Group::Type::ROOM, Group::Class::ATTIC}, {1, "g1", {}, {1,2,3}, Group::Type::ROOM, Group::Class::BALCONY}, false},
	};

	for(int testIdx=0; testIdx<tests.size(); testIdx++) {
		Group group = std::get<0>(tests.at(testIdx));
		Group other = std::get<1>(tests.at(testIdx));
		bool areEqual = std::get<2>(tests.at(testIdx));

		if(areEqual) {
			REQUIRE(group == other);
			REQUIRE_FALSE(group != other);
		} else {
			REQUIRE(group != other);
			REQUIRE_FALSE(group == other);
		}
	}
}

TEST_CASE("::typeToStr && ::strToType", "[typeToStr]") {
	SECTION ("valid type/string") {
		std::vector<std::tuple<Group::Type, const char*>> tests {
			{Group::Type::ZERO, "0"},
			{Group::Type::ENTERTAINMENT, "Entertainment"},
			{Group::Type::LIGHTSOURCE, "Lightsource"},
			{Group::Type::LIGHTGROUP, "LightGroup"},
			{Group::Type::LUMINAIRE, "Luminaire"},
			{Group::Type::ROOM, "Room"},
			{Group::Type::ZONE, "Zone"},
			{Group::Type::_DEFAULT, "LightGroup"},
		};

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
			Group::Type type = std::get<0>(tests.at(testIdx));
			const char* expectedStr = std::get<1>(tests.at(testIdx));

			CAPTURE(testIdx, type);

			const char* str = Group::typeToStr(type);
			CHECK_THAT(str, Catch::Matchers::Equals(expectedStr));

			int type_idx = Group::strToType(expectedStr);
			REQUIRE(type_idx >= 0);
			REQUIRE(static_cast<Group::Type>(type_idx) == type);
		}
	}

	SECTION("invalid ::strToType argument") {
		std::vector<std::tuple<const char*>> tests {
			{0},
			{""},
			{"Luminair"},
			{"Luminaire but longer"},
			{"Luminaires"},
		};

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
			const char* type = std::get<0>(tests.at(testIdx));

			CAPTURE(testIdx, type);

			int type_idx = Group::strToType(type);
			REQUIRE(type_idx == -1);
		}
	}
}

TEST_CASE("::classToStr && ::strToClass", "[classToStr]") {
	SECTION("valid class/string") {
		std::vector<std::tuple<Group::Class, const char*>> tests {
			{Group::Class::ATTIC, "Attic"},
			{Group::Class::BALCONY, "Balcony"},
			{Group::Class::BARBECUE, "Barbecue"},
			{Group::Class::BATHROOM, "Bathroom"},
			{Group::Class::BEDROOM, "Bedroom"},
			{Group::Class::CARPORT, "Carport"},
			{Group::Class::CLOSET, "Closet"},
			{Group::Class::COMPUTER, "Computer"},
			{Group::Class::DINING, "Dining"},
			{Group::Class::DOWNSTAIRS, "Downstairs"},
			{Group::Class::DRIVEWAY, "Driveway"},
			{Group::Class::FRONT_DOOR, "Front door"},
			{Group::Class::GARAGE, "Garage"},
			{Group::Class::GARDEN, "Garden"},
			{Group::Class::GUEST_ROOM, "Guest room"},
			{Group::Class::GYM, "Gym"},
			{Group::Class::HALLWAY, "Hallway"},
			{Group::Class::HOME, "Home"},
			{Group::Class::KIDS_BEDROOM, "Kids bedroom"},
			{Group::Class::KITCHEN, "Kitchen"},
			{Group::Class::LAUNDRY_ROOM, "Laundry room"},
			{Group::Class::LIVING_ROOM, "Living room"},
			{Group::Class::LOUNGE, "Lounge"},
			{Group::Class::MAN_CAVE, "Man cave"},
			{Group::Class::MUSIC, "Music"},
			{Group::Class::NURSERY, "Nursery"},
			{Group::Class::OFFICE, "Office"},
			{Group::Class::OTHER, "Other"},
			{Group::Class::POOL, "Pool"},
			{Group::Class::PORCH, "Porch"},
			{Group::Class::READING, "Reading"},
			{Group::Class::RECREATION, "Recreation"},
			{Group::Class::STAIRCASE, "Staircase"},
			{Group::Class::STORAGE, "Storage"},
			{Group::Class::STUDIO, "Studio"},
			{Group::Class::TV, "TV"},
			{Group::Class::TERRACE, "Terrace"},
			{Group::Class::TOILET, "Toilet"},
			{Group::Class::TOP_FLOOR, "Top floor"},
			{Group::Class::UPSTAIRS, "Upstairs"},
			{Group::Class::_DEFAULT, "Other"}
		};

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
			Group::Class roomClass = std::get<0>(tests.at(testIdx));
			const char* expectedStr = std::get<1>(tests.at(testIdx));

			CAPTURE(testIdx, roomClass);

			const char* str = Group::classToStr(roomClass);
			CHECK_THAT(str, Catch::Matchers::Equals(expectedStr));

			int class_idx = Group::strToClass(expectedStr);
			REQUIRE(class_idx >= 0);
			REQUIRE(static_cast<Group::Class>(class_idx) == roomClass);
		}
	}

	SECTION("invalid ::strToClass argument") {
		std::vector<std::tuple<const char*>> tests {
			{0},
			{""},
			{"Laundry roo"},
			{"Laundry"},
			{"Laundry room with window"},
		};

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
			const char* str = std::get<0>(tests.at(testIdx));

			CAPTURE(testIdx, str);

			int class_idx = Group::strToType(str);
			REQUIRE(class_idx == -1);
		}
	}
}

TEST_CASE("Group::getClass", "[getClass]") {
		std::vector<std::tuple<Group::Type, int>> tests {
			{Group::Type::ZERO, -1},
			{Group::Type::ENTERTAINMENT, -1},
			{Group::Type::LIGHTGROUP, -1},
			{Group::Type::LIGHTSOURCE, -1},
			{Group::Type::LUMINAIRE, -1},
			{Group::Type::ROOM, (int)Group::Class::_DEFAULT},
			{Group::Type::ZONE, -1},
		};

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
			Group::Type type = std::get<0>(tests.at(testIdx));
			int expectedTypeIdx = std::get<1>(tests.at(testIdx));

			CAPTURE(testIdx, type);

			Group group = Group(1, "agroup", {}, {}, type);
			int classIdx = group.getClass();
			REQUIRE(classIdx == expectedTypeIdx);
		}
}

TEST_CASE("Group::createAction", "[createAction]") {
    std::vector<std::tuple<String, const char*>> tests {
        {Group::createAction(), "{}"},

		// Alert property
        {Group::createAction().alert(Light::Alert::NONE), "{\"alert\":\"none\"}"},
        {Group::createAction().alert(Light::Alert::SELECT), "{\"alert\":\"select\"}"},
        {Group::createAction().alert(Light::Alert::LSELECT), "{\"alert\":\"lselect\"}"},

        // Effect property
        {Group::createAction().effect(Light::Effect::NONE), "{\"effect\":\"none\"}"},
        {Group::createAction().effect(Light::Effect::COLORLOOP), "{\"effect\":\"colorloop\"}"},

        // Other properties
        {Group::createAction().on(true), "{\"on\":true}"},
        {Group::createAction().bri(1), "{\"bri\":1}"},
        {Group::createAction().hue(1), "{\"hue\":1}"},
        {Group::createAction().sat(1), "{\"sat\":1}"},
        {Group::createAction().xy(1.2,1.2), "{\"xy\":[1.2,1.2]}"},
        {Group::createAction().ct(1), "{\"ct\":1}"},
        {Group::createAction().alert(Light::Alert::SELECT), "{\"alert\":\"select\"}"},
        {Group::createAction().effect(Light::Effect::COLORLOOP), "{\"effect\":\"colorloop\"}"},
        {Group::createAction().transitiontime(1), "{\"transitiontime\":1}"},
        {Group::createAction().bri_inc(1), "{\"bri_inc\":1}"},
        {Group::createAction().sat_inc(1), "{\"sat_inc\":1}"},
        {Group::createAction().hue_inc(1), "{\"hue_inc\":1}"},
        {Group::createAction().ct_inc(1), "{\"ct_inc\":1}"},
        {Group::createAction().xy_inc(0.1), "{\"xy_inc\":0.1}"},
        {Group::createAction().scene("abc"), "{\"scene\":\"abc\"}"},

        // Property combinations
        {Group::createAction().on(true).bri(1).hue(1).sat(1).xy(1.2,1.2).ct(1).alert(Light::Alert::SELECT).effect(Light::Effect::COLORLOOP).transitiontime(1).bri_inc(1).sat_inc(1).hue_inc(1).ct_inc(1).xy_inc(0.1).scene("abc"),
            "{\"on\":true,\"bri\":1,\"hue\":1,\"sat\":1,\"xy\":[1.2,1.2],\"ct\":1,\"alert\":\"select\",\"effect\":\"colorloop\",\"transitiontime\":1,\"bri_inc\":1,\"sat_inc\":1,\"hue_inc\":1,\"ct_inc\":1,\"xy_inc\":0.1,\"scene\":\"abc\"}"}
    };

    for(int testIdx=0; testIdx<tests.size(); testIdx++) {
        const char* actionBody = std::get<0>(tests.at(testIdx)).c_str();
        const char* expectedBody = std::get<1>(tests.at(testIdx));

        CAPTURE(testIdx, actionBody, expectedBody);
        CHECK_THAT(actionBody, Catch::Matchers::Equals(expectedBody));
    }
}