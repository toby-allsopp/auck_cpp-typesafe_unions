#include "multivisitor.hpp"
#include "variant.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using toby::variant;
using toby::make_multivisitor;

struct special_member_counts {
  int num_default_constructor;
  int num_copy_constructor;
  int num_move_constructor;
  int num_copy_assignment;
  int num_move_assignment;
  int num_destructor;
} counts;

struct special_member_counter {
  special_member_counter() { ++counts.num_default_constructor; }
  special_member_counter(const special_member_counter&) { ++counts.num_copy_constructor; }
  special_member_counter(special_member_counter&&) { ++counts.num_move_constructor; }
  special_member_counter& operator=(const special_member_counter&) { ++counts.num_copy_assignment; return *this; }
  special_member_counter& operator=(special_member_counter&&) { ++counts.num_move_assignment; return *this; }
  ~special_member_counter() { ++counts.num_destructor; }
};

TEST_CASE("constructing a variant from a temporary uses the move constructor", "[variant]") {
  counts = {};
  {
    variant<special_member_counter> v(special_member_counter{});
  }
  REQUIRE(counts.num_default_constructor == 1);
  REQUIRE(counts.num_copy_constructor == 0);
  REQUIRE(counts.num_move_constructor == 1);
  REQUIRE(counts.num_copy_assignment == 0);
  REQUIRE(counts.num_move_assignment == 0);
  REQUIRE(counts.num_destructor == 2);
}
TEST_CASE("using multivisitor does not make any copies", "[multivisitor]") {
  {
    variant<special_member_counter> v1(special_member_counter{});
    variant<special_member_counter> v2(special_member_counter{});
    counts = {};
    auto visitor = make_multivisitor<void>([](const auto&, const auto&){});
    REQUIRE(counts.num_default_constructor == 0);
    REQUIRE(counts.num_copy_constructor == 0);
    REQUIRE(counts.num_move_constructor == 0);
    REQUIRE(counts.num_copy_assignment == 0);
    REQUIRE(counts.num_move_assignment == 0);
    REQUIRE(counts.num_destructor == 0);
    visitor(v1, v2);
    REQUIRE(counts.num_default_constructor == 0);
    REQUIRE(counts.num_copy_constructor == 0);
    REQUIRE(counts.num_move_constructor == 0);
    REQUIRE(counts.num_copy_assignment == 0);
    REQUIRE(counts.num_move_assignment == 0);
    REQUIRE(counts.num_destructor == 0);
  }
}

auto variant_logger = ::spdlog::stderr_logger_st("variant", true);
