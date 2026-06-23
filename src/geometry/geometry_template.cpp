#include "geometry/geometry_template.h"
#include "utils/string_utils.h"

GeometryTemplate& GeometryTemplate::create(const std::string& name, GeometryType type)
{
    std::string lc_name = StringUtils::lowercase(name);
    auto& tmpl = registry[lc_name];
    tmpl.name = lc_name;
    tmpl.type = type;
    current = &tmpl;
    return tmpl;
}

GeometryTemplate* GeometryTemplate::get(const std::string& name)
{
    auto it = registry.find(StringUtils::lowercase(name));
    return it != registry.end() ? &it->second : nullptr;
}
