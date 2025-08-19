#ifndef BVH_HPP
#define BVH_HPP

#include <vector>
#include <algorithm>
#include <memory>
#include "objects.hpp"
#include "aabb.hpp"

class bvh_node : public hittableObject {
public:
    bvh_node(const hittableObjectsList& list) : bvh_node(list.objects_list, 0, list.objects_list.size()) {}

    bvh_node(const std::vector<std::shared_ptr<hittableObject>>& src_objects, size_t start, size_t end) {
        auto objects = src_objects;

        bbox = aabb::empty;
        for (size_t object_index = start; object_index < end; object_index++)
            bbox = aabb(bbox, objects[object_index]->get_aabb());

        int axis = bbox.longest_axis();
        auto comparator = (axis == 0) ? box_x_compare
            : (axis == 1) ? box_y_compare
            : box_z_compare;

        size_t object_span = end - start;

        if (object_span == 1) {
            left = right = objects[start];
        }
        else if (object_span == 2) {
            if (comparator(objects[start], objects[start + 1])) {
                left = objects[start];
                right = objects[start + 1];
            }
            else {
                left = objects[start + 1];
                right = objects[start];
            }
        }
        else {
            std::sort(objects.begin() + start, objects.begin() + end, comparator);
            auto mid = start + object_span / 2;
            left = std::make_shared<bvh_node>(objects, start, mid);
            right = std::make_shared<bvh_node>(objects, mid, end);
        }
    }

    bool intersect_ray(const ray& r, double& r_inter_max_t, double& r_inter_min_t, intersection& inter) const override {
        double tmin = EPSILON, tmax = 1e30;
        interval t_interv;
        t_interv.min = tmin; t_interv.max = tmax;
        if (!bbox.intersect_ray(r, t_interv)) return false;

        intersection L, R;
        bool hit_left = left->intersect_ray(r, tmax, tmin, L);
        double closest = hit_left ? L.time_spread_to : tmax;
        bool hit_right = right->intersect_ray(r, closest, tmin, R);

        if (hit_left && hit_right) { inter = (L.time_spread_to < R.time_spread_to) ? L : R; return true; }
        if (hit_left) { inter = L; return true; }
        if (hit_right) { inter = R; return true; }
        return false;
    }

    aabb get_aabb() const override { return bbox; }

private:
    std::shared_ptr<hittableObject> left;
    std::shared_ptr<hittableObject> right;
    aabb bbox;

    static bool box_compare(const std::shared_ptr<hittableObject> a, const std::shared_ptr<hittableObject> b, int axis_index) {
        return a->get_aabb().axis_interval(axis_index).min < b->get_aabb().axis_interval(axis_index).min;
    }

    static bool box_x_compare(const std::shared_ptr<hittableObject> a, const std::shared_ptr<hittableObject> b) {
        return box_compare(a, b, 0);
    }
    static bool box_y_compare(const std::shared_ptr<hittableObject> a, const std::shared_ptr<hittableObject> b) {
        return box_compare(a, b, 1);
    }
    static bool box_z_compare(const std::shared_ptr<hittableObject> a, const std::shared_ptr<hittableObject> b) {
        return box_compare(a, b, 2);
    }
};

#endif // BVH_HPP