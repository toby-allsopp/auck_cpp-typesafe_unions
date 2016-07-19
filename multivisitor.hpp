#ifndef INCLUDED_TOBY_MULTIVISITOR_H
#define INCLUDED_TOBY_MULTIVISITOR_H

#include "overload_set.hpp"

#include <tuple>

namespace toby {

namespace detail {
template <typename Callable, typename Tuple,
          size_t... I>
auto apply_impl(Callable&& f, Tuple&& t,
                std::index_sequence<I...>) {
  return f(std::get<I>(t)...);
}
}

template <typename Callable, typename Tuple>
auto apply(Callable&& f, Tuple&& t) {
  using is = std::make_index_sequence<
      std::tuple_size<std::decay_t<Tuple>>::value>;
  return detail::apply_impl(std::forward<Callable>(f),
                            std::forward<Tuple>(t),
                            is());
}

template <typename R, typename F>
class multivisitor {
 private:
  F m_f;

 public:
  explicit multivisitor(F&& f) : m_f(f) {}

  template <typename... Vs>
  auto operator()(const Vs&... args) {
    return collect(std::tuple<>(), args...);
  }

 private:
  template <typename T>
  auto collect(const T& t) {
    return apply(m_f, t);
  }

  template <typename T, typename V, typename... Vs>
  auto collect(const T& t, const V& arg,
               const Vs&... args) {
    return arg.template visit<R>([&](const auto& v) {
      return this->collect(
          std::tuple_cat(t, std::tuple<decltype(v)>(v)),
          args...);
    });
  }
};

template <typename T, typename F>
auto make_multivisitor(F&& f) {
  return multivisitor<T, F>(std::forward<F>(f));
}

template <typename T, typename... Fs>
auto make_multivisitor(Fs&&... fs) {
  return make_multivisitor<T>(
      make_overload_set(std::forward<Fs>(fs)...));
}

}  // namespace toby

#endif
