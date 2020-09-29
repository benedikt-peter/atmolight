//
// Created by Benedikt on 30.08.2020.
//

#include <atmo/device.hpp>
#include <fmt/format.h>

#include <utility>

namespace atmo {

    BaseAtmoDevice::BaseAtmoDevice(std::size_t channels) :
            m_channels{channels, Color{}} {
        if (channels < 1) {
            throw std::runtime_error{fmt::format("Illegal number of channels: {}", channels)};
        }
    }

    void BaseAtmoDevice::clear() {
        std::fill(std::begin(m_channels), std::end(m_channels), Color{});
        update(m_channels);
    }

    Color BaseAtmoDevice::getChannel(Channel channel) {
        if (channel >= m_channels.size()) {
            throw std::runtime_error{fmt::format("Invalid channel index: {}", channel)};
        }
        return m_channels[channel];
    }

    std::vector<Color> BaseAtmoDevice::getChannels() {
        return m_channels;
    }

    void BaseAtmoDevice::setChannel(Channel channel, Color color) {
        if (channel >= m_channels.size()) {
            throw std::runtime_error{fmt::format("Invalid channel index: {}", channel)};
        }
        m_channels[channel] = color;
        update(m_channels);
    }

    void BaseAtmoDevice::setChannels(gsl::span<const Color> channels) {
        if (channels.size() > static_cast<int>(m_channels.size())) {
            throw std::out_of_range{
                    fmt::format("Invalid number of channels: {} available but {} given",
                                m_channels.size(), channels.size())};
        }
        std::copy(std::begin(channels), std::end(channels), std::begin(m_channels));
        update(m_channels);
    }

    std::size_t BaseAtmoDevice::channels() const {
        return m_channels.size();
    }

}