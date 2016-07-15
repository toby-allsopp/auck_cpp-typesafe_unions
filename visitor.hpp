#include "variant.hpp"

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

template <typename Callable, typename Tuple,
          size_t... I>
auto apply_impl(Callable&& f, Tuple&& t,
                std::index_sequence<I...>) {
  return f(std::get<I>(t)...);
}

template <typename Callable, typename Tuple>
auto apply(Callable&& f, Tuple&& t) {
  using is = std::make_index_sequence<
      std::tuple_size<std::decay_t<Tuple>>::value>;
  return apply_impl(std::forward<Callable>(f),
                    std::forward<Tuple>(t), is());
}

template <typename R, typename F>
class visitor {
 private:
  F m_f;

 public:
  explicit visitor(F&& f) : m_f(f) {}

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
    return arg.template visit<R>([&](auto v) {
      return this->collect(
          std::tuple_cat(t, std::make_tuple(v)),
          args...);
    });
  }
};

template <typename T, typename F>
auto make_visitor(F&& f) {
  return visitor<T, F>(std::forward<F>(f));
}

template <typename T, typename... Fs>
auto make_visitor(Fs&&... fs) {
  return make_visitor<T>(
      make_overload_set(std::forward<Fs>(fs)...));
}
}
