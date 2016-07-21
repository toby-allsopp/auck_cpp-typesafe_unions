#include <iostream>
#include <string>

struct turn_on {};
struct turn_off {};
struct start_turning {
  float target;
};
struct reset {
  std::string reason;
};
struct heading_changed {
  float heading;
};

std::ostream& operator<<(std::ostream& os, const start_turning& e) {
  return os << "start_turning{" << e.target << "}";
}
std::ostream& operator<<(std::ostream& os, const reset& e) {
  return os << "reset{" << e.reason << "}";
}
std::ostream& operator<<(std::ostream& os, const heading_changed& e) {
  return os << "heading_changed" << e.heading << "}";
}
