/*
 * Copyright 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "handler.h"

#include <sys/eventfd.h>
#include <thread>

#include <gtest/gtest.h>
#include "base/logging.h"

namespace bluetooth {
namespace common {
namespace {

class HandlerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    thread_ = new Thread("test_thread", Thread::Priority::NORMAL);
    handler_ = new Handler(thread_);
  }
  void TearDown() override {
    delete handler_;
    delete thread_;
  }

  Handler* handler_;
  Thread* thread_;
};

TEST_F(HandlerTest, empty) {}

TEST_F(HandlerTest, post_task_invoked) {
  int val = 0;
  Closure closure = [&val]() { val++; };
  handler_->Post(closure);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  EXPECT_EQ(val, 1);
}

TEST_F(HandlerTest, post_task_cleared) {
  int val = 0;
  Closure closure = [&val]() {
    val++;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  };
  handler_->Post(std::move(closure));
  closure = []() { EXPECT_EQ(0, 1) << "Should not happen"; };
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  handler_->Post(std::move(closure));
  handler_->Clear();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  EXPECT_EQ(val, 1);
}

TEST_F(HandlerTest, post_task_from_callback) {
  int val = 0;
  Closure closure = [&val, this] {
    Closure inner_closure = [&val] { val++; };
    handler_->Post(inner_closure);
  };
  handler_->Post(closure);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  EXPECT_EQ(val, 1);
}

TEST_F(HandlerTest, clear_task_from_callback) {
  int val = 0;
  Closure closure = [&val, this] {
    val++;
    handler_->Clear();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  };
  handler_->Post(std::move(closure));
  closure = []() { EXPECT_EQ(0, 1) << "Should not happen"; };
  handler_->Post(std::move(closure));
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  EXPECT_EQ(val, 1);
}

}  // namespace
}  // namespace common
}  // namespace bluetooth