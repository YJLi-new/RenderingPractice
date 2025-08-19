#pragma once  // 保证头文件只被编译一次，防止头文件被重复引用。
#include <iostream>
#include <cmath>
#include <random>
#include <Eigen>
#include <vector>
#include <memory>

using namespace Eigen;

const float M_PI = 3.141592653589793f;

const double EPSILON = 0.0016;

const float INFINITYf = INFINITY;
const double INFINITYd = INFINITY;
const int INFINITYi = INT_MAX;

const float MAXf = std::numeric_limits<float>::max();
const double MAXd = std::numeric_limits<double>::max();
const int MAXi = std::numeric_limits<int>::max();

inline double degrees_to_radians(double degrees) {
    return degrees * M_PI / 180.0;
}

/*
inline double random_double() {
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0);
}*/


inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

/*
inline double random_double() {
    std::random_device rd{};
    std::mt19937 gen{ rd() };

    // values near the mean are the most likely
    // standard deviation affects the dispersion of generated values from the mean
    std::normal_distribution d{ 0.3, 0.3 };

    // draw a sample from the normal distribution and round it to an integer
    auto random_int = [&d, &gen] { return std::round(d(gen)); };

    return random_int();
}*/

inline static double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

static Vector3d random_vec3d() {
    return Vector3d(random_double(), random_double(), random_double());
}

static Vector3d random_vec3d(double min, double max) {
    return Vector3d(random_double(min, max), random_double(min, max), random_double(min, max));
}

inline static Vector3d random_in_unit_sphere() {
    Vector3d p = Vector3d::Zero();
    while (p.dot(p) < 1) {
        p = random_vec3d(-1, 1);
        if (p.dot(p) < 1)
            return p;
    }
}

inline static Vector3d random_unit_vector() {
    Vector3d res = random_in_unit_sphere();
    res.normalize();
    return res;
}

inline static Vector3d random_on_hemisphere(const Vector3d& normal) {
    Vector3d on_unit_sphere = random_unit_vector();
    double judge = on_unit_sphere.dot(normal);
    if (judge > 0.0) // In the same hemisphere as the normal
        return on_unit_sphere;
    else
        return -on_unit_sphere;
}

inline bool is_near_zero_vec3d(Vector3d vec) {
    return (std::fabs(vec.x()) < EPSILON) && (std::fabs(vec.y()) < EPSILON) && (std::fabs(vec.z()) < EPSILON);
}

inline Vector3d reflect(const Vector3d& the_vec, const Vector3d& normal_at_inter) {
    return the_vec - 2 * the_vec.dot(normal_at_inter) * normal_at_inter;
}

inline Vector3d refract(const Vector3d& uv, const Vector3d& n, double etai_over_etat) {
    double cos_theta = std::min(-uv.dot(n), 1.0);
    Vector3d r_out_perp = etai_over_etat * (uv + cos_theta * n);
    Vector3d r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.dot(r_out_perp))) * n;
    return r_out_perp + r_out_parallel;
}

inline double reflectance(double cos, double ri) {
    double r0 = (1 - ri) / (1 + ri);
    r0 *= r0;
    return r0 + (1 - r0) * std::pow((1 - cos), 5);
}

inline static Vector3d random_in_unit_disk() {
    Vector3d p(random_double(-1, 1), random_double(-1, 1), 0);
    while (p.squaredNorm() >= 1) {
        p = Vector3d(random_double(-1, 1), random_double(-1, 1), 0);
    }
    return p;
}