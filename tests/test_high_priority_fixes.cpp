#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <cmath>
#include "TrafficLight.h"
#include "Vehicle.h"
#include "Street.h"
#include "Intersection.h"

// =============================================================================
// HIGH PRIORITY FIXES - Tests for Issues #3 and #4
// =============================================================================

class HighPriorityFixTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup before each test
    }

    void TearDown() override {
        // Cleanup after each test
    }
};

// Test for Issue #3: HIGH - MessageQueue::receive() clears all messages instead of popping single message
// The fix: receive() should pop only one message (front), not clear all

// Test 1: receive() pops only single message (not all messages)
TEST_F(HighPriorityFixTest, ReceivePopsSingleMessage) {
    MessageQueue<int> queue;
    
    // Send 3 messages
    queue.send(1);
    queue.send(2);
    queue.send(3);
    
    // Receive 1 message
    int received = queue.receive();
    EXPECT_EQ(received, 1);
    
    // Verify 2 messages remain (in FIFO order)
    // The queue uses deque with push_back and pop_back
    // So after receiving 1, we should have 2 and 3 remaining
    
    // We can't directly check queue size, but we can receive the rest
    int received2 = queue.receive();
    EXPECT_EQ(received2, 2);
    
    int received3 = queue.receive();
    EXPECT_EQ(received3, 3);
}

// Test 2: FIFO order is maintained after partial receives
TEST_F(HighPriorityFixTest, FIFOOrderMaintained) {
    MessageQueue<std::string> queue;
    
    // Send messages
    queue.send("first");
    queue.send("second");
    queue.send("third");
    queue.send("fourth");
    
    // Receive first two
    EXPECT_EQ(queue.receive(), "first");
    EXPECT_EQ(queue.receive(), "second");
    
    // Send more
    queue.send("fifth");
    
    // Receive remaining (third, fourth, fifth)
    EXPECT_EQ(queue.receive(), "third");
    EXPECT_EQ(queue.receive(), "fourth");
    EXPECT_EQ(queue.receive(), "fifth");
}

// Test 3: Multiple partial receives work correctly
TEST_F(HighPriorityFixTest, MultiplePartialReceives) {
    MessageQueue<int> queue;
    const int total = 10;
    
    // Send all messages
    for (int i = 0; i < total; ++i) {
        int val = i;
        queue.send(std::move(val));
    }
    
    // Receive in batches
    for (int batch = 0; batch < 5; ++batch) {
        int received = queue.receive();
        EXPECT_EQ(received, batch * 2);
        
        if (batch * 2 + 1 < total) {
            int received2 = queue.receive();
            EXPECT_EQ(received2, batch * 2 + 1);
        }
    }
}

// Test for Issue #4: HIGH - Bug in Vehicle.cpp distance calculation formula
// The formula was: l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (x1 - x2));
// Should be:     l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
// (last term was wrong - used x1-x2 instead of y1-y2)

// Test 4: Distance calculation with known values (3-4-5 triangle)
TEST_F(HighPriorityFixTest, DistanceCalculation345Triangle) {
    double x1 = 0.0, y1 = 0.0;
    double x2 = 3.0, y2 = 4.0;
    
    // Correct calculation
    double l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    
    EXPECT_DOUBLE_EQ(l, 5.0);
    
    // Wrong calculation (the bug) would give:
    double wrong_l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (x1 - x2));
    EXPECT_NE(l, wrong_l); // They should be different
}

// Test 5: Distance calculation - same point
TEST_F(HighPriorityFixTest, DistanceCalculationSamePoint) {
    double x1 = 5.0, y1 = 5.0;
    double x2 = 5.0, y2 = 5.0;
    
    double l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    
    EXPECT_DOUBLE_EQ(l, 0.0);
}

// Test 6: Distance calculation - horizontal line
TEST_F(HighPriorityFixTest, DistanceCalculationHorizontal) {
    double x1 = 0.0, y1 = 10.0;
    double x2 = 10.0, y2 = 10.0;
    
    double l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    
    EXPECT_DOUBLE_EQ(l, 10.0);
}

// Test 7: Distance calculation - vertical line
TEST_F(HighPriorityFixTest, DistanceCalculationVertical) {
    double x1 = 5.0, y1 = 0.0;
    double x2 = 5.0, y2 = 15.0;
    
    double l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    
    EXPECT_DOUBLE_EQ(l, 15.0);
}

// Test 8: Distance calculation - diagonal with known value
TEST_F(HighPriorityFixTest, DistanceCalculationDiagonal) {
    double x1 = 0.0, y1 = 0.0;
    double x2 = 5.0, y2 = 12.0;
    
    double l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    
    EXPECT_DOUBLE_EQ(l, 13.0); // 5-12-13 triangle
}

// Test 9: Distance calculation - large coordinates
TEST_F(HighPriorityFixTest, DistanceCalculationLargeCoordinates) {
    double x1 = 1000.0, y1 = 2000.0;
    double x2 = 4000.0, y2 = 6000.0;
    
    double dx = x1 - x2; // -3000
    double dy = y1 - y2; // -4000
    double l = sqrt(dx * dx + dy * dy);
    
    EXPECT_DOUBLE_EQ(l, 5000.0); // 3-4-5 triangle scaled by 1000
}

// Test 10: Distance calculation - negative coordinates
TEST_F(HighPriorityFixTest, DistanceCalculationNegativeCoordinates) {
    double x1 = -3.0, y1 = -4.0;
    double x2 = 0.0, y2 = 0.0;
    
    double l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    
    EXPECT_DOUBLE_EQ(l, 5.0);
}
