#pragma once
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <sys/ioctl.h>
#include <unistd.h>

namespace term {

namespace c {
    inline const char* reset()  { return "\033[0m";  }
    inline const char* bold()   { return "\033[1m";  }
    inline const char* dim()    { return "\033[2m";  }
    inline const char* red()    { return "\033[31m"; }
    inline const char* yellow() { return "\033[33m"; }
    inline const char* cyan()   { return "\033[36m"; }
    inline const char* clear()  { return "\r\033[K"; }
}

namespace input {
    inline std::string& buf()  { static std::string s; return s; }
    inline std::string& nick() { static std::string s; return s; }
}

namespace screen {
    inline int& rows() { static int r = 24; return r; }
    inline bool& active() { static bool b = false; return b; }

    inline int query_rows() {
        struct winsize ws{};
        if (::ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 2)
            return static_cast<int>(ws.ws_row);
        return 24;
    }

    inline void init() {
        rows() = query_rows();
        std::cout
            << "\033[1;" << (rows() - 1) << "r"
            << "\033[" << rows() << ";1H"
            << c::dim() << std::string(80, '-') << c::reset()
            << "\033[" << rows() << ";1H"
            << std::flush;
        active() = true;
    }

    inline void resize() {
        rows() = query_rows();
        std::cout
            << "\033[1;" << (rows() - 1) << "r"
            << "\033[" << rows() << ";1H"
            << "\033[2K"
            << std::flush;
    }

    inline void cleanup() {
        active() = false;
        std::cout
            << "\033[r"
            << "\033[" << rows() << ";1H"
            << "\n"
            << std::flush;
    }
}

inline std::string ts() {
    std::time_t t  = std::time(nullptr);
    std::tm*    tm = std::localtime(&t);
    char buf[9];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
             tm->tm_hour, tm->tm_min, tm->tm_sec);
    return buf;
}

inline void _redraw_input() {
    if (!screen::active() || input::nick().empty()) return;
    std::cout
        << "\033[" << screen::rows() << ";1H"
        << "\033[K"
        << c::dim()  << "["  << c::reset()
        << c::bold() << input::nick() << c::reset()
        << c::dim()  << "]"  << c::reset()
        << c::cyan() << " ▸ " << c::reset()
        << input::buf()
        << std::flush;
}

inline void _emit(const std::string& line) {
    if (!screen::active()) {
        std::cout << c::clear() << line << "\n" << std::flush;
        return;
    }
    int r = screen::rows();
    std::cout
        << "\033[" << (r - 1) << ";1H"
        << "\r\n"
        << "\r\033[K"
        << line;
    _redraw_input();
}

inline void msg(const std::string& nick, const std::string& text) {
    std::ostringstream s;
    s << c::dim()  << "[" << ts() << "] " << c::reset()
      << c::bold() << nick << c::reset()
      << c::dim()  << ": " << c::reset()
      << text;
    _emit(s.str());
}

inline void sys(const std::string& text) {
    std::ostringstream s;
    s << c::dim()  << "[" << ts() << "] "
      << c::cyan() << "* " << c::reset()
      << c::dim()  << text << c::reset();
    _emit(s.str());
}

inline void err(const std::string& text) {
    std::ostringstream s;
    s << c::dim() << "[" << ts() << "] "
      << c::red() << "! " << c::reset()
      << text;
    _emit(s.str());
}

inline void sec(const std::string& text) {
    std::ostringstream s;
    s << c::dim()    << "[" << ts() << "] "
      << c::yellow() << "~ " << c::reset()
      << text;
    _emit(s.str());
}

inline void prompt(const std::string& nick) {
    std::cout
        << c::dim()  << "["  << c::reset()
        << c::bold() << nick << c::reset()
        << c::dim()  << "]"  << c::reset()
        << c::cyan() << " ▸ " << c::reset()
        << std::flush;
}

}