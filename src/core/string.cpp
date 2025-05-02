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

    bool operator==(const string other) {
        return strcmp(data, other.data) == 0;
    }
    
    bool operator==(const string& other) const {
    return strcmp(data, other.data) == 0;
}

    bool operator!=(const string other) {
        return strcmp(data, other.data) != 0;
    }

    bool operator<(const string other) {
        return strcmp(data, other.data) < 0;
    }

    bool operator>(const string other) {
        return strcmp(data, other.data) > 0;
    }

    bool operator<=(const string other) {
        return strcmp(data, other.data) <= 0;
    }

    bool operator>=(const string other) {
        return strcmp(data, other.data) >= 0;
    }
};

string to_string(char* c_str) {
    string result;
    result.data = c_str;
    result.length = strlen(c_str);
    return result;
}

string to_string(const char* c_str) {
    string result;
    result.length = strlen(c_str);
    result.data = static_cast<char*>(malloc(result.length + 1));
    memcpy(result.data, c_str, result.length);
    result.data[result.length] = '\0';
    return result;
}