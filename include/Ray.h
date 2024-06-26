#pragma once

#include "heliostat_tracking_export.h"
#include "gcf.h"
#include "vec3d.h"

class HELIOSTAT_TRACKING_EXPORT Ray
{
public:
    Ray():
        tMin(gcf::Epsilon), tMax(gcf::infinity)
    {
    }

    Ray(const vec3d& origin, const vec3d& direction, double start = gcf::Epsilon, double end = gcf::infinity):
        origin(origin), tMin(start), tMax(end)
    {
        setDirection(direction);
    }

    vec3d point(double t) const {return origin + m_direction*t;}

    const vec3d& direction() const {return m_direction;}

    const vec3d& invDirection() const {return m_directionInv;}

    void setDirection(const vec3d& direction)
    {
        m_direction = direction;
        m_directionInv.x = 1./m_direction.x;
        m_directionInv.y = 1./m_direction.y;
        m_directionInv.z = 1./m_direction.z;
    }

    bool operator==(const Ray& ray) const
    {
        if (this == &ray) return true;
        return origin == ray.origin && m_direction == ray.m_direction &&
               fabs(tMin - ray.tMin) < gcf::Epsilon && fabs(tMax - ray.tMax) < gcf::Epsilon;
    }

    vec3d origin;
    mutable double tMin;
    mutable double tMax;

private:
    vec3d m_direction;
    vec3d m_directionInv;
};
