#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <iostream>
#include <Eigen>
#include <vector>

#include "global.hpp"
#include "ray.hpp"
#include "intersection.hpp"
#include "material.hpp"

using namespace Eigen;

class aabb {
public:
	Vector3d p_min, p_max;
};

class hittableObject {
private:
	mutable aabb testtest;
	
public:
	mutable aabb bbox;
	std::shared_ptr<Material> material;

	virtual ~hittableObject() = default;

	virtual bool intersect_ray(const ray& r, double& r_inter_max_t, double& r_inter_min_t, intersection& inter) const = 0;
	virtual void get_aabb() const;
};


class hittableObjectsList: public hittableObject {
public:
	std::vector<std::shared_ptr<hittableObject>> objects_list;

	hittableObjectsList() {}
	hittableObjectsList(std::shared_ptr<hittableObject> object, std::shared_ptr<Material> m) { add(object, m); }

	void clear() { objects_list.clear(); }
	void add(std::shared_ptr<hittableObject> object, std::shared_ptr<Material> m) {
		object->material = m;
		objects_list.push_back(object); 
	}

	bool intersect_ray(const ray& r, double& r_inter_max_t, double& r_inter_min_t, intersection& inter) const override;
	void get_aabb() const override;
};


class sphere :public hittableObject {
private:
	Vector3d center;  // ÇòÐÄ
	double radius;  // ÇòµÄ°ë¾¶

public:
	sphere() {
		center = Vector3d::Zero();
		radius = 0.0;
	};

	sphere(const Vector3d center_point, const double radius_length) {
		center = center_point;
		radius = radius_length;
	}
	
	Vector3d get_center() { return center; }
	double get_radius() { return radius; }
	bool intersect_ray(const ray& r, double& r_inter_max_t, double& r_inter_min_t, intersection& inter) const override;
	void get_aabb() const override;
};

#endif // !OBJECTS_HPP