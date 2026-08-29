#pragma once

#include "utils/string_utils.h"
#include <mutex>
#include <typeindex>

class TemplateManager
{
public:

    template<typename T, typename... Args>
    T* create(const std::string& name, Args&&... args)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        std::string key = StringUtils::lowercase(name);
        
        auto& storage = getStorage<T>();
        auto it = storage.find(key);
        if (it != storage.end()) {
            return it->second.get();
        }

        storage[key] = std::make_unique<T>(name, std::forward<Args>(args)...);
        return storage[key].get();
    }

    template<typename T>
    T* get(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        std::string key = StringUtils::lowercase(name);
        
        auto& storage = getStorage<T>();
        auto it = storage.find(key);
        if (it != storage.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _stores.clear();
    }

private:
    struct IStorage { virtual ~IStorage() = default; };

    template<typename T>
    struct Storage : public IStorage {
        std::unordered_map<std::string, std::unique_ptr<T>> map;
    };

    template<typename T>
    std::unordered_map<std::string, std::unique_ptr<T>>& getStorage()
    {
        auto type_key = std::type_index(typeid(T));
        auto it = _stores.find(type_key);
        if (it == _stores.end()) {
            it = _stores.emplace(type_key, std::make_unique<Storage<T>>()).first;
        }
        return static_cast<Storage<T>*>(it->second.get())->map;
    }

    mutable std::mutex _mutex;
    std::unordered_map<std::type_index, std::unique_ptr<IStorage>> _stores;
};