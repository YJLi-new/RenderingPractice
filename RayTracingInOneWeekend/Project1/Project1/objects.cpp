#include <cmath>
#include "objects.hpp"

bool sphere::intersect_ray(const ray& r, double& r_inter_max_t, double& r_inter_min_t, intersection& inter) const {
	Vector3d dir = r.get_direction();
	Vector3d orig = r.get_origin();

	// the equation: t^2 *(dir(dot*)dir) + 2*t*((O-C)(dot*)dir + ((O-C)(dot*)(O-C)-radius^2) = 0
	Vector3d L = center - orig;

	double a = dir.dot(dir);
	double c = L.dot(L) - radius * radius;
	double h = dir.dot(L);  //double b = -2.0 * L.dot(dir);

	double discriminant = h * h - a * c;  //double discriminant = b * b - 4 * a * c;

	if (discriminant < 0) {
		return false; // No intersection
	}
	else {
		inter.material = material;

		double sqrtd = std::sqrt(discriminant);
		r_inter_max_t = (h + sqrtd) / a;
		r_inter_min_t = (h - sqrtd) / a;

		Vector3d rd = center - orig;
		if ((rd.norm() - radius) < EPSILON && rd.dot(dir) > 0.0) {
			inter.time_spread_to = r_inter_max_t;
		}
		else {
			inter.time_spread_to = r_inter_min_t;
		}

		inter.position = r.positionAtTheTime(inter.time_spread_to);

		Vector3d normalzd = (inter.position - center).normalized();

		if (r.get_direction().dot(normalzd) > 0.0) {  // 击中内表面
			inter.normal = -normalzd;
			inter.is_front_face = false;
		}
		else {  // 击中外表面
			inter.normal = normalzd;
			inter.is_front_face = true;
		}

		return true; // Do intersect
	}
}

bool hittableObjectsList::intersect_ray(const ray& r, double& r_inter_max_t, double& r_inter_min_t, intersection& inter) const {
	intersection temp_inter;
	temp_inter.time_spread_to = r_inter_max_t;
	//int n = 1;
	for (const auto& object : objects_list) {
		//std::clog << n;
		bool is_intersected_ray = object->intersect_ray(r, r_inter_max_t, r_inter_min_t, temp_inter);
		if (is_intersected_ray) {
			//std::clog << "yes";
			if (temp_inter.time_spread_to > 0 && inter.time_spread_to > temp_inter.time_spread_to) {
				temp_inter.happened = true;
				inter = temp_inter;
				//std::cout << inter.position << std::endl;
			}
		}
		//else { std::clog << "no"; }
		//n++;
	}

	return inter.happened;
}

void hittableObjectsList::get_aabb() const {
	if (objects_list.empty()) {
		return;
	}

	objects_list[0]->get_aabb();
	bbox = objects_list[0]->bbox;

	for (const auto& object : objects_list) {
		object->get_aabb();
	}
}

void hittableObject::get_aabb() const {
	// 此为纯virtual函数
}

void sphere::get_aabb() const {
	Vector3d delta(radius, radius, radius);
	bbox.p_min = center - delta;  // 
	bbox.p_max = center + delta;  // 
}