#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <string>
#include <string_view>

#include <fcntl.h>
#include <sys/stat.h>
#include <system_error>
#include <tuple>
#include <unistd.h>
#include <utility>

#include "test_Utils.hpp"

extern "C" {
#include "FdUtils.h"
}

using namespace std::literals;

namespace {

static void CloseAndDeleteFile(const std::filesystem::path &path,
                               int fd) noexcept {
  const auto msg_header = utils::ConcatString(
      "\"", path.c_str(), "\" (FD = ", std::to_string(fd), ")");

  std::cout << "Closing " << msg_header << ":...\n";
  if ((fd > 0) and (close(fd) != 0)) {
    std::cerr << "Closing " << msg_header << ": FAILED - "
              << std::strerror(errno);
  } else {
    std::cout << "Closing " << msg_header << ": DONE\n";
  }

  std::cout << "Removing " << msg_header << ":...\n";
  if (unlink(path.c_str()) != 0) {
    std::cerr << "Removing " << msg_header << ": FAILED - "
              << std::strerror(errno);
  } else {
    std::cout << "Removing " << msg_header << ": DONE\n";
  }
}

TEST(CoreFd, ReadUntilFile) {
  constexpr auto build_file = utils::BuilderWrapper{
      [](std::filesystem::path path, int flags, mode_t mode) {
        const auto msg_header = utils::ConcatString(
            "Opening \"", path.c_str(), "\" (FLAGS = ", std::to_string(flags),
            ", MODE = ", std::to_string(mode), ")");

        std::cout << msg_header << ":...\n";
        int fd = open(path.c_str(), flags, mode);
        if (fd == -1) {
          throw std::system_error{
              errno,
              std::generic_category(),
              msg_header,
          };
        }

        std::cout << msg_header << ": DONE (FD = " << fd << ")\n";

        return std::make_pair(std::move(path), fd);
      },
      [](auto &&d) {
        std::apply(&CloseAndDeleteFile, std::forward<decltype(d)>(d));
      },
  };

  auto file = utils::MakeScopeGuard<std::pair<std::filesystem::path, int>>(
      build_file, std::filesystem::current_path() / "tmp_file",
      O_RDWR | O_CREAT, 0777);

  constexpr auto GetFdFrom = [](const decltype(file) &file_guard) {
    return file_guard.GetData().value().second;
  };

  constexpr auto data =
      "Coucou - Si qqn lit ce texte, bravo !\n"
      "Tout le monde ici à l'air allergique aux tests unitaires, alors que "
      "c'est nécessaire, en plus d'être fun avec les frameworks adéquats !"sv;

  write(GetFdFrom(file), data.data(), data.size());
  lseek(GetFdFrom(file), 0, SEEK_SET);

  std::array<char, data.size() + 1> buffer = {'\0'};
  // std::fill(std::begin(buffer), std::end(buffer), '\0');

  EXPECT_EQ(atb_Fd_ReadBytesUntil(GetFdFrom(file), buffer.data(), 0, ' '), 0);

  for (const auto expected_str : {
           "Coucou -"sv,
           " Si qqn lit ce texte, bravo !\n"sv,
           "Tout "sv,
           "le monde ici à l'air allergique aux tests unitaires, "
           "alors que c'est nécessaire, en plus d'être fun avec les "
           "frameworks adéquats !"sv,
       }) {
    EXPECT_EQ(atb_Fd_ReadBytesUntil(GetFdFrom(file), buffer.data(),
                                    buffer.size() - 1, expected_str.back()),
              expected_str.size());
    EXPECT_EQ(std::string_view(buffer.data(), expected_str.size()),
              expected_str);
  }

  lseek(GetFdFrom(file), 0, SEEK_SET);
  std::fill(std::begin(buffer), std::end(buffer), '\0');

  EXPECT_EQ(atb_Fd_ReadBytesUntil(GetFdFrom(file), buffer.data(), 2, '\n'), 2);
  EXPECT_EQ(std::string_view(buffer.data(), 2), "Co");

  lseek(GetFdFrom(file), 0, SEEK_SET);
  std::fill(std::begin(buffer), std::end(buffer), '\0');

  EXPECT_EQ(
      atb_Fd_ReadBytesUntil(GetFdFrom(file), buffer.data(), buffer.size(), '@'),
      data.size());
  EXPECT_EQ(std::string_view(buffer.data(), data.size()), data);

  EXPECT_EQ(
      atb_Fd_ReadBytesUntil(GetFdFrom(file), buffer.data(), buffer.size(), '@'),
      0);
}

TEST(CoreFd, ReadUntilFifo) {
  constexpr auto build_fifo = utils::BuilderWrapper{
      [](std::filesystem::path path, int flags, mode_t mode) {
        const auto msg_header = utils::ConcatString(
            "Fifo \"", path.c_str(), "\" (FLAGS = ", std::to_string(flags),
            ", MODE = ", std::to_string(mode), ")");

        std::cout << "Mkfifo " << msg_header << ":...\n";
        if (mkfifo(path.c_str(), mode) == -1) {
          throw std::system_error{
              errno,
              std::generic_category(),
              "Mkfifo " + msg_header,
          };
        }
        std::cout << "Mkfifo " << msg_header << ": DONE\n";

        std::cout << "Opening " << msg_header << ":...\n";
        int fd = open(path.c_str(), flags, mode);
        if (fd == -1) {
          throw std::system_error{
              errno,
              std::generic_category(),
              "Opening " + msg_header,
          };
        }
        std::cout << "Opening " << msg_header << ": DONE (FD = " << fd << ")\n";

        return std::make_pair(std::move(path), fd);
      },
      [](auto &&d) {
        std::apply(&CloseAndDeleteFile, std::forward<decltype(d)>(d));
      },
  };

  auto fifo = utils::MakeScopeGuard<std::pair<std::filesystem::path, int>>(
      build_fifo, std::filesystem::current_path() / "tmp_fifo",
      O_RDWR | O_NONBLOCK, 0777);

  constexpr auto GetFdFrom = [](const decltype(fifo) &file_guard) {
    return file_guard.GetData().value().second;
  };

  constexpr auto data =
      "Coucou - Si qqn lit ce texte, bravo !\n"
      "Tout le monde ici à l'air allergique aux tests unitaires, alors que "
      "c'est nécessaire, en plus d'être fun avec les frameworks adéquats !"sv;

  write(GetFdFrom(fifo), data.data(), data.size());

  std::array<char, data.size() + 1> buffer = {'\0'};
  // std::fill(std::begin(buffer), std::end(buffer), '\0');

  EXPECT_EQ(atb_Fd_ReadBytesUntil(GetFdFrom(fifo), buffer.data(), 0, ' '), 0);

  for (const auto expected_str : {
           "Coucou -"sv,
           " Si qqn lit ce texte, bravo !\n"sv,
           "Tout "sv,
           "le monde ici à l'air allergique aux tests unitaires, "
           "alors que c'est nécessaire, en plus d'être fun avec les "
           "frameworks adéquats !"sv,
       }) {
    EXPECT_EQ(atb_Fd_ReadBytesUntil(GetFdFrom(fifo), buffer.data(),
                                    buffer.size() - 1, expected_str.back()),
              expected_str.size());
    EXPECT_EQ(std::string_view(buffer.data(), expected_str.size()),
              expected_str);
  }

  // Empty fifo
  EXPECT_EQ(
      atb_Fd_ReadBytesUntil(GetFdFrom(fifo), buffer.data(), data.size(), '@'),
      -1);
}

} // namespace
