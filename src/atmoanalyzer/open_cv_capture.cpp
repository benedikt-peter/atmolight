//
// Created by Benedikt on 20.08.2020.
//

#ifdef WITH_OPENCV

#include <atmo/open_cv_capture.hpp>

#include <fmt/format.h>
#include <opencv2/opencv.hpp>

namespace atmo {

    OpenCVCapture::OpenCVCapture(int index, ChannelConfigs channel_configs) :
            m_channel_configs{channel_configs},
            m_capture_device{index},
            m_current_frame{} {
        if (!m_capture_device.isOpened()) {
            throw std::runtime_error{fmt::format("OpenCV: Could not open capture device with index {}", index)};
        }
    }

    OpenCVCapture::OpenCVCapture(const std::string& filename, ChannelConfigs channel_configs) :
            m_channel_configs{channel_configs},
            m_capture_device{0},
            m_current_frame{} {
        if (!m_capture_device.isOpened()) {
            throw std::runtime_error{fmt::format("OpenCV: Could not open capture from file '{}'", filename)};
        }
    }

    Color convertColor(const cv::Scalar& color) {
        return Color{static_cast<std::uint8_t>(color[2]),
                     static_cast<std::uint8_t>(color[1]),
                     static_cast<std::uint8_t>(color[0])};
    }

    Color OpenCVCapture::getMean(const cv::Rect& rect) const {
        auto area = m_current_frame(rect);
        const auto mean = cv::mean(area);
        auto color = convertColor(mean);
        area = mean;
        return color;
    }

    std::vector<Color> OpenCVCapture::calculateTopChannels() const {
        std::vector<Color> channels{};
        channels.resize(m_channel_configs.top.count);
        const auto top_width = static_cast<float>(m_current_frame.cols
                                                  - m_channel_configs.left.crop
                                                  - m_channel_configs.right.crop)
                               / m_channel_configs.top.count;
        for (Channel channel = 0; channel < m_channel_configs.top.count; ++channel) {
            channels[channel] = getMean(cv::Rect{
                    m_channel_configs.left.crop + static_cast<Width>(top_width * channel),
                    m_channel_configs.top.crop,
                    static_cast<Width>(top_width),
                    m_channel_configs.top.depth});

        }
        return channels;
    }

    std::vector<Color> OpenCVCapture::calculateBottomChannels() const {
        std::vector<Color> channels{};
        channels.resize(m_channel_configs.bottom.count);
        const auto bottom_width = static_cast<float>(m_current_frame.cols
                                                     - m_channel_configs.left.crop
                                                     - m_channel_configs.right.crop)
                                  / m_channel_configs.bottom.count;
        for (Channel channel = 0; channel < m_channel_configs.bottom.count; ++channel) {
            channels[channel] = getMean(cv::Rect{
                    m_channel_configs.left.crop + static_cast<Width>(bottom_width * channel),
                    m_current_frame.rows - m_channel_configs.bottom.crop - m_channel_configs.bottom.depth,
                    static_cast<Width>(bottom_width),
                    m_channel_configs.bottom.depth});

        }
        return channels;
    }

    std::vector<Color> OpenCVCapture::calculateLeftChannels() const {
        std::vector<Color> channels{};
        channels.resize(m_channel_configs.left.count);
        const auto left_height = static_cast<float>(m_current_frame.rows
                                                    - m_channel_configs.top.crop
                                                    - m_channel_configs.bottom.crop)
                                 / m_channel_configs.left.count;
        for (Channel channel = 0; channel < m_channel_configs.left.count; ++channel) {
            channels[channel] = getMean(cv::Rect{
                    m_channel_configs.left.crop,
                    m_channel_configs.top.crop + static_cast<Width>(left_height * channel),
                    m_channel_configs.left.depth,
                    static_cast<Width>(left_height)});

        }
        return channels;
    }

    std::vector<Color> OpenCVCapture::calculateRightChannels() const {
        std::vector<Color> channels{};
        channels.resize(m_channel_configs.right.count);
        const auto right_height = static_cast<float>(m_current_frame.rows
                                                     - m_channel_configs.top.crop
                                                     - m_channel_configs.bottom.crop)
                                  / m_channel_configs.right.count;
        for (Channel channel = 0; channel < m_channel_configs.right.count; ++channel) {
            channels[channel] = getMean(cv::Rect{
                    m_current_frame.cols - m_channel_configs.right.crop - m_channel_configs.right.depth,
                    m_channel_configs.top.crop + static_cast<Width>(right_height * channel),
                    m_channel_configs.right.depth,
                    static_cast<Width>(right_height)});

        }
        return channels;
    }

    Channels OpenCVCapture::capture() {
        m_capture_device >> m_current_frame;
        if (m_current_frame.empty()) {
            return Channels{};
        }

        if (m_current_frame.channels() < 3) {
            throw std::runtime_error{fmt::format("Expected at least 3 channels but got {}",
                                                 m_current_frame.channels())};
        }

        Channels channels{};
        channels.top = calculateTopChannels();
        channels.bottom = calculateBottomChannels();
        channels.left = calculateLeftChannels();
        channels.right = calculateRightChannels();
        return channels;
    }

}

#endif