//
// Created by Benedikt on 22.08.2020.
//

#include <CLI/Error.hpp>
#include <CLI/App.hpp>
#include <CLI/Formatter.hpp>
#include <CLI/Config.hpp>
#include <CLI/CLI.hpp>
#include <map>
#include <memory>
#include <fmt/format.h>
#include <thread>
#include <atmo/serial_devices.hpp>
#include <atmo/spi_devices.hpp>

namespace atmo {

    enum class DeviceType {
        AtmoLight,
        KarateLight,
        DotStar
    };

    std::unique_ptr<AtmoDevice> createDevice(DeviceType device_type, const std::string& device) {
        switch (device_type) {
            case DeviceType::AtmoLight:
                return std::make_unique<AtmoLight>(device);
            case DeviceType::KarateLight:
                return std::make_unique<KarateLight>(device);
#ifdef WITH_SPI
            case DeviceType::DotStar:
                return std::make_unique<DotStar>(device, 256);
#endif
            default:
                throw std::runtime_error{fmt::format("Device type {} is unsupported by this application", device_type)};
        }
    }

}


int main(int argc, char** argv) {
    using namespace atmo;

    CLI::App app{"atmoctrl"};

    DeviceType device_type{};
    std::map<std::string, DeviceType> map{{"atmolight",   DeviceType::AtmoLight},
                                          {"karatelight", DeviceType::KarateLight},
                                          {"dotstar", DeviceType::DotStar}};
    app.add_option("-t,--type", device_type, "the device type")
            ->required()
            ->transform(CLI::CheckedTransformer(map, CLI::ignore_case));
    std::string device{};
    app.add_option("-d,--device", device, "the device file to connect to")
            ->required();
    int channel_option{-1};
    app.add_option("-c,--channel", channel_option, "the channel");
    Color color{0, 0, 0};
    app.add_option("-r,--red", color.red, "red color intensity [0..255]");
    app.add_option("-g,--green", color.green, "green color intensity [0..255]");
    app.add_option("-b,--blue", color.blue, "blue color intensity [0..255]");

    CLI11_PARSE(app, argc, argv);

    auto atmo_device = createDevice(device_type, device);
    atmo_device->clear();

    if (channel_option < 0) {
        for (Channel channel = 0; channel < atmo_device->channels(); ++channel) {
            atmo_device->setChannel(channel, color);
        }
    } else {
        auto channel = static_cast<Channel>(channel_option);
        atmo_device->setChannel(channel, color);
    }
}