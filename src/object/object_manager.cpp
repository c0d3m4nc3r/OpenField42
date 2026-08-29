#include "object/object_manager.h"

ObjectTemplate& ObjectManager::createTemplate(const std::string& name, ObjectType type)
{
    std::string lc_name = StringUtils::lowercase(name);
    auto& tmpl = _templates[lc_name];
    tmpl.name = lc_name;
    tmpl.type = type;
    return tmpl;
}

ObjectTemplate* ObjectManager::getTemplate(const std::string& name)
{
    auto it = _templates.find(StringUtils::lowercase(name));
    if (it == _templates.end())
        return nullptr;
    return &it->second;
}
