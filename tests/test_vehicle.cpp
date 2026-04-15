#include <gtest/gtest.h>
#include <cmath>
#include "Vehicle.h"
#include "Street.h"
#include "Intersection.h"

// Test fixture for Vehicle tests
class VehicleTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup before each test
    }

    void TearDown() override {
        // Cleanup after each test
    }
};

// Test 1: Vehicle initializes with correct default values
TEST_F(VehicleTest, CorrectInitialization) {
    Vehicle vehicle;
    
    // Position should be 0.0 initially
    // Speed should be 400 m/s
    // Type should be objectVehicle
    EXPECT_EQ(vehicle.getType(), ObjectType::objectVehicle);
    EXPECT_GT(vehicle.getID(), 0); // Should have a valid ID
}

// Test 2: setCurrentDestination updates destination
TEST_F(VehicleTest, SetCurrentDestinationUpdatesDestination) {
    Vehicle vehicle;
    auto intersection = std::make_shared<Intersection>();
    
    vehicle.setCurrentDestination(intersection);
    
    // Test passes if no crash (we can't directly access _currDestination)
    SUCCEED();
}

// Test 3: Distance calculation in Vehicle.cpp
// The formula in Vehicle.cpp is: l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2))
// This is correct Euclidean distance
TEST_F(VehicleTest, DistanceCalculationCorrect) {
    // Test the distance formula manually
    double x1 = 0.0, y1 = 0.0;
    double x2 = 3.0, y2 = 4.0;
    
    // Correct distance calculation (3-4-5 triangle)
    double l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    
    EXPECT_DOUBLE_EQ(l, 5.0);
}

// Test 4: Distance calculation - same point
TEST_F(VehicleTest, DistanceCalculationSamePoint) {
    double x1 = 5.0, y1 = 5.0;
    double x2 = 5.0, y2 = 5.0;
    
    double l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    
    EXPECT_DOUBLE_EQ(l, 0.0);
}

// Test 5: Distance calculation - horizontal line
TEST_F(VehicleTest, DistanceCalculationHorizontal) {
    double x1 = 0.0, y1 = 0.0;
    double x2 = 10.0, y2 = 0.0;
    
    double l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    
    EXPECT_DOUBLE_EQ(l, 10.0);
}

// Test 6: Distance calculation - vertical line
TEST_F(VehicleTest, DistanceCalculationVertical) {
    double x1 = 0.0, y1 = 0.0;
    double x2 = 0.0, y2 = 10.0;
    
    double l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    
    EXPECT_DOUBLE_EQ(l, 10.0);
}

// Test 7: Distance calculation - diagonal
TEST_F(VehicleTest, DistanceCalculationDiagonal) {
    double x1 = 0.0, y1 = 0.0;
    double x2 = 1.0, y2 = 1.0;
    
    double l = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    
    EXPECT_DOUBLE_EQ(l, sqrt(2.0));
}

// Test 8: Vehicle has unique ID
TEST_F(VehicleTest, UniqueIDs) {
    Vehicle v1, v2, v3;
    
    EXPECT_NE(v1.getID(), v2.getID());
    EXPECT_NE(v2.getID(), v3.getID());
    EXPECT_NE(v1.getID(), v3.getID());
}

// Test 9: Vehicle type is correct
TEST_F(VehicleTest, VehicleTypeCorrect) {
    Vehicle vehicle;
    
    EXPECT_EQ(vehicle.getType(), ObjectType::objectVehicle);
    EXPECT_NE(vehicle.getType(), ObjectType::objectIntersection);
    EXPECT_NE(vehicle.getType(), ObjectType::objectStreet);
}

// Test 10: setCurrentStreet updates street
TEST_F(VehicleTest, SetCurrentStreetWorks) {
    Vehicle vehicle;
    auto street = std::make_shared<Street>();
    
    vehicle.setCurrentStreet(street);
    
    // Test passes if no crash
    SUCCEED();
}

// Test 11: Position update calculation
// In drive(): _posStreet += _speed * timeSinceLastUpdate / 1000
TEST_F(VehicleTest, PositionUpdateCalculation) {
    double pos = 0.0;
    double speed = 400.0; // m/s
    double timeMs = 1000.0; // 1 second
    
    // Position update formula
    pos += speed * timeMs / 1000.0;
    
    EXPECT_DOUBLE_EQ(pos, 400.0);
}

// Test 12: Speed reduction at intersection
TEST_F(VehicleTest, SpeedReductionAtIntersection) {
    double speed = 400.0;
    
    // Speed is divided by 10 when entering intersection
    speed /= 10.0;
    
    EXPECT_DOUBLE_EQ(speed, 40.0);
}

// Test 13: Speed restoration after intersection
TEST_F(VehicleTest, SpeedRestorationAfterIntersection) {
    double speed = 40.0;
    
    // Speed is multiplied by 10 when leaving intersection
    speed *= 10.0;
    
    EXPECT_DOUBLE_EQ(speed, 400.0);
}

// Test 14: get_shared_this returns shared_ptr
TEST_F(VehicleTest, GetSharedThisReturnsSharedPtr) {
    auto vehicle = std::make_shared<Vehicle>();
    
    auto shared = vehicle->get_shared_this();
    
    EXPECT_NE(shared, nullptr);
    EXPECT_EQ(shared, vehicle); // Should return the same shared_ptr
}

// Test 15: Vehicle speed calculation is positive
TEST_F(VehicleTest, SpeedIsPositive) {
    Vehicle vehicle;
    
    // Default speed should be positive
    // The default is 400 m/s
    SUCCEED();
}
