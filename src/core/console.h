#pragma once

#include "object/object_template.h"

#include <string>
#include <string_view>
#include <functional>
#include <unordered_map>
#include <vector>

struct GeometryTemplate;
struct Object;

enum class CommandStatus : unsigned char { Success, Error, Warning, Info };

struct CommandResult
{
    std::string message = "";
    CommandStatus status = CommandStatus::Success;

    bool empty() const { return message.empty(); }
};

namespace detail
{
    template <typename F>
    struct setter_traits : setter_traits<decltype(&F::operator())> {};

    template <typename C, typename Ret, typename Arg>
    struct setter_traits<Ret(C::*)(Arg) const>
    {
        using arg_type = std::decay_t<Arg>;
    };

    template <typename C, typename Ret, typename Arg>
    struct setter_traits<Ret(C::*)(Arg)>
    {
        using arg_type = std::decay_t<Arg>;
    };
}

class Console
{
public:
    struct ExecContext
    {
        GeometryTemplate* last_geom_tmpl = nullptr;
        ObjectTemplate* last_obj_tmpl = nullptr;
        ObjectTemplate::Child* last_child = nullptr;
        Object* last_obj = nullptr;
    };

    using CommandArgs = std::vector<std::string>;
    using CommandHandler = std::function<CommandResult(ExecContext&, const CommandArgs&)>;

    void init();
    void registerCmd(const std::string& name, CommandHandler handler);
    void addAlias(const std::string& alias, const std::string& command);

    template <typename Getter, typename Setter>
    void bindProperty(
        std::string_view name,
        Getter getter,
        Setter setter
    ) {
        using ValueType = typename detail::setter_traits<Setter>::arg_type;

        registerCmd(std::string(name), makePropertyHandler<ValueType>(
            [getter]() { return getter(); },
            [setter](const ValueType& val) { setter(val); },
            name
        ));
    }

    template <typename Target, typename ValueType>
    void bindProperty(std::string_view name, Target* target, ValueType Target::* field)
    {
        bindProperty(name,
            [target, field]() { return target->*field; },
            [target, field](const ValueType& val) { target->*field = val; }
        );
    }

    template <typename Target, typename GetterRet, typename SetterArg>
    void bindProperty(std::string_view name, Target* target, GetterRet (Target::*getter)() const, void (Target::*setter)(SetterArg))
    {
        using ValueType = std::decay_t<GetterRet>;
        bindProperty(name,
            [target, getter]() { return (target->*getter)(); },
            [target, setter](const ValueType& val) { (target->*setter)(val); }
        );
    }

    template <typename Target, typename ValueType>
    void bindContextProperty(std::string_view name, Target* ExecContext::* ctx_member, ValueType Target::* field)
    {
        registerCmd(std::string(name), makeContextHandler<Target, ValueType>(ctx_member, name,
            [field](Target* t) { return t->*field; },
            [field](Target* t, const ValueType& val) { t->*field = val; }
        ));
    }

    template <typename Target, typename GetterRet, typename SetterArg>
    void bindContextProperty(std::string_view name, Target* ExecContext::* ctx_member, GetterRet (Target::*getter)() const, void (Target::*setter)(SetterArg))
    {
        using ValueType = std::decay_t<GetterRet>;
        registerCmd(std::string(name), makeContextHandler<Target, ValueType>(ctx_member, name,
            [getter](Target* t) { return (t->*getter)(); },
            [setter](Target* t, const ValueType& val) { (t->*setter)(val); }
        ));
    }

    CommandResult exec(const std::string& line, ExecContext& ctx);
    CommandResult exec(const std::string& line) { return exec(line, _main_exec_ctx); }

    static std::string joinArgs(const CommandArgs& args);
    

private:

    std::unordered_map<std::string, CommandHandler> _commands;
    std::unordered_map<std::string, std::string> _aliases;
    ExecContext _main_exec_ctx;

    template <typename ValueType, typename Getter, typename Setter>
    CommandHandler makePropertyHandler(Getter getter, Setter setter, std::string_view name) {
        return [getter, setter, name_str = std::string(name)](ExecContext&, const CommandArgs& args) -> CommandResult {
            if (args.empty()) {
                return { StringUtils::toString(getter()), CommandStatus::Success };
            }
            std::string val_str = Console::joinArgs(args);
            setter(StringUtils::fromString<ValueType>(val_str));
            return { name_str + " set to: " + val_str, CommandStatus::Success };
        };
    }

    template <typename Target, typename ValueType, typename Getter, typename Setter>
    CommandHandler makeContextHandler(Target* ExecContext::* ctx_member, std::string_view name, Getter getter, Setter setter) {
        return [ctx_member, getter, setter, name_str = std::string(name)](ExecContext& ctx, const CommandArgs& args) -> CommandResult {
            Target* target = ctx.*ctx_member;
            if (!target) {
                return { name_str + ": No active context object!", CommandStatus::Warning };
            }
            if (args.empty()) {
                return { StringUtils::toString(getter(target)), CommandStatus::Success };
            }
            std::string val_str = Console::joinArgs(args);
            setter(target, StringUtils::fromString<ValueType>(val_str));
            return { name_str + " set to: " + val_str, CommandStatus::Success };
        };
    }
};