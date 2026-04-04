#include <iostream>
#include <random>
#include <thread>
#include <future>
#include <algorithm>
#include "TrafficLight.h"



/* Implementation of class "MessageQueue" is now in TrafficLight.h as templates */


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight(){}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true){
        TrafficLightPhase trafficLightPhase = TrafficLight::queue.receive();
        if(trafficLightPhase == green) return;

    }
}

void TrafficLight::toggleLight(){
    // toggle between red and green
    if(_currentPhase == red) _currentPhase = green;
    else _currentPhase = red;
}

TrafficLight::TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    std::unique_lock<std::mutex> lck(_mutex);
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

float TrafficLight::generateCycleDuration(){
    // random device will seed the generator
    std::random_device seeder;
    // make a Mersenne twister engine
    std::mt19937 engine(seeder());
    // distribution
    std::uniform_int_distribution<int> dist(4000, 6000);	// use int instead of float
    return dist(engine);
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // std::shared_ptr<MessageQueue<TrafficLightPhase>> queue(new MessageQueue<TrafficLightPhase>);
    // std::vector<std::future<void>> futures;
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
    std::unique_lock<std::mutex> lck(_mutex);

    auto cycleDuration = generateCycleDuration();
    // std::chrono::time_point<std::chrono::system_clock> t1;
    // init stop watch
    auto t1 = std::chrono::system_clock::now();

    while(true){

        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute the difference to stop watch
        auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>
                                    (std::chrono::system_clock::now() - t1).count();

        if(timeSinceLastUpdate >= cycleDuration){
            TrafficLight::toggleLight();
            t1 = std::chrono::system_clock::now();
            cycleDuration = generateCycleDuration();
            queue.send(std::move(TrafficLight::getCurrentPhase()));
            // futures.emplace_back(std::async(std::launch::async,
            // &MessageQueue<TrafficLightPhase>::send, queue, std::move(_currentPhase)));
            }

            
   
    }

}

