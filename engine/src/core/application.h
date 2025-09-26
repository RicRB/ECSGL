#pragma once
#include "engine.h"
#include "window.h"
namespace Core {
	class Application
	{
	private:
		Engine engine;
		std::shared_ptr<Window> window = nullptr;
	public:
		Application();
		~Application();
		void Start();

	private:

	};
}
