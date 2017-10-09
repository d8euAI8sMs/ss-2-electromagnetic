#pragma once

#include <util/common/math/dsolve.h>

namespace model
{

    /**
     * Motion law in a static isotropic electromagnetic field
     *
     * (let $x be 'x vector', [$x ^ $y] be a cross-product)
     *
     *      $F/m = $a = q/m $E + q/m [$v ^ $B]
     *
     * Let e = q/m E/e0, b = q/m B/b0, u = v/u0
     *
     *      $a = $v` = e0 $e + b0 u0 [$u ^ $b]
     *
     * and, dividing by u0
     *
     *      $u` = e0/u0 $e + b0 [$u ^ $b]
     *           ^~~~~~ c1  ^~~ c2
     *
     *      $x` = $v = u0 $u
     */

    /**
     *  summary:
     *
     *      Creates a differential equation function
     *      specific to the problem of electron law of
     *      motion in static isotropic electromagnetic field
     *
     *  params:
     *
     *      e  - electrostatic field (normalized, i.e. e = E / e0)
     *      b  - static magnetic field (normalized, i.e. b = B / b0)
     *      c1 - e0 / u0
     *      c2 - b0
     */
    inline math::dfunc3_t make_electromagnetic_dfunc
    (
        const math::v3 & e,
        const math::v3 & b,
        double c1,
        double c2
    )
    {
        return [&] (double, const math::v3 &, const math::v3 & u)
        {
            return c1 * e + c2 * (u ^ b);
        };
    }
}
