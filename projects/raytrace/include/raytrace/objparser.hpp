#ifndef RAYTRACE_OBJPARSER_H_
#define RAYTRACE_OBJPARSER_H_

#include <cstdint>
#include <cstdio>
#include <vector>
#include <cinttypes>

constexpr bool is_newline(char const c) {
    return (c == '\n' or c == '\r');
}

namespace raytrace {
namespace obj {

static constexpr bool debug = false;

class Observer {
public:
    /// @brief Adds a vertex to the Observer
    virtual void addVertex(float x, float y, float z) = 0;
    /// @brief Adds a Normal  to the Observer
    virtual void addNormal(float x, float y, float z) = 0;
    /// @brief Adds a Texutre Coordinate to the Observer
    virtual void addTexture(float u, float v) = 0;
    /// @brief Adds a Triangular Face to the Observer given the indices of the vertexes.
    /// @note Texture coords don't work here and Normal don't work here.
    virtual void addFace(uint32_t v1, uint32_t v2, uint32_t v3) = 0;
    /// @brief Adds a Triangular Face to the Observer given the indices of the vertexes, and normals
    virtual void addFace(uint32_t v1, uint32_t n1, uint32_t v2, uint32_t n2, uint32_t v3, uint32_t n3) = 0;
    /// @brief Adds a Triangular Face to the Observer given the indices of the vertexes, normals, and textures
    virtual void addFace(uint32_t v1, uint32_t t1, uint32_t n1, uint32_t v2, uint32_t t2, uint32_t n2, uint32_t v3,
                         uint32_t t3, uint32_t n3)
        = 0;

protected:
    ~Observer() = default;
};

class Parser {
public:
    /// @brief The state of the parser.
    enum class State : char {
        Unknown = 'u',   ///< Unknown state
        Comment = 'c',   ///< Comment line
        Object = 'o',    ///< The object name is being parsed
        Vertices = 'v',  ///< A vertex is being parsed
        Textures = 't',  ///< A texture vertex is being parsed
        Normals = 'n',   ///< A normal is being parsed
        Faces = 'f'      ///< A face is being parsed.
    };
    /// @brief  The state within each state.
    enum class SubState : char {
        None = ' ',           ///< An invalid state
        String = 's',         ///< Any characters up to the line terminators
        FloatingPoint = 'f',  ///< Digits, -, +, .
        Integer = 'i',        ///< Unsigned Digits, no point.
    };

    struct Statistics {
        size_t object{0u};
        size_t vertices{0u};
        size_t normals{0u};
        size_t textures{0u};
        size_t faces{0u};
    };

    /// Constructs a parser with a given observer
    Parser(Observer& observer) : m_buffer_{}, m_index_{0u}, m_line_{1u}, m_observer_{observer} {
    }

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
        for (char const* tmp = str.c_str(); *tmp != '\0'; tmp++) {
            Parse(*tmp);
        }
    }

    /// Parses a string literal
    inline void Parse(char const* const str) {
        for (char const* tmp = str; *tmp != '\0'; tmp++) {
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
                if (std::isdigit(c) or c == ' ' or c == '/') {
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
                if constexpr (debug) {
                    printf("Vertices subtype?: %c\n", c);
                }
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
            case State::Textures:
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
                memset(&m_buffer_[0], 0, sizeof(m_buffer_));
            }
        }
        if (c == '\n') {
            m_line_++;
        }
    }

    size_t GetNumberOfLines(void) const {
        return m_line_;
    }

    Statistics const& GetStatistics(void) const {
        return m_statistics_;
    }

protected:
    void Complete(void) {
        if constexpr (debug) {
            printf("state=%c sub=%c Buffer=%s\n", basal::to_underlying(m_state_), basal::to_underlying(m_substate_),
                   m_buffer_);
        }
        switch (m_state_) {
            case State::Vertices:
                if (GetSubState() == SubState::FloatingPoint) {
                    float x = basal::nan;
                    float y = basal::nan;
                    float z = basal::nan;
                    int scans = sscanf(m_buffer_, " %f %f %f", &x, &y, &z);
                    if (3u == scans) {
                        m_observer_.addVertex(x, y, z);
                        m_statistics_.vertices++;
                    }
                }
                break;
            case State::Normals:
                if (GetSubState() == SubState::FloatingPoint) {
                    float x = basal::nan;
                    float y = basal::nan;
                    float z = basal::nan;
                    int scans = sscanf(m_buffer_, " %f %f %f", &x, &y, &z);
                    if (3u == scans) {
                        m_observer_.addNormal(x, y, z);
                        m_statistics_.normals++;
                    }
                }
                break;
            case State::Textures:
                if (GetSubState() == SubState::FloatingPoint) {
                    float u = basal::nan;
                    float v = basal::nan;
                    int scans = sscanf(m_buffer_, " %f %f", &u, &v);
                    if (2u == scans) {
                        m_observer_.addTexture(u, v);
                        m_statistics_.textures++;
                    }
                }
                break;
            case State::Faces:
                if (GetSubState() == SubState::Integer) {
                    uint32_t a = UINT32_MAX;
                    uint32_t b = UINT32_MAX;
                    uint32_t c = UINT32_MAX;
                    int scans = sscanf(m_buffer_, " %" SCNu32 " %" SCNu32 " %" SCNu32 "", &a, &b, &c);
                    if (3u == scans) {
                        m_observer_.addFace(a, b, c);
                        m_statistics_.faces++;
                        break;
                    }
                    uint32_t d = UINT32_MAX;
                    uint32_t e = UINT32_MAX;
                    uint32_t f = UINT32_MAX;
                    scans
                        = sscanf(m_buffer_, " %" SCNu32 "/%" SCNu32 " %" SCNu32 "/%" SCNu32 " %" SCNu32 "/%" SCNu32 "",
                                 &a, &d, &b, &e, &c, &f);
                    if (6u == scans) {
                        m_observer_.addFace(a, d, b, e, c, f);
                        m_statistics_.faces++;
                        break;
                    }
                    uint32_t g = UINT32_MAX;
                    uint32_t h = UINT32_MAX;
                    uint32_t i = UINT32_MAX;
                    scans = sscanf(m_buffer_,
                                   " %" SCNu32 "/%" SCNu32 "/%" SCNu32 " %" SCNu32 "/%" SCNu32 "/%" SCNu32 " %" SCNu32
                                   "/%" SCNu32 "/%" SCNu32 "",
                                   &a, &d, &g, &b, &e, &h, &c, &f, &i);
                    if (9u == scans) {
                        m_observer_.addFace(a, d, g, b, e, h, c, f, i);
                        m_statistics_.faces++;
                        break;
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
    Statistics m_statistics_;
};

}  // namespace obj
}  // namespace raytrace

#endif  // RAYTRACE_OBJPARSER_H_
