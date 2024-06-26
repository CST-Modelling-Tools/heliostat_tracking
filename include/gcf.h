#pragma once

#include <cfloat>
#include <cmath>
#include <string>

#include "heliostat_tracking_export.h"

// global constants and functions
namespace gcf
{
    const double Pi = 3.1415926535897932385;
    const double TwoPi = 2.*Pi;
    const double degree = Pi/180.0;
    const double infinity = HUGE_VAL;
    const double Epsilon = DBL_EPSILON;

    template<class T>
    bool equals(T x, T y)
    {
        return std::abs(x - y) < std::numeric_limits<T>::epsilon();
    }

    double elevationAngleActuatorLength( double elevation_angle );
    double hourAngleActuatorLength( double hour_angle );
    
    HELIOSTAT_TRACKING_EXPORT double normalizeAngle(double phi, double phi0);
    HELIOSTAT_TRACKING_EXPORT void SevereError(std::string errorMessage);
 
}
