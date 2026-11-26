#ifndef _VEC_H
#define _VEC_H
#include <stdbool.h>
#include <math.h>
typedef struct {
    int x, y;
} Vec2;

struct Vec2f {
    float x, y;
};
typedef struct Vec2f Vec2f;

typedef struct {
    int x, y, z;
} Vec3;

typedef struct {
    float x, y, z;
} Vec3f;

inline Vec2 vec2(int x, int y) {
    return (Vec2){x, y};
}

inline Vec3 vec3(int x, int y, int z) {
    return (Vec3){x, y, z};
}
inline Vec3f vec3f(float x, float y, float z) {
    return (Vec3f){x, y, z};
}
inline Vec2f vec2f(float x, float y) {
    return (Vec2f){.x = x, .y = y};
}

inline Vec2 vec2_add(Vec2 a, Vec2 b) {
    return vec2(a.x + b.x, a.y + b.y);
}

inline Vec2 vec2_mul(Vec2 v, int x) {
    return vec2(v.x * x, v.y * x);
}

inline bool vec2_eq(Vec2 a, Vec2 b) {
    return a.x == b.x && a.y == b.y;
}

inline Vec3 vec3_add(Vec3 a, Vec3 b) {
    return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline bool vec3_eq(Vec3 a, Vec3 b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

inline Vec2f vec2f_add(Vec2f a, Vec2f b) {
    return vec2f(a.x + b.x, a.y + b.y);
}
inline Vec2f vec2f_sub(Vec2f a, Vec2f b) {
    return vec2f(a.x - b.x, a.y - b.y);
}
inline Vec2f vec2f_mul(Vec2f a, float b) {
    return vec2f(a.x * b, a.y * b);
}
inline Vec2f vec2f_div(Vec2f a, float b) {
    return vec2f(a.x / b, a.y / b);
}

inline bool vec2f_eq(Vec2f a, Vec2f b) {
    return a.x == b.x && a.y == b.y;
}

inline Vec2 vec2f_to_vec2(struct Vec2f v) {
    return vec2((int)v.x, (int)v.y);
}

inline Vec2f vec2f_rotate(Vec2f v, float theta) {
    return vec2f(
        v.x * cos(theta) - v.y * sin(theta),
        v.x * sin(theta) + v.y * cos(theta)
    );
}

inline Vec2f vec2f_to_norm(Vec2f v) {
    float mag = sqrt(v.x * v.x + v.y * v.y);
    return vec2f_div(v, mag);
}


inline Vec3f vec3f_add(Vec3f a, Vec3f b) {
    return vec3f(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline Vec3f vec3f_sub(Vec3f a, Vec3f b) {
    return vec3f(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline Vec3f vec3f_mul(Vec3f a, float b) {
    return vec3f(a.x * b, a.y * b, a.z * b);
}
inline Vec3f vec3f_div(Vec3f a, float b) {
    return vec3f(a.x / b, a.y / b, a.z / b);
}

inline bool vec3f_eq(Vec3f a, Vec3f b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

inline Vec3f vec3f_to_norm(Vec3f v) {
    float mag = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return vec3f_div(v, mag);
}

#endif
