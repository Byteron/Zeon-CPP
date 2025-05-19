// NOTE: we go with right handed

#include <math.h>


const float PI = 3.1415927f;


struct Vec3 {
    float x, y, z;

    inline float &operator[](int index) { return (&x)[index]; }
    inline const float &operator[](int index) const { return (&x)[index]; }
    
    inline Vec3 operator*(float value) { return Vec3 { x * value, y * value, z * value }; }
};

struct Vec4 {
    float x, y, z, w;

    inline float &operator[](int index) { return (&x)[index]; }
    inline const float &operator[](int index) const { return (&x)[index]; }
};

struct Quat {
    float x, y, z, w;

    inline float &operator[](int index) { return (&x)[index]; }
    inline const float &operator[](int index) const { return (&x)[index]; }
};

union Mat4
{
    float m[4][4]{};
    Vec4 columns[4];

    inline Vec4 &operator[](int index) { return columns[index]; }
    inline const Vec4 &operator[](int index) const { return columns[index]; }
};

struct Transform {
    Vec3 position{};
    Quat rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
    Vec3 scale{ 1.0f, 1.0f, 1.0f };
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

Mat4 mat4_identity()
{
    Mat4 result = {0};
    result.m[0][0] = 1;
    result.m[1][1] = 1;
    result.m[2][2] = 1;
    result.m[3][3] = 1;

    return result;
}


Mat4 perspective_projection(float fov, float aspect_ratio, float near, float far)
{
    Mat4 result{};

    float cotangent = 1.0f / tanf(fov * 0.5f);
    result.m[0][0] = cotangent / aspect_ratio;
    result.m[1][1] = cotangent;
    result.m[2][3] = -1.0f;

    result.m[2][2] = (near + far) / (near - far);
    result.m[3][2] = (2.0f * near * far) / (near - far);

    return result;
}

Mat4 translate(Vec3 translation)
{
    Mat4 result = mat4_identity();

    result.m[3][0] = translation.x;
    result.m[3][1] = translation.y;
    result.m[3][2] = translation.z;

    return result;
}

float inv_sqrtf(float value) {
    return 1.0f / sqrtf(value);
}

float dot(Vec3 left, Vec3 right) {
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

Vec3 cross(Vec3 left, Vec3 right) {
    Vec3 result;
    result.x = left.y * right.z - left.z * right.y;
    result.y = left.z * right.x - left.x * right.z;
    result.z = left.x * right.y - left.y * right.x;
    return result;
}

Vec3 normalize(Vec3 vec) {
    return vec * inv_sqrtf(dot(vec, vec));
}

Quat normalize(Quat q)
{
    const float len_sq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;

    if (len_sq > 0.0f)
    {
        const float inv_len = inv_sqrtf(len_sq);

        q.x *= inv_len;
        q.y *= inv_len;
        q.z *= inv_len;
        q.w *= inv_len;
    }
    else
    {
        q.x = q.y = q.z = 0.0f;
        q.w = 1.0f;
    }
    
    return q;
}

Vec4 linear_combine_vec4_mat4(Vec4 left, Mat4 right)
{
    Vec4 result;

    result.x = left.x * right.columns[0].x;
    result.y = left.x * right.columns[0].y;
    result.z = left.x * right.columns[0].z;
    result.w = left.x * right.columns[0].w;

    result.x += left.y * right.columns[1].x;
    result.y += left.y * right.columns[1].y;
    result.z += left.y * right.columns[1].z;
    result.w += left.y * right.columns[1].w;

    result.x += left.z * right.columns[2].x;
    result.y += left.z * right.columns[2].y;
    result.z += left.z * right.columns[2].z;
    result.w += left.z * right.columns[2].w;

    result.x += left.w * right.columns[3].x;
    result.y += left.w * right.columns[3].y;
    result.z += left.w * right.columns[3].z;
    result.w += left.w * right.columns[3].w;

    return result;
}

inline Mat4 operator*(Mat4& left, Mat4 right) { 
    Mat4 result;
    
    result.columns[0] = linear_combine_vec4_mat4(right.columns[0], left);
    result.columns[1] = linear_combine_vec4_mat4(right.columns[1], left);
    result.columns[2] = linear_combine_vec4_mat4(right.columns[2], left);
    result.columns[3] = linear_combine_vec4_mat4(right.columns[3], left);
    
    return result;
}

Mat4 translation_mat(Vec3 translation) {
    Mat4 result = mat4_identity();

    result.m[3][0] = translation.x;
    result.m[3][1] = translation.y;
    result.m[3][2] = translation.z;

    return result;
}

void translate(Mat4& mat, Vec3 translation) {
    mat = mat * translation_mat(translation);
}

Mat4 rotation_mat(Quat q)
{
    q = normalize(q);

    const float xx = q.x * q.x;
    const float yy = q.y * q.y;
    const float zz = q.z * q.z;
    const float xy = q.x * q.y;
    const float xz = q.x * q.z;
    const float yz = q.y * q.z;
    const float wx = q.w * q.x;
    const float wy = q.w * q.y;
    const float wz = q.w * q.z;

    Mat4 m = mat4_identity();        // column-major: m[col][row]

    // column 0
    m.m[0][0] = 1.0f - 2.0f * (yy + zz);
    m.m[0][1] = 2.0f * (xy + wz);
    m.m[0][2] = 2.0f * (xz - wy);

    // column 1
    m.m[1][0] = 2.0f * (xy - wz);
    m.m[1][1] = 1.0f - 2.0f * (xx + zz);
    m.m[1][2] = 2.0f * (yz + wx);

    // column 2
    m.m[2][0] = 2.0f * (xz + wy);
    m.m[2][1] = 2.0f * (yz - wx);
    m.m[2][2] = 1.0f - 2.0f * (xx + yy);

    // column 3 (translation) and bottom row already set to {0,0,0,1}
    return m;
}

void rotate(Mat4& mat, Quat quat) {
    mat = mat * rotation_mat(quat);
}

Mat4 scale_mat(Vec3 scale) {
    Mat4 result = mat4_identity();

    result.m[0][0] = scale.x;
    result.m[1][1] = scale.y;
    result.m[2][2] = scale.z;

    return result;
}

void scale(Mat4& mat, Vec3 scale) {
    mat = mat * scale_mat(scale);
}

float to_radians(float angle) {
    return angle * PI / 180.0;
}

Quat quat_from_axis_angle(Vec3 axis, float angle) {
    float half_angle = angle * 0.5f;
    float s = sinf(half_angle);
    return {
        axis.x * s,
        axis.y * s,
        axis.z * s,
        cosf(half_angle)
    };
}

Quat operator*(Quat a, Quat b) {
    return {
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    };
}

Mat4 to_mat4(Transform transform) {
    Mat4 mat = mat4_identity();
    translate(mat, transform.position);
    rotate(mat, transform.rotation);
    scale(mat, transform.scale);
    return mat;
}