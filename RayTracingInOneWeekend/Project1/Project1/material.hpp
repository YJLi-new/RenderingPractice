#include <iostream>
#include "global.hpp"

#ifndef MATERIAL_HPP
#define MATERIAL_HPP

enum MaterialType { DIFFUSE_NO_RANDOM, METAL, DIFFUSE_NORMAL, DIFFUSE_LAMBERTIAN, METAL_FUZZ, GLASS, GLASS_BUBBLE, GLASS_FUZZ, DIRECT, LIGHT_SOURCE };

class Material {
private:
	MaterialType type;
	Vector3d color;

public:
	Material() { type = DIFFUSE_NO_RANDOM, color = Vector3d(0.0, 0.0, 0.0); }
	Material(MaterialType t, Vector3d c) { type = t, color = c; }

	void set_type(MaterialType m) { type = m; }
	MaterialType get_type() { return type; }

	void set_color(Vector3d c) { color = c; }
	Vector3d get_color() { return color; }

};

#endif // !MATERIAL_HPP