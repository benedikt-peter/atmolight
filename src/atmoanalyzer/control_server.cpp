//
// Created by Benedikt on 24.08.2020.
//

#include <atmo/control_server.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <spdlog/spdlog.h>
#include <memory>
#include "request_handler.hpp"

namespace atmo {

    using namespace boost::asio::ip;
    namespace beast = boost::beast;

    class Connection : public std::enable_shared_from_this<Connection> {
    public:
        Connection(RequestHandler& request_handler, tcp::socket&& socket) :
                m_request_handler{&request_handler},
                m_stream{std::move(socket)},
                m_buffer{} {}

        void accept(const boost::system::error_code& error) {
            m_endpoint = m_stream.next_layer().socket().remote_endpoint();
            if (error) {
                spdlog::info("Connection from {}:{} has been closed: {}",
                             m_endpoint.address().to_string(),
                             m_endpoint.port(),
                             error.message());
                return;
            }

            spdlog::info("Accepted connection from {}:{}",
                         m_endpoint.address().to_string(),
                         m_endpoint.port());

            boost::asio::dispatch(m_stream.get_executor(),
                                  beast::bind_front_handler(
                                          &Connection::start,
                                          shared_from_this()));
        }

    private:
        void start() {
            m_stream.set_option(
                    beast::websocket::stream_base::timeout::suggested(
                            beast::role_type::server));
            m_stream.set_option(beast::websocket::stream_base::decorator(
                    [](beast::websocket::response_type& res) {
                        res.set(beast::http::field::server,
                                std::string(BOOST_BEAST_VERSION_STRING) +
                                " websocket-server-async");
                    }));

            m_stream.async_accept(
                    beast::bind_front_handler(
                            &Connection::run,
                            shared_from_this()));
        }

        void run(const beast::error_code& error = {}) {
            if (error) {
                spdlog::info("Connection from {}:{} has been closed: {}",
                             m_endpoint.address().to_string(),
                             m_endpoint.port(),
                             error.message());
                return;
            }

            m_stream.async_read(m_buffer,
                                beast::bind_front_handler(
                                        &Connection::readRequest,
                                        shared_from_this()));
        }

        void readRequest(const boost::system::error_code& error, std::size_t) {
            if (error) {
                spdlog::info("Connection from {}:{} has been closed: {}",
                             m_endpoint.address().to_string(),
                             m_endpoint.port(),
                             error.message());
                return;
            }

            if (!m_stream.got_text()) {
                spdlog::warn("Received binary data from client {}:{} instead of JSON ",
                             m_endpoint.address().to_string(),
                             m_endpoint.port());
                return;
            }

            const auto response = m_request_handler->handleRequest(beast::buffers_to_string(m_buffer.cdata()));
            m_buffer.consume(m_buffer.cdata().size());
            boost::asio::buffer_copy(m_buffer.prepare(response.size()), boost::asio::buffer(response));
            m_buffer.commit(response.size());

            m_stream.async_write(m_buffer.cdata(),
                                 beast::bind_front_handler(
                                         &Connection::responseWritten,
                                         shared_from_this()));
        }

        void responseWritten(const boost::system::error_code& error, std::size_t) {
            if (error) {
                spdlog::info("Connection from {}:{} has been closed: {}",
                             m_endpoint.address().to_string(),
                             m_endpoint.port(),
                             error.message());
                return;
            }

            m_buffer.consume(m_buffer.size());

            run();
        }

        RequestHandler* m_request_handler;
        beast::websocket::stream<beast::tcp_stream> m_stream;
        tcp::endpoint m_endpoint;
        beast::flat_buffer m_buffer;
    };

    ControlServer::ControlServer(Devices& devices,
                                 ModeCallbacks mode_callbacks,
                                 const std::string& host,
                                 uint16_t port) :
            m_request_handler{std::make_unique<RequestHandler>(devices, std::move(mode_callbacks))},
            m_io_context{1},
            m_acceptor{m_io_context, tcp::endpoint{make_address(host), port}},
            m_worker{[this] { runWorker(); }} {}

    ControlServer::~ControlServer() {
        m_io_context.stop();
        if (m_worker.joinable()) {
            m_worker.join();
        }
    }

    void ControlServer::startAccept() {
        m_acceptor.async_accept(
                boost::asio::make_strand(m_io_context),
                [this](auto error, auto socket) {
                    auto connection = std::make_shared<Connection>(*m_request_handler, std::move(socket));
                    connection->accept(error);
                    startAccept();
                });
    }

    void ControlServer::runWorker() {
        try {
            spdlog::info("Listening on {}:{}",
                         m_acceptor.local_endpoint().address().to_string(),
                         m_acceptor.local_endpoint().port());
            startAccept();
            m_io_context.run();
            spdlog::info("Control server shutdown");
        } catch (std::exception& e) {
            spdlog::error("Error on command server worker thread: {}", e.what());
        }
    }
}