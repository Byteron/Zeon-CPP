struct Transform {
    HMM_Vec3 position{};
    HMM_Vec3 scale{};
    HMM_Quat rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
};

struct AABB {
    HMM_Vec3 min{};
    HMM_Vec3 max{};
};

HMM_Vec4 make_vec4(float floats[4]) {
    HMM_Vec4 vec;
    std::memcpy(&vec, &floats[0], sizeof(float) * 4);
    return vec;
}

HMM_Vec3 vec_min(HMM_Vec3 a, HMM_Vec3 b) {
    HMM_Vec3 vec;
    vec.X = HMM_MIN(a.X, b.X);
    vec.Y = HMM_MIN(a.Y, b.Y);
    vec.Z = HMM_MIN(a.Z, b.Z);
    return vec;
}

HMM_Vec3 vec_max(HMM_Vec3 a, HMM_Vec3 b) {
    HMM_Vec3 vec;
    vec.X = HMM_MAX(a.X, b.X);
    vec.Y = HMM_MAX(a.Y, b.Y);
    vec.Z = HMM_MAX(a.Z, b.Z);
    return vec;
}
