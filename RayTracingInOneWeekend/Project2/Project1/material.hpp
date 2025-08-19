#include <iostream>
#include "global.hpp"
#include <memory>
#include "texture.hpp"

#ifndef MATERIAL_HPP
#define MATERIAL_HPP

enum MaterialType { DIFFUSE_NO_RANDOM, METAL, DIFFUSE_NORMAL, DIFFUSE_LAMBERTIAN, METAL_FUZZ, GLASS, GLASS_BUBBLE, GLASS_FUZZ, DIRECT, LIGHT_SOURCE, ISOTROPIC };

class Material {
private:
	MaterialType type;
	Vector3d color;
	std::shared_ptr<texture> albedo_texture;

public:
	Material() { type = DIFFUSE_NO_RANDOM, color = Vector3d(0.0, 0.0, 0.0); }
	Material(MaterialType t, Vector3d c) { type = t, color = c; }
	Material(MaterialType t, std::shared_ptr<texture> tex) {
		type = t, color = Vector3d::Zero(), albedo_texture = tex;
	}

	void set_type(MaterialType m) { type = m; }
	MaterialType get_type() { return type; }

	void set_color(Vector3d c) { color = c; }
	Vector3d get_color() { return color; }

	std::shared_ptr<texture> get_texture() const { return albedo_texture; }
	bool has_texture() const { return (bool)albedo_texture; }

	Vector3d emitted(double u, double v, const Vector3d& p) const {
		if (type == LIGHT_SOURCE) {
			if (albedo_texture) {
				return albedo_texture->value(u, v, p);
			}

			return color;
		}
		return Vector3d::Zero();
		
	}
};

#endif // !MATERIAL_HPP