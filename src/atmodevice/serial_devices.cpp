//
// Created by benedikt on 16.03.18.
//

#include <atmo/serial_devices.hpp>
#include <algorithm>
#include <numeric>
#include <fmt/format.h>

namespace {
    constexpr std::array<unsigned char, 7> ATMOLIGHT_HEADER = {
            0xFF,
            0x00,
            0x00,
            0x0F,
            0x00,
            0x00,
            0x00};
    constexpr auto ATMOLIGHT_PACKET_SIZE = ATMOLIGHT_HEADER.size() + 4 * 3;
    constexpr std::array<unsigned char, 4> KARATELIGHT_HEADER = {
            0xAA,
            0x12,
            0x00,
            0x30};
    constexpr auto KARATELIGHT_PACKET_SIZE = 52;
    constexpr unsigned char KARATELIGHT_CHECKSUM_BYTE_POS = 2;

    auto calculateChecksum(gsl::span<const unsigned char> buffer) {
        return std::accumulate(std::begin(buffer), std::end(buffer), 0, [](auto a, auto b) { return a ^ b; });
    }
}

namespace atmo {

    SerialPortAtmoDevice::SerialPortAtmoDevice(const std::string& filename,
                                               unsigned int baud_rate,
                                               std::size_t channels) :
            BaseAtmoDevice{channels},
            m_filename{filename},
            m_baud_rate{baud_rate},
            m_io_service{},
            m_serial_port{m_io_service} {
        reset();
    }

    void SerialPortAtmoDevice::update(gsl::span<const Color> channels) {
        boost::asio::write(m_serial_port, boost::asio::buffer(writeBuffer(channels)));
    }

    void SerialPortAtmoDevice::reset() {
        m_serial_port = {m_io_service, m_filename.c_str()};
        m_serial_port.set_option(boost::asio::serial_port_base::baud_rate(m_baud_rate));
    }

    AtmoLight::AtmoLight(const std::string& port_name) :
            SerialPortAtmoDevice{port_name, 38400, 4} {
        clear();
    }

    std::vector<unsigned char> AtmoLight::writeBuffer(gsl::span<const Color> channels) {
        std::vector<unsigned char> buffer{};
        buffer.resize(ATMOLIGHT_PACKET_SIZE);
        auto payload_position = std::copy(std::begin(ATMOLIGHT_HEADER), std::end(ATMOLIGHT_HEADER), std::begin(buffer));
        for (auto color : channels) {
            std::array<unsigned char, 3> channel{color.red, color.green, color.blue};
            payload_position = std::copy(std::begin(channel), std::end(channel), payload_position);
        }
        return buffer;
    }

    KarateLight::KarateLight(const std::string& port_name) :
            SerialPortAtmoDevice{port_name, 57600, 16} {
        clear();
    }

    std::vector<unsigned char> KarateLight::writeBuffer(gsl::span<const Color> channels) {
        std::vector<unsigned char> buffer{};
        buffer.resize(KARATELIGHT_PACKET_SIZE);
        auto payload_position = std::copy(std::begin(KARATELIGHT_HEADER),
                                          std::end(KARATELIGHT_HEADER),
                                          std::begin(buffer));
        for (auto color : channels) {
            std::array<unsigned char, 3> channel{color.green, color.blue, color.red};
            payload_position = std::copy(std::begin(channel), std::end(channel), payload_position);
        }
        buffer[KARATELIGHT_CHECKSUM_BYTE_POS] = calculateChecksum(buffer);
        return buffer;
    }

}