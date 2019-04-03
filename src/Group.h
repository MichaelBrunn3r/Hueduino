#pragma once

#include <vector>
#include <memory>

#include <WString.h>

#include <Internals/Types.h>
#include <Light.h>

namespace Hueduino {
    class ActionStrBuilder;

    class Group {
        public:
            enum class Type : byte {ZERO, ENTERTAINMENT, LIGHTGROUP, LIGHTSOURCE, LUMINAIRE, ROOM, ZONE, _MAX=ZONE, _DEFAULT=LIGHTGROUP}; 
            static const char* typeToStr(Type t);
            static int strToType(const char* str);

            enum class Class : byte {ATTIC, BALCONY, BARBECUE, BATHROOM, BEDROOM, CARPORT, CLOSET, COMPUTER, DINING, 
                DOWNSTAIRS, DRIVEWAY, FRONT_DOOR, GARAGE, GARDEN, GUEST_ROOM, GYM, HALLWAY, HOME, KIDS_BEDROOM, KITCHEN, 
                LAUNDRY_ROOM, LIVING_ROOM, LOUNGE, MAN_CAVE, MUSIC, NURSERY, OFFICE, OTHER, POOL, PORCH, READING, RECREATION, 
                STAIRCASE, STORAGE, STUDIO, TV, TERRACE, TOILET, TOP_FLOOR, UPSTAIRS, _MAX=UPSTAIRS, _DEFAULT=OTHER};
            static const char* classToStr(Class c);
            static int strToClass(const char* str);

            typedef unsigned int id_type;

            id_type id;
            String name;
            std::vector<Light::id_type> lightIDs;
            std::vector<unsigned int> sensorIDs;
            Type type;

            Group(id_type id, String name, std::vector<Light::id_type> lights, std::vector<unsigned int> sensors, Type type = Type::_DEFAULT, Class cls = Class::_DEFAULT);
 
            static ActionStrBuilder createAction();
            /**
             * @brief Returns the class of the group
             * @return char The class if the group is a room, -1 otherwise
             */
            char getClass() const;

            bool operator== (const Group& other) const;
            bool operator!= (const Group& other) const;
        
        private:
            static const char* typeToStrArr[(size_t)Type::_MAX + 1];
            static const char* classToStrArr[(size_t)Class::_MAX + 1];
            Class cls;
    };

    class ActionStrBuilder {
        public:
            ActionStrBuilder& on(bool on);
            ActionStrBuilder& bri(Light::bri_type bri);
            ActionStrBuilder& hue(Light::hue_type hue);
            ActionStrBuilder& sat(Light::sat_type sat);
            ActionStrBuilder& xy(Light::xy_type x, Light::xy_type y);
            ActionStrBuilder& ct(Light::ct_type ct);
            ActionStrBuilder& alert(Light::Alert alert);
            ActionStrBuilder& effect(Light::Effect effect);
            ActionStrBuilder& transitiontime(uint16_t transitiontime);
            ActionStrBuilder& bri_inc(short bri_inc);
            ActionStrBuilder& sat_inc(short sat_inc);
            ActionStrBuilder& hue_inc(short hue_inc);
            ActionStrBuilder& ct_inc(short ct_inc);
            ActionStrBuilder& xy_inc(float xy_inc, unsigned short decimalPlaces = 8);
            ActionStrBuilder& scene(const char* scene);

            operator String&&();

        private:
            friend class Group;
            String action = "{";
            bool hasAtLeastOneArg = false;

            ActionStrBuilder() {}
    };
} // Hueduino
