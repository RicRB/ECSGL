#pragma once
#include <vector>
#include <functional>
template<typename... Args>
class Event {
	using Listener = std::function<void(Args...)>;
private:
	std::vector<Listener> m_listeners;

public:
	Event() = default;
	Event(const Event&) = default;
	Event(Event&&) = default;
	Event& operator=(const Event&) = default;
	Event& operator=(Event&&) = default;
	~Event() = default;



	void AddListener(Listener function) {
		m_listeners.push_back(function);
	}
	void RemoveListener(Listener function) {
		// removes listener functions that are the same as the ones passed
		auto it = std::remove_if(m_listeners.begin(), m_listeners.end(), [&](const auto& listener) {
			return listener.target_type() == function.target_type() && listener.target<Args...>() == function.target<Args...>();
			});

		m_listeners.erase(it, m_listeners.end());
	}
	void Invoke(Args... args) {
		for (auto& listener : m_listeners) {
			listener(args...);
		}
	}
private:
};