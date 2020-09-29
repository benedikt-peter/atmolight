//
// Created by Benedikt on 21.08.2020.
//

#pragma once

#include <memory>
#include <atomic>
#include <vector>
#include <thread>
#include "capture_device.hpp"
#include "devices.hpp"

namespace atmo {

    /**
     * The Analyzer processes the color input from a CaptureDevice and maps these input channels to the output channels
     * of one or more output devices. Processing is done in a background thread and starts once the class has been
     * constructed.
     */
    class Analyzer {
    public:
        /**
         * Constructor.
         *
         * @param capture_device the capture device
         * @param devices the devices manager
         * @param mappings the mappings from input channels to devices and output channels
         */
        Analyzer(std::unique_ptr<CaptureDevice> capture_device,
                 Devices& devices,
                 Mappings mappings);

        ~Analyzer();

    private:
        std::atomic<bool> m_interrupted;
        std::unique_ptr<CaptureDevice> m_capture_device;
        Devices* m_devices;
        Mappings m_mappings;
        std::vector<std::vector<Color>> m_channels_per_device;
        std::thread m_worker;

        void handleChannels(const std::vector<Color>& capture_channels,
                            const std::vector<Mapping>& mappings);

        void submitChannels();

        void processFrame();

        void run();
    };

}
