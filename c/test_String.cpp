#include "gtest/gtest.h"

#include "test_StringUtils.hpp"

#include <array>
#include <cstring>
#include <string>
#include <string_view>

namespace {

class CoreString : public ::testing::Test {
protected:
  void SetUp() override {
    std::fill(std::begin(str_pool), std::end(str_pool), atb_String_MakeEmpty());
  }

  void TearDown() override {
    for (auto &str : str_pool)
      atb_String_Delete(&str);
  }

  std::array<atb_String, 5> str_pool;
};

TEST_F(CoreString, StringViewCStr) {
  std::string str = "Coucou";
  auto view = atb_StringView_FromCStr(str.data());
  EXPECT_EQ(view, std::string_view{"Coucou"});
}

TEST_F(CoreString, ConstStringViewCStr) {
  constexpr std::string_view str = "Coucou";
  auto view = atb_ConstStringView_FromCStr(str.data());
  EXPECT_EQ(view, std::string_view{"Coucou"});
}

TEST_F(CoreString, CtorEmpty) {
  auto str = atb_String_MakeEmpty();
  EXPECT_EQ(str.data, nullptr);
  EXPECT_EQ(str.capacity, 0);
  EXPECT_EQ(str.size, 0);
}

TEST_F(CoreString, CtorSubStr) {
  constexpr auto initial_str = std::string_view{"Chocolatine !"};

  {
    auto str = atb_String_MakeCopyFromSubStr(
        atb_ConstStringView{initial_str.data(), initial_str.size()});

    EXPECT_NE(str.data, nullptr);
    EXPECT_NE(str.data, initial_str.data());
    EXPECT_EQ(str.capacity, initial_str.size() + 1);
    EXPECT_EQ(str.size, initial_str.size());
    EXPECT_EQ(str, initial_str);

    atb_String_Delete(&str);
  }

  {
    auto str = atb_String_MakeCopyFromSubStr(
        atb_ConstStringView{initial_str.data(), 2});

    EXPECT_NE(str.data, nullptr);
    EXPECT_NE(str.data, initial_str.data());
    EXPECT_EQ(str.capacity, 3);
    EXPECT_EQ(str.size, 2);
    EXPECT_EQ(str, "Ch");

    atb_String_Delete(&str);
  }
}

TEST_F(CoreString, CtorCopy) {
  constexpr auto initial_str = std::string_view{"Chocolatine !"};

  auto str = atb_String_MakeCopyFromSubStr(
      atb_ConstStringView{initial_str.data(), initial_str.size()});
  auto copy = atb_String_MakeCopyFrom(&str);

  EXPECT_NE(copy.data, nullptr);
  EXPECT_NE(copy.data, str.data);
  EXPECT_NE(copy.data, initial_str.data());

  EXPECT_EQ(copy.capacity, str.capacity);
  EXPECT_EQ(copy.size, str.size);
  EXPECT_EQ(copy, str);

  atb_String_Delete(&copy);
  atb_String_Delete(&str);
}

TEST_F(CoreString, CtorMoveSubStr) {
  constexpr auto capacity = 10u;
  constexpr auto size = 5u;

  char *buffer = reinterpret_cast<char *>(malloc(capacity * sizeof(*buffer)));
  char *buffer_before = buffer;

  auto moved_str = atb_String_MakeByMovingSubStr(&buffer, capacity, size);

  EXPECT_EQ(buffer, nullptr);
  EXPECT_NE(moved_str.data, nullptr);
  EXPECT_EQ(moved_str.data, buffer_before);
  EXPECT_EQ(moved_str.capacity, capacity);
  EXPECT_EQ(moved_str.size, size);

  atb_String_Delete(&moved_str);

  if (buffer != nullptr) {
    free(buffer);
  }
}

TEST_F(CoreString, CtorMoveCStr) {
  constexpr auto expected_c_str = std::string_view{"Chocolatine"};
  char *c_str = strdup(expected_c_str.data());
  char *c_str_before = c_str;

  auto moved_str = atb_String_MakeByMovingCStr(&c_str);

  EXPECT_EQ(c_str, nullptr);
  EXPECT_NE(moved_str.data, nullptr);
  EXPECT_EQ(moved_str.data, c_str_before);
  EXPECT_EQ(moved_str.capacity, expected_c_str.size() + 1);
  EXPECT_EQ(moved_str.size, expected_c_str.size());
  EXPECT_EQ(moved_str, expected_c_str);

  atb_String_Delete(&moved_str);

  if (c_str != nullptr) {
    free(c_str);
  }
}

TEST_F(CoreString, CtorMove) {
  atb_String str = {
      (char *)0x123,
      0x4,
      0x5,
  };
  atb_String move = atb_String_MakeByMoving(&str);
  EXPECT_EQ(str.data, nullptr);
  EXPECT_EQ(str.capacity, 0);
  EXPECT_EQ(str.size, 0);

  EXPECT_NE(move.data, nullptr);
  EXPECT_EQ(move.data, (char *)0x123);
  EXPECT_EQ(move.capacity, 0x4);
  EXPECT_EQ(move.size, 0x5);
}

TEST_F(CoreString, ReserveDelete) {
  atb_String_Reserve(&str_pool[0], 10);
  EXPECT_NE(str_pool[0].data, nullptr);
  EXPECT_EQ(str_pool[0].capacity, 10);
  EXPECT_EQ(str_pool[0].size, 0);

  atb_String_Reserve(&str_pool[0], 20);
  EXPECT_NE(str_pool[0].data, nullptr);
  EXPECT_EQ(str_pool[0].capacity, 20);
  EXPECT_EQ(str_pool[0].size, 0);

  atb_String_Reserve(&str_pool[0], 5);
  EXPECT_NE(str_pool[0].data, nullptr);
  EXPECT_EQ(str_pool[0].capacity, 5);
  EXPECT_EQ(str_pool[0].size, 0);

  atb_String_Delete(&str_pool[0]);
  EXPECT_EQ(str_pool[0].data, nullptr);
  EXPECT_EQ(str_pool[0].capacity, 0);
  EXPECT_EQ(str_pool[0].size, 0);

  atb_String_Delete(&str_pool[0]);
  EXPECT_EQ(str_pool[0].data, nullptr);
  EXPECT_EQ(str_pool[0].capacity, 0);
  EXPECT_EQ(str_pool[0].size, 0);
}

TEST_F(CoreString, ResizeAndFill) {
  auto expected_str = std::string_view{"1234567890"};
  atb_String_ResizeAndFill(
      &str_pool[0], 10,
      atb_String_Generator{
          &expected_str, [](void *const self, char *first, char *last) {
            auto *str_to_cpy = reinterpret_cast<std::string_view *>(self);
            for (; first != last;) {
              std::size_t count = std::min(
                  str_to_cpy->size(),
                  static_cast<std::size_t>(std::distance(first, last)));
              std::memcpy(first, str_to_cpy->data(), count);
              first += count;
            }
          }});

  EXPECT_EQ(str_pool[0], expected_str);
  EXPECT_EQ(str_pool[0].capacity, expected_str.size() + 1);
}

TEST_F(CoreString, Resize) {
  atb_String_Resize(&str_pool[0], 10, 'A');
  EXPECT_EQ(str_pool[0].capacity, 11);
  EXPECT_EQ(str_pool[0], std::string(str_pool[0].size, 'A'));

  atb_String_Reserve(&str_pool[0], str_pool[0].capacity - 5);
  EXPECT_EQ(str_pool[0].capacity, 6);
  EXPECT_EQ(str_pool[0], std::string(str_pool[0].size, 'A'));

  atb_String_Resize(&str_pool[0], str_pool[0].size + 5, 'B');
  EXPECT_EQ(str_pool[0].capacity, 11);
  EXPECT_EQ(str_pool[0], std::string(5, 'A') + std::string(5, 'B'));

  atb_String_Resize(&str_pool[0], str_pool[0].size + 5, -1);
  EXPECT_NE(str_pool[0].data, nullptr);
  EXPECT_EQ(str_pool[0].capacity, 16);
  EXPECT_EQ(str_pool[0].size, 15);
  EXPECT_EQ((std::string_view{str_pool[0].data, 10}),
            (std::string(5, 'A') + std::string(5, 'B')));
}

TEST_F(CoreString, ShrinkToFit) {
  atb_String_Resize(&str_pool[0], 10, 'A');
  EXPECT_EQ(str_pool[0].capacity, 11);

  atb_String_Resize(&str_pool[0], 5, 'A');
  EXPECT_GT(str_pool[0].capacity, str_pool[0].size + 1);

  atb_String_ShrinkToFit(&str_pool[0]);
  EXPECT_EQ(str_pool[0].capacity, str_pool[0].size + 1);

  atb_String_ShrinkToFit(&str_pool[0]);
  EXPECT_EQ(str_pool[0].capacity, str_pool[0].size + 1);
}

TEST_F(CoreString, AppendSubStr) {
  constexpr auto other_str = std::string_view{"Chocolatine"};

  atb_String_Reserve(&str_pool[0], other_str.size() + 3);
  EXPECT_EQ(str_pool[0].capacity, other_str.size() + 3);
  EXPECT_EQ(str_pool[0].size, 0);

  std::string expected_str{other_str};

  // Does not trigger realloc
  atb_String_AppendSubStr(
      &str_pool[0], atb_ConstStringView{other_str.data(), other_str.size()});
  EXPECT_EQ(str_pool[0].capacity, other_str.size() + 3);
  EXPECT_EQ(str_pool[0].size, expected_str.size());
  EXPECT_EQ(str_pool[0], expected_str);

  expected_str += other_str;
  // triggers realloc
  atb_String_AppendSubStr(
      &str_pool[0], atb_ConstStringView{other_str.data(), other_str.size()});
  EXPECT_EQ(str_pool[0].capacity, expected_str.size() + 1);
  EXPECT_EQ(str_pool[0].size, expected_str.size());
  EXPECT_EQ(str_pool[0], expected_str);

  expected_str += std::string_view(other_str.data(), 3);
  // sub string
  atb_String_AppendSubStr(&str_pool[0],
                          atb_ConstStringView{other_str.data(), 3});
  EXPECT_EQ(str_pool[0].capacity, expected_str.size() + 1);
  EXPECT_EQ(str_pool[0].size, expected_str.size());
  EXPECT_EQ(str_pool[0], expected_str);
}

TEST_F(CoreString, Append) {

  constexpr auto coucou = std::string_view{"Coucou"};
  str_pool[1] = atb_String_MakeCopyFromSubStr(
      atb_ConstStringView{coucou.data(), coucou.size()});

  atb_String_Reserve(&str_pool[0], str_pool[1].size + 3);
  EXPECT_EQ(str_pool[0].capacity, str_pool[1].size + 3);
  EXPECT_EQ(str_pool[0].size, 0);

  std::string expected_str{str_pool[1].data};

  // Does not trigger realloc
  atb_String_Append(&str_pool[0], &str_pool[1]);
  EXPECT_EQ(str_pool[0].capacity, str_pool[1].size + 3);
  EXPECT_EQ(str_pool[0].size, expected_str.size());
  EXPECT_EQ(str_pool[0], expected_str);

  expected_str += str_pool[1].data;
  // triggers realloc
  atb_String_Append(&str_pool[0], &str_pool[1]);
  EXPECT_EQ(str_pool[0].capacity, expected_str.size() + 1);
  EXPECT_EQ(str_pool[0].size, expected_str.size());
  EXPECT_EQ(str_pool[0], expected_str);
}

TEST_F(CoreString, PopN) {
  // Does nothing on empty string
  atb_String_PopN(&str_pool[0], 31415);
  EXPECT_EQ(str_pool[0].capacity, 0);
  EXPECT_EQ(str_pool[0].size, 0);

  constexpr auto expected_str = std::string_view{"Coucou"};

  atb_String_AppendSubStr(
      &str_pool[0],
      atb_ConstStringView{expected_str.data(), expected_str.size()});

  EXPECT_EQ(str_pool[0].capacity, expected_str.size() + 1);
  EXPECT_EQ(str_pool[0].size, expected_str.size());
  EXPECT_EQ(str_pool[0], expected_str);

  atb_String_PopN(&str_pool[0], 3);
  EXPECT_EQ(str_pool[0].capacity, expected_str.size() + 1);
  EXPECT_EQ(str_pool[0].size, expected_str.size() - 3);
  EXPECT_EQ(str_pool[0],
            std::string_view(expected_str.data(), expected_str.size() - 3));

  atb_String_PopN(&str_pool[0], 31415);
  EXPECT_EQ(str_pool[0].capacity, expected_str.size() + 1);
  EXPECT_EQ(str_pool[0].size, 0);
  EXPECT_EQ(str_pool[0], "");
}

TEST_F(CoreString, IsEqualToSubStr) {
  constexpr auto expected_str = std::string_view{"Coucou"};
  constexpr auto other_str = std::string_view{"Chocolatine"};

  atb_String_AppendSubStr(
      &str_pool[0],
      atb_ConstStringView{expected_str.data(), expected_str.size()});
  EXPECT_PRED2(atb_String_IsEqualToSubStr, &str_pool[0],
               (atb_ConstStringView{str_pool[0].data, str_pool[0].size}));

  EXPECT_PRED2(atb_String_IsEqualToSubStr, &str_pool[0],
               (atb_ConstStringView{expected_str.data(), expected_str.size()}));

  EXPECT_FALSE(atb_String_IsEqualToSubStr(
      &str_pool[0], atb_ConstStringView{other_str.data(), other_str.size()}));

  EXPECT_FALSE(atb_String_IsEqualToSubStr(
      &str_pool[0],
      atb_ConstStringView{other_str.data(), other_str.size() - 3}));

  atb_String_PopN(&str_pool[0], 2);

  EXPECT_FALSE(atb_String_IsEqualToSubStr(
      &str_pool[0],
      atb_ConstStringView{expected_str.data(), expected_str.size()}));
}

TEST_F(CoreString, IsEqualTo) {
  constexpr auto expected_str = std::string_view{"Coucou"};
  atb_String_AppendSubStr(
      &str_pool[0],
      atb_ConstStringView{expected_str.data(), expected_str.size()});
  atb_String_AppendSubStr(
      &str_pool[1],
      atb_ConstStringView{expected_str.data(), expected_str.size()});

  EXPECT_PRED2(atb_String_IsEqualTo, &str_pool[0], &str_pool[0]);
  EXPECT_PRED2(atb_String_IsEqualTo, &str_pool[0], &str_pool[1]);

  atb_String_AppendSubStr(
      &str_pool[1],
      atb_ConstStringView{expected_str.data(), expected_str.size()});
  EXPECT_FALSE(atb_String_IsEqualTo(&str_pool[0], &str_pool[1]));

  atb_String_PopN(&str_pool[1], expected_str.size() + 3);
  EXPECT_FALSE(atb_String_IsEqualTo(&str_pool[0], &str_pool[1]));

  atb_String_PopN(&str_pool[0], 5);
  EXPECT_FALSE(atb_String_IsEqualTo(&str_pool[0], &str_pool[1]));
}

} // namespace
