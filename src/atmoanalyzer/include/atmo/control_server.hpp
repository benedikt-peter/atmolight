//
// Created by Benedikt on 24.08.2020.
//

#pragma once

#include <boost/asio.hpp>
#include <functional>
#include <thread>
#include <gsl/span>
#include "types.hpp"
#include "devices.hpp"
#include "mode.hpp"

namespace atmo {

    class RequestHandler;

    /**
     * The ControlServer allows to individually control output channels. If an analyzer has been configured, the
     * analyzer can also be started and stopped.
     *
     * Currently the ControlServer uses JSON for request and response messages over websockets.
     */
    class ControlServer {
    public:
        /**
         * Constructs a new instance.
         *
         * @param devices the output device manager
         * @param mode_callbacks a struct containing
         * @param host
         * @param port
         */
        ControlServer(Devices& devices,
                      ModeCallbacks mode_callbacks,
                      const std::string& host,
                      uint16_t port);

        ~ControlServer();

    private:
        std::unique_ptr<RequestHandler> m_request_handler;
        boost::asio::io_context m_io_context;
        boost::asio::ip::tcp::acceptor m_acceptor;
        std::thread m_worker;

        void runWorker();

        void startAccept();
    };

}
