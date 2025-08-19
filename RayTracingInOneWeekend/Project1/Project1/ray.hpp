#ifndef RAY_HPP

#define RAY_HPP

#include <iostream>
#include <Eigen>

using namespace Eigen;


class ray {

private:
	Vector3d origin;
	Vector3d direction;

public:
	ray() {
		origin = Vector3d::Zero();
		direction = Vector3d::Zero();
	}
	ray(const Vector3d& origin_point, const Vector3d& direction_vec) {
		origin = origin_point;
		direction = direction_vec;
	}

	Vector3d positionAtTheTime(double time) const {
		return (origin + time * direction);
	}

	const Vector3d& get_origin() const { return origin; }
	const Vector3d& get_direction() const { return direction; }
};

#endif // !RAY_HPP