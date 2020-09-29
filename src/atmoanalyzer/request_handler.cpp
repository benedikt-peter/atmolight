//
// Created by Benedikt on 03.09.2020.
//

#include "request_handler.hpp"

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>

namespace {

    using json = nlohmann::json;
    using namespace atmo;

    auto getRequiredNode(const json& json_node, const std::string& property) {
        if (!json_node.contains(property)) {
            throw std::runtime_error{fmt::format("Missing property '{}'", property)};
        }
        return json_node[property];
    }

    template<class T>
    auto getRequired(const json& json_node, const std::string& property) {
        return getRequiredNode(json_node, property).get<T>();
    }

    auto modeToString(Mode mode) {
        switch (mode) {
            case Mode::Analyzer:
                return "analyzer";
            case Mode::Control:
                return "control";
            default:
                throw std::runtime_error{fmt::format("Invalid mode: '{}'", mode)};
        }
    }

    template<class T>
    auto successfulResponse(MessageId msg_id) {
        return T{Response{msg_id, true, "OK"}};
    }

    auto successfulResponse(MessageId msg_id) {
        return successfulResponse<Response>(msg_id);
    }

}

namespace atmo {

    using json = nlohmann::json;

    class RequestError : public std::runtime_error {
    public:
        explicit RequestError(const std::string& message) :
                std::runtime_error{message} {}
    };

    Request::Request(const std::string& request_json) {
        try {
            // See: https://github.com/nlohmann/json/issues/2311
            m_json = json::parse(request_json);
        } catch (const json::parse_error& e) {
            throw RequestError{fmt::format("Deserializer error: {}", e.what())};
        }
    }

    RequestHandler::RequestHandler(Devices& devices, ModeCallbacks mode_callbacks) :
            m_devices{&devices},
            m_mode_callbacks{std::move(mode_callbacks)} {}

    std::string RequestHandler::handleRequest(const std::string& request_json) {
        MessageId msg_id{-1};
        try {
            const Request request{request_json};
            msg_id = request.msgId();
            const auto response = onRequest(request);
            return response.toString();
        } catch (const RequestError& e) {
            spdlog::error("Error while executing request: {}", e.what());
            return Response{msg_id, false, e.what()}.toString();
        }
    }

    Response RequestHandler::onGetModes(const Request& request) const {
        const auto modes = m_mode_callbacks.get_modes_callback();
        return successfulResponse<GetModesResponse>(request.msgId()).modes(modes);
    }

    Response RequestHandler::onGetMode(const Request& request) const {
        const auto mode = m_mode_callbacks.get_mode_callback();
        return successfulResponse<GetModeResponse>(request.msgId()).mode(mode);
    }

    Response RequestHandler::onSetMode(const Request& request) const {
        SetModeRequest set_mode_request{request};
        m_mode_callbacks.set_mode_callback(set_mode_request.mode());
        return successfulResponse(request.msgId());
    }

    Response RequestHandler::onGetDevices(const Request& request) const {
        return successfulResponse<GetDevicesResponse>(request.msgId()).devices(m_devices->devices());
    }

    Response RequestHandler::onGetChannel(const Request& request) const {
        GetChannelRequest get_channel_request{request};
        const auto color = m_devices->getChannel(get_channel_request.device(),
                                                 get_channel_request.channel());
        return successfulResponse<GetChannelResponse>(request.msgId()).color(color);
    }

    Response RequestHandler::onGetChannels(const Request& request) const {
        GetChannelsRequest get_channels_request{request};
        const auto channels = m_devices->getChannels(get_channels_request.device());
        return successfulResponse<GetChannelsResponse>(request.msgId()).channels(channels);
    }

    Response RequestHandler::onSetChannel(const Request& request) const {
        SetChannelRequest set_channel_request{request};
        m_devices->setChannel(set_channel_request.device(),
                              set_channel_request.channel(),
                              set_channel_request.color());
        return successfulResponse(request.msgId());
    }

    Response RequestHandler::onSetChannels(const Request& request) const {
        SetChannelsRequest set_channels_request{request};
        const auto channels = set_channels_request.channels();
        m_devices->setChannels(set_channels_request.device(),
                               channels);
        return successfulResponse(request.msgId());
    }

    Response RequestHandler::onRequest(const Request& request) {
        try {
            if (request.cmd() == "set_channel") {
                return onSetChannel(request);
            } else if (request.cmd() == "set_channels") {
                return onSetChannels(request);
            } else if (request.cmd() == "get_devices") {
                return onGetDevices(request);
            } else if (request.cmd() == "get_channel") {
                return onGetChannel(request);
            } else if (request.cmd() == "get_channels") {
                return onGetChannels(request);
            } else if (request.cmd() == "get_modes") {
                return onGetModes(request);
            } else if (request.cmd() == "get_mode") {
                return onGetMode(request);
            } else if (request.cmd() == "set_mode") {
                return onSetMode(request);
            } else {
                throw RequestError{fmt::format("Illegal command: '{}'", request.cmd())};
            }
        } catch (const RequestError&) {
            throw;
        } catch (const std::exception& e) {
            throw RequestError{fmt::format("Request failed: {}", e.what())};
        }
    }

    std::string Request::cmd() const {
        return getRequired<std::string>(m_json, "cmd");
    }

    MessageId Request::msgId() const {
        return getRequired<MessageId>(m_json, "msg_id");
    }

    Mode SetModeRequest::mode() const {
        const auto mode = getRequired<std::string>(m_json, "mode");
        if (mode == "analyzer") {
            return Mode::Analyzer;
        } else if (mode == "control") {
            return Mode::Control;
        } else {
            throw std::runtime_error{fmt::format("Invalid mode: '{}'", mode)};
        }
    }

    DeviceIndex GetChannelRequest::device() const {
        return getRequired<DeviceIndex>(m_json, "device");
    }

    Channel GetChannelRequest::channel() const {
        return getRequired<Channel>(m_json, "channel");
    }

    DeviceIndex GetChannelsRequest::device() const {
        return getRequired<DeviceIndex>(m_json, "device");
    }

    DeviceIndex SetChannelRequest::device() const {
        return getRequired<DeviceIndex>(m_json, "device");
    }

    Channel SetChannelRequest::channel() const {
        return getRequired<Channel>(m_json, "channel");
    }

    Color SetChannelRequest::color() const {
        const auto red = getRequired<std::uint8_t>(m_json, "red");
        const auto green = getRequired<std::uint8_t>(m_json, "green");
        const auto blue = getRequired<std::uint8_t>(m_json, "blue");
        return Color{red, green, blue};
    }

    DeviceIndex SetChannelsRequest::device() const {
        return getRequired<DeviceIndex>(m_json, "device");
    }

    std::vector<Color> SetChannelsRequest::channels() const {
        const auto channels_node = getRequiredNode(m_json, "channels");
        if (!channels_node.is_array()) {
            throw std::runtime_error{"Expected property 'channels' to be an array element"};
        }
        std::vector<Color> channels{};
        channels.reserve(channels_node.size());
        for (const auto& el : channels_node.items()) {
            const auto value = el.value();
            const auto red = getRequired<std::uint8_t>(value, "red");
            const auto green = getRequired<std::uint8_t>(value, "green");
            const auto blue = getRequired<std::uint8_t>(value, "blue");
            channels.emplace_back(red, green, blue);
        }
        return channels;
    }

    Response::Response(MessageId msg_id, bool success, std::string message) :
            m_json{} {
        m_json["msg_id"] = msg_id;
        m_json["success"] = success;
        m_json["message"] = message;
    }

    std::string Response::toString() const {
        try {
            return m_json.dump();
        } catch (const json::type_error& e) {
            throw RequestError{fmt::format("Serializer error: {}", e.what())};
        }
    }

    GetModesResponse& GetModesResponse::modes(gsl::span<const Mode> modes) {
        auto& modes_node = m_json["modes"];
        modes_node = json::array();
        for (const auto& mode : modes) {
            modes_node.push_back(modeToString(mode));
        }
        return *this;
    }

    GetModeResponse& GetModeResponse::mode(Mode mode) {
        m_json["mode"] = modeToString(mode);
        return *this;
    }

    GetDevicesResponse& GetDevicesResponse::devices(gsl::span<const DeviceInfo> devices) {
        auto& devices_node = m_json["devices"];
        devices_node = json::array();
        for (const auto& device : devices) {
            auto& device_node = devices_node.emplace_back();
            device_node["name"] = device.name;
            device_node["channels"] = device.channels;
        }
        return *this;
    }

    GetChannelResponse& GetChannelResponse::color(const Color& color) {
        m_json["red"] = color.red;
        m_json["green"] = color.green;
        m_json["blue"] = color.blue;
        return *this;
    }

    GetChannelsResponse& GetChannelsResponse::channels(gsl::span<const Color> channels) {
        auto& channels_node = m_json["channels"];
        channels_node = json::array();
        for (const auto& color : channels) {
            auto& color_node = channels_node.emplace_back();
            color_node["red"] = color.red;
            color_node["green"] = color.green;
            color_node["blue"] = color.blue;
        }
        return *this;
    }

}