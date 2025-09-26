#pragma once
#include <memory>
struct Program {
	static std::shared_ptr<Program> Create();
};