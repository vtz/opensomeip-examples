#include "config.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace examples {

static fs::path find_repo_root() {
    fs::path dir = fs::current_path();
    for (int i = 0; i < 10; ++i) {
        if (fs::exists(dir / "config") && fs::is_directory(dir / "config")) {
            return dir;
        }
        if (dir.has_parent_path() && dir.parent_path() != dir) {
            dir = dir.parent_path();
        } else {
            break;
        }
    }
    return fs::current_path();
}

std::string find_config_dir() {
    return (find_repo_root() / "config").string();
}

Config::Config(const std::string& example_name, const std::string& config_path) {
    fs::path path;
    if (!config_path.empty()) {
        path = config_path;
    } else {
        path = find_repo_root() / "config" / (example_name + ".yaml");
    }

    if (!fs::exists(path)) {
        throw std::runtime_error("Config file not found: " + path.string());
    }

    root_ = YAML::LoadFile(path.string());
}

YAML::Node Config::resolve(const std::string& dotted_path) const {
    YAML::Node node = YAML::Clone(root_);
    std::string token;
    std::istringstream stream(dotted_path);
    while (std::getline(stream, token, '.')) {
        if (!node.IsMap() || !node[token]) {
            return YAML::Node();
        }
        node = node[token];
    }
    return node;
}

std::string Config::env_override(const std::string& section,
                                  const std::string& key) const {
    std::string env_key = "OPENSOMEIP_" + section + "_" + key;
    for (auto& c : env_key) c = static_cast<char>(toupper(c));

    const char* val = std::getenv(env_key.c_str());
    return (val && *val) ? std::string(val) : "";
}

std::string Config::get_string(const std::string& section,
                                const std::string& key,
                                const std::string& default_val) const {
    std::string env = env_override(section, key);
    if (!env.empty()) return env;

    auto node = resolve(section + "." + key);
    return node.IsDefined() ? node.as<std::string>(default_val) : default_val;
}

int Config::get_int(const std::string& section,
                    const std::string& key,
                    int default_val) const {
    std::string env = env_override(section, key);
    if (!env.empty()) {
        try {
            return std::stoi(env, nullptr, 0);
        } catch (...) {
            return default_val;
        }
    }

    auto node = resolve(section + "." + key);
    return node.IsDefined() ? node.as<int>(default_val) : default_val;
}

double Config::get_double(const std::string& section,
                          const std::string& key,
                          double default_val) const {
    std::string env = env_override(section, key);
    if (!env.empty()) {
        try {
            return std::stod(env);
        } catch (...) {
            return default_val;
        }
    }

    auto node = resolve(section + "." + key);
    return node.IsDefined() ? node.as<double>(default_val) : default_val;
}

bool Config::get_bool(const std::string& section,
                      const std::string& key,
                      bool default_val) const {
    std::string env = env_override(section, key);
    if (!env.empty()) {
        return env == "1" || env == "true" || env == "yes" || env == "on";
    }

    auto node = resolve(section + "." + key);
    return node.IsDefined() ? node.as<bool>(default_val) : default_val;
}

uint16_t Config::get_uint16(const std::string& section,
                            const std::string& key,
                            uint16_t default_val) const {
    int val = get_int(section, key, static_cast<int>(default_val));
    return static_cast<uint16_t>(val);
}

}  // namespace examples
