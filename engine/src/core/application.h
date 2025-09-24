#pragma once
namespace Core {
	class Application
	{
	public:
		Application();
		virtual ~Application() = default;
		virtual int Init(void);
	private:

	};
}
