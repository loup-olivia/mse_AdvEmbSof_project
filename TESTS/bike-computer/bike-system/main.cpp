// Copyright 2022 Haute école d'ingénierie et d'architecture de Fribourg
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/****************************************************************************
 * @file main.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Bike computer test suite: scheduling
 *
 * @date 2023-08-26
 * @version 0.1.0
 ***************************************************************************/

#include <cassert>
#include <chrono>
#include <cstdint>

#include "EventQueue.h"
#include "gear_device.hpp"
#include "greentea-client/test_env.h"
#include "mbed.h"
#include "multi_tasking/bike_system.hpp"
#include "static_scheduling/bike_system.hpp"
#include "static_scheduling_with_event/bike_system.hpp"
#include "task_logger.hpp"
#include "unity/unity.h"
#include "utest/utest.h"
#include "utest_case.h"

using namespace utest::v1;

// test_bike_system_event_queue handler function
static void test_bike_system_event_queue() {
    // create the BikeSystem instance
    static_scheduling::BikeSystem bikeSystem;

    // run the bike system in a separate thread
    Thread thread;
    thread.start(
        callback(&bikeSystem, &static_scheduling::BikeSystem::startWithEventQueue));

    // let the bike system run for 20 secs
    ThisThread::sleep_for(20s);

    // stop the bike system
    bikeSystem.stop();

    // check whether scheduling was correct
    // Order is kGearTaskIndex, kSpeedTaskIndex, kTemperatureTaskIndex,
    //          kResetTaskIndex, kDisplayTask1Index, kDisplayTask2Index
    // When we use the event queue, we do not check the computation time
    constexpr std::chrono::microseconds taskPeriods[] = {
        800000us, 400000us, 1600000us, 800000us, 1600000us, 1600000us};

    // allow for 2 msecs offset (with EventQueue)
    uint64_t deltaUs = 2000;
    for (uint8_t taskIndex = 0; taskIndex < advembsof::TaskLogger::kNbrOfTasks;
         taskIndex++) {
        TEST_ASSERT_UINT64_WITHIN(
            deltaUs,
            taskPeriods[taskIndex].count(),
            bikeSystem.getTaskLogger().getPeriod(taskIndex).count());
    }
}

// test_bike_system handler function
static void test_bike_system() {
    // create the BikeSystem instance
    static_scheduling::BikeSystem bikeSystem;

    // run the bike system in a separate thread
    Thread thread;
    thread.start(callback(&bikeSystem, &static_scheduling::BikeSystem::start));

    // let the bike system run for 20 secs
    ThisThread::sleep_for(20s);

    // stop the bike system
    bikeSystem.stop();

    // check whether scheduling was correct
    // Order is kGearTaskIndex, kSpeedTaskIndex, kTemperatureTaskIndex,
    //          kResetTaskIndex, kDisplayTask1Index, kDisplayTask2Index
    constexpr std::chrono::microseconds taskComputationTimes[] = {
        100000us, 200000us, 100000us, 100000us, 200000us, 100000us};
    constexpr std::chrono::microseconds taskPeriods[] = {
        800000us, 400000us, 1600000us, 800000us, 1600000us, 1600000us};

    // allow for 2 msecs offset
    uint64_t deltaUs = 2000;
    for (uint8_t taskIndex = 0; taskIndex < advembsof::TaskLogger::kNbrOfTasks;
         taskIndex++) {
        TEST_ASSERT_UINT64_WITHIN(
            deltaUs,
            taskPeriods[taskIndex].count(),
            bikeSystem.getTaskLogger()
                .getPeriod(taskIndex)
                .count());  // getTaskLogger is only defined when in testmode
        TEST_ASSERT_UINT64_WITHIN(
            deltaUs,
            taskComputationTimes[taskIndex].count(),
            bikeSystem.getTaskLogger().getComputationTime(taskIndex).count());
    }
}

// test_bike_system_with_event handler function
static void test_bike_system_with_event() {
    // create the BikeSystem instance
    static_scheduling_with_event::BikeSystem bikeSystem;

    // run the bike system in a separate thread
    Thread thread;
    thread.start(callback(&bikeSystem, &static_scheduling_with_event::BikeSystem::start));

    // let the bike system run for 20 secs
    ThisThread::sleep_for(20s);

    // stop the bike system
    bikeSystem.stop();

    // check whether scheduling was correct
    // Order is kGearTaskIndex, kSpeedTaskIndex, kTemperatureTaskIndex,
    //          kResetTaskIndex, kDisplayTask1Index, kDisplayTask2Index
    // When we use event handling, we do not check the computation time
    constexpr std::chrono::microseconds taskPeriods[] = {
        800000us, 400000us, 1600000us, 800000us, 1600000us, 1600000us};

    // allow for 2 msecs offset (with EventQueue)
    uint64_t deltaUs = 2000;
    for (uint8_t taskIndex = 0; taskIndex < advembsof::TaskLogger::kNbrOfTasks;
         taskIndex++) {
        TEST_ASSERT_UINT64_WITHIN(
            deltaUs,
            taskPeriods[taskIndex].count(),
            bikeSystem.getTaskLogger().getPeriod(taskIndex).count());
        // comment for oli
    }
}

// test_multi_tasking_bike_system handler function
static void test_multi_tasking_bike_system() {
    // create the BikeSystem instance
    multi_tasking::BikeSystem bikeSystem;

    // run the bike system in a separate thread
    Thread thread;
    thread.start(callback(&bikeSystem, &multi_tasking::BikeSystem::start));

    // let the bike system run for 20 secs
    ThisThread::sleep_for(20s);

    // stop the bike system
    bikeSystem.stop();

    // check whether scheduling was correct
    // Order is kGearTaskIndex, kSpeedTaskIndex, kTemperatureTaskIndex,
    //          kResetTaskIndex, kDisplayTask1Index, kDisplayTask2Index
    // When we use event handling, we do not check the computation time
    constexpr std::chrono::microseconds taskPeriods[] = {
        800000us, 400000us, 1600000us, 800000us, 1600000us, 1600000us};

    // allow for 2 msecs offset (with EventQueue)
    constexpr uint64_t kDeltaUs = 2000;
    TEST_ASSERT_UINT64_WITHIN(
        kDeltaUs,
        taskPeriods[advembsof::TaskLogger::kTemperatureTaskIndex].count(),
        bikeSystem.getTaskLogger()
            .getPeriod(advembsof::TaskLogger::kTemperatureTaskIndex)
            .count());
    TEST_ASSERT_UINT64_WITHIN(
        kDeltaUs,
        taskPeriods[advembsof::TaskLogger::kDisplayTask1Index].count(),
        bikeSystem.getTaskLogger()
            .getPeriod(advembsof::TaskLogger::kDisplayTask1Index)
            .count());
}

static void test_gear_multi_tasking_bike_system() {
    // create the BikeSystem instance
    multi_tasking::BikeSystem bikeSystem;

    // run the bike system in a separate thread
    Thread thread;
    thread.start(callback(&bikeSystem, &multi_tasking::BikeSystem::start));

    // let the bike system run for 2 secs
    ThisThread::sleep_for(2s);

    multi_tasking::GearDevice& gearDevice = bikeSystem.getGearDevice();

    // check for gear up
    constexpr uint8_t kNbrOfGearChange = 9;
    uint8_t nbrOfRightIncr             = 0;
    uint8_t nbrOfRightDecr             = 0;
    for (uint8_t i = 0; i < kNbrOfGearChange; i++) {
        uint8_t actualGear = bikeSystem.getCurrentGear();
        gearDevice.onJoystickUp();
        ThisThread::sleep_for(20ms);  // waiting for update of the task

        uint8_t newGear = bikeSystem.getCurrentGear();

        if ((actualGear + 1) == newGear) {
            nbrOfRightIncr++;
        }

        printf("New Gear : %d \n", newGear);
        printf("Actual Gear : %d\n", actualGear);
    }

    for (uint8_t i = 0; i < kNbrOfGearChange; i++) {
        uint8_t actualGear = bikeSystem.getCurrentGear();
        gearDevice.onJoystickDown();
        ThisThread::sleep_for(20ms);  // waiting for update of the task

        uint8_t newGear = bikeSystem.getCurrentGear();

        if ((actualGear - 1) == newGear) {
            nbrOfRightDecr++;
        }

        printf("New Gear : %d \n", newGear);
        printf("Actual Gear : %d\n", actualGear);
    }
    printf("number of gear down %d \n", nbrOfRightDecr);
    TEST_ASSERT_TRUE((nbrOfRightIncr + 1 == kNbrOfGearChange) &&
                     (nbrOfRightDecr + 1 == kNbrOfGearChange))
    // let the bike system run for 2 secs
    ThisThread::sleep_for(2s);

    // check for gear down
    constexpr uint8_t kNbrOfGearDown = 10;
    for (uint8_t i = 0; i < kNbrOfGearDown; i++) {
    }

    // stop the bike system
    bikeSystem.stop();
}

// test_reset_multi_tasking_bike_system handler function
Timer timer;
static std::chrono::microseconds resetTime = std::chrono::microseconds::zero();
static EventFlags eventFlags;
static constexpr uint32_t kResetEventFlag = (1UL << 0);
static void resetCallback() {
    resetTime = timer.elapsed_time();
    eventFlags.set(kResetEventFlag);
}

static void test_reset_multi_tasking_bike_system() {
    // create the BikeSystem instance
    multi_tasking::BikeSystem bikeSystem;

    // run the bike system in a separate thread
    Thread thread;
    thread.start(callback(&bikeSystem, &multi_tasking::BikeSystem::start));

    // let the bike system run for 2 secs
    ThisThread::sleep_for(2s);

    // test reset on BikeSystem
    bikeSystem.getSpeedometer().setOnResetCallback(resetCallback);

    // start the timer instance
    timer.start();

    // check for reset response time
    constexpr uint8_t kNbrOfResets             = 10;
    std::chrono::microseconds lastResponseTime = std::chrono::microseconds::zero();
    for (uint8_t i = 0; i < kNbrOfResets; i++) {
        // take time before reset
        auto startTime = timer.elapsed_time();

        // reset the BikeSystem
        bikeSystem.onReset();

        // wait for resetCallback to be called
        eventFlags.wait_all(kResetEventFlag);

        // get the response time and check it
        auto responseTime = resetTime - startTime;

        printf("Reset task: response time is %lld usecs\n", responseTime.count());

        // cppcheck generates an internal error with 20us
        constexpr std::chrono::microseconds kMaxExpectedResponseTime(20);
        TEST_ASSERT_TRUE(responseTime.count() <= kMaxExpectedResponseTime.count());

        constexpr uint64_t kDeltaUs = 4;
        constexpr std::chrono::microseconds kMaxExpectedJitter(3);
        if (i > 0) {
            auto jitter = responseTime - lastResponseTime;
            TEST_ASSERT_UINT64_WITHIN(
                kDeltaUs, kMaxExpectedJitter.count(), std::abs(jitter.count()));
        }
        lastResponseTime = responseTime;

        // let the bike system run for 2 secs
        ThisThread::sleep_for(2s);
    }

    // stop the bike system
    bikeSystem.stop();
}

static utest::v1::status_t greentea_setup(const size_t number_of_cases) {
    // Here, we specify the timeout (60s) and the host test (a built-in host test or the
    // name of our Python file)
    GREENTEA_SETUP(180, "default_auto");

    return greentea_test_setup_handler(number_of_cases);
}

// List of test cases in this file
static Case cases[] = {
    Case("test bike system", test_bike_system),
    Case("test bike system with event queue", test_bike_system_event_queue),
    Case("test bike system with event", test_bike_system_with_event),
    Case("test bike system multi tasking", test_multi_tasking_bike_system),
    Case("test bike system reset multi tasking", test_reset_multi_tasking_bike_system),
    Case("test bike system gear multi tasking", test_gear_multi_tasking_bike_system)};

static Specification specification(greentea_setup, cases);

int main() { return !Harness::run(specification); }
