struct Entity {
    int id{};
    int gen{};

    bool operator==(const Entity& rhs) const noexcept {
        return id == rhs.id && gen == rhs.gen;
    }

    bool operator!=(const Entity& rhs) const noexcept {
        return !(*this == rhs);
    }
};

constexpr Entity ENTITY_NONE{};