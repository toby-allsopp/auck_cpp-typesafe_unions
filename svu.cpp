#include "multivisitor.hpp"
#include "overload_set.hpp"

#include "spdlog/spdlog.h"

#include <stdexcept>
#include <string>
#include <vector>

using toby::overload_set;
using toby::make_multivisitor;

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
    return const_cast<svu*>(this)->visit<R>([f = std::forward<F>(f)](auto&& x) {
      return f(const_cast<std::add_const_t<decltype(x)>>(x));
    });
  }

  template <typename R, typename... Fs>
  auto visit(Fs&&... fs) {
    return visit<R>(overload_set<Fs...>(std::forward<Fs>(fs)...));
  }
  template <typename R, typename... Fs>
  auto visit(Fs&&... fs) const {
    return visit<R>(overload_set<Fs...>(std::forward<Fs>(fs)...));
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
    other.visit<void>([this](auto&& v) { construct(v); });
    return *this;
  }

  ~svu() { destruct(); }
};

struct Visitor {
  int operator()(const std::string& s) { return s.size(); }
  int operator()(const std::vector<int>& v) { return v[0]; }
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
  auto visitor = make_multivisitor<int>(
      [](const string& s1, const string& s2) { return s1.size() + s2.size(); },
      [](const string& s1, const vector<int>& v2) { return s1.size() + v2[0]; },
      [](const vector<int> v1, const string& s2) { return v1.size() + s2[0]; },
      [](const vector<int> v1, const vector<int>& v2) {
        return v1[0] + v2[0];
      });
  return visitor(u1, u2);
}

void svu_test() {
  auto logger = spdlog::get("main");
  svu x("Hello");
  x.visit<void>(
      [&](auto v) { logger->info("x contains a {}", typeid(v).name()); });
  x = std::vector<int>{1, 2, 3};
  x.visit<void>(
      [&](auto v) { logger->info("x contains a {}", typeid(v).name()); });
  x.visit<int>([](const std::string& s) { return s.size(); },
               [](const std::vector<int> v) { return v[0]; });
  // x.visit(toby::make_overload_set());
}
