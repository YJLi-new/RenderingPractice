#include <cmath>
#include "objects.hpp"

bool sphere::intersect_ray(const ray& r, double& r_inter_max_t, double& r_inter_min_t, intersection& inter) const {
    const Vector3d dir = r.get_direction();
    const Vector3d orig = r.get_origin();
    const Vector3d C = get_current_center(r.time());
    const Vector3d OC = orig - C;

    const double a = dir.dot(dir);
    const double h = -dir.dot(OC);
    const double c = OC.dot(OC) - radius * radius;

    const double disc = h * h - a * c;
    if (disc < 0.0) return false;

    const double sqrtd = std::sqrt(disc);
    double t0 = (h - sqrtd) / a;
    double t1 = (h + sqrtd) / a;
    if (t0 > t1) std::swap(t0, t1);

    double t = t0;
    if (t <= EPSILON) t = t1;
    if (t <= EPSILON) return false;

    inter.time_spread_to = t;
    inter.position = r.positionAtTheTime(t);

    Vector3d outward = (inter.position - C).normalized();
    inter.is_front_face = dir.dot(outward) < 0.0;
    inter.normal = inter.is_front_face ? outward : -outward;

    inter.material = material;
    get_sphere_uv(outward, inter.u, inter.v);

    inter.happened = true;

	return true;  // Do intersect_ray
}

bool hittableObjectsList::intersect_ray(const ray& r, double& r_inter_max_t, double& r_inter_min_t, intersection& inter) const {
	inter.happened = false;
	inter.time_spread_to = std::numeric_limits<double>::infinity();

	intersection tmp;
	for (const auto& obj : objects_list) {
		double tmax = std::numeric_limits<double>::infinity();
		double tmin = -std::numeric_limits<double>::infinity();
		if (obj->intersect_ray(r, tmax, tmin, tmp)) {
			if (tmp.time_spread_to > EPSILON && tmp.time_spread_to < inter.time_spread_to) {
				inter = tmp;
				inter.happened = true;
			}
		}
	}
	return inter.happened;
}

aabb hittableObjectsList::get_aabb() const {
	return bbox;
}

aabb sphere::get_aabb() const {
	return bbox;
}