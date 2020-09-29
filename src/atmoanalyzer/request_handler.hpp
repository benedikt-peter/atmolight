//
// Created by Benedikt on 03.09.2020.
//

#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <atmo/devices.hpp>
#include <atmo/mode.hpp>

namespace atmo {

    using MessageId = std::int32_t;

    /**
     * Basic request parent class. A request consists of a JSON body that has at least the 'cmd' field.
     */
    class Request {
    public:
        explicit Request(const std::string& request_json);

        /**
         * Return the request command as string.
         *
         * @return the request cmd
         */
        std::string cmd() const;

        /**
         * Return the request message id.
         *
         * @return the request message id
         */
        MessageId msgId() const;

    protected:
        nlohmann::json m_json;
    };

    /**
     * Switch the current mode of this AtmoLight instance.
     */
    class SetModeRequest : public Request {
    public:
        /**
         * Return the requested mode to switch to. Viable options are:
         * - control:  All devices and channels can be manually controlled by a client.
         * - analyzer: The channels are controlled by the analyzer to provide colors matching to an input data stream.
         *             Only available if an analyzer has been configured.
         * @return
         */
        Mode mode() const;
    };

    /**
     * Return the current color of a specific device channel.
     */
    class GetChannelRequest : public Request {
    public:
        /**
         * The device to query.
         *
         * @return the index of the device
         */
        DeviceIndex device() const;

        /**
         * The channel to query.
         *
         * @return the index of the channel
         */
        Channel channel() const;
    };

    /**
     * Return the colors of all channels of a specific device.
     */
    class GetChannelsRequest : public Request {
    public:
        /**
         * The device to query.
         *
         * @return the index of the device
         */
        DeviceIndex device() const;
    };

    /**
     * Change the current color of a specific device channel.
     */
    class SetChannelRequest : public Request {
    public:
        /**
         * The device to change.
         *
         * @return the index of the device
         */
        DeviceIndex device() const;

        /**
         * The channel to change.
         *
         * @return the index of the channel
         */
        Channel channel() const;

        /**
         * The new color.
         *
         * @return the color
         */
        Color color() const;
    };

    /**
     * Change the colors of all channels of a specific device.
     */
    class SetChannelsRequest : public Request {
    public:
        /**
         * The device to change.
         *
         * @return the index of the device
         */
        DeviceIndex device() const;

        /**
         * The channels to change.
         *
         * @return the list of channels
         */
        std::vector<Color> channels() const;
    };

    /**
     * Basic response parent class.
     */
    class Response {
    public:
        Response(MessageId msg_id, bool success, std::string message);

        /**
         * Serialize the response to a string.
         *
         * @return the serialized JSON
         */
        std::string toString() const;

    protected:
        nlohmann::json m_json;
    };

    /**
     * Return the available modes to the client.
     */
    class GetModesResponse : public Response {
    public:
        /**
         * Add the given modes to the response.
         *
         * @param modes the modes
         * @return a reference to this
         */
        GetModesResponse& modes(gsl::span<const Mode> modes);
    };

    /**
     * Return the current mode to the client.
     */
    class GetModeResponse : public Response {
    public:
        /**
         * Add the given mode to the response.
         *
         * @param mode the mode
         * @return a reference to this
         */
        GetModeResponse& mode(Mode mode);
    };

    /**
     * Return the available devices to the client.
     */
    class GetDevicesResponse : public Response {
    public:
        /**
         * Add the given devices to the response.
         *
         * @param devices the devices
         * @return a reference to this
         */
        GetDevicesResponse& devices(gsl::span<const DeviceInfo> devices);
    };

    /**
     * Return the current color of a channel to the client.
     */
    class GetChannelResponse : public Response {
    public:
        /**
         * Add the given color to the response.
         *
         * @param color the color
         * @return a reference to this
         */
        GetChannelResponse& color(const Color& color);
    };

    /**
     * Return the current colors of all channels of one device to the client.
     */
    class GetChannelsResponse : public Response {
    public:
        /**
         * Add the given colors to the response.
         *
         * @param channels the colors
         * @return a reference to this
         */
        GetChannelsResponse& channels(gsl::span<const Color> channels);
    };

    /**
     * The RequestHandler executes commands and provides the respective responses in a protocol-independent manner.
     */
    class RequestHandler {
    public:
        /**
         * Constructor.
         *
         * @param devices the devices manager
         * @param mode_callbacks the callbacks to access the modes
         */
        RequestHandler(Devices& devices,
                       ModeCallbacks mode_callbacks);

        /**
         * Parse the incoming request from the JSON string, execute the command and return the response as JSON string.
         *
         * @param request_json the request body as a JSON string
         * @return the response body as a JSON string
         */
        std::string handleRequest(const std::string& request_json);

    private:
        Devices* m_devices;
        ModeCallbacks m_mode_callbacks;

        Response onGetModes(const Request& request) const;

        Response onGetMode(const Request& request) const;

        Response onSetMode(const Request& request) const;

        Response onGetDevices(const Request& request) const;

        Response onGetChannel(const Request& request) const;

        Response onGetChannels(const Request& request) const;

        Response onSetChannel(const Request& request) const;

        Response onSetChannels(const Request& request) const;

        Response onRequest(const Request& request);
    };

}
