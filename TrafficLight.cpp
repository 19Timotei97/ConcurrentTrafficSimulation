#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <random>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
	// FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
	// to wait for and receive new messages and pull them from the queue using move semantics. 
	// The received object should then be returned by the receive function.

	// peform modifications under the lock
	std::unique_lock<std::mutex> uLock(_mtx);
	_cond_var.wait(uLock, [this] { return !_messages.empty(); });

	T t = std::move(_messages.back());
	_messages.pop_back();

	return t;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
	// FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
	// as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

	// Simulate work
	// std::this_tread::sleep_for(std::chrono::milliseconds(100);

	// under the lock, perform operations on deque member
	std::lock_guard<std::mutex> uLock(_mtx);

	_messages.push_back(std::move(msg));
	_cond_var.notify_one(); // notify client after new mesages is added    
}

/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
	_currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
	// FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
	// runs and repeatedly calls the receive function on the message queue. 
	// Once it receives TrafficLightPhase::green, the method returns.
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(3));

		if (TrafficLightPhase::green == _msgQueue.receive())
		{
			return;
		}
	}
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
	return _currentPhase;
}

void TrafficLight::simulate()
{
	// FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
	threads.push_back(std::move(std::thread(&TrafficLight::cycleThroughPhases, this)));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
	// FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
	// and toggles the current phase of the traffic light between red and green and sends an update method 
	// to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
	// Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(4000, 6000);
	int cycleDuration = dist(gen);

	auto lastSwitchedTime = std::chrono::system_clock::now();

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		auto tmpSeconds = std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now() - lastSwitchedTime);
		int durationSinceSwitched = tmpSeconds.count();

		if (durationSinceSwitched >= cycleDuration) {
			_currentPhase = _currentPhase == red ? green : red;

			auto sentFuture = std::async(std::launch::async,
				&MessageQueue<TrafficLightPhase>::send,
				&_msgQueue,
				std::move(_currentPhase));
			sentFuture.wait();

			lastSwitchedTime = std::chrono::system_clock::now();
			cycleDuration = dist(gen);
		}
	}
}