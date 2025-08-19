#ifndef PERLIN_HPP
#define PERLIN_HPP

#include <cmath>
#include <memory>
#include <algorithm>
#include "global.hpp"

using point3 = Vector3d;
using vec3 = Vector3d;

inline int random_int(int min_v, int max_v) {
    return static_cast<int>(std::floor(random_double(min_v, max_v + 1)));
}

inline vec3 random_vec_in_range(double min_v, double max_v) {
    return vec3(random_double(min_v, max_v),
        random_double(min_v, max_v),
        random_double(min_v, max_v));
}

inline vec3 unit_vector(const vec3& v) {
    return v.normalized();
}

class perlin {
public:
    perlin() {
        for (int i = 0; i < point_count; ++i) {
            randvec[i] = unit_vector(random_vec_in_range(-1.0, 1.0));
        }
        perlin_generate_perm(perm_x);
        perlin_generate_perm(perm_y);
        perlin_generate_perm(perm_z);
    }

    double noise(const point3& p) const {
        auto u = p.x() - std::floor(p.x());
        auto v = p.y() - std::floor(p.y());
        auto w = p.z() - std::floor(p.z());

        u = u * u * (3 - 2 * u);
        v = v * v * (3 - 2 * v);
        w = w * w * (3 - 2 * w);

        int i = static_cast<int>(std::floor(p.x()));
        int j = static_cast<int>(std::floor(p.y()));
        int k = static_cast<int>(std::floor(p.z()));

        vec3 c[2][2][2];
        for (int di = 0; di < 2; ++di)
            for (int dj = 0; dj < 2; ++dj)
                for (int dk = 0; dk < 2; ++dk)
                    c[di][dj][dk] = randvec[
                        perm_x[(i + di) & 255] ^
                            perm_y[(j + dj) & 255] ^
                            perm_z[(k + dk) & 255]
                    ];

        return perlin_interp(c, u, v, w);
    }

    double turb(point3 p, int depth = 7) const {
        double accum = 0.0;
        double weight = 1.0;
        for (int i = 0; i < depth; ++i) {
            accum += weight * noise(p);
            weight *= 0.5;
            p *= 2.0;
        }
        return std::fabs(accum);
    }

private:
    static constexpr int point_count = 256;
    vec3 randvec[point_count];
    int  perm_x[point_count];
    int  perm_y[point_count];
    int  perm_z[point_count];

    static void perlin_generate_perm(int* p) {
        for (int i = 0; i < point_count; ++i) p[i] = i;
        permute(p, point_count);
    }

    static void permute(int* p, int n) {
        for (int i = n - 1; i > 0; --i) {
            int target = random_int(0, i);
            std::swap(p[i], p[target]);
        }
    }

    static double perlin_interp(const vec3 c[2][2][2], double u, double v, double w) {
        auto uu = u * u * (3 - 2 * u);
        auto vv = v * v * (3 - 2 * v);
        auto ww = w * w * (3 - 2 * w);
        double accum = 0.0;

        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 2; ++j)
                for (int k = 0; k < 2; ++k) {
                    vec3 weight_v(u - i, v - j, w - k);
                    double blend =
                        (i * uu + (1 - i) * (1 - uu)) *
                        (j * vv + (1 - j) * (1 - vv)) *
                        (k * ww + (1 - k) * (1 - ww));
                    accum += blend * c[i][j][k].dot(weight_v);
                }
        return accum;
    }
};

#endif // PERLIN_HPP
