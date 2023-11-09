#include "gtest/gtest.h"

#include <array>
#include <cstring>
#include <filesystem>
#include <thread>

#include <fcntl.h>
#include <unistd.h>

#include "test_StringUtils.hpp"
#include "test_Utils.hpp"

extern "C" {
#include "Process.h"
}

using namespace std::literals;

namespace {

TEST(CoreProcessArgs, InitStatic) {
  atb_ProcessArgs args = atb_ProcessArgs_INITIALIZER();
  EXPECT_EQ(args.list, nullptr);
  EXPECT_EQ(args.size, 0u);
  atb_ProcessArgs_Delete(&args);
}

TEST(CoreProcessArgs, Init) {
  atb_ProcessArgs args;
  atb_ProcessArgs_Init(&args);
  EXPECT_EQ(args.list, nullptr);
  EXPECT_EQ(args.size, 0u);
  atb_ProcessArgs_Delete(&args);
}

TEST(CoreProcessArgs, AppendCStrList) {
  auto argv = std::array{
      "Bonjour",
      "Foo",
      "Bar",
  };

  atb_ProcessArgs args = atb_ProcessArgs_INITIALIZER();
  atb_ProcessArgs_AppendCStrList(&args, argv.data(), argv.size());
  ASSERT_NE(args.list, nullptr);
  ASSERT_EQ(args.size, argv.size());

  for (auto i = 0u; i < argv.size(); ++i) {
    EXPECT_STREQ(args.list[i], argv[i]);
  }

  EXPECT_EQ(args.list[args.size], nullptr);

  atb_ProcessArgs_AppendCStrList(&args, argv.data(), 0);
  ASSERT_NE(args.list, nullptr);
  ASSERT_EQ(args.size, argv.size());
  EXPECT_EQ(args.list[args.size], nullptr);

  atb_ProcessArgs_AppendCStrList(&args, argv.data(), argv.size());
  ASSERT_EQ(args.size, argv.size() * 2);
  ASSERT_NE(args.list, nullptr);

  for (auto i = 0u; i < args.size; ++i) {
    EXPECT_STREQ(args.list[i], argv[i % argv.size()]);
  }

  EXPECT_EQ(args.list[args.size], nullptr);

  atb_ProcessArgs_Delete(&args);
  EXPECT_EQ(args.list, nullptr);
  EXPECT_EQ(args.size, 0);
}

TEST(CoreProcessArgs, AppendCopy) {
  atb_ProcessArgs args = atb_ProcessArgs_INITIALIZER();

  constexpr auto argv = std::array{
      "Bonjour",
      "Foo",
      "Bar",
  };
  atb_ProcessArgs other_args = atb_ProcessArgs_INITIALIZER();
  atb_ProcessArgs_AppendCStrList(&other_args, argv.data(), argv.size());

  atb_ProcessArgs_AppendCopy(&args, &other_args);
  ASSERT_EQ(args.size, other_args.size);
  ASSERT_NE(args.list, other_args.list);
  ASSERT_NE(args.list, nullptr);

  for (auto i = 0u; i < args.size; ++i) {
    EXPECT_STREQ(args.list[i], other_args.list[i]);
  }
  EXPECT_EQ(args.list[args.size], nullptr);

  atb_ProcessArgs_AppendCopy(&args, &other_args);
  ASSERT_EQ(args.size, other_args.size * 2);
  ASSERT_NE(args.list, other_args.list);
  ASSERT_NE(args.list, nullptr);

  for (auto i = 0u; i < args.size; ++i) {
    EXPECT_STREQ(args.list[i], other_args.list[i % other_args.size]);
  }
  EXPECT_EQ(args.list[args.size], nullptr);

  atb_ProcessArgs_Delete(&other_args);
  atb_ProcessArgs_Delete(&args);
}

TEST(CoreProcess, InitStatic) {
  struct atb_Process proc = atb_Process_INITIALIZER();
  EXPECT_EQ(proc.path.data, nullptr);
  EXPECT_EQ(proc.path.size, 0u);
  EXPECT_EQ(proc.path.capacity, 0u);
  EXPECT_EQ(proc.pid, 0);
  EXPECT_FALSE(atb_Process_IsAlive(&proc));
}

TEST(CoreProcess, Init) {
  struct atb_Process proc;
  atb_Process_Init(&proc);
  EXPECT_EQ(proc.path.data, nullptr);
  EXPECT_EQ(proc.path.size, 0u);
  EXPECT_EQ(proc.path.capacity, 0u);
  EXPECT_EQ(proc.pid, 0);
  EXPECT_FALSE(atb_Process_IsAlive(&proc));
}

TEST(CoreProcess, LaunchKill) {
  struct atb_Process proc;
  atb_Process_Init(&proc);

  constexpr auto build_fifo = utils::BuilderWrapper{
      [](std::filesystem::path path, int flags, mode_t mode) {
        const auto msg_header = utils::ConcatString(
            "Fifo \"", path.c_str(), "\" (FLAGS = ", std::to_string(flags),
            ", MODE = ", std::to_string(mode), ")");

        std::cout << "Mkfifo " << msg_header << ":...\n";
        if ((mkfifo(path.c_str(), mode) == -1) and (errno != EEXIST)) {
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
      [](const auto &d) {
        const auto &[path, fd] = d;
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
      },
  };

  auto fifo = utils::MakeScopeGuard<std::pair<std::filesystem::path, int>>(
      build_fifo, std::filesystem::current_path() / "tmp_fifo",
      O_RDWR | O_NONBLOCK, 0777);

  constexpr auto GetPath = [](const decltype(fifo) &file_guard) {
    return file_guard.GetData().value().first;
  };

  constexpr auto GetFdFrom = [](const decltype(fifo) &file_guard) {
    return file_guard.GetData().value().second;
  };

  constexpr auto expected_fifo_content = std::string_view{"Coucou"};
  const auto proc_cmd =
      utils::ConcatString("echo -n \"", expected_fifo_content, "\" > ",
                          GetPath(fifo).c_str(), " && sleep 1m");

  auto argv_list = std::array{
      (const char *)"-c",
      (const char *)proc_cmd.data(),
  };

  atb_ProcessArgs argv = atb_ProcessArgs_INITIALIZER();
  atb_ProcessArgs_AppendCStrList(&argv, argv_list.data(), argv_list.size());

  // TEST LAUNCH SUCCESS //////////////////////////////////////////////////////
  constexpr auto expected_path = std::string_view{"/bin/sh"};
  auto path = atb_String_MakeCopyFromSubStr(
      atb_ConstStringView{expected_path.data(), expected_path.size()});
  ASSERT_EQ(atb_Process_Launch(&proc, &path, &argv),
            ATB_PROCESS_LAUNCH_SUCCESS);
  EXPECT_EQ(path.data, nullptr);
  EXPECT_EQ(path.size, 0);
  EXPECT_EQ(path.capacity, 0);
  atb_String_Delete(&path);

  EXPECT_EQ(proc.path, expected_path);
  EXPECT_NE(proc.pid, -1);
  EXPECT_TRUE(atb_Process_IsAlive(&proc));

  // TEST LAUNCH FAILURE ALREADY RUNNING //////////////////////////////////////
  atb_String_AppendSubStr(
      &path, atb_ConstStringView{expected_path.data(), expected_path.size()});
  EXPECT_EQ(atb_Process_Launch(&proc, &path, &argv),
            ATB_PROCESS_LAUNCH_ALREADY_RUNNING);
  EXPECT_NE(path.data, nullptr);
  EXPECT_NE(path.size, 0);
  EXPECT_NE(path.capacity, 0);
  atb_String_Delete(&path);

  atb_ProcessArgs_Delete(&argv);
  // TEST GET STATUS -> RUNNING ///////////////////////////////////////////////
  atb_ProcessStatus status;
  EXPECT_TRUE(atb_Process_GetStatus(&proc, &status));
  EXPECT_EQ(status.state, atb_ProcessStatus::ATB_PROCESS_RUNNING);

  std::this_thread::sleep_for(50ms);
  EXPECT_TRUE(atb_Process_IsAlive(&proc));

  // TEST KILL ////////////////////////////////////////////////////////////////
  EXPECT_EQ(atb_Process_Kill(&proc, &status), ATB_PROCESS_KILL_SUCCESS);
  EXPECT_EQ(status.state, atb_ProcessStatus::ATB_PROCESS_KILLED);
  EXPECT_EQ(status.info.sig_id, SIGKILL);

  EXPECT_NE(proc.path, expected_path);
  EXPECT_EQ(proc.pid, -1);

  EXPECT_FALSE(atb_Process_IsAlive(&proc));

  // TEST FIFO CONTENT ////////////////////////////////////////////////////////
  std::array<char, 50> buffer = {'\0'};
  EXPECT_EQ(read(GetFdFrom(fifo), buffer.data(), buffer.size()),
            expected_fifo_content.size());
  EXPECT_EQ(std::string_view(buffer.data(), expected_fifo_content.size()),
            expected_fifo_content);
}

TEST(CoreProcess, Status) {
  struct atb_Process proc;
  atb_Process_Init(&proc);

  auto argv_list = std::array{
      (const char *)"-c",
      (const char *)"sleep 0.05s && exit 123",
  };

  atb_ProcessArgs argv = atb_ProcessArgs_INITIALIZER();
  atb_ProcessArgs_AppendCStrList(&argv, argv_list.data(), argv_list.size());

  constexpr auto expected_path = std::string_view{"/bin/sh"};
  auto path = atb_String_MakeCopyFromSubStr(
      atb_ConstStringView{expected_path.data(), expected_path.size()});
  ASSERT_EQ(atb_Process_Launch(&proc, &path, &argv),
            ATB_PROCESS_LAUNCH_SUCCESS);
  atb_String_Delete(&path);
  atb_ProcessArgs_Delete(&argv);

  atb_ProcessStatus status;
  EXPECT_TRUE(atb_Process_GetStatus(&proc, &status));
  EXPECT_EQ(status.state, atb_ProcessStatus::ATB_PROCESS_RUNNING);

  std::this_thread::sleep_for(60ms);
  EXPECT_TRUE(atb_Process_IsAlive(&proc));

  EXPECT_TRUE(atb_Process_GetStatus(&proc, &status));
  EXPECT_EQ(status.state, atb_ProcessStatus::ATB_PROCESS_EXITED);
  EXPECT_EQ(status.info.exit_code, 123);
  EXPECT_TRUE(atb_Process_IsAlive(&proc));

  EXPECT_EQ(atb_Process_Kill(&proc, NULL), ATB_PROCESS_KILL_SUCCESS);
  EXPECT_FALSE(atb_Process_IsAlive(&proc));
}

} // namespace
