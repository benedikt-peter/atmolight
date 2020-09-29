//
// Created by Benedikt on 20.08.2020.
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <atmo/channel.hpp>

namespace atmo {

    /**
     * A measure of width in pixels.
     */
    using Width = std::uint16_t;

    /**
     * The configuration for one screen area (top, bottom, left, right).
     */
    struct ChannelConfig {
        /**
         * The number of channels for this area.
         */
        Channel count{1};

        /**
         * The depth in pixels.
         */
        Width depth{20};

        /**
         * The number of outer pixels to crop and ignore.
         */
        Width crop{0};
    };

    /**
     * The channel configurations for all four areas.
     */
    struct ChannelConfigs {
        ChannelConfig top{};
        ChannelConfig bottom{};
        ChannelConfig left{};
        ChannelConfig right{};
    };

    /**
     * The color data for all four available areas. The number of channels per area can be configured by the user.
     */
    struct Channels {
        /**
         * The top channels.
         */
        std::vector<Color> top{};

        /**
         * The bottom channels.
         */
        std::vector<Color> bottom{};

        /**
         * The left channels.
         */
        std::vector<Color> left{};

        /**
         * The right channels.
         */
        std::vector<Color> right{};
    };

    /**
     * The device index as configured by the user (ordering in the config file).
     */
    using DeviceIndex = std::size_t;

    /**
     * Target for one single mapping.
     */
    struct Mapping {
        DeviceIndex device_index;
        Channel device_channel;
    };

    /**
     * All mappings for all four areas.
     */
    struct Mappings {
        std::vector<Mapping> top{};
        std::vector<Mapping> bottom{};
        std::vector<Mapping> left{};
        std::vector<Mapping> right{};
    };

    /**
     * The supported CaptureDevice types.
     */
    enum class CaptureType {
        /**
         * OpenCV capture and analysis.
         */
        OpenCV
    };

    /**
     * The supported AtmoDevice types.
     */
    enum class DeviceType {
        /**
         * ca.rstenpresser AtmoLight.
         */
        AtmoLight,

        /**
         * ca.rstenpresser KarateLight.
         */
        KarateLight,

        /**
         * Adafruit Dotstar.
         */
        DotStar
    };

}