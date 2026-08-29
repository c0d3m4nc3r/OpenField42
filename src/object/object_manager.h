#pragma once

#include "object/object_template.h"

class ObjectManager
{
public:

    ObjectTemplate& createTemplate(const std::string& name, ObjectType type);
    ObjectTemplate* getTemplate(const std::string& name);

private:

    std::unordered_map<std::string, ObjectTemplate> _templates;
};
