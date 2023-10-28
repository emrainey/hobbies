#ifndef RAYTRACE_OBJPARSER_H_
#define RAYTRACE_OBJPARSER_H_

#include <cstdint>
#include <cstdio>
#include <cinttypes>

constexpr bool is_newline(char const c) {
    return (c == '\n' or c == '\r');
}

namespace raytrace {
namespace obj {

class Observer {
public:
    /// @brief Adds a vertex to the Observer
    virtual void addVertex(float x, float y, float z) = 0;
    /// @brief Adds a Normal  to the Observer
    virtual void addNormal(float x, float y, float z) = 0;
    /// @brief Adds a Triangular Face to the Observer given the indices of the vertexes.
    /// @note Texture coords don't work here.
    virtual void addFace(uint32_t v1, uint32_t v2, uint32_t v3) = 0;
protected:
    ~Observer() = default;
};

class Parser {
public:
    /// @brief The state of the parser.
    enum State {
        Unknown, ///< Unknown state
        Comment, ///< Comment line
        Object, ///< The object name is being parsed
        Vertices, ///< A vertex is being parsed
        Textures, ///< A texture vertex is being parsed
        Normals,  ///< A normal is being parsed
        Faces ///< A face is being parsed.
    };
    /// @brief  The state within each state.
    enum SubState {
        None, ///< An invalid state
        String, ///< Any characters up to the line terminators
        FloatingPoint, ///< Digits, -, +, .
        Integer, ///< Unsigned Digits, no point.
    };

    /// Constructs a parser with a given observer
    Parser(Observer& observer) : m_buffer_{}, m_index_{0u}, m_line_{1u}, m_observer_{observer} {}

    /// Returns the current state
    inline State GetState(void) const {
        return m_state_;
    }

    /// Returns the current substate
    inline SubState GetSubState(void) const {
        return m_substate_;
    }

    /// Parses a string character by character
    inline void Parse(std::string str) {
        for (char const * tmp = str.c_str(); *tmp != '\0'; tmp++) {
            Parse(*tmp);
        }
    }

    /// Parses a string literal
    inline void Parse(char const * const str) {
        for (char const * tmp = str; *tmp != '\0'; tmp++) {
            Parse(*tmp);
        }
    }

    void Subparse(char const c) {
        bool skip = true;
        switch (m_substate_) {
            case SubState::None:
                // skip it, and truncate whatever we had
                m_index_ = 0u;
                break;
            case SubState::String:
                if (std::isprint(c)) {
                    skip = false;
                }
                break;
            case SubState::FloatingPoint:
                if (std::isdigit(c) or c == '.' or c == '-' or c == ' ') {
                    skip = false;
                }
                break;
            case SubState::Integer:
                if (std::isdigit(c) or c == ' ') {
                    skip = false;
                }
                break;
        }
        if (not skip) {
            if (m_index_ < sizeof(m_buffer_)) {
                m_buffer_[m_index_++] = c;
            } else {
                m_buffer_[sizeof(m_buffer_) - 1u] = '\0';
            }
        }
    }

    /// Parses a single character at a time.
    void Parse(char const c) {
        bool common_exit = true;
        switch (m_state_) {
            case State::Unknown:
                if (c == '#') {
                    m_state_ = State::Comment;
                    m_substate_ = SubState::String;
                } else if (c == 'o') {
                    m_state_ = State::Object;
                    m_substate_ = SubState::String;
                } else if (c == 'v') {
                    m_state_ = State::Vertices;
                    m_substate_ = SubState::FloatingPoint;
                } else if (c == 'f') {
                    m_state_ = State::Faces;
                    m_substate_ = SubState::Integer;
                }
                break;
            case State::Comment:
                Subparse(c);
                break;
            case State::Object:
                Subparse(c);
                break;
            case State::Vertices:
                if (c == 'n') {
                    m_state_ = State::Normals;
                    m_substate_ = SubState::FloatingPoint;
                } else if (c == 't') {
                    m_state_ = State::Textures;
                    m_substate_ = SubState::FloatingPoint;
                } else if (not is_newline(c)) {
                    Subparse(c);
                }
                break;
            case State::Normals:
                if (not is_newline(c)) {
                    Subparse(c);
                }
                break;
            case State::Faces:
                if (not is_newline(c)) {
                    Subparse(c);
                }
                break;
            default:
                common_exit = false;
                break;
        }
        if (common_exit and is_newline(c)) {
            if (m_state_ != State::Unknown) {
                Complete();
                m_state_ = State::Unknown;
                m_substate_ = SubState::None;
                m_index_ = 0u;
            }
        }
        if (c == '\n') {
            m_line_++;
        }
    }

    size_t GetNumberOfLines(void) const {
        return m_line_;
    }

protected:
    void Complete(void) {
        // printf("state=%d sub=%d Buffer=%s\n", (int)m_state_, (int)m_substate_, m_buffer_);
        switch (m_state_) {
            case State::Vertices:
                if (GetSubState() == SubState::FloatingPoint) {
                    float x = std::nan("");
                    float y = std::nan("");
                    float z = std::nan("");
                    if (3u == sscanf(m_buffer_, " %f %f %f", &x, &y, &z)) {
                        m_observer_.addVertex(x, y, z);
                    }
                }
                break;
            case State::Normals:
                if (GetSubState() == SubState::FloatingPoint) {
                    float x = std::nan("");
                    float y = std::nan("");
                    float z = std::nan("");
                    if (3u == sscanf(m_buffer_, " %f %f %f", &x, &y, &z)) {
                        m_observer_.addNormal(x, y, z);
                    }
                }
                break;
            case State::Textures:
                if (GetSubState() == SubState::FloatingPoint) {
                    float u = std::nan("");
                    float v = std::nan("");
                    if (2u == sscanf(m_buffer_, " %f %f", &u, &v)) {
                        // m_observer_.addTextureCoord(u, v);
                    }
                }
                break;
            case State::Faces:
                if (GetSubState() == SubState::Integer) {
                    uint32_t a = UINT32_MAX;
                    uint32_t b = UINT32_MAX;
                    uint32_t c = UINT32_MAX;
                    if (3u == sscanf(m_buffer_, " %" PRIu32 " %" PRIu32 " %" PRIu32 "", &a, &b, &c)) {
                        m_observer_.addFace(a, b, c);
                        break;
                    }
                    uint32_t d = UINT32_MAX;
                    uint32_t e = UINT32_MAX;
                    uint32_t f = UINT32_MAX;
                    if (6u == sscanf(m_buffer_, " %" PRIu32 "/%" PRIu32 " %" PRIu32 "/%" PRIu32 " %" PRIu32 "/%" PRIu32 "",
                                     &a, &d, &b, &e, &c, &f)) {
                        m_observer_.addFace(a, b, c);
                        // add some other face from d,e,f?
                    }
                }
            default:
                break;
        }
    }

    State m_state_{State::Unknown};
    SubState m_substate_{SubState::String};
    static constexpr size_t internal_buffer_size{1024};
    char m_buffer_[internal_buffer_size];
    size_t m_index_;
    size_t m_line_;
    Observer& m_observer_;
};

} // namespace obj
} // namespace raytrace

#endif // RAYTRACE_OBJPARSER_H_
