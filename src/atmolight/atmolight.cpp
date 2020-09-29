#include <CLI/Error.hpp>
#include <CLI/App.hpp>
#include <CLI/Formatter.hpp>
#include <CLI/Config.hpp>
#include <CLI/CLI.hpp>
#include <atmo/application.hpp>

#ifdef __unix__

#include <csignal>

atmo::Application* G_APP{nullptr};

void interrupt(int) {
    if (G_APP) {
        G_APP->interrupt();
    }
}
#endif

int main(int argc, char** argv) {
    CLI::App cli{"atmolight"};
    std::string config_file{};
    cli.add_option("-c,--config_file", config_file, "the YAML configuration file")
            ->required();
    CLI11_PARSE(cli, argc, argv);

    atmo::Application app{config_file};

#ifdef __unix__
    G_APP = &app;
    std::signal(SIGINT, interrupt);
#endif

    app.run();

#ifdef __unix__
    G_APP = nullptr;
#endif

    return 0;
}