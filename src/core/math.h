#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

struct Vec3 {
    float x, y, z;

    inline float &operator[](int index) { return (&this->x)[index]; }
    inline const float &operator[](int index) const { return (&this->x)[index]; }
};

struct Vec4 {
    float x, y, z, w;

    inline float &operator[](int index) { return (&this->x)[index]; }
    inline const float &operator[](int index) const { return (&this->x)[index]; }
};

struct Quat {
    float x, y, z, w;

    inline float &operator[](int index) { return (&this->x)[index]; }
    inline const float &operator[](int index) const { return (&this->x)[index]; }
};

union Mat4
{
    float m[4][4];
    Vec4 columns[4];

    inline Vec4 &operator[](int index) { return columns[index]; }
    inline const Vec4 &operator[](int index) const { return columns[index]; }
};

struct Transform {
    Vec3 position{};
    Vec3 scale{};
    Quat rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
};

struct AABB {
    Vec3 min{};
    Vec3 max{};
};

Vec4 make_vec4(float floats[4]) {
    Vec4 vec;
    memcpy(&vec, &floats[0], sizeof(float) * 4);
    return vec;
}

Vec3 vec_min(Vec3 a, Vec3 b) {
    Vec3 vec;
    vec.x = MIN(a.x, b.x);
    vec.y = MIN(a.y, b.y);
    vec.z = MIN(a.z, b.z);
    return vec;
}

Vec3 vec_max(Vec3 a, Vec3 b) {
    Vec3 vec;
    vec.x = MAX(a.x, b.x);
    vec.y = MAX(a.y, b.y);
    vec.z = MAX(a.z, b.z);
    return vec;
}
