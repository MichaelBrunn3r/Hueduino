#pragma once

#include <vector>
#include <set>
#include <limits.h>

#include <ESP8266HTTPClient.h>
#include <WString.h>

#include <Group.h>
#include <Scene.h>
#include <JsonParser.h>
#include <Internals/WiFiClientWrapper.h>

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

            bool requestGroups(HTTPClient& http, WiFiClient& client);
            bool requestScenes(HTTPClient& http, WiFiClient& client);

            GroupStream parseGroupStream(WiFiClient& client);
            SceneStream parseSceneStream(WiFiClient& client);
    };

    class GroupStream {
        public:
            GroupStream(WiFiClient& client);
            
            Group* next();

            GroupStream& filterIDs(Group::id_type min_id, Group::id_type max_id);
            GroupStream& filterNames(std::vector<const char*> names);
            GroupStream& filterTypes(std::set<Group::Type> types);
            std::vector<std::unique_ptr<Group>> collect();

        protected:
            bool mEnded = false;
            Internals::WiFiClientWrapper mStream;
            JStream::JsonParser mParser;

            Group::id_type min_id = 0;
            Group::id_type max_id = UINT_MAX;

            bool filterName = false;
            std::vector<const char*> searchedNames;

            bool filterType = false;
            std::set<Group::Type> searchedTypes;
    };

    class SceneStream {
        public:
            SceneStream(WiFiClient& client);

            Scene* next();

            SceneStream& filterIDs(std::vector<const char*> IDs);
            SceneStream& filterNames(std::vector<const char*> names);
            SceneStream& filterType(Scene::Type type);
            SceneStream& filterGroups(std::vector<Group::id_type> groupIDs);
            std::vector<std::unique_ptr<Scene>> collect();
        protected:
            bool mEnded = false;
            JStream::JsonParser mParser;
            Internals::WiFiClientWrapper mStream;

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