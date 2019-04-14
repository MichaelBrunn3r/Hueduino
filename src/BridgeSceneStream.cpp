#include "Bridge.h"
#include <Internals/JsonUtils.h>

namespace Hueduino {
    SceneStream::SceneStream(WiFiClient& client) : mStream(Internals::WiFiClientWrapper(client)) {
        mParser = JStream::JsonParser(mStream);
        if(!mParser.enterObj()) mEnded = true;
    }

    Scene* SceneStream::next() {
        if(mEnded) return nullptr;

        String id = "";
        while(mParser.nextKey(&id)) { // Test if another key and hence another scene exists
            if(mFilterIDs) { // filter IDs
                auto it = mSearchedIDs.begin();
                for(; it != mSearchedIDs.end(); ++it) { if(id == *it) break; }
                if(it == mSearchedIDs.end()) continue;
            }

            // Parse scene attributes
            if(!mParser.enterObj()) continue; // Try entering scene, else try next scene
            {
                // name //
                if(!mParser.findKey("name")) goto PARSING_FAILED;
                String name = "";
                if(!mParser.readString(name)) goto PARSING_FAILED;

                if(mFilterNames) { // filter name
                    auto it = mSearchedNames.begin();
                    for(; it != mSearchedNames.end(); ++it) { if(name == *it) break; }
                    if(it == mSearchedNames.end()) goto PARSING_FAILED;
                }
                
                // type //
                if(!mParser.findKey("type")) goto PARSING_FAILED;
                String type_str = "";
                if(!mParser.readString(type_str)) goto PARSING_FAILED;
                int type_idx = Scene::strToType(type_str.c_str());
                if(type_idx < 0) goto PARSING_FAILED;
                Scene::Type type = static_cast<Scene::Type>(type_idx);
                if(mFilterType && type != mSearchedType) goto PARSING_FAILED; // filter name

                // groupID //
                long group = 0;
                if(type == Scene::Type::GROUPSCENE) {
                    if(!mParser.findKey("group")) goto PARSING_FAILED;
                    String group_str = "";
                    if(!mParser.readString(group_str)) goto PARSING_FAILED;
                    group = JStream::Internals::stol(group_str.c_str(), -1);
                    if(group < 0) goto PARSING_FAILED;

                    if(mFilterGroups) { // filter group IDs
                        auto it = mSearchedGroupIDs.begin();
                        for(; it != mSearchedGroupIDs.end(); ++it) { if(group == *it) break; }
                        if(it == mSearchedGroupIDs.end()) goto PARSING_FAILED;
                    }
                }

                // lights //
                if(!mParser.findKey("lights")) goto PARSING_FAILED;
                std::vector<Light::id_type> lights;
                if(!mParser.parseIntArray<Light::id_type>(lights)) goto PARSING_FAILED;
                if(lights.size() < 1) goto PARSING_FAILED;
                lights.shrink_to_fit();

                // Scene was parsed successfully -> clean up and return scene
                mParser.exitCollection(); // Exit scene
                return new Scene(id, name, lights, type, group);
            }

            // Failed to parse scene -> clean up and try the next scene
            PARSING_FAILED:
            mParser.exitCollection(); // Exit scene
            continue;
        }

        // No more scenes exist
        mEnded = true;
        return nullptr;
    }

    SceneStream& SceneStream::filterIDs(std::vector<const char*> IDs) {
        if(IDs.size() < 1) return *this;
        
        mFilterIDs = true;
        mSearchedIDs = IDs;
        return *this;
    }

    SceneStream& SceneStream::filterNames(std::vector<const char*> names) {
        if(names.size() < 1) return *this;

        mFilterNames = true;
        mSearchedNames = names;
        return *this;
    }

    SceneStream& SceneStream::filterType(Scene::Type type) {
        mFilterType = true;
        mSearchedType = type;
        return *this;
    }

    SceneStream& SceneStream::filterGroups(std::vector<Group::id_type> groupIDs) {
        if(groupIDs.size() < 1) return *this;

        // Filtering group IDs implies that Scenes are of type Type::GROUPSCENE
        mFilterType = true;
        mSearchedType = Scene::Type::GROUPSCENE;

        mFilterGroups = true;
        mSearchedGroupIDs = groupIDs;
        return *this;
    }

    std::vector<std::unique_ptr<Scene>> SceneStream::collect() {
        std::vector<std::unique_ptr<Scene>> scenes;
        Scene* scene = next();
        while(scene != nullptr) {
            scenes.push_back(std::unique_ptr<Scene>(scene));
            scene = next();
        }
        return scenes;
    }
} // Hueduino