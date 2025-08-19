#include <iostream>

#include "global.hpp"
#include "camera.hpp"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb-master/stb_image_write.h"


int main() {

	hittableObjectsList project_scene;

	/*
	std::shared_ptr<Material> m1 = std::make_shared<Material> (METAL_FUZZ, Vector3d(0.0, 0.5, 0.9));
	std::shared_ptr<Material> m2 = std::make_shared<Material>(DIFFUSE_NORMAL, Vector3d(0.8, 0.8, 0.8));
	std::shared_ptr<Material> m3 = std::make_shared<Material> (GLASS, Vector3d(0.9, 0.9, 0.9));
	std::shared_ptr<Material> m4 = std::make_shared<Material>(GLASS_BUBBLE, Vector3d(1, 1, 1));
	//std::shared_ptr<Material> m3 = std::make_shared<Material>(GLASS, Vector3d(0.5, 0.5, 0.5));
	//std::shared_ptr<Material> m3 = std::make_shared<Material> (GLASS, Vector3d(1.0, 1.0, 1.0));
	std::shared_ptr<Material> m_ground = std::make_shared<Material>(DIFFUSE_LAMBERTIAN, Vector3d(0.5, 1.0, 0.0));

	project_scene.add(std::make_shared<sphere>(Vector3d(1.0, 0, -1), 0.5), m1);
	project_scene.add(std::make_shared<sphere>(Vector3d(0, 0, -1.2), 0.5), m2);
	//project_scene.add(std::make_shared<sphere>(Vector3d(-1.1, 0, -1), 0.5), m3);
	project_scene.add(std::make_shared<sphere>(Vector3d(-1.0, 0, -1), 0.5), m3);
	//project_scene.add(std::make_shared<sphere>(Vector3d(-1.0, 0, -1), 0.4), m4);
	project_scene.add(std::make_shared<sphere>(Vector3d(0, -100.5, -1), 100), m_ground);*/

	std::shared_ptr<Material> material1 = std::make_shared<Material>(GLASS, Vector3d(1.0, 1.0, 1.0));
	project_scene.add(std::make_shared<sphere>(Vector3d(0, 1, 0), 1.0), material1);

	std::shared_ptr<Material> material2 = std::make_shared<Material>(DIFFUSE_LAMBERTIAN, Vector3d(0.4, 0.2, 0.1));
	project_scene.add(std::make_shared<sphere>(Vector3d(-4, 1, 0), 1.0), material2);

	std::shared_ptr<Material> material3 = std::make_shared<Material>(METAL, Vector3d(0.7, 0.6, 0.5));
	project_scene.add(std::make_shared<sphere>(Vector3d(4, 1, 0), 1.0), material3);

	std::shared_ptr<Material> materialkkk = std::make_shared<Material>(LIGHT_SOURCE, Vector3d(1,1,1));
	project_scene.add(std::make_shared<sphere>(Vector3d(-1, 7.5, 0), 5), materialkkk);

	std::shared_ptr<Material> ground_material = std::make_shared<Material>(DIFFUSE_LAMBERTIAN, Vector3d(0.5, 0.5, 0.5));
	project_scene.add(std::make_shared<sphere>(Vector3d(0, -1000, 0), 1000), ground_material);

	int n = 0;
	while (n < 10) {
		for (int a = -11; a < 11; a++) {
			for (int b = -11; b < 11; b++) {
				double choose_mat = random_double();
				Vector3d center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

				if ((center - Vector3d(4, 0.2, 0)).norm() > 0.9) {
					std::shared_ptr<Material> sphere_material;

					if (choose_mat < 0.8) {
						// diffuse
						Vector3d albedo = random_vec3d().cwiseProduct(random_vec3d());
						sphere_material = std::make_shared<Material>(DIFFUSE_LAMBERTIAN, albedo);
						project_scene.add(std::make_shared<sphere>(center, 0.2), sphere_material);
						n++;
					}
					else if (choose_mat < 0.95) {
						// metal
						Vector3d albedo = random_vec3d(0.5, 1);
						sphere_material = std::make_shared<Material>(METAL_FUZZ, albedo);
						project_scene.add(std::make_shared<sphere>(center, 0.2), sphere_material);
						n++;
					}
					else {
						// glass
						sphere_material = std::make_shared<Material>(GLASS, Vector3d(1.0, 1.0, 1.0));
						project_scene.add(std::make_shared<sphere>(center, 0.2), sphere_material);
						n++;
					}
				}
			}
		}
	}

	camera cam;
	//cam.initialize(501, 501, Vector3d(-2.0, 2.0, 1.0), Vector3d(0, 0, -1.0), Vector3d(0, 1.0, 0));
	//cam.initialize(501, 501, Vector3d(0, 0, 0), Vector3d(0, 0, -1.0), Vector3d(0, 1.0, 0));
	//cam.initialize(640, 360, Vector3d(-2.0, 2.0, 1.0), Vector3d(0, 0, -1.0), Vector3d(0, 1.0, 0));
	int window_width = 1280;
	int window_height = 720;
	Vector3d look_from(13, 2, 3);
	Vector3d look_to(0, 0, 0);
	Vector3d cam_vup(0, 1, 0);
	cam.initialize(window_width, window_height, look_from, look_to, cam_vup);
	cam.render(project_scene);

	std::clog << "\rDone.                 \n";
}