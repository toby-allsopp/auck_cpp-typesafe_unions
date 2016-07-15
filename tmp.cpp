#include "event.hpp"
#include "state.hpp"
#include "variant.hpp"
#include "visitor.hpp"

#include <cmath>
#include <iostream>
#include <typeinfo>
#include <utility>

using toby::variant;

using state = variant<idle, turning>;

union state_u {
  idle i;
  turning t;
};

using event = variant<start_turning, stop_turning,
                      heading_changed>;

struct event_u {
  enum {
    START_TURNING,
    STOP_TURNING,
    HEADING_CHANGED
  } discriminator;
  union {
    start_turning v_start_turning;
    stop_turning v_stop_turning;
    heading_changed v_heading_changed;
  } value;
};

state_u transition_u(const state_u& current_state,
                     const event_u& e) {
  return current_state;
}

state transition(const state& current_state,
                 const event& e) {
  return toby::make_visitor<state>(
      [](auto, start_turning e) {
        return turning{e.targetHeading};
      },
      [](auto, stop_turning) { return idle{}; },
      [](auto s, heading_changed) { return s; },
      [](turning s, heading_changed e) -> state {
        if (std::abs(e.heading - s.targetHeading) <
            .1f) {
          return idle{};
        } else {
          return s;
        }
      })(current_state, e);
}

using empty_variant = variant<>;

using string_variant =
    variant<std::string, std::vector<int>>;

using toby::overload_set;

class svu {
 private:
  enum { STRING, VECTOR } tag;
  union {
    std::string s;
    std::vector<int> v;
  };

 public:
  template <typename R, typename F>
  R visit(F&& f) {
    switch (tag) {
      case STRING:
        return f(s);
      case VECTOR:
        return f(v);
    }
    throw std::logic_error("Bad tag");
  }
  template <typename R, typename F>
  R visit(F&& f) const {
    return const_cast<svu*>(this)
        ->visit<R>([f = std::forward<F>(f)](
            auto&& x) {
          return f(const_cast<
                   std::add_const_t<decltype(x)>>(x));
        });
  }

  template <
      typename R, typename... Fs,
      typename = std::enable_if_t<sizeof...(Fs) >= 2>>
  auto visit(Fs&&... fs) {
    return visit<R>(
        overload_set<Fs...>(std::forward<Fs>(fs)...));
  }
  template <
      typename R, typename... Fs,
      typename = std::enable_if_t<sizeof...(Fs) >= 2>>
  auto visit(Fs&&... fs) const {
    return visit<R>(
        overload_set<Fs...>(std::forward<Fs>(fs)...));
  }

 private:
  void construct(const std::string& _s) {
    tag = STRING;
    new (&s) std::string(_s);
  }
  void construct(const std::vector<int>& _v) {
    tag = VECTOR;
    new (&v) std::vector<int>(_v);
  }
  void destruct() {
    visit<void>([](auto&& x) {
      using T = std::decay_t<decltype(x)>;
      x.~T();
    });
  }

 public:
  template <typename T>
  svu(const T& x) {
    construct(x);
  }

  svu& operator=(const svu& other) {
    destruct();
    other.visit<void>(
        [this](auto&& v) { construct(v); });
    return *this;
  }

  ~svu() { destruct(); }
};

struct Visitor {
  int operator()(const std::string& s) {
    return s.size();
  }
  int operator()(const std::vector<int> v) {
    return v[0];
  }
};

int plux(const svu& u1, const svu& u2) {
  using namespace std;
  return u1.visit<int>(
      [&](const string& s1) {
        return u2.visit<int>(
            [&](const string& s2) { return s1.size() + s2.size(); },
            [&](const vector<int>& v2) { return s1.size() + v2[0]; });
      },
      [&](const vector<int> v1) {
        return u2.visit<int>(
            [&](const string& s2) { return v1.size() + s2[0]; },
            [&](const vector<int>& v2) { return v1[0] + v2[0]; });
      });
}

int plux2(const svu& u1, const svu& u2) {
  using namespace std;
  auto visitor = toby::make_visitor<int>(
      [](const string& s1, const string& s2) { return s1.size() + s2.size(); },
      [](const string& s1, const vector<int>& v2) { return s1.size() + v2[0]; },
      [](const vector<int> v1, const string& s2) { return v1.size() + s2[0]; },
      [](const vector<int> v1, const vector<int>& v2) {
        return v1[0] + v2[0];
      });
  return visitor(u1, u2);
}

auto logger = spdlog::stderr_logger_st("main", true);
auto variant_logger =
    ::spdlog::stderr_logger_st("variant", true);

int main() {
  spdlog::set_level(spdlog::level::debug);
  string_variant sv("Hello");
  sv = std::vector<int>{1, 2, 3};
  svu x("Hello");
  x.visit<void>([](auto v) {
    logger->info("x contains a {}", typeid(v).name());
  });
  x = std::vector<int>{1, 2, 3};
  x.visit<void>([](auto v) {
    logger->info("x contains a {}", typeid(v).name());
  });
  x.visit<int>(
      [](const std::string& s) { return s.size(); },
      [](const std::vector<int> v) { return v[0]; });
  // x.visit(toby::make_overload_set());
  logger->info("sizeof(empty_variant) = {}",
               sizeof(empty_variant));
  logger->info("sizeof(empty_variant.tag) = {}",
               sizeof(empty_variant::tag));
  logger->info("sizeof(empty_variant.storage) = {}",
               sizeof(empty_variant::storage));
  logger->info("sizeof(event) = {}", sizeof(event));
  logger->info("sizeof(event.tag) = {}",
               sizeof(event::tag));
  logger->info("sizeof(event.storage) = {}",
               sizeof(event::storage));
  logger->info("sizeof(event_u) = {}",
               sizeof(event_u));
  logger->info("sizeof(event_u.discriminator) = {}",
               sizeof(event_u::discriminator));
  logger->info("sizeof(event_u.value) = {}",
               sizeof(event_u::value));
  // empty_variant ev;
  logger->info("creating state from temporary idle");
  state s0{idle{}};
  logger->info("creating state from temporary state");
  state s = std::move(s0);
  logger->info(
      "calling transition with temporary event "
      "from "
      "temporary event");
  s = transition(s, start_turning{42});
  logger->info("state = {}", s);
  logger->info(
      "calling transition with temporary event "
      "from "
      "temporary event");
  s = transition(s, heading_changed{101});
  logger->info("state = {}", s);
  s = transition(s, heading_changed{42});
  logger->info("state = {}", s);
}
