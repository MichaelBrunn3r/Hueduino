#pragma once

#include <vector>
#include <memory>

#include <WString.h>
#include <Internals/Types.h>
#include <Group.h>
#include <Light.h>

namespace Hueduino {
    class Scene {
        public:
            enum class Type : byte {GROUPSCENE, LIGHTSCENE, _MAX=LIGHTSCENE, _DEFAULT=LIGHTSCENE};
            static const char* typeToStr(Type t);
            static int strToType(const char* str);

            String id;
            String name;
            Type type;
            Group::id_type groupId; // The id of the group the Scene is linked to if its type is Type::GROUPSCENE
            std::vector<Light::id_type> lightIDs;

            /**
             * @brief Construct a new Scene object
             * 
             * @param id The unique id of the Scene 
             * @param name The name of the Scene
             * @param lights The light IDs in the Scene. Must contain at least 1 id 
             * @param type The type of the Scene. Defaults to Type::LIGHTSCENE
             * @param group The id of the group the Scene is linked to if its type is Type::GROUPSCENE
             */
            Scene(String id, String name, std::vector<Light::id_type> lights, Type type = Type::_DEFAULT, Group::id_type group = 0);

            bool operator== (const Scene& other) const;
            bool operator!= (const Scene& other) const;

        private:
            static const char* typeToStrArr[(size_t)Type::_MAX + 1];
    };
} // Hueduino