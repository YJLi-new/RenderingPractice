#include <iostream>

Vector3d ray_background_color(const ray& r, int bounce_num);
Vector3d ray_sphere_color(const ray& r, int bounce_num, const hittableObjectsList& project_scene, const Vector3d& background, int max_depth);