#ifndef __BVH_H__
#define __BVH_H__

#include "rtweekend.h"
#include "hittable.h"
#include "hittable_list.h"

class bvh_node : public hittable {
    public:
        bvh_node();
        bvh_node(const hittable_list &list, double time0, double time1) : bvh_node(list.objects, 0, list.objects.size(), time0, time1) {}
        bvh_node(std::vector<shared_ptr<hittable>> &src_objects, size_t start, size_t end, double time0, double time1);

        virtual bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const override;
        virtual bool bounding_box(double time0, double time1, aabb &output_box) const override;

    public:
        shared_ptr<hittable> left;
        shared_ptr<hittable> right;
        aabb box;
};

bool bvh_node::bounding_box(double time0, double time1, aabb &output_box) const {
    output_box = box;
    return true;
}

bool bvh_node::hit(const ray &r, double t_min, double t_max, hit_record &rec) const {
    if (!box.hit(r, t_min, t_max)) return false;

    bool hit_left = left->hit(r, t_min, t_max, rec);
    bool hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);

    return hit_left || hit_right;
}

bvh_node::bvh_node(std::vector<shared_ptr<hittable>> &src_objects, size_t start, size_t end, double time0, double time1) {
    auto objects = src_objects;

    int axis = random_int(0, 2);
    auto comparator = (axis == 0) ? box_x_compare : (axis == 1) ? box_y_compare : box_z_compare;
}

inline bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis) {
    aabb box_a;
    aabb box_b;

    if (!a->bounding_box(0, 0, box_a) || !b->bounding_box(0, 0, box_b))
        std::cerr << "No bounding box in bvh_node constructor.\n";

    return box_a.min().e[axis] < box_b.min().e[axis];
}

bool box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return box_compare(a, b, 0);
}

bool box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return box_compare(a, b, 1);
}

bool box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return box_compare(a, b, 2);
}


#endif //__BVH_H__