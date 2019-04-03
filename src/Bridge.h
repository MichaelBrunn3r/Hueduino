#pragma once

#include <vector>
#include <set>
#include <limits.h>

#include <ESP8266HTTPClient.h>
#include <WString.h>

#include <Group.h>
#include <Scene.h>
#include <JsonParser.h>
#include <Internals/TimedStreamWrapper.h>

namespace Hueduino {
    class GroupStream;
    class SceneStream;

    class Bridge {
        public:
            struct Capabilities {
                unsigned int max_lights=0;
                unsigned int num_lights=0;

                unsigned int max_sensors=0;
                unsigned int num_sensors=0;
                
                unsigned int max_groups=0;

                unsigned int max_scenes=0;
                unsigned int num_scenes=0;

                static Capabilities get(HTTPClient& http, WiFiClient& client, const char* base_url);
            };

        public:
            String base_url;

            Bridge(const char* ip, const char* apiKey, bool https=false);

            GroupStream getGroups(HTTPClient& http, WiFiClient& client);
            SceneStream getScenes(HTTPClient& http, WiFiClient& client);
    };

    class GroupStream {
        public:
            GroupStream(HTTPClient& http, WiFiClient& client, const char* base_url);
            ~GroupStream();
            
            Group* next();

            GroupStream& filterIDs(Group::id_type min_id, Group::id_type max_id);
            GroupStream& filterNames(std::vector<const char*> names);
            GroupStream& filterTypes(std::set<Group::Type> types);
            std::vector<std::unique_ptr<Group>> collect();

        private:
            HTTPClient& mHTTP;
            JStream::JsonParser mParser;
            Internals::TimedStreamWrapper* mStream = nullptr;
            bool reachedEOF = false; // No more groups possible

            Group::id_type min_id = 0;
            Group::id_type max_id = UINT_MAX;

            bool filterName = false;
            std::vector<const char*> searchedNames;

            bool filterType = false;
            std::set<Group::Type> searchedTypes;
    };

    class SceneStream {
        public:
            SceneStream(HTTPClient& http, WiFiClient& client, const char* base_url);
            ~SceneStream();

            Scene* next();

            SceneStream& filterIDs(std::vector<const char*> IDs);
            SceneStream& filterNames(std::vector<const char*> names);
            SceneStream& filterType(Scene::Type type);
            SceneStream& filterGroups(std::vector<Group::id_type> groupIDs);
            std::vector<std::unique_ptr<Scene>> collect();
        private:
            HTTPClient& mHTTP;
            JStream::JsonParser mParser;
            Internals::TimedStreamWrapper* mStream = nullptr;
            bool mReachedEOF = false; // No more scenes possible

            bool mFilterIDs = false;
            std::vector<const char*> mSearchedIDs;

            bool mFilterNames = false;
            std::vector<const char*> mSearchedNames;

            bool mFilterType = false;
            Scene::Type mSearchedType = Scene::Type::_DEFAULT;

            bool mFilterGroups = false;
            std::vector<Group::id_type> mSearchedGroupIDs;
    };
}