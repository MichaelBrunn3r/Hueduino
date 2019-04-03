#include <iostream>

#define private public
#define protected public
#include <Group.h>
#include <Bridge.h>
#include <Scene.h>
#undef protected
#undef private

#include <vector>

using namespace Hueduino;

template <typename T>
std::ostream& operator << (std::ostream& stream, const std::vector<T> vec) {
    stream << "[";
    for(auto it = vec.begin(); it != vec.end(); ++it) {
        if(it != vec.begin()) stream << ",";
        stream << *it;
    }
    stream << "]";
}

///////////
// Group //
///////////

std::ostream& operator << (std::ostream& stream, const Group::Type type) { stream << Group::typeToStr(type); }

std::ostream& operator << (std::ostream& stream, const Group::Class cls) { stream << Group::classToStr(cls); }

std::ostream& operator << (std::ostream& stream, const Group& group) {
    stream << "{id: " << group.id;
    stream << ", name: \"" << group.name << "\"";
    stream << ", lights: " << group.lightIDs;
    stream << ", sensors: " << group.sensorIDs;
    stream << ", type: \"" << group.type << "\"";
    char cls = group.getClass(); 
    if(cls != -1) stream << ", class: " << (Group::Class)cls;
    stream << "}";
}

///////////
// Scene //
//////////

std::ostream& operator << (std::ostream& stream, const Scene::Type type) { stream << Scene::typeToStr(type); }

std::ostream& operator << (std::ostream& stream, const Scene& scene) {
    stream << "{id: \"" << scene.id << "\"";
    stream << ", name: \"" << scene.name << "\"";
    stream << ", type: \"" << scene.type << "\"";
    if(scene.type == Scene::Type::GROUPSCENE) stream << ", group: " << scene.groupId;
    stream << ", lights: " << scene.lightIDs;
    stream << "}";
}

////////////////////////
// BridgeCapabilities //
////////////////////////

std::ostream& operator << (std::ostream& stream, Bridge::Capabilities& cap) {
    stream << "{lights: " << cap.num_lights << "/" << cap.max_lights;
    stream << ", sensors: " << cap.num_sensors << "/" << cap.max_sensors;
    stream << ", groups: ?/" << cap.max_groups;
    stream << ", scenes: " << cap.num_scenes << "/" << cap.max_scenes << "}";
}