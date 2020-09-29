//
// Created by Benedikt on 25.08.2020.
//

#include <atmo/devices.hpp>

#include <thread>
#include <spdlog/spdlog.h>

namespace {

    using namespace atmo;

    constexpr uint8_t MAX_RETRIES = 7;

    void reset(Device& device) {
        try {
            device.device->reset();
        } catch (const std::exception& e) {
            spdlog::warn("Resetting device '{}' failed: {}", device.name, e.what());
        }
    }

    template<class Method, class... Args>
    auto executeWithRetry(Device& device, Method method, Args&& ... args) {
        uint8_t retry{0};
        while (true) {
            try {
                return std::invoke(method, *device.device, std::forward<Args>(args)...);
            } catch (const DeviceNotFound&) {
                // Do not retry on illegal device index.
                throw;
            } catch (const std::exception& e) {
                if (!device.reset_on_error) {
                    spdlog::error("Device '{}' failed: {}", device.name, e.what());
                    throw;
                }

                if (retry >= MAX_RETRIES) {
                    spdlog::error("Giving up on device '{}' error: {}", device.name, e.what());
                    throw;
                }

                std::chrono::seconds wait_time{1U << retry};
                spdlog::warn("Device '{}' failed: {}. Resetting device and retrying in {} seconds...",
                             device.name, e.what(), wait_time.count());
                std::this_thread::sleep_for(wait_time);

                reset(device);

                ++retry;
            }
        }
    }
}

namespace atmo {

    Devices::Devices(std::vector<Device> devices) :
            m_mutex{},
            m_devices{std::move(devices)} {}

    std::vector<DeviceInfo> Devices::devices() const {
        std::vector<DeviceInfo> devices{};
        devices.reserve(m_devices.size());
        for (const auto& device : m_devices) {
            devices.emplace_back(device.name, device.device->channels());
        }
        return devices;
    }

    void Devices::clear() {
        for (auto& device : m_devices) {
            executeWithRetry(device, &AtmoDevice::clear);
        }
    }

    Color Devices::getChannel(DeviceIndex device, Channel channel) {
        std::lock_guard<std::mutex> lock{m_mutex};
        return executeWithRetry(getDevice(device), &AtmoDevice::getChannel, channel);
    }

    std::vector<Color> Devices::getChannels(DeviceIndex device) {
        std::lock_guard<std::mutex> lock{m_mutex};
        return executeWithRetry(getDevice(device), &AtmoDevice::getChannels);
    }

    void Devices::setChannel(DeviceIndex device, Channel channel, Color color) {
        std::lock_guard<std::mutex> lock{m_mutex};
        executeWithRetry(getDevice(device), &AtmoDevice::setChannel, channel, color);
    }

    void Devices::setChannels(DeviceIndex device, gsl::span<const Color> channels) {
        std::lock_guard<std::mutex> lock{m_mutex};
        executeWithRetry(getDevice(device), &AtmoDevice::setChannels, channels);
    }

    Device& Devices::getDevice(DeviceIndex device) {
        if (device >= m_devices.size()) {
            throw DeviceNotFound{device};
        }
        return m_devices[device];
    }

    DeviceNotFound::DeviceNotFound(DeviceIndex device) :
            std::runtime_error{fmt::format("Invalid device index: {}", device)} {}
}