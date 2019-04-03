#include "Group.h"
#include <Internals/HueUtils.h>

namespace Hueduino {

    /////////////////
    // Constructor //
    /////////////////

    Group::Group(unsigned int id, String name, std::vector<Light::id_type> lights, std::vector<unsigned int> sensors, Type type, Class cls) : 
        id(id), name(name), lightIDs(lights), sensorIDs(sensors), type(type), cls(cls) {}

    /////////////
    // Methods //
    /////////////

    const char* Group::typeToStrArr[] = {"0", "Entertainment", "LightGroup", "Lightsource", "Luminaire", "Room", "Zone"};
    const char* Group::typeToStr(Type t) {
        return typeToStrArr[static_cast<byte>(t)];
    }

    int Group::strToType(const char* str) {
        return Internals::searchStr(str, typeToStrArr, static_cast<size_t>(Type::_MAX)+1);
    }

    const char* Group::classToStrArr[] = {"Attic", "Balcony", "Barbecue", "Bathroom", "Bedroom", "Carport", "Closet", "Computer", "Dining", "Downstairs", 
        "Driveway", "Front door", "Garage", "Garden", "Guest room", "Gym", "Hallway", "Home", "Kids bedroom", "Kitchen", "Laundry room", "Living room", 
        "Lounge", "Man cave", "Music", "Nursery", "Office", "Other", "Pool", "Porch", "Reading", "Recreation", "Staircase", "Storage", "Studio", "TV", 
        "Terrace", "Toilet", "Top floor", "Upstairs"};

    const char* Group::classToStr(Class c) {
        return classToStrArr[static_cast<byte>(c)];
    }

    int Group::strToClass(const char* str) {
        return Internals::searchStr(str, classToStrArr, static_cast<size_t>(Class::_MAX)+1);
    }

    ActionStrBuilder Group::createAction() { return ActionStrBuilder(); }

    char Group::getClass() const {
        if(type != Type::ROOM) return -1;
        return (int)cls;
    }

    ///////////////
    // Operators //
    ///////////////

    bool Group::operator== (const Group& other) const { 
        return id == other.id && name == other.name && lightIDs == other.lightIDs && sensorIDs == other.sensorIDs && type == other.type && getClass() == other.getClass(); 
    }
    bool Group::operator!= (const Group& other) const { return !(*this == other); }
} // Hueduino