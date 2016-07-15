#include <iostream>

struct idle {};
struct turning {
  float targetHeading;
};

std::ostream& operator<<(std::ostream& os, const idle&) {
  return os << "idle{}";
}
std::ostream& operator<<(std::ostream& os, const turning& s) {
  return os << "turning{" << s.targetHeading << "}";
}

