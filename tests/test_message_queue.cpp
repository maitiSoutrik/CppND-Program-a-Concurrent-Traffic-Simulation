#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <vector>
#include "TrafficLight.h"

// Test fixture for MessageQueue tests
class MessageQueueTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup before each test
    }

    void TearDown() override {
        // Cleanup after each test
    }
};

// Test 1: send() adds message to queue
TEST_F(MessageQueueTest, SendAddsMessageToQueue) {
    MessageQueue<int> queue;
    
    // Send a message
    queue.send(42);
    
    // Receive should return the message
    int received = queue.receive();
    EXPECT_EQ(received, 42);
}

// Test 2: receive() returns message in FIFO order
TEST_F(MessageQueueTest, ReceiveReturnsMessagesInFIFOOrder) {
    MessageQueue<int> queue;
    
    // Send multiple messages
    queue.send(1);
    queue.send(2);
    queue.send(3);
    
    // Receive should return in order
    EXPECT_EQ(queue.receive(), 1);
    EXPECT_EQ(queue.receive(), 2);
    EXPECT_EQ(queue.receive(), 3);
}

// Test 3: receive() blocks until message is available
TEST_F(MessageQueueTest, ReceiveBlocksUntilMessageAvailable) {
    MessageQueue<int> queue;
    bool received = false;
    int value = 0;
    
    // Start a thread that will receive
    std::thread receiver([&]() {
        value = queue.receive();
        received = true;
    });
    
    // Give receiver time to start waiting
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // At this point, receiver should be blocked
    EXPECT_FALSE(received);
    
    // Send a message to unblock
    queue.send(100);
    
    // Wait for receiver to finish
    receiver.join();
    
    EXPECT_TRUE(received);
    EXPECT_EQ(value, 100);
}

// Test 4: Thread-safe operation with single producer, single consumer
TEST_F(MessageQueueTest, ThreadSafeSingleProducerConsumer) {
    MessageQueue<int> queue;
    const int num_messages = 100;
    std::vector<int> received_values;
    std::mutex received_mutex;
    
    std::thread producer([&]() {
        for (int i = 0; i < num_messages; ++i) {
            int val = i;
            queue.send(std::move(val));
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    });
    
    std::thread consumer([&]() {
        for (int i = 0; i < num_messages; ++i) {
            int val = queue.receive();
            std::lock_guard<std::mutex> lock(received_mutex);
            received_values.push_back(val);
        }
    });
    
    producer.join();
    consumer.join();
    
    EXPECT_EQ(received_values.size(), num_messages);
    
    // Verify all values were received
    std::sort(received_values.begin(), received_values.end());
    for (int i = 0; i < num_messages; ++i) {
        EXPECT_EQ(received_values[i], i);
    }
}

// Test 5: Thread-safe operation with multiple producers, single consumer
TEST_F(MessageQueueTest, ThreadSafeMultipleProducers) {
    MessageQueue<int> queue;
    const int num_producers = 4;
    const int messages_per_producer = 25;
    std::vector<int> received_values;
    std::mutex received_mutex;
    std::atomic<int> produced_count{0};
    
    std::vector<std::thread> producers;
    for (int p = 0; p < num_producers; ++p) {
        producers.emplace_back([&queue, &produced_count, p]() {
            for (int i = 0; i < messages_per_producer; ++i) {
                int val = p * 1000 + i;
                queue.send(std::move(val));
                produced_count++;
            }
        });
    }
    
    std::thread consumer([&]() {
        const int total_messages = num_producers * messages_per_producer;
        for (int i = 0; i < total_messages; ++i) {
            int val = queue.receive();
            std::lock_guard<std::mutex> lock(received_mutex);
            received_values.push_back(val);
        }
    });
    
    for (auto& t : producers) {
        t.join();
    }
    consumer.join();
    
    EXPECT_EQ(received_values.size(), num_producers * messages_per_producer);
}

// Test 6: Multiple consumers
// NOTE: This test uses a sentinel value approach to prevent deadlock.
// When multiple consumers compete for messages, some may block on receive()
// forever if all messages are consumed by other threads. We send sentinel
// values (-1) after all data messages to unblock any waiting consumers.
TEST_F(MessageQueueTest, ThreadSafeMultipleConsumers) {
    MessageQueue<int> queue;
    const int num_consumers = 3;
    const int total_messages = 30;
    std::atomic<int> received_count{0};
    std::atomic<int> sentinel_count{0};  // Count of -1 sentinel values received
    
    std::thread producer([&]() {
        for (int i = 0; i < total_messages; ++i) {
            int val = i;
            queue.send(std::move(val));
        }
        // Send sentinel values to wake up any blocked consumers
        // Each consumer needs at most one sentinel
        for (int c = 0; c < num_consumers; ++c) {
            queue.send(-1);
        }
    });
    
    std::vector<std::thread> consumers;
    for (int c = 0; c < num_consumers; ++c) {
        consumers.emplace_back([&queue, &received_count, &sentinel_count, total_messages]() {
            while (received_count < total_messages) {
                int val = queue.receive();
                if (val == -1) {
                    // Received sentinel, increment sentinel count
                    sentinel_count++;
                    // If we've received all messages, break
                    // Otherwise, another consumer might still be processing
                    if (received_count >= total_messages) {
                        break;
                    }
                    // Otherwise continue to help process messages
                } else {
                    received_count++;
                }
            }
        });
    }
    
    producer.join();
    for (auto& t : consumers) {
        t.join();
    }
    
    EXPECT_EQ(received_count, total_messages);
}

// Test 7: Test with TrafficLightPhase enum (actual use case)
TEST_F(MessageQueueTest, WorksWithTrafficLightPhase) {
    MessageQueue<TrafficLight::TrafficLightPhase> queue;
    
    // Send both phases
    queue.send(TrafficLight::TrafficLightPhase::red);
    queue.send(TrafficLight::TrafficLightPhase::green);
    queue.send(TrafficLight::TrafficLightPhase::red);
    
    EXPECT_EQ(queue.receive(), TrafficLight::TrafficLightPhase::red);
    EXPECT_EQ(queue.receive(), TrafficLight::TrafficLightPhase::green);
    EXPECT_EQ(queue.receive(), TrafficLight::TrafficLightPhase::red);
}
