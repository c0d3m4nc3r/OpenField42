#include "geometry_manager.h"

#include "core/console.h"
#include "core/globals.h"
#include "core/template_manager.h"
#include "geometry/geometry.h"
#include "geometry/geometry_template.h"
#include "geometry/standard_mesh.h"
#include "geometry/tree_mesh.h"

void GeometryManager::registerCmds()
{
    g_Console->registerCmd("GeometryTemplate.create", [](Console::ExecContext& ctx, const Console::CommandArgs& args) -> CommandResult
    {
        if (args.size() < 2)
        {
            return CommandResult{ "Not enough arguments! Usage: GeometryTemplate.create <type> <name>", CommandStatus::Error };
        }

        GeometryType type = geometryTypeFromString(args[0]);

        if (type == GeometryType::Unknown)
        {
            return CommandResult{ "Unknown geometry type '" + args[0] + "'!", CommandStatus::Error };
        }

        ctx.last_geom_tmpl = g_TemplateMgr->create<GeometryTemplate>(args[1], type);

        return {};
    });


    g_Console->registerCmd("GeometryTemplate.setLodDistance", [](Console::ExecContext& ctx, const Console::CommandArgs& args) -> CommandResult {
        if (args.size() < 2)
        {
            return CommandResult{ "Not enough arguments! Usage: GeometryTemplate.setLodDistance <level> <distance>", CommandStatus::Error };
        }

        if (!ctx.last_geom_tmpl)
        {
            return CommandResult{ "No active geometry template!", CommandStatus::Error };
        }

        int level = StringUtils::fromString<int>(args[0]);
        float distance = StringUtils::fromString<float>(args[1]);

        if (ctx.last_geom_tmpl)
        {
            ctx.last_geom_tmpl->lod_distances[level] = distance;
        }

        return CommandResult{ "LOD" + args[0] + " distance set to " + args[1] + " for " + ctx.last_geom_tmpl->name };
    });

    g_Console->bindContextProperty("GeometryTemplate.file", &Console::ExecContext::last_geom_tmpl, &GeometryTemplate::file);
    g_Console->bindContextProperty("GeometryTemplate.materialMap", &Console::ExecContext::last_geom_tmpl, &GeometryTemplate::material_map);
    g_Console->bindContextProperty("GeometryTemplate.texBaseName", &Console::ExecContext::last_geom_tmpl, &GeometryTemplate::tex_base_name);
    g_Console->bindContextProperty("GeometryTemplate.detailTexName", &Console::ExecContext::last_geom_tmpl, &GeometryTemplate::detail_tex_name);
    g_Console->bindContextProperty("GeometryTemplate.materialSize", &Console::ExecContext::last_geom_tmpl, &GeometryTemplate::material_size);
    g_Console->bindContextProperty("GeometryTemplate.worldSize", &Console::ExecContext::last_geom_tmpl, &GeometryTemplate::world_size);
    g_Console->bindContextProperty("GeometryTemplate.texOffsetX", &Console::ExecContext::last_geom_tmpl, &GeometryTemplate::tex_offset_x);
    g_Console->bindContextProperty("GeometryTemplate.texOffsetY", &Console::ExecContext::last_geom_tmpl, &GeometryTemplate::tex_offset_y);
    g_Console->bindContextProperty("GeometryTemplate.waterLevel", &Console::ExecContext::last_geom_tmpl, &GeometryTemplate::water_level);
    g_Console->bindContextProperty("GeometryTemplate.yScale", &Console::ExecContext::last_geom_tmpl, &GeometryTemplate::y_scale);
}

void GeometryManager::update(int uploads_per_frame)
{
    int uploaded = 0;

    while (uploaded < uploads_per_frame)
    {
        auto geo_opt = _geometries_to_upload.pop();
        if (!geo_opt.has_value()) break;

        geo_opt.value()->upload();
    }
}

Geometry* GeometryManager::createGeometry(const GeometryTemplate* tmpl)
{
    if (!tmpl)
    {
        LOG_ERROR("GeometryManager::createGeometry: Template is NULL!");
        return nullptr;
    }

    {
        auto it = _geometries.find(tmpl->name);
        if (it != _geometries.end())
            return it->second.get();
    }

    auto [it, _] = _geometries.try_emplace(tmpl->name);
    auto& geom = it->second;

    std::string type_str = geometryTypeToString(tmpl->type);

    switch (tmpl->type)
    {
    case GeometryType::StandardMesh:
        geom = std::make_unique<StandardMesh>(); break;
    case GeometryType::TreeMesh:
        geom = std::make_unique<TreeMesh>(); break;
    default:
        LOG_ERROR("GeometryManager::createGeometry: Unsupported geometry type: %s!",
            type_str.c_str());
        _geometries.erase(it);
        return nullptr;
    }

    g_ThreadPool.enqueue([this, &geom, tmpl] {
        if (geom->load(tmpl)) {
            _geometries_to_upload.push(geom.get());
        }
    });

    geom->type = tmpl->type;

    for (size_t i = 0; i < geom->lods.size(); ++i)
        geom->lods[i].distance = tmpl->lod_distances[i];

    return geom.get();
}

Geometry* GeometryManager::getGeometry(const std::string& name)
{
    auto it = _geometries.find(name);
    if (it == _geometries.end())
        return nullptr;
    return it->second.get();
}
