#include <vector>

#include "camera.hpp"
#include "global.hpp"

void camera::render(const hittableObjectsList& project_scene) {
	
	FILE* fp;
	fopen_s(&fp, "binary.ppm", "wb");
	(void)fprintf(fp, "P6\n%d %d\n255\n", FRAME_WIDTH, FRAME_HEIGHT);

	int m = 0;

	std::vector<Vector3d> frame_buffer((FRAME_WIDTH + 1) * (FRAME_HEIGHT + 1));  // 

	// For each pixel
	for (int j = 0; j < (FRAME_HEIGHT + 1); j++) {  // 
		std::clog << "\r---Scanlines remaining: " << (FRAME_HEIGHT - j) << ' ' << std::flush;

		for (int i = 0; i < (FRAME_WIDTH + 1); i++) {  // 

			Vector3d color_vec = Vector3d::Zero();

			if (true) {  // (i == 295 && j == 111) || (i == 307 && j == 122)

				//std::cout << "----------------------------" << i << "," << j << std::endl;

				for (int k = 0; k < SAMPLES_PER_PIXEL; k++) {
					color_vec += (ray_sphere_color(get_random_ray(i, j), 0, project_scene) / SAMPLES_PER_PIXEL);
					//std::cout << "Result color: " << color_vec << std::endl;
				}

				m++;

				frame_buffer[j * FRAME_WIDTH + i] = color_vec;
			}
		}
	}

	for (int j = 0; j < FRAME_HEIGHT; j++) {

		for (int i = 0; i < FRAME_WIDTH; i++) {

			// 4像素平均抗锯齿
			Vector3d col = ((frame_buffer[j * FRAME_WIDTH + i] + frame_buffer[(j + 1) * FRAME_WIDTH + i] + frame_buffer[j * FRAME_WIDTH + i + 1] + frame_buffer[(j + 1) * FRAME_WIDTH + i + 1]) / 4.0);
			// 无抗锯齿
			//Vector3d col = frame_buffer[j * FRAME_WIDTH + i];

			static unsigned char color[3];

			int r_scaled = 255 * std::pow(std::max(0.0, std::min(1.0, col.x())), GAMMA);
			int g_scaled = 255 * std::pow(std::max(0.0, std::min(1.0, col.y())), GAMMA);
			int b_scaled = 255 * std::pow(std::max(0.0, std::min(1.0, col.z())), GAMMA);

				
			color[0] = (unsigned char)(r_scaled);
			color[1] = (unsigned char)(g_scaled);
			color[2] = (unsigned char)(b_scaled);

			fwrite(color, 1, 3, fp);
		}
	}


	fclose(fp);
}

ray camera::get_random_ray(int i, int j) const {
	double scale = tan((CAMERA_FOV * 0.5) * M_PI / 180.0);  // h
	double image_aspect_ratio = FRAME_WIDTH / (double)FRAME_HEIGHT;

	//double focal_length = (camera_pos - camera_dir).norm();
	double viewport_height = 2 * scale * focus_dist;
	double viewport_width = viewport_height * image_aspect_ratio;

	Vector3d viewport_u = viewport_width * coord_e1;  // 
	Vector3d viewport_v = viewport_height * (-coord_e2);  // 

	Vector3d pixel_delta_u = viewport_u / FRAME_WIDTH;
	Vector3d pixel_delta_v = viewport_v / FRAME_HEIGHT;

	Vector3d viewport_upper_left = camera_pos - (focus_dist * coord_e3) - viewport_u / 2 - viewport_v / 2;

	/*
	double x = (2 * (i + 0.5) / (double)FRAME_WIDTH - 1) * image_aspect_ratio * scale;
	double y = (1 - 2 * (j + 0.5) / (double)FRAME_HEIGHT) * scale;

	double z;

	Vector3d dir = Vector3d(x, y, -1.0).normalized();
	return ray(camera_pos, dir); */

	Vector3d pixel_center = (viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v)) + (i * pixel_delta_u) + (j * pixel_delta_v);

	double defocus_radius = focus_dist * tan((defocus_angle * 0.5) * M_PI / 180.0);
	defocus_disk_u = coord_e1 * defocus_radius;
	defocus_disk_v = coord_e2 * defocus_radius;


	Vector3d orig;
	if (defocus_angle <= 0) { orig = camera_pos; }
	else { orig = defocus_disk_sample(); }

	Vector3d dir = (pixel_center - orig).normalized();

	return ray(orig, dir);

	// 随机光线方向抗锯齿
	/*
	Vector3d offset = Vector3d(random_double() - 0.5, random_double() - 0.5, 0);
	Vector3d pixel_sample = pixel00_loc
		+ ((i + offset.x()) * pixel_delta_u)
		+ ((j + offset.y()) * pixel_delta_v);

	Vector3d dir = (pixel_sample - camera_pos).normalized();*/
}

Vector3d camera::defocus_disk_sample() const{
	Vector3d p = random_in_unit_disk();
	return camera_pos + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
}