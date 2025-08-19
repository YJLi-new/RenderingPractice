#include <iostream>

#include "global.hpp"
#include "camera.hpp"
#include "bvh.hpp"
#include "texture.hpp"
#include "perlin.hpp"
#include "objects.hpp"
#include "constant_medium.hpp"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb-master/stb_image_write.h"

void bouncing_spheres() {
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

	std::shared_ptr<Material> materialkkk = std::make_shared<Material>(LIGHT_SOURCE, Vector3d(1, 1, 1));
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

	hittableObjectsList final_scene;
	auto world_bvh = std::make_shared<bvh_node>(project_scene);
	final_scene.add(world_bvh, nullptr);
	cam.render(final_scene);

	std::clog << "\rDone.                 \n";
}

void checkered_spheres() {
	hittableObjectsList project_scene;

	auto checker = std::make_shared<checker_texture>(0.32,
		Vector3d(0.2, 0.3, 0.1),
		Vector3d(0.9, 0.9, 0.9));

	auto ground_tex = checker;
	std::shared_ptr<Material> ground_mat = std::make_shared<Material>(DIFFUSE_LAMBERTIAN, ground_tex);
	project_scene.add(std::make_shared<sphere>(Vector3d(0, -1000, 0), 1000.0), ground_mat);

	auto mat = std::make_shared<Material>(DIFFUSE_LAMBERTIAN, checker);
	project_scene.add(std::make_shared<sphere>(Vector3d(0, -10, 0), 10.0), mat);
	project_scene.add(std::make_shared<sphere>(Vector3d(0, 10, 0), 10.0), mat);

	camera cam;
	int window_width = 400, window_height = 225;
	cam.initialize(window_width, window_height, Vector3d(13, 2, 3), Vector3d(0, 0, 0), Vector3d(0, 1, 0));
	auto bvh_rt = std::make_shared<bvh_node>(project_scene);
	hittableObjectsList final_scene; final_scene.add(bvh_rt, nullptr);
	cam.render(final_scene);
}

void earth() {
	hittableObjectsList project_scene;

	std::shared_ptr<image_texture> earth_tex = std::make_shared<image_texture>("earthmap.jpg");
	std::shared_ptr<Material> earth_mat = std::make_shared<Material>(DIFFUSE_LAMBERTIAN, earth_tex);
	std::shared_ptr<sphere> globe = std::make_shared<sphere>(Vector3d(0, 0, 0), 2.0);

	project_scene.add(globe, earth_mat);

	camera cam;
	cam.initialize(400, 225, Vector3d(0, 0, 12), Vector3d(0, 0, 0), Vector3d(0, 1, 0));

	auto bvh_rt = std::make_shared<bvh_node>(project_scene);
	hittableObjectsList final_scene; final_scene.add(bvh_rt, nullptr);
	cam.render(final_scene);
}

static inline Vector3d rotate_y_point(const Vector3d& p, double degrees) {
	double radians = degrees * M_PI / 180.0;
	double s = std::sin(radians), c = std::cos(radians);
	return Vector3d(c * p.x() + s * p.z(), p.y(), -s * p.x() + c * p.z());
	
}

void final_scene(int image_width, int samples_per_pixel, int max_depth) {
	hittableObjectsList world;
	
	auto ground_tex = std::make_shared<checker_texture>(0.32, Vector3d(0.48, 0.83, 0.53), Vector3d(0.9, 0.9, 0.9));
	auto ground = std::make_shared<Material>(DIFFUSE_LAMBERTIAN, ground_tex);
	world.add(std::make_shared<sphere>(Vector3d(0, -1000, 0), 1000.0), ground);
	
	auto light = std::make_shared<Material>(LIGHT_SOURCE, Vector3d(7, 7, 7));
	world.add(std::make_shared<sphere>(Vector3d(123, 554, 147), 100.0), light);
	//world.add(std::make_shared<quad>(Vector3d(123, 554, 147), Vector3d(300, 0, 0), Vector3d(0, 0, 265), light), nullptr);
	
	Vector3d center1(400, 400, 200);
	Vector3d center2 = center1 + Vector3d(30, 0, 0);
	auto sphere_mat = std::make_shared<Material>(DIFFUSE_LAMBERTIAN, Vector3d(0.7, 0.3, 0.1));
	world.add(std::make_shared<sphere>(center1, center2, 50.0), sphere_mat);
	
	auto glass = std::make_shared<Material>(GLASS, Vector3d(1.0, 1.0, 1.0));
	world.add(std::make_shared<sphere>(Vector3d(260, 150, 45), 50.0), glass);
	auto metal = std::make_shared<Material>(METAL, Vector3d(0.8, 0.8, 0.9));
	world.add(std::make_shared<sphere>(Vector3d(0, 150, 145), 50.0), metal);
	
	auto blue_glass_boundary = std::make_shared<sphere>(Vector3d(360, 150, 145), 70.0);
	world.add(blue_glass_boundary, glass);
	world.add(std::make_shared<constant_medium>(blue_glass_boundary, 0.2, Vector3d(0.2, 0.4, 0.9)), nullptr);
	
	auto global_boundary = std::make_shared<sphere>(Vector3d(0, 0, 0), 5000.0);
	world.add(std::make_shared<constant_medium>(global_boundary, 0.0001, Vector3d(1, 1, 1)), nullptr);
	
	auto earth_tex = std::make_shared<image_texture>("earthmap.jpg");
	auto earth_mat = std::make_shared<Material>(DIFFUSE_LAMBERTIAN, earth_tex);
	world.add(std::make_shared<sphere>(Vector3d(400, 200, 400), 100.0), earth_mat);
	
	auto pertext = std::make_shared<noise_texture>(0.2);
	auto perlin_mat = std::make_shared<Material>(DIFFUSE_LAMBERTIAN, pertext);
	world.add(std::make_shared<sphere>(Vector3d(220, 280, 300), 80.0), perlin_mat);
	
	/*hittableObjectsList cluster;
	auto white = std::make_shared<Material>(DIFFUSE_LAMBERTIAN, Vector3d(.73, .73, .73));
	int ns = 1000;
	for (int j = 0; j < ns; ++j) {
		Vector3d p = Vector3d(165.0 * random_double(), 165.0 * random_double(), 165.0 * random_double());
		p = rotate_y_point(p, 15.0) + Vector3d(-100, 270, 395);
		cluster.add(std::make_shared<sphere>(p, 10.0), white);
	}
	world.add(std::make_shared<bvh_node>(cluster), nullptr);*/
	
	camera cam;
	int H = image_width;
	cam.initialize(image_width, H, Vector3d(478, 278, -600), Vector3d(278, 278, 0), Vector3d(0, 1, 0));
	cam.SAMPLES_PER_PIXEL = samples_per_pixel;
	cam.MAX_DEPTH = max_depth;
	cam.background = Vector3d(0, 0, 0);
	cam.defocus_angle = 0.0;
	
	auto world_bvh = std::make_shared<bvh_node>(world);
	hittableObjectsList final_world; final_world.add(world_bvh, nullptr);
	//hittableObjectsList final_world = world;

	cam.render(final_world);
	
}

int main() {
	switch (4) {
	case 1: bouncing_spheres(); break;
	case 2: checkered_spheres(); break;
	case 3: earth(); break;
	case 4: final_scene(500, 250, 8); break;
	}
}