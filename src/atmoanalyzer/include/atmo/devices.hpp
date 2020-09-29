//
// Created by Benedikt on 25.08.2020.
//

#pragma once

#include <utility>
#include <vector>
#include <memory>
#include <mutex>
#include "types.hpp"
#include <atmo/device.hpp>

namespace atmo {

    struct Device {
        std::string name;
        std::unique_ptr<AtmoDevice> device;
        bool reset_on_error{false};
    };

    class DeviceNotFound : public std::runtime_error {
    public:
        explicit DeviceNotFound(DeviceIndex device);
    };

    /**
     * A data structure that contains information about one device.
     */
    struct DeviceInfo {
        /**
         * Constructor.
         *
         * @param name the device name
         * @param channels the number of available channels
         */
        DeviceInfo(std::string name, size_t channels) : name{std::move(name)}, channels{channels} {}

        /**
         * The given name of the device. This name can be arbitrarily chosen by the user.
         */
        std::string name;

        /**
         * The number of available controllable channels of this device. Each channel can display one RGB color.
         */
        std::size_t channels;
    };

    /**
     * The Devices manager maintains all configured devices and manages lifetime and concurrency. All methods of this
     * class are thread-safe.
     */
    class Devices {
    public:
        /**
         * Constructor.
         *
         * @param devices the AtmoDevices to manage. The Devices manager will assume ownership of the given devices
         */
        explicit Devices(std::vector<Device> devices);

        /**
         * Return the number of available devices.
         *
         * @return the number of available devices
         */
        [[nodiscard]]
        std::size_t size() const {
            return m_devices.size();
        }

        /**
         * Return information about all available devices.
         *
         * @return information about all available devices, accessible by the device index
         */
        [[nodiscard]]
        std::vector<DeviceInfo> devices() const;

        /**
         * Set the channels of all devices to black (R: 0, G: 0, B: 0).
         */
        void clear();

        /**
         * Return the color of the given channel.
         *
         * @param device the device index
         * @param channel the channel index
         * @return the color
         */
        Color getChannel(DeviceIndex device, Channel channel);

        /**
         * Return the colors of all channels of a device.
         *
         * @param device the device index
         * @return a vector of colors, accessible by their channel indices
         */
        std::vector<Color> getChannels(DeviceIndex device);

        /**
         * Set the current color of a specific channel.
         *
         * @param device the device index
         * @param channel the channel index
         * @param color the new color
         */
        void setChannel(DeviceIndex device, Channel channel, Color color);

        /**
         * Set all channels of a device to the given colors, ordered by their channel index.
         *
         * @param device the device index
         * @param channels the colors for the device channels, ordered by channel index
         */
        void setChannels(DeviceIndex device, gsl::span<const Color> channels);

    private:
        std::mutex m_mutex;
        std::vector<Device> m_devices;

        Device& getDevice(DeviceIndex device);
    };

}

