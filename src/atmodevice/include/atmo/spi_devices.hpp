//
// Created by Benedikt on 30.08.2020.
//

#pragma once

#ifdef WITH_SPI

#include <string>
#include <vector>
#include <gsl/span>
#include "device.hpp"

namespace atmo {

    /**
     * Abstract base class for devices accessible by a SPI interface.
     *
     * Child classes have to override the writeBuffer() method. This method is called to assemble the output buffer that
     * is written to the SPI interface.
     */
    class SPIDevice : public BaseAtmoDevice {
    public:
        /**
         * Constructor.
         *
         * @param filename the (file) name of the SPI port (e.g. /dev/spidev0.0)
         * @param channels the number of available channels
         */
        SPIDevice(const std::string& filename,
                  std::size_t channels);

        ~SPIDevice() override;

        void reset() final;

    protected:
        /**
         * Write an output buffer that contains the new colors of all channels.
         *
         * @param channels the channel colors
         * @return the output buffer
         */
        virtual std::vector<unsigned char> writeBuffer(gsl::span<const Color> channels) = 0;

        void update(gsl::span<const Color> channels) final;

    private:
        std::string m_filename;
        int m_file_descriptor;

        void closeDevice();
    };

    /**
     * The DotStar class allows to control Adafruit Dotstar LEDs (or other APA102C controlled LEDs).
     */
    class DotStar : public SPIDevice {
    public:
        /**
         * Constructor.
         *
         * @param filename the (file) name of the SPI port (e.g. /dev/spidev0.0)
         * @param channels the number of available channels
         */
        DotStar(const std::string& filename,
                std::size_t channels);

    protected:
        std::vector<unsigned char> writeBuffer(gsl::span<const Color> channels) override;
    };

}

#endif