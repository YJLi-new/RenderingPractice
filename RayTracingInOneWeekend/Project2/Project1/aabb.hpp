#ifndef AABB_HPP
#define AABB_HPP

#include "global.hpp"
#include "ray.hpp"
#include <cmath>
#include <algorithm>

class interval {
public:
    double min, max;

    interval() : min(+INFINITYd), max(-INFINITYd) {}
    interval(double _min, double _max) : min(_min), max(_max) {}

    interval(const interval& a, const interval& b) {
        min = std::min(a.min, b.min);
        max = std::max(a.max, b.max);
    }

    double size() const {
        return max - min;
    }
    

    static const interval empty, universe;

    inline interval expand(double delta) const {
        if (size() >= delta) return *this;
        double mid = 0.5 * (min + max);
        double half = 0.5 * delta;
        return interval(mid - half, mid + half);
    }
};

inline const interval interval::empty = interval(+INFINITYd, -INFINITYd);
inline const interval interval::universe = interval(-INFINITYd, +INFINITYd);

class aabb {
public:
    interval x, y, z;

    aabb() {}
    aabb(const interval& ix, const interval& iy, const interval& iz) : x(ix), y(iy), z(iz) { pad_to_minimums(); }

    aabb(const Vector3d& a, const Vector3d& b) {
        x = interval(std::min(a.x(), b.x()), std::max(a.x(), b.x()));
        y = interval(std::min(a.y(), b.y()), std::max(a.y(), b.y()));
        z = interval(std::min(a.z(), b.z()), std::max(a.z(), b.z()));
    }

    aabb(const aabb& box0, const aabb& box1) {
        x = interval(box0.x, box1.x);
        y = interval(box0.y, box1.y);
        z = interval(box0.z, box1.z);
        pad_to_minimums();
    }

    const interval& axis_interval(int n) const {
        if (n == 1) return y;
        if (n == 2) return z;
        return x;
    }

    bool intersect_ray(const ray& r, interval ray_t) const {
        for (int axis = 0; axis < 3; axis++) {
            const interval& ax = axis_interval(axis);
            const double adinv = 1.0 / r.get_direction()[axis];
            const double orig_comp = r.get_origin()[axis];

            auto t0 = (ax.min - orig_comp) * adinv;
            auto t1 = (ax.max - orig_comp) * adinv;

            if (t0 < t1) {
                if (t0 > ray_t.min) ray_t.min = t0;
                if (t1 < ray_t.max) ray_t.max = t1;
            }
            else {
                if (t1 > ray_t.min) ray_t.min = t1;
                if (t0 < ray_t.max) ray_t.max = t0;
            }

            if (ray_t.max <= ray_t.min)
                return false;
        }
        return true;
    }

    int longest_axis() const {
        if (x.size() > y.size())
            return x.size() > z.size() ? 0 : 2;
        else
            return y.size() > z.size() ? 1 : 2;
    }

    static const aabb empty;

private:
    void pad_to_minimums() {
        const double delta = 1e-4;
        if (x.size() < delta) x = x.expand(delta);
        if (y.size() < delta) y = y.expand(delta);
        if (z.size() < delta) z = z.expand(delta);
    }
};

inline const aabb aabb::empty = aabb(interval::empty, interval::empty, interval::empty);

#endif // AABB_HPP