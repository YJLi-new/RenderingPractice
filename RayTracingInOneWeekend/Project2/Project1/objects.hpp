#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <iostream>
#include <Eigen>
#include <vector>

#include "global.hpp"
#include "ray.hpp"
#include "intersection.hpp"
#include "material.hpp"
#include "aabb.hpp"

using namespace Eigen;

class hittableObject {
public:
	mutable aabb bbox;
	std::shared_ptr<Material> material;

	virtual ~hittableObject() = default;

	virtual bool intersect_ray(const ray& r, double& r_inter_max_t, double& r_inter_min_t, intersection& inter) const = 0;
	virtual aabb get_aabb() const = 0;
};


class hittableObjectsList: public hittableObject {
public:
	std::vector<std::shared_ptr<hittableObject>> objects_list;

	hittableObjectsList() {}
	hittableObjectsList(std::shared_ptr<hittableObject> object, std::shared_ptr<Material> m) { add(object, m); }

	void clear() { objects_list.clear(); }
	void add(std::shared_ptr<hittableObject> object, std::shared_ptr<Material> m) {
		object->material = m;
		objects_list.push_back(object); 
	}

	bool intersect_ray(const ray& r, double& r_inter_max_t, double& r_inter_min_t, intersection& inter) const override;
	aabb get_aabb() const override;
};


class sphere :public hittableObject {
private:
	ray center;  // 球心（的运动轨迹）
	double radius;  // 球的半径

public:
	sphere() {
		center = ray();
		radius = 0.0;
	};
	// 静态球体
	sphere(const Vector3d center_point, const double radius_length) {
		center = ray(center_point, Vector3d::Zero());
		radius = radius_length;
	}
	// 移动球体
	sphere(const Vector3d center_point_start, const Vector3d center_point_end, const double radius_length) {
		center = ray(center_point_start, center_point_end - center_point_start); // 构造一条以起始点为原点，位移为方向的 ray
		radius = radius_length;
	}
	
	Vector3d get_current_center(double time) const { return center.positionAtTheTime(time); } // 获取特定时间的球心位置
	double get_radius() { return radius; }
	static void get_sphere_uv(const Vector3d& p, double& u, double& v) {
		double theta = std::acos(-p.y());
		double phi = std::atan2(-p.z(), p.x()) + M_PI;
		u = phi / (2 * M_PI);
		v = theta / M_PI;
	}
	bool intersect_ray(const ray& r, double& r_inter_max_t, double& r_inter_min_t, intersection& inter) const override;
	aabb get_aabb() const override;
};

class quad : public hittableObject {
public:
	quad(const Vector3d & Q_, const Vector3d & u_, const Vector3d & v_, std::shared_ptr<Material> m): Q(Q_), u(u_), v(v_), mat(std::move(m)) {
		Vector3d n = u.cross(v);
		normal = n.normalized();
		D = normal.dot(Q);
		w = n / n.dot(n);
		bbox = quad_bbox(Q, u, v);
		double uu = u.dot(u);
		double vv = v.dot(v);
		set_bounding_box();
		}
	
	bool intersect_ray(const ray& r, double&, double&, intersection& rec) const override {
		const double denom = normal.dot(r.get_direction());
		if (std::fabs(denom) < 1e-12) return false;

		const double t = (D - normal.dot(r.get_origin())) / denom;
		if (t <= EPSILON) return false;

		const Vector3d p = r.positionAtTheTime(t);
		const Vector3d rel = p - Q;

		const double a = rel.dot(u) / uu;
		const double b = rel.dot(v) / vv;
		if (a < 0.0 || a > 1.0 || b < 0.0 || b > 1.0) return false;

		rec.happened = true;
		rec.time_spread_to = t;
		rec.position = p;

		// 正反面与法线
		rec.is_front_face = (denom < 0.0);
		rec.normal = rec.is_front_face ? normal : -normal;

		rec.u = a; rec.v = b;
		return true;
	}
	
	aabb get_aabb() const override { return bbox; }

	static inline aabb quad_bbox(const Vector3d& Q,
		const Vector3d& u,
		const Vector3d& v) {
		Vector3d p0 = Q;
		Vector3d p1 = Q + u;
		Vector3d p2 = Q + v;
		Vector3d p3 = Q + u + v;

		Vector3d mn = p0.cwiseMin(p1).cwiseMin(p2).cwiseMin(p3);
		Vector3d mx = p0.cwiseMax(p1).cwiseMax(p2).cwiseMax(p3);

		const double pad = 1e-4;
		for (int i = 0; i < 3; ++i) {
			if (mx[i] - mn[i] < pad) {
				mn[i] -= 0.5 * pad;
				mx[i] += 0.5 * pad;
			}
		}
		return aabb(mn, mx);
	}
	
private:
	Vector3d Q, u, v;
	Vector3d w;
	std::shared_ptr<Material> mat;
	aabb bbox;
	Vector3d normal;
	double D{ 0.0 };
	double uu, vv;
	
	void set_bounding_box() {
		Vector3d q0 = Q;
		Vector3d q1 = Q + u;
		Vector3d q2 = Q + v;
		Vector3d q3 = Q + u + v;
		
		Vector3d mn = q0.cwiseMin(q1).cwiseMin(q2).cwiseMin(q3);
		Vector3d mx = q0.cwiseMax(q1).cwiseMax(q2).cwiseMax(q3);
		bbox = aabb(mn, mx);
		
	}
	
};

#endif // !OBJECTS_HPP