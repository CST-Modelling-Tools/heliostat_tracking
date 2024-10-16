#include <math.h>
#include "sunpos.h"

// All angles, including longitude and latitude in radians!!
void sunpos(cTime udtTime,cLocation udtLocation, cSunCoordinates *udtSunCoordinates)
{
    // Main variables
    double dElapsedJulianDays;
    double dDecimalHours;
    double dEclipticLongitude;
    double dEclipticObliquity;

    double dAzimuth;
    double dZenithAngle;
    double dBoundedHourAngle;
    double dRightAscension;
    double dDeclination;



    // Auxiliary variables
    double dY;
    double dX;

    // Calculate difference in days between the current Julian Day
    // and JD 2451545.0, which is noon 1 January 2000 Universal Time
    {
        double dJulianDate;
        long int liAux1;
        long int liAux2;
        // Calculate time of the day in UT decimal hours
        dDecimalHours = udtTime.dHours + (udtTime.dMinutes
            + udtTime.dSeconds / 60.0 ) / 60.0;
        // Calculate current Julian Day
        liAux1 =(udtTime.iMonth-14)/12;
        liAux2=(1461*(udtTime.iYear + 4800 + liAux1))/4 + (367*(udtTime.iMonth
            - 2-12*liAux1))/12- (3*((udtTime.iYear + 4900
        + liAux1)/100))/4+udtTime.iDay-32075;
        dJulianDate=(double)(liAux2)-0.5+dDecimalHours/24.0;
        // Calculate difference between current Julian Day and JD 2451545.0
        dElapsedJulianDays = dJulianDate-2451545.0;
    }

    // Calculate ecliptic coordinates (ecliptic longitude and obliquity of the
    // ecliptic in radians but without limiting the angle to be less than 2*Pi
    // (i.e., the result may be greater than 2*Pi)
    {
        double dMeanLongitude;
        double dMeanAnomaly;
        double dOmega;
        dOmega=2.267127827-0.00093003392670*dElapsedJulianDays;
        dMeanLongitude = 4.895036035+0.01720279602*dElapsedJulianDays; // Radians
        dMeanAnomaly = 6.239468336+0.01720200135*dElapsedJulianDays;
        dEclipticLongitude = dMeanLongitude + 0.03338320972*sin( dMeanAnomaly )
            + 0.0003497596876*sin( 2*dMeanAnomaly ) - 0.0001544353226
            - 0.00000868972936*sin(dOmega);
        dEclipticObliquity = 0.4090904909 - 6.213605399e-9*dElapsedJulianDays
            + 0.00004418094944*cos(dOmega);
    }

    // Calculate celestial coordinates ( right ascension and declination ) in radians
    // but without limiting the angle to be less than 2*Pi (i.e., the result may be
    // greater than 2*Pi)
    {
        double dSin_EclipticLongitude;
        dSin_EclipticLongitude= sin( dEclipticLongitude );
        dY = cos( dEclipticObliquity ) * dSin_EclipticLongitude;
        dX = cos( dEclipticLongitude );
        dRightAscension = atan2( dY,dX );
        if( dRightAscension < 0.0 ) dRightAscension = dRightAscension + twopi;
        dDeclination = asin( sin( dEclipticObliquity )*dSin_EclipticLongitude );
    }

    // Calculate local coordinates ( azimuth and zenith angle ) in degrees
    {
        double dGreenwichMeanSiderealTime;
        double dLocalMeanSiderealTime;
        double dHourAngle;
        double dCos_Latitude;
        double dSin_Latitude;
        double dCos_HourAngle;
        double dSin_HourAngle;
        double dParallax;

        dGreenwichMeanSiderealTime = 6.697096103 + 0.06570984737*dElapsedJulianDays + dDecimalHours;

        dLocalMeanSiderealTime = (dGreenwichMeanSiderealTime*15) * degree + udtLocation.dLongitude;
        dHourAngle = dLocalMeanSiderealTime - dRightAscension;
        dCos_Latitude = cos( udtLocation.dLatitude );
        dSin_Latitude = sin( udtLocation.dLatitude );
        dCos_HourAngle= cos( dHourAngle );
        dSin_HourAngle= sin( dHourAngle );
        dZenithAngle = (acos( dCos_Latitude*dCos_HourAngle*cos(dDeclination) + sin( dDeclination )*dSin_Latitude));
        dY = -dSin_HourAngle;
        dX = tan( dDeclination )*dCos_Latitude - dSin_Latitude*dCos_HourAngle;
        dAzimuth = atan2( dY, dX );
        if ( dAzimuth < 0.0 ) dAzimuth = dAzimuth + twopi;
        // Parallax Correction
        dParallax=(dEarthMeanRadius/dAstronomicalUnit) * sin(dZenithAngle);
        dZenithAngle=(dZenithAngle + dParallax);
        dBoundedHourAngle = atan2(dSin_HourAngle, dCos_HourAngle);

        udtSunCoordinates->dAzimuth = dAzimuth;
        udtSunCoordinates->dZenithAngle = dZenithAngle;
        udtSunCoordinates->dBoundedHourAngle = dBoundedHourAngle;
        udtSunCoordinates->dRightAscension = dRightAscension;
        udtSunCoordinates->dDeclination = dDeclination;
    }
}
