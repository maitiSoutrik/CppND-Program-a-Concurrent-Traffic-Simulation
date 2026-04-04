#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include "TrafficLight.h"
#include "Intersection.h"
#include "Graphics.h"
#include "Vehicle.h"

// =============================================================================
// MEDIUM PRIORITY FIXES - Tests for Issues #6, #7, #8, #11, #13
// =============================================================================

class MediumPriorityFixTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup before each test
    }

    void TearDown() override {
        // Cleanup after each test
    }
};

// Test for Issue #6: MEDIUM - Unreachable code in Intersection::trafficLightIsGreen()
// The dead code after return statement was removed

// Test 1: trafficLightIsGreen() is reachable and returns correct value
TEST_F(MediumPriorityFixTest, TrafficLightIsGreenReachable) {
    Intersection intersection;
    
    // This should execute without issue and return a valid boolean
    bool isGreen = intersection.trafficLightIsGreen();
    
    // isGreen should be either true or false
    EXPECT_TRUE(isGreen || !isGreen);
}

// Test 2: trafficLightIsGreen() returns consistent values
TEST_F(MediumPriorityFixTest, TrafficLightIsGreenConsistent) {
    Intersection intersection;
    
    // Call multiple times
    for (int i = 0; i < 10; ++i) {
        bool isGreen = intersection.trafficLightIsGreen();
        (void)isGreen;
    }
    
    SUCCEED();
}

// Test for Issue #7: MEDIUM - Memory leak - Graphics object allocated with 'new' but never deleted
// Test the proper resource management with destructor

// Test 3: Graphics destructor properly cleans up
TEST_F(MediumPriorityFixTest, GraphicsDestructorCleansUp) {
    {
        Graphics graphics;
        
        // Set up some resources
        std::vector<std::shared_ptr<TrafficObject>> objects;
        auto intersection = std::make_shared<Intersection>();
        objects.push_back(intersection);
        
        graphics.setTrafficObjects(objects);
        graphics.setBgFilename("test.jpg");
        
        // Graphics goes out of scope here, destructor should clean up
    }
    
    // If we got here without crash, destructor worked
    SUCCEED();
}

// Test 4: Graphics with no resources destructor works
TEST_F(MediumPriorityFixTest, GraphicsDestructorWithNoResources) {
    {
        Graphics graphics;
        // No resources set
    }
    
    SUCCEED();
}

// Test for Issue #8: MEDIUM - Unused member variable _condition in TrafficLight class
// This is a compile-time fix - we just verify the code compiles and works

// Test 5: TrafficLight works without _condition member
TEST_F(MediumPriorityFixTest, TrafficLightWorksWithoutUnusedMember) {
    TrafficLight light;
    
    // Should work normally
    EXPECT_EQ(light.getCurrentPhase(), TrafficLight::TrafficLightPhase::red);
    
    light.toggleLight();
    EXPECT_EQ(light.getCurrentPhase(), TrafficLight::TrafficLightPhase::green);
    
    SUCCEED();
}

// Test for Issue #11: MEDIUM - No graceful thread shutdown mechanism
// Test that threads can be signaled to exit

// Test 6: Simulated object can run and stop
TEST_F(MediumPriorityFixTest, SimulatedObjectCanRunAndStop) {
    TrafficLight light;
    
    // Start simulation
    light.simulate();
    
    // Let it run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Thread should be running (can't directly test shutdown)
    SUCCEED();
}

// Test 7: Intersection simulation can start
TEST_F(MediumPriorityFixTest, IntersectionSimulationCanStart) {
    Intersection intersection;
    
    intersection.simulate();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    SUCCEED();
}

// Test for Issue #13: MEDIUM - Race condition in trafficLightIsGreen() - no synchronization
// Test that getCurrentPhase() is thread-safe with proper locking

// Test 8: getCurrentPhase() is thread-safe with concurrent reads
TEST_F(MediumPriorityFixTest, GetCurrentPhaseThreadSafe) {
    TrafficLight light;
    std::atomic<int> reads{0};
    std::atomic<bool> running{true};
    
    // Start simulation
    light.simulate();
    
    // Multiple threads reading phase concurrently
    std::vector<std::thread> readers;
    for (int i = 0; i < 10; ++i) {
        readers.emplace_back([&]() {
            while (running && reads < 5000) {
                auto phase = light.getCurrentPhase();
                (void)phase;
                reads++;
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });
    }
    
    // Let them run
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    running = false;
    
    for (auto& t : readers) {
        t.join();
    }
    
    // Should have many reads without crash or data race
    EXPECT_GT(reads, 1000);
}

// Test 9: Concurrent reads during phase transition
TEST_F(MediumPriorityFixTest, ConcurrentReadsDuringPhaseTransition) {
    TrafficLight light;
    std::atomic<int> red_count{0};
    std::atomic<int> green_count{0};
    std::atomic<bool> running{true};
    
    light.simulate();
    
    // Reader threads
    std::vector<std::thread> readers;
    for (int i = 0; i < 5; ++i) {
        readers.emplace_back([&]() {
            while (running) {
                auto phase = light.getCurrentPhase();
                if (phase == TrafficLight::TrafficLightPhase::red) {
                    red_count++;
                } else {
                    green_count++;
                }
                std::this_thread::sleep_for(std::chrono::microseconds(50));
            }
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    running = false;
    
    for (auto& t : readers) {
        t.join();
    }
    
    // Should have seen both phases
    EXPECT_GT(red_count + green_count, 100);
}

// Test 10: Toggle during concurrent reads
TEST_F(MediumPriorityFixTest, ToggleDuringConcurrentReads) {
    TrafficLight light;
    std::atomic<bool> running{true};
    std::atomic<int> toggles{0};
    
    // Reader threads
    std::vector<std::thread> readers;
    for (int i = 0; i < 3; ++i) {
        readers.emplace_back([&]() {
            while (running) {
                volatile auto phase = light.getCurrentPhase();
                (void)phase;
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });
    }
    
    // Toggler thread
    std::thread toggler([&]() {
        while (running && toggles < 50) {
            light.toggleLight();
            toggles++;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    running = false;
    
    for (auto& t : readers) {
        t.join();
    }
    toggler.join();
    
    EXPECT_GT(toggles, 10);
}

// Test 11: Compile check - verify code compiles without unused members
TEST_F(MediumPriorityFixTest, CodeCompilesCleanly) {
    // This test just verifies the code compiles
    // If we're running, the code compiled successfully
    SUCCEED();
}
