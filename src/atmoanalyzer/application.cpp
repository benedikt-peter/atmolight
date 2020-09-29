//
// Created by benedikt on 13.03.18.
//

#include <atmo/application.hpp>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <atmo/serial_devices.hpp>
#include <atmo/spi_devices.hpp>
#include <atmo/open_cv_capture.hpp>
#include <atmo/config.hpp>

namespace {

    using namespace atmo;

    std::unique_ptr<AtmoDevice> createAtmoDevice(const Configuration::Device& device) {
        switch (device.type) {
            case DeviceType::AtmoLight:
                return std::make_unique<AtmoLight>(device.filename);
            case DeviceType::KarateLight:
                return std::make_unique<KarateLight>(device.filename);
#ifdef WITH_SPI
                case DeviceType::DotStar:
                    return std::make_unique<DotStar>(device.filename, device.channels);
#endif
            default:
                throw std::runtime_error{fmt::format(
                        "Cannot initialize atmo device of unsupported type {}", device.type)};
        }
    }

    auto createDevice(const Configuration::Device& device) {
        return Device{device.name, createAtmoDevice(device), device.reset_on_error};
    }

    auto createAtmoDevices(const Configuration& config) {
        std::vector<Device> atmo_devices{};
        for (const auto& device : config.devices()) {
            atmo_devices.push_back(createDevice(device));
        }
        return atmo_devices;
    }

    auto createDevices(const Configuration& config) {
        auto atmo_devices = createAtmoDevices(config);
        return std::make_unique<Devices>(std::move(atmo_devices));
    }

    std::unique_ptr<CaptureDevice> createCaptureDevice(const Configuration::Capture& capture) {
        switch (capture.type) {
#ifdef WITH_OPENCV
            case CaptureType::OpenCV:
                if (capture.index >= 0) {
                    return std::make_unique<OpenCVCapture>(0, capture.channel_configs);
                } else {
                    return std::make_unique<OpenCVCapture>(capture.filename, capture.channel_configs);
                }
#endif
            default:
                throw std::runtime_error{fmt::format(
                        "Cannot initialize capture device of unsupported type {}", capture.type)};
        }
    }

    std::unique_ptr<ControlServer> createControlServer(const Configuration& config,
                                                       Devices& devices,
                                                       ModeCallbacks mode_callbacks) {
        if (!config.control()) {
            return {};
        }
        return std::make_unique<ControlServer>(devices,
                                               std::move(mode_callbacks),
                                               config.control()->address,
                                               config.control()->port);
    }

    std::unique_ptr<Analyzer> createAnalyzer(const Configuration& config,
                                             Devices& devices) {
        if (!config.analyzer()) {
            return {};
        }
        auto capture_device = createCaptureDevice(config.analyzer()->capture);
        return std::make_unique<Analyzer>(std::move(capture_device),
                                          devices,
                                          config.analyzer()->mappings);
    }

}

namespace atmo {

    Application::Application(const std::string& config_file) :
            m_mutex{},
            m_condition_variable{},
            m_interrupted{false},
            m_configuration{config_file},
            m_devices{createDevices(m_configuration)},
            m_control_server{createControlServer(
                    m_configuration,
                    *m_devices, {
                            [this]() { return getModes(); },
                            [this]() { return getMode(); },
                            [this](auto process_mode) { setMode(process_mode); }})} {
        if (!m_analyzer && !m_control_server) {
            throw std::runtime_error{"Either analyzer or control has to be configured"};
        }
    }

    void Application::run() {
        std::unique_lock<std::mutex> lock{m_mutex};
        m_condition_variable.wait(lock, [this] { return m_interrupted; });
    }

    void Application::interrupt() {
        std::unique_lock<std::mutex> lock{m_mutex};
        m_interrupted = true;
        lock.unlock();
        m_condition_variable.notify_all();
    }

    std::vector<Mode> Application::getModes() const {
        std::vector<Mode> modes{};
        if (m_configuration.control()) {
            modes.push_back(Mode::Control);
        }
        if (m_configuration.analyzer()) {
            modes.push_back(Mode::Analyzer);
        }
        return modes;
    }

    Mode Application::getMode() const {
        if (m_analyzer) {
            return Mode::Analyzer;
        }
        return Mode::Control;
    }

    void Application::setMode(Mode process_mode) {
        switch (process_mode) {
            case Mode::Control:
                spdlog::info("Switching to Control mode");
                m_analyzer.reset();
                m_devices->clear();
                break;
            case Mode::Analyzer:
                if (m_configuration.analyzer()) {
                    spdlog::info("Switching to Analyzer mode");
                    m_analyzer = createAnalyzer(m_configuration, *m_devices);
                } else {
                    throw std::runtime_error{
                            "Analyzer mode cannot be activated, because the mode has not been configured"};
                }
                break;
        }
    }

}