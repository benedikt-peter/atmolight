//
// Created by benedikt on 13.03.18.
//

#pragma once

#include <mutex>
#include <condition_variable>
#include <vector>
#include <variant>
#include <memory>
#include <atmo/config.hpp>
#include <atmo/analyzer.hpp>
#include <atmo/devices.hpp>
#include <atmo/control_server.hpp>

namespace atmo {

    /**
     * Main class for AtmoLight application.
     */
    class Application {
    public:
        /**
         * Creates a new AtmoLight instance and loads the configuration from the specified file.
         *
         * @param config_file the file path to load
         */
        explicit Application(const std::string& config_file);

        /**
         * Run the application and block until the application is interrupted by using interrupt().
         */
        void run();

        /**
         * Interrupt the running application and cause it to exit. This method is thread safe and can be called from a
         * signal handler.
         *
         * Note: An interrupted application may not be resumed again.
         */
        void interrupt();

    private:
        std::mutex m_mutex;
        std::condition_variable m_condition_variable;
        bool m_interrupted;
        Configuration m_configuration;
        std::unique_ptr<Devices> m_devices;
        std::unique_ptr<ControlServer> m_control_server;
        std::unique_ptr<Analyzer> m_analyzer;

        [[nodiscard]]
        std::vector<Mode> getModes() const;

        [[nodiscard]]
        Mode getMode() const;

        void setMode(Mode process_mode);
    };

}
