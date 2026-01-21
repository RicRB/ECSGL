#pragma once
#include <memory>

struct Image {
	static std::shared_ptr<Image> Create();
};