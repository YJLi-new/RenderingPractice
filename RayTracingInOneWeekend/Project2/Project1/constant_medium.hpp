// constant_medium.hpp  (NEW)
#ifndef CONSTANT_MEDIUM_HPP
#define CONSTANT_MEDIUM_HPP

#include <iostream>
#include <memory>
#include <cmath>
#include "objects.hpp"     // for hittableObject, aabb, etc.
#include "texture.hpp"     // for texture / solid_color
#include "global.hpp"      // random_double(), INFINITYd

// Simple extension: add an "isotropic" scattering material via MaterialType::ISOTROPIC.
// We'll construct a Material with a texture or solid color (albedo) and the ISOTROPIC type.
class constant_medium : public hittableObject {
public:
    constant_medium(std::shared_ptr<hittableObject> boundary,
        double density,
        std::shared_ptr<texture> tex)
        : boundary_(std::move(boundary)),
        neg_inv_density_(-1.0 / density) {
        phase_function_ = std::make_shared<Material>(ISOTROPIC, tex);
    }

    constant_medium(std::shared_ptr<hittableObject> boundary,
        double density,
        const Vector3d& albedo)
        : boundary_(std::move(boundary)),
        neg_inv_density_(-1.0 / density) {
        auto tex = std::make_shared<solid_color>(albedo);
        phase_function_ = std::make_shared<Material>(ISOTROPIC, tex);
    }

    // Ray/volume interaction: sample an exponential free-flight distance.
    bool intersect_ray(const ray& r, double& r_inter_max_t, double& r_inter_min_t, intersection& inter) const override {
        // Find boundary enter/exit parameters by intersecting boundary once; it outputs both roots.
        double tmax = +INFINITYd;
        double tmin = -INFINITYd;
        intersection tmp;

        if (!boundary_->intersect_ray(r, tmax, tmin, tmp)) { return false; }

        // Order the two roots
        double t0 = std::min(tmin, tmax);
        double t1 = std::max(tmin, tmax);

        if (t0 < 0.0) t0 = 0.0;

        const double ray_length = r.get_direction().norm();
        const double distance_inside = std::max(0.0, (t1 - t0)) * ray_length;

        const double hit_distance = neg_inv_density_ * std::log(random_double()); // log in (−∞,0]

        if (hit_distance > distance_inside) return false;

        const double t = t0 + hit_distance / ray_length;

        inter.happened = true;
        inter.time_spread_to = t;
        inter.position = r.positionAtTheTime(t);
        inter.normal = Vector3d(1, 0, 0);
        inter.is_front_face = true;
        inter.material = phase_function_;

        return true;
    }

    aabb get_aabb() const override {
        return boundary_->get_aabb();
    }

private:
    std::shared_ptr<hittableObject> boundary_;
    double neg_inv_density_;
    std::shared_ptr<Material> phase_function_;
};

#endif // CONSTANT_MEDIUM_HPP
