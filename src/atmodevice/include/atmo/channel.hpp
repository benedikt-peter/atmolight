//
// Created by Benedikt on 14.09.2020.
//

#pragma once

#include <cstddef>
#include <cstdint>

namespace atmo {

    /**
       * Colors are represented as RGB tuples with 8bit precision for every color.
       */
    struct Color {
        /**
         * Constructor.
         */
        Color() : red{0}, green{0}, blue{0} {}

        /**
         * Constructor.
         *
         * @param red the red value (0...255)
         * @param green the green value (0...255)
         * @param blue the blue value (0...255)
         */
        Color(uint8_t red, uint8_t green, uint8_t blue) : red{red}, green{green}, blue{blue} {}

        /**
         * The red value (0...255).
         */
        std::uint8_t red;

        /**
         * The green value (0...255).
         */
        std::uint8_t green;

        /**
         * The blue value (0...255).
         */
        std::uint8_t blue;
    };

    /**
     * The channel index, specific to one device.
     */
    using Channel = std::size_t;

};