#include <tuple>
#include <type_traits>
#include <vector>

#include "doctest.h"

#ifdef SINGLE_HEADER
#include "cista.h"
#else
#include "cista/equal_to.h"
#include "cista/hashing.h"
#include "cista/reflection/for_each_field.h"
#include "cista/serialization.h"
#include "cista/type_hash/type_hash.h"
#endif

namespace cista_members_pointers {

template <auto... Ps>
struct members {};

namespace data = cista::offset;

struct inner {
  inner() = default;
  static constexpr auto cista_members() noexcept {
    return members<&inner::b_, &inner::s_>{};
  }
  int b_{0};
  data::string s_;
};

struct parent {
  parent() = default;
  explicit parent(double u) : u_{u} {}
  double u_{1};
};

struct rec : public parent {
  rec() = default;
  static constexpr auto cista_members() noexcept {
    return members<&rec::u_, &rec::a_, &rec::v_, &rec::i_>{};
  }
  int a_{2};
  data::vector<unsigned> v_;
  inner i_;
};

// The same fields as rec, listed as a tuple of references.
struct rec_tie : public parent {
  rec_tie() = default;
  auto cista_members() noexcept { return std::tie(u_, a_, v_, i_); }
  int a_{2};
  data::vector<unsigned> v_;
  inner i_;
};

TEST_CASE("cista_members member pointers to tuple") {
  static_assert(cista::detail::has_cista_member_pointers_v<rec>);
  static_assert(!cista::detail::has_cista_member_pointers_v<rec_tie>);
  static_assert(cista::to_tuple_works_v<rec>);

  auto const c = rec{};
  static_assert(
      std::is_same_v<decltype(std::get<1>(cista::to_tuple(c))), int const&>);
  static_assert(std::is_same_v<
                std::decay_t<decltype(std::get<1>(cista::to_ptr_tuple(c)))>,
                int const*>);
  CHECK(std::get<0>(cista::to_tuple(c)) == 1.0);

  auto m = rec{};
  std::get<1>(cista::to_tuple(m)) = 77;
  CHECK(m.a_ == 77);
}

TEST_CASE("cista_members member pointers for_each_field") {
  auto r = rec{};
  auto n = 0;
  cista::for_each_field(r, [&](auto& field) {
    using field_t = std::decay_t<decltype(field)>;
    if constexpr (std::is_same_v<field_t, int>) {
      CHECK(n == 1);
      field = 5;
    }
    ++n;
  });
  CHECK(n == 4);
  CHECK(r.a_ == 5);
}

TEST_CASE("cista_members member pointers serialization") {
  cista::byte_buf buf;
  {
    auto r = rec{};
    r.u_ = 3.5;
    r.a_ = 7;
    r.v_.push_back(1U);
    r.v_.push_back(2U);
    r.i_.b_ = 9;
    r.i_.s_ = data::string{"a string long enough to live on the heap"};
    buf = cista::serialize(r);
  }
  auto const* r = cista::deserialize<rec>(buf);
  CHECK(r->u_ == 3.5);
  CHECK(r->a_ == 7);
  CHECK(r->v_.size() == 2U);
  CHECK(r->v_[1] == 2U);
  CHECK(r->i_.b_ == 9);
  CHECK(r->i_.s_ == data::string{"a string long enough to live on the heap"});

  auto copy = rec{};
  copy.u_ = 3.5;
  copy.a_ = 7;
  copy.v_.push_back(1U);
  copy.v_.push_back(2U);
  copy.i_.b_ = 9;
  copy.i_.s_ = data::string{"a string long enough to live on the heap"};
  CHECK(cista::hashing<rec>{}(*r) == cista::hashing<rec>{}(copy));
  CHECK(cista::equal_to<rec>{}(*r, copy));
}

TEST_CASE("cista_members member pointers keep the type hash") {
  CHECK(cista::type_hash<rec>() == cista::type_hash<rec_tie>());
}

}  // namespace cista_members_pointers
