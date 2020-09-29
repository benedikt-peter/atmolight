//
// Created by Benedikt on 03.09.2020.
//

#pragma once

#include <vector>
#include <functional>

namespace atmo {

    /**
     * The available modes of the AtmoLight process.
     */
    enum class Mode {
        /**
         * In Control mode all devices and channels can be manually controlled.
         */
        Control,

        /**
         * In Analyzer mode the Analyzer controls one or more devices according to the configuration. Other devices can
         * still be controlled by the user.
         */
        Analyzer
    };

    using GetModesCallback = std::function<std::vector<Mode>()>;
    using GetModeCallback = std::function<Mode()>;
    using SetModeCallback = std::function<void(Mode)>;

    struct ModeCallbacks {
        GetModesCallback get_modes_callback;
        GetModeCallback get_mode_callback;
        SetModeCallback set_mode_callback;
    };

}