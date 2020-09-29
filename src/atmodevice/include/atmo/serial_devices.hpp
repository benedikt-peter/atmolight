//
// Created by benedikt on 16.03.18.
//

#pragma once

#include <string>
#include <vector>
#include <gsl/span>
#include <boost/asio.hpp>
#include "device.hpp"

namespace atmo {

    /**
     * Abstract base class for devices that use a serial port (TTY) interface.
     *
     * Child classes have to override the writeBuffer() method. This method is called to assemble the output buffer that
     * is written to the serial port.
     */
    class SerialPortAtmoDevice : public BaseAtmoDevice {
    public:
        /**
         * Constructor.
         *
         * @param filename the (file) name of the serial device (e.g. COM1 on Windows or /dev/ttyUSB0 on Linux)
         * @param baud_rate the baud rate for the serial interface
         * @param channels the number of channels
         */
        SerialPortAtmoDevice(const std::string& filename,
                             unsigned int baud_rate,
                             std::size_t channels);

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
        unsigned int m_baud_rate;
        boost::asio::io_service m_io_service;
        boost::asio::serial_port m_serial_port;
    };

    /**
     * The AtmoLight class allows to control ca.rstenpresser AtmoLight devices.
     */
    class AtmoLight : public SerialPortAtmoDevice {
    public:
        /**
         * Constructor.
         *
         * @param filename the (file) name of the serial device (e.g. COM1 on Windows or /dev/ttyUSB0 on Linux)
         */
        explicit AtmoLight(const std::string& filename);

    protected:
        std::vector<unsigned char> writeBuffer(gsl::span<const Color> channels) final;
    };

    /**
     * The AtmoLight class allows to control ca.rstenpresser KarateLight devices.
     */
    class KarateLight : public SerialPortAtmoDevice {
    public:
        /**
         * Constructor.
         *
         * @param filename the (file) name of the serial device (e.g. COM1 on Windows or /dev/ttyACM0 on Linux)
         */
        explicit KarateLight(const std::string& port_name);

    protected:
        std::vector<unsigned char> writeBuffer(gsl::span<const Color> channels) final;
    };

}