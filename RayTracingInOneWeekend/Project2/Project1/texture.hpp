#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <memory>
#include <cmath>
#include <algorithm>
#include "global.hpp"
#include "rtw_stb_image.h"
#include "perlin.hpp"

class texture {
public:
    virtual ~texture() = default;
    virtual Vector3d value(double u, double v, const Vector3d& p) const = 0;
};

class solid_color : public texture {
public:
    solid_color(const Vector3d& albedo) : albedo_(albedo) {}
    solid_color(double r, double g, double b) : albedo_(Vector3d(r, g, b)) {}

    Vector3d value(double, double, const Vector3d&) const override { return albedo_; }

private:
    Vector3d albedo_;
};

class checker_texture : public texture {
public:
    checker_texture(double scale, std::shared_ptr<texture> even, std::shared_ptr<texture> odd)
        : inv_scale_(1.0 / scale), even_(even), odd_(odd) {}

    checker_texture(double scale, const Vector3d& c1, const Vector3d& c2)
        : checker_texture(scale,
            std::make_shared<solid_color>(c1),
            std::make_shared<solid_color>(c2)) {}

    Vector3d value(double u, double v, const Vector3d& p) const override {
        int xi = static_cast<int>(std::floor(inv_scale_ * p.x()));
        int yi = static_cast<int>(std::floor(inv_scale_ * p.y()));
        int zi = static_cast<int>(std::floor(inv_scale_ * p.z()));
        bool isEven = ((xi + yi + zi) % 2) == 0;
        return isEven ? even_->value(u, v, p) : odd_->value(u, v, p);
    }

private:
    double inv_scale_;
    std::shared_ptr<texture> even_;
    std::shared_ptr<texture> odd_;
};

class image_texture : public texture {
public:
    explicit image_texture(const char* filename) : image(filename) {}

    Vector3d value(double u, double v, const Vector3d&) const override {

        if (image.height() <= 0) return Vector3d(0, 1, 1);

        u = std::clamp(u, 0.0, 1.0);
        v = 1.0 - std::clamp(v, 0.0, 1.0);

        int i = static_cast<int>(u * image.width());
        int j = static_cast<int>(v * image.height());
        const unsigned char* px = image.pixel_data(i, j);

        const double s = 1.0 / 255.0;
        return Vector3d(s * px[0], s * px[1], s * px[2]);
    }

private:
    rtw_image image;
};

class noise_texture : public texture {

public:
    enum class Mode { Basic, Turbulence, Marble };

    perlin noise_;
    double scale_ = 1.0;
    Mode mode_ = Mode::Basic;

    explicit noise_texture(double scale = 1.0, Mode m = Mode::Basic) :scale_(scale), mode_(m) {}

    Vector3d value(double u, double v, const point3& p) const override {
        (void)u; (void)v;
        switch (mode_) {
        case Mode::Basic: {
            double n = noise_.noise(scale_ * p);
            return Vector3d(1, 1, 1) * 0.5 * (1.0 + n);
        }
        case Mode::Turbulence: {
            double t = noise_.turb(scale_ * p, 7);
            return Vector3d(1, 1, 1) * t;
        }
        case Mode::Marble: {
            double t = noise_.turb(p, 7);
            double s = std::sin(scale_ * p.z() + 10.0 * t);
            return Vector3d(0.5, 0.5, 0.5) * (1.0 + s);
        }
        }
        return Vector3d(1, 1, 1);

    }
};
#endif // TEXTURE_HPP
