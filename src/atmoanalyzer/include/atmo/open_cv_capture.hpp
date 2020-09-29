//
// Created by Benedikt on 20.08.2020.
//

#pragma once

#ifdef WITH_OPENCV

#include <opencv2/videoio.hpp>
#include <atmo/capture_device.hpp>

namespace atmo {

    /**
     * CaptureDevice implementation that uses a OpenCV VideoCapture device. This implementation supports generich
     * capture devices like webcams and screen grabbers.
     *
     * This implementation is only available if OpenCV has been installed and the WITH_OPENCV CMake option has been set
     * for AtmoLight.
     */
    class OpenCVCapture : public CaptureDevice {
    public:
        /**
         * Create a new CaptureDevice instance.
         *
         * @param index the device index (e.g. 0 to use the default capture device of the system)
         * @param channel_configs the channel mapping configuration to use for creating the output channels
         */
        OpenCVCapture(int index, ChannelConfigs channel_configs);

        /**
         * Create a new CaptureDevice instance.
         *
         * @param filename the device file (e.g. /dev/video0 on a Linux system)
         * @param channel_configs the channel mapping configuration to use for creating the output channels
         */
        OpenCVCapture(const std::string& filename, ChannelConfigs channel_configs);

        /*! @copydoc CaptureDevice::capture()
         */
        Channels capture() final;

    private:
        ChannelConfigs m_channel_configs;
        cv::VideoCapture m_capture_device;
        cv::Mat m_current_frame;

        std::vector<Color> calculateTopChannels() const;

        std::vector<Color> calculateBottomChannels() const;

        std::vector<Color> calculateLeftChannels() const;

        std::vector<Color> calculateRightChannels() const;

        Color getMean(const cv::Rect& rect) const;
    };

}

#endif