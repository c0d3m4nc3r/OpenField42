#pragma once

#include "object/object_type.h"

#include <string>
#include <vector>

struct ObjectTemplate
{
    ObjectTemplate(const std::string& name, ObjectType type = ObjectType::Unknown)
        : name(name), type(type) {}

    struct Child
    {
        std::string tmpl_name;
        glm::vec3 position;
        glm::vec3 rotation;
    };

    std::string name;
    ObjectType type;
    
    std::string geometry;
    glm::vec3 continous_rot_speed = glm::vec3(0.0f); 

    std::vector<Child> children;
};
