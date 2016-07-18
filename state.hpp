#include <iostream>

struct off {};
struct idle {};
struct turning {
  float target;
};

std::ostream& operator<<(std::ostream& os, const off&) {
  return os << "off{}";
}
std::ostream& operator<<(std::ostream& os, const idle&) {
  return os << "idle{}";
}
std::ostream& operator<<(std::ostream& os, const turning& s) {
  return os << "turning{" << s.target << "}";
}

