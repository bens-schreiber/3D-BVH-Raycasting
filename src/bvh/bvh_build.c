#include <stdlib.h>
#include <math.h>
#include "./util/longest_axis.h"
#include "raylib.h"
#include "../primitive/primitive.h"
#include "bvh.h"

void free_bvh_node(BVH_Node *node)
{
    if (node == NULL)
    {
        return;
    }

    free_bvh_node(node->left);
    free_bvh_node(node->right);
    free(node);
}

void bvh_tree_free(BVH_Tree *tree)
{
    free_bvh_node(tree->root);
    free(tree);
}

BVH_Tree *bvh_tree_create(Primitive *primitives, size_t primitives_size, BoundingBox scene_aabb)
{
    BVH_Tree *tree = malloc(sizeof(BVH_Tree));
    tree->root = bvh_tree_create_impl(primitives, primitives_size, scene_aabb);
    return tree;
}

/// @brief Expands a bounding box to contain another bounding box
/// @return The expanded bounding box containing both a and b
BoundingBox expand_aabb(BoundingBox a, BoundingBox b)
{
    BoundingBox result;

    // For each dimension, the lower bound of the result is the minimum of the lower bounds of a and b.
    result.min.x = fmin(a.min.x, b.min.x);
    result.min.y = fmin(a.min.y, b.min.y);
    result.min.z = fmin(a.min.z, b.min.z);

    // For each dimension, the upper bound of the result is the maximum of the upper bounds of a and b.
    result.max.x = fmax(a.max.x, b.max.x);
    result.max.y = fmax(a.max.y, b.max.y);
    result.max.z = fmax(a.max.z, b.max.z);

    return result;
}

/// @brief Creates a BVH node from the given primitives and bounding box
/// @return A pointer to the new BVH node
BVH_Node *bvh_node_create(const Primitive *primitives, size_t primitives_size, BoundingBox bounding_box)
{
    BVH_Node *new_node = malloc(sizeof(BVH_Node));
    new_node->bounding_box = bounding_box;
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->primitives_size = 0;

    if (primitives_size == 1)
    {
        new_node->primitives[0] = primitives[0];
        new_node->primitives[1] = primitives[0];
        new_node->primitives_size = 1;
        return new_node;
    }

    if (primitives_size == 2)
    {
        new_node->primitives[0] = primitives[0];
        new_node->primitives[1] = primitives[1];
        new_node->primitives_size = 2;
        return new_node;
    }

    return new_node;
}

BVH_Node *bvh_tree_create_impl(Primitive *primitives, size_t primitives_size, BoundingBox bounding_box)
{

    // If there are less than 2 primitives, the BVH node should be a leaf node, and contain the primitives.
    if (primitives_size < 2)
    {
        return NULL;
    }

    BVH_Node *new_node = bvh_node_create(primitives, primitives_size, bounding_box);

    // Sort the primitivess by the longest axis.
    qsort(primitives, primitives_size, sizeof(Primitive), compare_by_longest_axis(bounding_box));

    // Find the median of the primitivess.
    size_t median = primitives_size / 2;
    Primitive median_primitive = primitives[median];

    // Calculate the left AABB.
    // It should contain all primitivess from 0 to median.
    BoundingBox left_aabb = primitive_get_bounding_box(&median_primitive);
    for (size_t i = 0; i <= median; i++)
    {
        left_aabb = expand_aabb(left_aabb, primitive_get_bounding_box(&primitives[i]));
    }

    // Calculate the right AABB.
    // It should contain all primitivess from median to primitives_size.
    BoundingBox right_aabb = primitive_get_bounding_box(&median_primitive);
    for (size_t i = median + 1; i < primitives_size; i++)
    {
        right_aabb = expand_aabb(right_aabb, primitive_get_bounding_box(&primitives[i]));
    }

    // Recurse on the left and right AABBs.
    new_node->left = bvh_tree_create_impl(primitives, median, left_aabb);
    new_node->right = bvh_tree_create_impl(primitives + median, primitives_size - median, right_aabb);

    return new_node;
}
