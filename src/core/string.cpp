struct string {
    char* data;
    size_t length;

    string operator+(const string other) {
        string result;
        result.data = static_cast<char*>(malloc(length + other.length + 1));
        result.length = length + other.length;
        memcpy(result.data, data, length);
        memcpy(result.data + length, other.data, other.length);
        result.data[result.length] = '\0';
        return result;
    }

    string operator+(const char* other) {
        string result;
        size_t other_length = strlen(other);
        result.data = static_cast<char*>(malloc(length + other_length + 1));
        result.length = length + other_length;
        memcpy(result.data, data, length);
        memcpy(result.data + length, other, other_length);
        result.data[result.length] = '\0';
        return result;
    }
};