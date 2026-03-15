#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

namespace examples {

class Config {
public:
    explicit Config(const std::string& example_name,
                    const std::string& config_path = "");

    std::string get_string(const std::string& section,
                           const std::string& key,
                           const std::string& default_val = "") const;

    int get_int(const std::string& section,
                const std::string& key,
                int default_val = 0) const;

    double get_double(const std::string& section,
                      const std::string& key,
                      double default_val = 0.0) const;

    bool get_bool(const std::string& section,
                  const std::string& key,
                  bool default_val = false) const;

    uint16_t get_uint16(const std::string& section,
                        const std::string& key,
                        uint16_t default_val = 0) const;

    const YAML::Node& root() const { return root_; }

private:
    YAML::Node root_;

    YAML::Node resolve(const std::string& dotted_path) const;
    std::string env_override(const std::string& section,
                             const std::string& key) const;
};

std::string find_config_dir();

}  // namespace examples
