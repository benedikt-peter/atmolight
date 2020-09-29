//
// Created by Benedikt on 20.08.2020.
//

#pragma once

#include <cstddef>
#include "types.hpp"

namespace atmo {

    /**
     * A CaptureDevice captures and analyzes an input stream (e.g. a video input from a television) and creates a number
     * of output channels out of it. The number and layout of output channels are usually configurable by the user.
     */
    class CaptureDevice {
    public:
        virtual ~CaptureDevice() = default;

        /**
         * Capture and process one frame of input information. The output of this method has to match to the
         * configuration given by the user. This call may block until input data is available.
         *
         * @return the output color channels according to a user provided configuration
         */
        [[nodiscard]]
        virtual Channels capture() = 0;
    };

}