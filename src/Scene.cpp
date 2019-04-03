#include "Scene.h"
#include <Internals/HueUtils.h>

namespace Hueduino {

    /////////////////
    // Constructor //
    /////////////////

    Scene::Scene(String id, String name, std::vector<Light::id_type> lights, Type type, Group::id_type group) :
        id(id), name(name), lightIDs(lights), type(type), groupId(group) {}

    /////////////
    // Methods //
    /////////////

    const char* Scene::typeToStrArr[] = {"GroupScene", "LightScene"};

    const char* Scene::typeToStr(Type t) {
        return typeToStrArr[static_cast<byte>(t)];
    }

    int Scene::strToType(const char* str) {
        return Internals::searchStr(str, typeToStrArr, static_cast<size_t>(Type::_MAX)+1);
    }

    ///////////////
    // Operators //
    ///////////////

    bool Scene::operator== (const Scene& other) const {
        if(type == Type::GROUPSCENE) if(groupId != other.groupId) return false;
        return id == other.id && name == other.name && type == other.type && lightIDs == other.lightIDs;
    }

    bool Scene::operator!= (const Scene& other) const { return !(*this == other); }
} // Hueduino
