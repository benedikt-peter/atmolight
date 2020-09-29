//
// Created by Benedikt on 20.08.2020.
//

#pragma once

#include <string>
#include <vector>
#include <gsl/span>
#include "channel.hpp"

namespace atmo {

    /**
     * This interface represents an RGB color output device with at least one controllable channel, e.g. a LED strip.
     */
    class AtmoDevice {
    public:
        virtual ~AtmoDevice() = default;

        /**
         * Set all available channels to black (R: 0, G: 0, B: 0).
         */
        virtual void clear() = 0;

        /**
         * Return the current color of the channel given by its index.
         *
         * @param channel the channel index
         * @return the current color
         */
        [[nodiscard]]
        virtual Color getChannel(Channel channel) = 0;

        /**
         * Return the current colors of all channels of this device.
         *
         * @return the colors of all channels, sorted by channel index
         */
        [[nodiscard]]
        virtual std::vector<Color> getChannels() = 0;

        /**
         * Set one specific channel to the given color.
         *
         * @param channel the channel index
         * @param color the new color
         */
        virtual void setChannel(Channel channel, Color color) = 0;

        /**
         * Set all channels of the device to the specified colors. Each channel is set to its color by the corresponding
         * index in the list.
         *
         * @param channels the colors for all channels
         */
        virtual void setChannels(gsl::span<const Color> channels) = 0;

        /**
         * Try to reopen the device. This is a workaround for misbehaving USB devices and USB hubs.
         */
        virtual void reset() = 0;

        /**
         * Return the number of available channels.
         *
         * @return the number of available channels
         */
        [[nodiscard]]
        virtual std::size_t channels() const = 0;
    };

    /**
     * Abstract base class that provides a simplified interface and a generic implementation of common behaviour of an
     * AtmoDevice implementation.
     *
     * A child class has to override the protected update() method. This method is called each time one or more channels
     * are modified.
     */
    class BaseAtmoDevice : public AtmoDevice {
    public:
        /**
         * Constructor.
         *
         * @param channels the number of available channels
         */
        explicit BaseAtmoDevice(std::size_t channels);

        void clear() final;

        [[nodiscard]]
        Color getChannel(Channel channel) final;

        [[nodiscard]]
        std::vector<Color> getChannels() final;

        void setChannel(Channel channel, Color color) final;

        void setChannels(gsl::span<const Color> channels) final;

        [[nodiscard]]
        std::size_t channels() const final;

    protected:
        /**
         * Update the physical device with the given colors. This method is always called with colors for all channels,
         * even if only one channel is modified.
         *
         * @param channels the colors for all available channels
         */
        virtual void update(gsl::span<const Color> channels) = 0;

    private:
        std::vector<Color> m_channels;
    };

}