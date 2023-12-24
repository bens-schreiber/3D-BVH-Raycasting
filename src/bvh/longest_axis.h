#ifndef LONGEST_AXIS_H_
#define LONGEST_AXIS_H_

struct BoundingBox;

typedef int (*qsort_compare)(const void *, const void *);

int compareX(const void *a, const void *b);

int compareY(const void *a, const void *b);

int compareZ(const void *a, const void *b);

qsort_compare compare_by_longest_axis(struct BoundingBox scene_aabb);

#endif // LONGEST_AXIS_H_