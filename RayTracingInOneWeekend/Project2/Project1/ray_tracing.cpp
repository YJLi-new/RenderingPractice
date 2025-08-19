#include <iostream>
#include <Eigen>

#include "global.hpp"
#include "ray.hpp"
#include "objects.hpp"


// 蓝白渐变
Vector3d ray_background_color(const ray& r, int bounce_num) {
	Vector3d unit_direction = r.get_direction().normalized();
	double a = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - a) * Vector3d(1.0, 1.0, 1.0) + a * Vector3d(0.5, 0.7, 1.0);
	//return Vector3d(0.82, 0.9, 1.0);
}

Vector3d ray_sphere_color(const ray& r, int bounce_num, const hittableObjectsList& project_scene, const Vector3d& background, int max_depth) {
	if (bounce_num > max_depth) { return Vector3d::Zero(); }  // 这里检查max_depth
	double t1 = INFINITYd;
	double t2 = INFINITYd;
	intersection inter;
	if (project_scene.intersect_ray(r, t1, t2, inter)) {

		Vector3d new_ray_dir;
		Vector3d color = inter.material->get_color();
		if (inter.material && inter.material->has_texture()) {
			color = inter.material->get_texture()->value(inter.u, inter.v, inter.position);
		}
		Vector3d color_from_emission = Vector3d::Zero();
		if (inter.material) {
			color_from_emission = inter.material->emitted(inter.u, inter.v, inter.position);
		}
		//double fuzz_coeff = 0.2;
		double fuzz_coeff = random_double(0, 0.5);
		double refraction_coeff_from = 1.5;
		double refraction_coeff_to = 1.0;

		Vector3d unit_ray_dir;
		double cos_angle, sin_angle, ri;

		Vector3d color_at_ray;

		
		//std::cout << "\n---------------\ninter position: " << inter.position << std::endl;
		//std::cout << "Bounce Number: " << bounce_num << std::endl;

		switch (inter.material->get_type()) {

		case DIRECT:
			return color;

		case LIGHT_SOURCE:
			return color_from_emission;

		case DIFFUSE_NO_RANDOM:
			new_ray_dir = inter.normal + random_unit_vector();
			return color_from_emission + ray_sphere_color(ray(inter.position + EPSILON * new_ray_dir, new_ray_dir), bounce_num + 1, project_scene, background, max_depth).cwiseProduct(color);
			break;

		case METAL:
			new_ray_dir = reflect(r.get_direction(), inter.normal).normalized();
			return color_from_emission + ray_sphere_color(ray(inter.position + EPSILON * new_ray_dir, new_ray_dir), bounce_num + 1, project_scene, background, max_depth).cwiseProduct(color);
			break;

		case DIFFUSE_NORMAL:

			//std::cout << "DIFFUSE_NORMAL" << std::endl;
			//std::cout << "inter is_front_face: " << inter.is_front_face << std::endl;

			//Vector3d normalzd = (r.positionAtTheTime(t2) - sph.get_current_center());  // orig + h*dir/a - sqrtd*dir/a - center
			//normalzd.normalize();
			//dir = random_on_hemisphere(inter.normal);
			new_ray_dir = inter.normal + random_unit_vector();
			color = 0.5 * (inter.normal + Vector3d(1.0, 1.0, 1.0));
			return color_from_emission + ray_sphere_color(ray(inter.position + EPSILON * new_ray_dir, new_ray_dir), bounce_num + 1, project_scene, background, max_depth).cwiseProduct(color);
			break;

		case DIFFUSE_LAMBERTIAN:
			
			//std::cout << "DIFFUSE" << std::endl;
			//std::cout << "inter is_front_face: " << inter.is_front_face << std::endl;

			new_ray_dir = inter.normal + random_unit_vector();
			if (is_near_zero_vec3d(new_ray_dir)) {
				new_ray_dir = inter.normal;
			}
			return color_from_emission + ray_sphere_color(ray(inter.position + EPSILON * new_ray_dir, new_ray_dir), bounce_num + 1, project_scene, background, max_depth).cwiseProduct(color);

		case METAL_FUZZ:
			new_ray_dir = (r.get_direction() - 2 * r.get_direction().dot(inter.normal) * inter.normal).normalized() + fuzz_coeff * random_unit_vector();
			return color_from_emission + ray_sphere_color(ray(inter.position + EPSILON * new_ray_dir, new_ray_dir), bounce_num + 1, project_scene, background, max_depth).cwiseProduct(color);
			break;

		case GLASS:
			unit_ray_dir = r.get_direction().normalized();
			cos_angle = std::min((-unit_ray_dir).dot(inter.normal), 1.0);
			sin_angle = std::sqrt(1.0 - cos_angle * cos_angle);

			//std::cout << "inter is_front_face: " << inter.is_front_face << std::endl;

			if (inter.is_front_face) {
				ri = refraction_coeff_to / refraction_coeff_from;
			}
			else {
				ri = refraction_coeff_from / refraction_coeff_to;
			}

			//std::cout << "ri * sin_angle: " << ri * sin_angle << std::endl;

			if ((ri * sin_angle > 1.0) || ((reflectance(cos_angle, ri)) > random_double())) {
				new_ray_dir = reflect(unit_ray_dir, inter.normal).normalized();
				//new_ray_dir = refract(unit_ray_dir, inter.normal, ri).normalized();

				//std::cout << "Reflect Ray Direction: " << new_ray_dir << std::endl;
				//std::cout << "ri: "<< ri << std::endl;
			}
			else {
				//new_ray_dir = reflect(unit_ray_dir, inter.normal).normalized();
				new_ray_dir = refract(unit_ray_dir, inter.normal, ri).normalized();

				//std::cout << "Refract: "<< "Bounce Number: " << bounce_num << std::endl;
				//std::cout << "ri: " << ri << std::endl;

				//new_r = ray(inter.position + EPSILON * new_ray_dir, new_ray_dir);
			}

			//color_at_ray = ray_sphere_color(ray(inter.position, new_ray_dir), bounce_num + 1, project_scene).cwiseProduct(Vector3d);
			//std::cout << "Color at Ray: (bounce) " << bounce_num << "(Vector3d) " << color_at_ray << "\n" << std::endl;
			//return color_at_ray;
			return color_from_emission + ray_sphere_color(ray(inter.position + EPSILON * new_ray_dir, new_ray_dir), bounce_num + 1, project_scene, background, max_depth).cwiseProduct(color);
			break;

		case GLASS_BUBBLE:
			unit_ray_dir = r.get_direction().normalized();
			cos_angle = std::min((-unit_ray_dir).dot(inter.normal), 1.0);
			sin_angle = std::sqrt(1.0 - cos_angle * cos_angle);

			if (!inter.is_front_face) { ri = refraction_coeff_to / refraction_coeff_from; }  // 
			else { ri = refraction_coeff_from / refraction_coeff_to; }

			if ((ri * sin_angle > 1.0) || ((reflectance(cos_angle, ri)) > random_double())) { new_ray_dir = reflect(unit_ray_dir, inter.normal).normalized(); }
			else { new_ray_dir = refract(unit_ray_dir, inter.normal, ri).normalized(); }

			return color_from_emission + ray_sphere_color(ray(inter.position + EPSILON * new_ray_dir, new_ray_dir), bounce_num + 1, project_scene, background, max_depth).cwiseProduct(color);
			break;

		case GLASS_FUZZ:
			unit_ray_dir = r.get_direction().normalized();
			cos_angle = std::min((-unit_ray_dir).dot(inter.normal), 1.0);
			sin_angle = std::sqrt(1.0 - cos_angle * cos_angle);

			if (inter.is_front_face) { ri = refraction_coeff_to / refraction_coeff_from; }
			else { ri = refraction_coeff_from / refraction_coeff_to; }

			if ((ri * sin_angle > 1.0) || ((reflectance(cos_angle, ri)) > random_double())) { new_ray_dir = reflect(unit_ray_dir, inter.normal).normalized(); }
			else { new_ray_dir = refract(unit_ray_dir, inter.normal, ri).normalized(); }

			new_ray_dir += fuzz_coeff * random_unit_vector();;

			return color_from_emission + ray_sphere_color(ray(inter.position + EPSILON * new_ray_dir, new_ray_dir), bounce_num + 1, project_scene, background, max_depth).cwiseProduct(color);
			break;

		case ISOTROPIC:
			Vector3d new_ray_dir = random_unit_vector();
			return color_from_emission + ray_sphere_color(ray(inter.position + EPSILON * new_ray_dir, new_ray_dir), bounce_num + 1, project_scene, background, max_depth).cwiseProduct(color);
			break;

		}
	}
	else {
		//Vector3d bg_color = ray_background_color(r, bounce_num);
		//std::cout << "\nBackground here: " << bg_color << std::endl;
		//return bg_color;

		//return ray_background_color(r, bounce_num);
		//return Vector3d::Zero();
		return background;
		}
}