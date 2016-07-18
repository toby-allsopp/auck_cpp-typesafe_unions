#include "event.hpp"
#include "multivisitor.hpp"
#include "state.hpp"
#include "variant.hpp"

#include <cmath>
#include <iostream>
#include <utility>

using toby::variant;
using toby::make_multivisitor;

using state = variant<off, idle, turning>;
using on = variant<idle, turning>;

using event = variant<turn_on, turn_off, start_turning, reset,
                      heading_changed>;

// clang-format off
state transition(const state& s, const event& e) {
  return make_multivisitor<state>(
      [](off,       turn_on)         { return idle{}; },
      [](off,       auto)            { return off{}; },
      [](on,        turn_off)        { return off{}; },
      [](auto s,    turn_on)         { return s; },
      [](on,        reset)           { return idle{}; },
      [](on,        start_turning e) { return turning{e.target}; },
      [](idle s,    heading_changed) { return s; },
      [](turning s, heading_changed e) -> state {
        if (std::abs(e.heading - s.target) < .1f) {
          return idle{};
        } else {
          return s;
        }
      })(s, e);
}
// clang-format on

using empty_variant = variant<>;

using string_variant = variant<std::string, std::vector<int>>;

auto logger = spdlog::stderr_logger_st("main", true);
auto variant_logger = ::spdlog::stderr_logger_st("variant", true);

int main() {
  spdlog::set_level(spdlog::level::debug);
  string_variant sv("Hello");
  sv = std::vector<int>{1, 2, 3};
  logger->info("sizeof(empty_variant) = {}",
               sizeof(empty_variant));
  logger->info("sizeof(empty_variant.tag) = {}",
               sizeof(empty_variant::tag));
  logger->info("sizeof(empty_variant.storage) = {}",
               sizeof(empty_variant::storage));
  logger->info("sizeof(event) = {}", sizeof(event));
  logger->info("sizeof(event.tag) = {}", sizeof(event::tag));
  logger->info("sizeof(event.storage) = {}",
               sizeof(event::storage));
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
