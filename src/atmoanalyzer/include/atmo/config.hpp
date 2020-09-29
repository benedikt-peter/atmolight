//
// Created by benedikt on 23.03.18.
//

#pragma once

#include <string>
#include <optional>
#include <vector>
#include "types.hpp"

namespace atmo {

    /**
     * This class holds the configuration parsed from a YAML file.
     */
    class Configuration {
    public:
        /**
         * Configuration for one AtmoLight output device.
         */
        struct Device {
            /**
             * The display name of the device.
             */
            std::string name{};

            /**
             * The type of the device.
             */
            DeviceType type{};

            /**
             * The device filename of the device (e.g. /dev/ttyUSB0).
             */
            std::string filename{};

            /**
             * The number of channels supported by the device. This option is only supported by some devices (e.g.
             * devices with a variable amount of available lights), but then it is usually mandatory.
             */
            Channel channels{};

            /**
             * Try to reset the device on errors.
             */
            bool reset_on_error{false};
        };

        /**
         * Configuration for the optional control server.
         */
        struct Control {
            /**
             * The address to listen on (e.g. 127.0.0.1).
             */
            std::string address;

            /**
             * The port to listen on (e.g. 22222)
             */
            std::uint16_t port;
        };

        /**
         * Configuration for a capture device.
         */
        struct Capture {
            /**
             * Type of the capture device.
             */
            CaptureType type{};

            /**
             * Device filename of the capture device (e.g. /dev/video0). Mutually exclusive to index.
             */
            std::string filename{};

            /**
             * Device index of the capture device (e.g. 0 for the default device). Mutually exclusive to filename.
             */
            int index{-1};

            /**
             * The channel mapping configuration.
             */
            ChannelConfigs channel_configs{};
        };

        /**
         * Configuration for the optional analyzer process.
         */
        struct Analyzer {
            Capture capture;
            Mappings mappings;
        };

        /**
         * Create a new configuration instance by loading and parsing the given YAML file.
         * @param config_file
         */
        explicit Configuration(const std::string& config_file);

        /**
         * Return the configured AtmoLight devices.
         *
         * @return the AtmoLight devices
         */
        [[nodiscard]]
        const auto& devices() const {
            return m_devices;
        }

        /**
         * Return the optional control server configuration.
         * @return the optional control server configuration
         */
        [[nodiscard]]
        const auto& control() const {
            return m_control;
        }

        /**
         * Return the optional analyzer process configuration.
         * @return the optional analyzer process configuration
         */
        [[nodiscard]]
        const auto& analyzer() const {
            return m_analyzer;
        }

    private:
        std::vector<Device> m_devices;
        std::optional<Control> m_control;
        std::optional<Analyzer> m_analyzer;
    };

}
