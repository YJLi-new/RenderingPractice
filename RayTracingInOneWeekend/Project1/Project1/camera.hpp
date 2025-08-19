#ifndef CAMERA_H
#define CAMERA_H

#include <Eigen>
#include "global.hpp"
#include "objects.hpp"
#include "ray.hpp"
#include "ray_tracing.hpp"

class camera {
public:
	int FRAME_WIDTH;
	int FRAME_HEIGHT;
	const double CAMERA_FOV = 20;
	//const double GAMMA = 1.0;  // 伽马矫正指数
	const double GAMMA = 1.0 / 2.2;  // 伽马矫正指数
	int SAMPLES_PER_PIXEL = 32;

	Vector3d camera_pos;
	Vector3d camera_dir;
	Vector3d camera_vup;  // (rotated up(?) direction

	Vector3d coord_e1;
	Vector3d coord_e2;
	Vector3d coord_e3;

	double defocus_angle = 0.6;  // 散焦角度(光圈大小)
	double focus_dist = 10.0;  // 对焦距离

	mutable Vector3d defocus_disk_u;  // 虚焦光斑椭圆的水平半轴
	mutable Vector3d defocus_disk_v;  // 虚焦光斑椭圆的竖直半轴

	void render(const hittableObjectsList& project_scene);

	void initialize(int w = 256, int h = 128, Vector3d pos = Vector3d(0, 0, 0), Vector3d dir = Vector3d(0, 0, -1.0), Vector3d vup = Vector3d(0, 1.0, 0)) {
		FRAME_WIDTH = w;
		FRAME_HEIGHT = h;
		camera_pos = pos;
		camera_dir = dir;
		camera_vup = vup;

		coord_e3 = (pos - dir).normalized();  // w
		coord_e1 = (vup.cross(coord_e3)).normalized();  // u
		coord_e2 = coord_e3.cross(coord_e1);  // v
	}

	ray get_random_ray(int i, int j) const;
	Vector3d defocus_disk_sample() const;
};

#endif // CAMERA_H