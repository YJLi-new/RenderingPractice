#ifndef INTERSECTION_HPP
#define INTERSECTION_HPP

#include "global.hpp"
#include "ray.hpp"
#include "material.hpp"

class intersection {
public:
	Vector3d position;
	Vector3d normal;
	double time_spread_to = INFINITYd;
	bool happened = false;
	std::shared_ptr<Material> material;  // Material material;
	bool is_front_face;
};

#endif // !INTERSECTION_HPP
