#ifndef RAY_HPP

#define RAY_HPP

#include <iostream>
#include <Eigen>

using namespace Eigen;


class ray {

private:
	Vector3d origin;
	Vector3d direction;
	double tm;

public:
	ray() {
		origin = Vector3d::Zero();
		direction = Vector3d::Zero();
		tm = 0.0;
	}
	ray(const Vector3d& origin_point, const Vector3d& direction_vec) {
		origin = origin_point;
		direction = direction_vec;
		tm = 0.0;
	}
	ray(const Vector3d& origin_point, const Vector3d& direction_vec, double time) {
		origin = origin_point;
		direction = direction_vec;
		tm = time;
	}

	Vector3d positionAtTheTime(double t) const {
		return (origin + t * direction);
	}

	const Vector3d& get_origin() const { return origin; }
	const Vector3d& get_direction() const { return direction; }
	double time() const { return tm; }
};

#endif // !RAY_HPP