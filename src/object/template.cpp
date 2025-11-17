#include "object/template.h"
#include "core/log.h"
#include "utils/string_utils.h"

ObjectTemplate& ObjectTemplate::create(const std::string& name, ObjectType type)
{
    std::string lc_name = StringUtils::lowercase(name);
    auto& tmpl = registry[lc_name];
    tmpl.name = lc_name;
    tmpl.type = type;
    current = &tmpl;
    return tmpl;
}

ObjectTemplate* ObjectTemplate::get(const std::string& name)
{
    auto it = registry.find(StringUtils::lowercase(name));
    return it != registry.end() ? &it->second : nullptr;
}
