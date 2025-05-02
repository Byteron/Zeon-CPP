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

Vec4 to_vec4(float floats[4]) {
    Vec4 vec;
    memcpy(&vec, &floats[0], sizeof(float) * 4);
    return vec;
}

float min(float a, float b) {
    return a < b ? a : b;
}

float max(float a, float b) {
    return a > b ? a : b;
}

Vec3 min(Vec3 a, Vec3 b) {
    Vec3 vec;
    vec.x = min(a.x, b.x);
    vec.y = min(a.y, b.y);
    vec.z = min(a.z, b.z);
    return vec;
}

Vec3 max(Vec3 a, Vec3 b) {
    Vec3 vec;
    vec.x = max(a.x, b.x);
    vec.y = max(a.y, b.y);
    vec.z = max(a.z, b.z);
    return vec;
}
