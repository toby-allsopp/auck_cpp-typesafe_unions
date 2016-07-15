#ifndef INCLUDED_TOBY_OVERLOAD_SET_H
#define INCLUDED_TOBY_OVERLOAD_SET_H

#include <utility>

namespace toby {

template <typename... Fs>
class overload_set;

template <>
class overload_set<> {
 public:
  void operator()() = delete;
};

template <typename F, typename... Fs>
class overload_set<F, Fs...>
    : private overload_set<Fs...>, private F {
 public:
  explicit overload_set(F&& f, Fs&&... fs)
      : overload_set<Fs...>(std::forward<Fs>(fs)...),
        F(std::forward<F>(f)) {}

  using F::operator();
  using overload_set<Fs...>::operator();
};

template <typename... Fs>
auto make_overload_set(Fs&&... fs) {
  return overload_set<Fs...>(std::forward<Fs>(fs)...);
}

}  // namespace toby

#endif
