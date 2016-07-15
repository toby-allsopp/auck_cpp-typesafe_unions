#include <iostream>

struct start_turning {
  float targetHeading;
};
struct stop_turning {
  std::string reason;
};
struct heading_changed {
  float heading;
};

std::ostream& operator<<(std::ostream& os, const start_turning& e) {
  return os << "start_turning{" << e.targetHeading << "}";
}
std::ostream& operator<<(std::ostream& os, const stop_turning& e) {
  return os << "stop_turning{" << e.reason <<"}";
}
std::ostream& operator<<(std::ostream& os, const heading_changed& e) {
  return os << "heading_changed" << e.heading << "}";
}
