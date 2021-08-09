#include <iostream>
#include <random>
#include <thread>
#include <future>
#include <algorithm>
#include "TrafficLight.h"

// random device will seed the generator
std::random_device seeder;
// make a Mersenne twister engine
std::mt19937 engine(seeder());
// distribution
std::uniform_int_distribution<int> dist(4, 6);

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lck(_mtx);
    _cond.wait(lck, [this]{return !_queue.empty();});
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // Perform vector modification under lock
    std::lock_guard<std::mutex> lck(_mtx);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
    }


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

double TrafficLight::generateCycleDuration(){
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

    int cycleDuration = generateCycleDuration();
    std::chrono::time_point<std::chrono::system_clock> t1;
    // init stop watch
    t1 = std::chrono::system_clock::now();

    while(true){

        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute the difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>
                                    (std::chrono::system_clock::now() - t1).count();

        if(timeSinceLastUpdate >= cycleDuration){
            TrafficLight::toggleLight();
            t1 = std::chrono::system_clock::now();
            cycleDuration = generateCycleDuration();
            queue.send(std::move(TrafficLight::getCurrentPhase()));
            // futures.emplace_back(std::async(std::launch::async,
            // &MessageQueue<TrafficLightPhase>::send, queue, std::move(_currentPhase)));
            }

            
        // // _mutex.unlock();
        // std::for_each(futures.begin(), futures.end(), [](std::future<void>&ftr){
        //         ftr.wait();
        //     });
        // // _mutex.lock();
        // std::cout<<"Finished!"<<std::endl;
        // // _mutex.unlock();
    }

}

