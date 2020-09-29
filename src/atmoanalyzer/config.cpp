//
// Created by benedikt on 23.03.18.
//

#include <atmo/config.hpp>

#include <yaml-cpp/yaml.h>
#include <fmt/format.h>

namespace {

    using namespace atmo;

    auto getRequiredNode(const YAML::Node& parent, const std::string& node_name) {
        if (!parent[node_name]) {
            throw std::runtime_error{fmt::format("Missing required YAML node '{}'", node_name)};
        }
        return parent[node_name];
    }

    template<class Type>
    auto readRequired(const YAML::Node& parent, const std::string& node_name) {
        const auto node = getRequiredNode(parent, node_name);
        return node.as<Type>();
    }

    template<class Type>
    auto readOptional(const YAML::Node& parent, const std::string& node_name, Type default_value) {
        const auto node = parent[node_name];
        if (!node) {
            return default_value;
        }
        return node.as<Type>();
    }

    auto readRequiredCaptureType(const YAML::Node& parent, const std::string& node_name) {
        const auto type = readRequired<std::string>(parent, node_name);
        if (type == "opencv") {
            return CaptureType::OpenCV;
        } else {
            throw std::runtime_error{fmt::format("Illegal capture type: '{}'", type)};
        }
    }

    auto readChannelConfig(const YAML::Node& config_node) {
        ChannelConfig config{};
        if (config_node) {
            config.count = readOptional<Channel>(config_node, "count", 0);
            config.depth = readOptional<Width>(config_node, "depth", 20);
            config.crop = readOptional<Width>(config_node, "crop", 0);
        }
        return config;
    }

    auto readCaptureConfig(const YAML::Node& parent) {
        const auto capture_node = getRequiredNode(parent, "capture");
        Configuration::Capture capture{};
        capture.type = readRequiredCaptureType(capture_node, "type");
        capture.filename = readOptional<std::string>(capture_node, "filename", "");
        capture.index = readOptional<int>(capture_node, "index", -1);
        capture.channel_configs.top = readChannelConfig(capture_node["top"]);
        capture.channel_configs.bottom = readChannelConfig(capture_node["bottom"]);
        capture.channel_configs.left = readChannelConfig(capture_node["left"]);
        capture.channel_configs.right = readChannelConfig(capture_node["right"]);
        return capture;
    }

    auto readRequiredDeviceType(const YAML::Node& parent, const std::string& node_name) {
        const auto type = readRequired<std::string>(parent, node_name);
        if (type == "atmolight") {
            return DeviceType::AtmoLight;
        } else if (type == "karatelight") {
            return DeviceType::KarateLight;
        } else if (type == "dotstar") {
            return DeviceType::DotStar;
        } else {
            throw std::runtime_error{fmt::format("Illegal device type: '{}'", type)};
        }
    }

    auto readDevice(const YAML::Node& device_node) {
        Configuration::Device device{};
        device.name = readRequired<std::string>(device_node, "name");
        device.type = readRequiredDeviceType(device_node, "type");
        device.filename = readRequired<std::string>(device_node, "filename");
        device.channels = readOptional<Channel>(device_node, "channels", 0);
        device.reset_on_error = readOptional(device_node, "reset_on_error", false);
        return device;
    }

    auto readDevices(const YAML::Node& parent) {
        const auto devices_node = getRequiredNode(parent, "devices");
        std::vector<Configuration::Device> devices{};
        for (const auto device_node : devices_node) {
            devices.push_back(readDevice(device_node));
        }
        return devices;
    }

    auto readControl(const YAML::Node& parent) {
        const auto control_node = parent["control"];
        if (!control_node) {
            return std::optional<Configuration::Control>{};
        }
        Configuration::Control control{};
        control.address = readRequired<std::string>(control_node, "address");
        control.port = readRequired<std::uint16_t>(control_node, "port");
        return std::optional<Configuration::Control>{control};
    }

    auto readMapping(const YAML::Node& mapping_node) {
        Mapping mapping{};
        mapping.device_index = readRequired<DeviceIndex>(mapping_node, "device_index");
        mapping.device_channel = readRequired<Channel>(mapping_node, "device_channel");
        return mapping;
    }

    std::vector<Mapping> readMappings(const YAML::Node& mappings_node) {
        std::vector<Mapping> mappings{};
        for (const auto mapping : mappings_node) {
            mappings.push_back(readMapping(mapping));
        }
        return mappings;
    }

    auto readAllMappings(const YAML::Node& parent) {
        const auto mappings_node = getRequiredNode(parent, "mappings");
        Mappings mappings{};
        mappings.top = readMappings(mappings_node["top"]);
        mappings.bottom = readMappings(mappings_node["bottom"]);
        mappings.left = readMappings(mappings_node["left"]);
        mappings.right = readMappings(mappings_node["right"]);
        return mappings;
    }

    auto readAnalyzerConfig(const YAML::Node& parent) {
        const auto analyzer_node = parent["analyzer"];
        if (!analyzer_node) {
            return std::optional<Configuration::Analyzer>{};
        }
        Configuration::Analyzer analyzer{};
        analyzer.capture = readCaptureConfig(analyzer_node);
        analyzer.mappings = readAllMappings(analyzer_node);
        return std::optional<Configuration::Analyzer>{analyzer};
    }

}

namespace atmo {

    Configuration::Configuration(const std::string& config_file) {
        const auto config = YAML::LoadFile(config_file);
        m_devices = readDevices(config);
        m_control = readControl(config);
        m_analyzer = readAnalyzerConfig(config);
    }

}