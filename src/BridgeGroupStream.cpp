#include "Bridge.h"
#include <Internals/JsonUtils.h>

#define inBounds(x, lower, upper) (x >= lower && x <= upper)

namespace Hueduino {
    GroupStream::GroupStream(HTTPClient& http, WiFiClient& client, const char* base_url) : mHTTP(http) {
        String url = base_url;
        url += "/groups";
        mHTTP.begin(client, url);
        mHTTP.GET();

        mStream = new Internals::TimedStreamWrapper(client);
        mParser = JStream::JsonParser(*mStream);

        // Enter root json object
        if(!mParser.enterObj()) reachedEOF = true;
    }

    GroupStream::~GroupStream() {
        if(mStream) delete mStream;
    }

    Group* GroupStream::next() {
        if(reachedEOF) return nullptr; // Already determined that no more groups exist in prededing call

        if(!mHTTP.connected()) {
            reachedEOF = true;
            return nullptr;
        }

        String idBuf = "";
        while(mParser.nextKey(&idBuf)) { // Test if another key and hence another group exists
            long groupID = JStream::Internals::stol(idBuf.c_str(), -1);
            if(!inBounds(groupID, min_id, max_id)) continue; // Filter id

            // Parse group attributes
            if(!mParser.enterObj()) continue; // Try entering group, else try next group
            {
                // Group name
                if(!mParser.findKey("name")) goto PARSING_FAILED;
                String groupName = "";
                if(!mParser.readString(groupName)) goto PARSING_FAILED;
                if(filterName) { // Filter name
                    auto it=searchedNames.begin();
                    for(; it!=searchedNames.end(); ++it) { if(groupName == *it) break; }
                    if(it == searchedNames.end()) goto PARSING_FAILED;
                }

                // lights
                if(!mParser.findKey("lights")) goto PARSING_FAILED;
                std::vector<Light::id_type> lights;
                if(!mParser.parseIntArray<Light::id_type>(lights)) goto PARSING_FAILED;
                lights.shrink_to_fit();

                // sensors
                if(!mParser.findKey("sensors")) goto PARSING_FAILED;
                std::vector<unsigned int> sensors;
                if(!mParser.parseIntArray<unsigned int>(sensors)) goto PARSING_FAILED;
                sensors.shrink_to_fit();

                // type
                if(!mParser.findKey("type")) goto PARSING_FAILED;
                String type_str = "";
                if(!mParser.readString(type_str)) goto PARSING_FAILED;
                int type_idx = Group::strToType(type_str.c_str());
                if(type_idx < 0) goto PARSING_FAILED;
                Group::Type type = static_cast<Group::Type>(type_idx);
                
                // Filter type
                if(filterType) if(searchedTypes.find(type) == searchedTypes.end()) goto PARSING_FAILED;

                // class
                Group::Class cls = Group::Class::_DEFAULT;
                if(type == Group::Type::ROOM) {
                    if(!mParser.findKey("class")) goto PARSING_FAILED;
                    String class_name = "";
                    if(!mParser.readString(class_name)) goto PARSING_FAILED;
                    int class_idx = Group::strToClass(class_name.c_str());
                    if(class_idx < 0) goto PARSING_FAILED;
                    cls = static_cast<Group::Class>(class_idx);
                }

                // Group was parsed successfully -> clean up and return group
                mParser.exitCollection(); // Exit group
                return new Group(groupID, groupName, lights, sensors, type, cls); 
            }

            // Failed to parse group -> clean up and try the next group
            PARSING_FAILED:
            mParser.exitCollection(); // Exit group
            continue;
        }

        // No more groups exist
        reachedEOF = true;
        return nullptr;
    }

    GroupStream& GroupStream::filterIDs(Group::id_type min_id, Group::id_type max_id) {
        this->min_id = min_id;
        this->max_id = max_id;
        return *this; 
    }

    GroupStream& GroupStream::filterNames(std::vector<const char*> names) {
        if(names.size() < 1) return *this;
        
        filterName = true;
        searchedNames = names;
        return *this;
    }

    GroupStream& GroupStream::filterTypes(std::set<Group::Type> types) {
        if(types.size() < 1) return *this;

        filterType = true;
        searchedTypes = types;
        return *this;
    }

    std::vector<std::unique_ptr<Group>> GroupStream::collect() {
        std::vector<std::unique_ptr<Group>> groups;
        Group* group = next();
        while(group != nullptr) {
            groups.push_back(std::unique_ptr<Group>(group));
            group = next();
        }
        return groups;
    }
} // Hueduino

#undef inBounds