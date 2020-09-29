//
// Created by Benedikt on 21.08.2020.
//

#include <spdlog/spdlog.h>
#include <atmo/analyzer.hpp>

namespace {

    using namespace atmo;

    auto createDeviceChannels(Devices& devices) {
        std::vector<std::vector<Color>> channels_per_device{};
        channels_per_device.reserve(devices.size());
        for (const auto& device : devices.devices()) {
            auto& device_channels = channels_per_device.emplace_back();
            device_channels.resize(device.channels);
        }
        return channels_per_device;
    }

}

namespace atmo {

    Analyzer::Analyzer(std::unique_ptr<CaptureDevice> capture_device,
                       Devices& devices,
                       Mappings mappings) :
            m_interrupted{false},
            m_capture_device{std::move(capture_device)},
            m_devices{&devices},
            m_mappings{std::move(mappings)},
            m_channels_per_device{createDeviceChannels(devices)},
            m_worker{[this]() { run(); }} {}

    Analyzer::~Analyzer() {
        m_interrupted = true;
        if (m_worker.joinable()) {
            m_worker.join();
        }
    }

    void Analyzer::processFrame() {
        const auto channels = m_capture_device->capture();
        handleChannels(channels.top, m_mappings.top);
        handleChannels(channels.bottom, m_mappings.bottom);
        handleChannels(channels.left, m_mappings.left);
        handleChannels(channels.right, m_mappings.right);
        submitChannels();
    }

    void Analyzer::handleChannels(const std::vector<Color>& capture_channels,
                                  const std::vector<Mapping>& mappings) {
        for (Channel i = 0; i < capture_channels.size(); ++i) {
            const auto& mapping = mappings.at(i);
            auto& device_channels = m_channels_per_device.at(mapping.device_index);
            device_channels.at(mapping.device_channel) = capture_channels[i];
        }
    }

    void Analyzer::submitChannels() {
        for (DeviceIndex device = 0; device < m_devices->size(); ++device) {
            m_devices->setChannels(device, m_channels_per_device[device]);
        }
    }

    void Analyzer::run() {
        while (!m_interrupted) {
            try {
                processFrame();
            } catch (const std::exception& e) {
                spdlog::error("Error in Ambilight analyzer: {}", e.what());
            } catch (...) {
                spdlog::error("Unknown error in Ambilight analyzer");
            }
        }
        spdlog::info("Exiting Ambilight analyzer");
    }

}