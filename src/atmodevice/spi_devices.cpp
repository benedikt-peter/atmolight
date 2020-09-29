//
// Created by Benedikt on 30.08.2020.
//

#ifdef WITH_SPI

#include <atmo/spi_devices.hpp>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <array>
#include <spdlog/spdlog.h>

namespace {

    constexpr std::array<unsigned char, 4> DOTSTAR_START_FRAME{0x0, 0x0, 0x0, 0x0};
    constexpr std::array<unsigned char, 4> DOTSTAR_END_FRAME{0xFF, 0xFF, 0xFF, 0xFF};
    constexpr std::size_t DOTSTAR_FRAME_LENGTH{4};
    constexpr unsigned char DOTSTAR_BRIGHTNESS{0b11100000};

    void ioError(const std::string& error_message) {
        const auto error = errno;
        throw std::runtime_error{fmt::format("{}: {} {}",
                                             error_message, error, std::strerror(error))};
    }
}

namespace atmo {

    SPIDevice::SPIDevice(const std::string& filename,
                         std::size_t channels) :
            BaseAtmoDevice{channels},
            m_filename{filename},
            m_file_descriptor{-1} {
        reset();
    }

    void SPIDevice::update(gsl::span<const Color> channels) {
        auto buffer = writeBuffer(channels);
        spi_ioc_transfer transfer{};
        std::memset(&transfer, 0, sizeof(transfer));
        transfer.tx_buf = reinterpret_cast<__u64>(buffer.data());
        transfer.len = buffer.size();
        if (ioctl(m_file_descriptor, SPI_IOC_MESSAGE(1), &transfer) < 0) {
            ioError("Could not write SPI packet");
        }
    }

    SPIDevice::~SPIDevice() {
        closeDevice();
    }

    void SPIDevice::reset() {
        closeDevice();

        m_file_descriptor = open(m_filename.c_str(), O_RDWR);
        if (m_file_descriptor < 0) {
            ioError(fmt::format("Could not open SPI device '{}'", m_filename));
        }

        std::uint8_t mode{SPI_MODE_0};
        if (ioctl(m_file_descriptor, SPI_IOC_WR_MODE, &mode) < 0) {
            ioError("Could not set SPI write mode");
        }

        if (ioctl(m_file_descriptor, SPI_IOC_RD_MODE, &mode) < 0) {
            ioError("Could not set SPI read mode");
        }

        std::uint8_t word_length{8};
        if (ioctl(m_file_descriptor, SPI_IOC_WR_BITS_PER_WORD, &word_length) < 0) {
            ioError("Could not set SPI write word length");
        }

        if (ioctl(m_file_descriptor, SPI_IOC_RD_BITS_PER_WORD, &word_length) < 0) {
            ioError("Could not set SPI read word length");
        }

        std::uint32_t speed{500000};//8000000L
        if (ioctl(m_file_descriptor, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
            ioError("Could not set SPI write speed");
        }

        if (ioctl(m_file_descriptor, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) {
            ioError("Could not set SPI read speed");
        }
    }

    void SPIDevice::closeDevice() {
        if (m_file_descriptor > -1) {
            if (close(m_file_descriptor) < 0) {
                const auto error = errno;
                spdlog::error("Could not close SPI file descriptor: {} {}", error, std::strerror(error));
                m_file_descriptor = -1;
            }
        }
    }

    DotStar::DotStar(const std::string& filename,
                     std::size_t channels) :
            SPIDevice{filename, channels} {}

    auto computeDotStarBrightness(const Color& color) {
        auto sum{0};
        sum += color.red;
        sum += color.blue;
        sum += color.green;
        auto intensity = sum / (3.0f * 255.0f);
        return static_cast<unsigned char>(intensity * 31);
    }

    std::vector<unsigned char> DotStar::writeBuffer(gsl::span<const Color> channels) {
        std::vector<unsigned char> buffer{};
        buffer.resize(DOTSTAR_START_FRAME.size() + channels.size() * DOTSTAR_FRAME_LENGTH + DOTSTAR_END_FRAME.size());
        auto current_position = std::copy(std::begin(DOTSTAR_START_FRAME),
                                          std::end(DOTSTAR_START_FRAME),
                                          std::begin(buffer));
        for (const auto color : channels) {
            unsigned char brightness = DOTSTAR_BRIGHTNESS | computeDotStarBrightness(color);
            std::array<unsigned char, DOTSTAR_FRAME_LENGTH> frame{brightness, color.blue, color.green, color.red};
            current_position = std::copy(std::begin(frame),
                                         std::end(frame),
                                         current_position);
        }
        std::copy(std::begin(DOTSTAR_END_FRAME),
                  std::end(DOTSTAR_END_FRAME),
                  current_position);
        return buffer;
    }

}

#endif