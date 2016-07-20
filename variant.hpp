#ifndef INCLUDED_TOBY_VARIANT_H
#define INCLUDED_TOBY_VARIANT_H

#include <cstddef>
#include <stdexcept>

#include <spdlog/spdlog.h>

namespace toby {
namespace detail {
inline auto logger() { return spdlog::get("variant"); }

template <typename I, I N, typename... Ts>
struct variant_construct;

template <typename I, I N>
struct variant_construct<I, N> {
  static I construct();
};

template <typename I, I N, typename T, typename... Ts>
struct variant_construct<I, N, T, Ts...>
    : private variant_construct<I, N + 1, Ts...> {
  using super = variant_construct<I, N + 1, Ts...>;

  static I construct(void* storage, const T& value) {
    logger()->debug() << "copy construct<" << N << ">";
    new (storage) T(value);
    return N;
  }
  static I construct(void* storage, T&& value) {
    logger()->debug() << "move construct<" << N << ">";
    new (storage) T(std::forward<T>(value));
    return N;
  }
  using super::construct;
};

template <typename I, I N, typename... Ts>
struct variant_visit;

template <typename I, I N>
struct variant_visit<I, N> {
  template <typename R, typename F>
  static R visit_helper_const(I tag, const void*, F&&) {
    throw std::logic_error("variant tag invalid: " + std::to_string(tag));
  }
  template <typename R, typename F>
  static R visit_helper_rvalue(I tag, void*, F&&) {
    throw std::logic_error("variant tag invalid: " + std::to_string(tag));
  }
};

template <typename I, I N, typename T, typename... Ts>
struct variant_visit<I, N, T, Ts...> : private variant_visit<I, N + 1, Ts...> {
  using super = variant_visit<I, N + 1, Ts...>;

  template <typename R, typename F>
  static R visit_helper_const(I tag, const void* storage, F&& f) {
    logger()->debug() << "visit_helper<" << N << "> const& (" << tag << ")";
    if (tag == N) {
      return f(*reinterpret_cast<const T*>(storage));
    }
    return super::template visit_helper_const<R>(tag, storage,
                                                 std::forward<F>(f));
  }
  template <typename R, typename F>
  static R visit_helper_rvalue(I tag, void* storage, F&& f) {
    logger()->debug() << "visit_helper<" << N << "> && (" << tag << ")";
    if (tag == N) {
      return f(std::move(*reinterpret_cast<T*>(storage)));
    }
    return super::template visit_helper_rvalue<R>(tag, storage,
                                                  std::forward<F>(f));
  }
};

template <uintmax_t N, typename Enable = void>
struct smallest_unisnged_type;

template <uintmax_t N>
struct smallest_unisnged_type<
    N, typename std::enable_if_t<N <= std::numeric_limits<uint8_t>::max()>> {
  using type = uint8_t;
};

template <uintmax_t N>
using smallest_unisnged_type_t = typename smallest_unisnged_type<N>::type;

template <typename... Ts>
struct variant_helper {
  using tag_type = smallest_unisnged_type_t<sizeof...(Ts)>;
  using super_construct = variant_construct<tag_type, 0, Ts...>;
  using super_visit = variant_visit<tag_type, 0, Ts...>;
};
}

template <typename... Ts>
class variant : private detail::variant_helper<Ts...>::super_construct,
                private detail::variant_helper<Ts...>::super_visit {
  using helper = detail::variant_helper<Ts...>;
  using super_construct = typename helper::super_construct;
  using super_visit = typename helper::super_visit;

 public:
  typename std::aligned_union_t<0, char, Ts...> storage;
  typename helper::tag_type tag;

 private:
  using super_construct::construct;

  void destruct() {
    std::move(*this).template visit<void>([this](auto&& v) {
      using T = std::decay_t<decltype(v)>;
      reinterpret_cast<T*>(&storage)->~T();
    });
  }

 public:
  template <typename T, typename = decltype(construct(
                            &storage, std::forward<T>(std::declval<T>())))>
  variant(T&& value) {
    tag = construct(&storage, std::forward<T>(value));
  }

  variant(const variant& other) {
    detail::logger()->debug("variant copy constructor");
    other.visit<void>(
        [this](auto&& value) { tag = construct(&storage, value); });
  }
  variant(variant&& other) {
    detail::logger()->debug("variant move constructor");
    std::move(other).template visit<void>([this](auto&& value) {
      tag = construct(&storage, std::forward<decltype(value)>(value));
    });
  }

  template <typename T, typename = decltype(construct(
                            &storage, std::forward<T>(std::declval<T>())))>
  variant& operator=(T&& value) {
    destruct();
    tag = construct(&storage, std::forward<T>(value));
    return *this;
  }

  variant& operator=(const variant& other) {
    detail::logger()->debug("variant copy assignment");
    destruct();
    other.visit<void>(
        [this](auto&& value) { tag = construct(&storage, value); });
    return *this;
  }
  variant& operator=(variant&& other) {
    detail::logger()->debug("variant move assignment");
    destruct();
    std::move(other).template visit<void>([this](auto&& value) {
      tag = construct(&storage, std::forward<decltype(value)>(value));
    });
    return *this;
  }

  ~variant() { destruct(); }

  template <typename R, typename F>
  auto visit(F&& f) const& {
    detail::logger()->debug("visit const &");
    return super_visit::template visit_helper_const<R>(tag, &storage,
                                                       std::forward<F>(f));
  }
  template <typename R, typename F>
  auto visit(F&& f) && {
    detail::logger()->debug("visit &&");
    return std::move(*this).super_visit::template visit_helper_rvalue<R>(
        tag, &storage, std::forward<F>(f));
  }
};

template <typename... Ts>
std::ostream& operator<<(std::ostream& os, const variant<Ts...>& v) {
  os << "variant[" << v.tag << "]: ";
  v.template visit<void>([&os](const auto& x) { os << x; });
  return os;
}

}  // namespace toby

#endif
