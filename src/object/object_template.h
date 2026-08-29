#pragma once

#include "object/object.h"

#include <string>
#include <vector>

struct ObjectTemplate
{
    struct Child
    {
        std::string tmpl_name;
        glm::vec3 position;
        glm::vec3 rotation;
    };

    std::string name;
    ObjectType type = ObjectType::Unknown;
    
    std::string geometry;
    glm::vec3 continous_rot_speed = glm::vec3(0.0f); 

    std::vector<Child> children;
    
    static inline ObjectTemplate* current = nullptr;
    static inline Child* last_added_child = nullptr;
};
