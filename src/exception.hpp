#ifndef OPENDSAS_EXCEPTION_HPP_
#define OPENDSAS_EXCEPTION_HPP_

#include <sstream>
#include <stdexcept>
#include <string>

struct DSASError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

inline std::string make_error_message(const std::string& msg, const char* file,
                                      int line) {
#ifndef NDEBUG
  std::ostringstream oss;
  oss << msg << "\n  at " << file << ":" << line;
  return oss.str();
#else
  return msg;  // clean message: no file/line in release
#endif
}

#define OPENDSAS_THROW(msg) \
  throw DSASError(make_error_message((msg), __FILE__, __LINE__))

#endif  // OPENDSAS_EXCEPTION_HPP_