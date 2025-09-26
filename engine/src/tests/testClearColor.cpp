#include "test_clear_color.h"
#include "platform/opengl/gl_def.h"
#include "imgui/imgui.h"
test::TestClearColor::TestClearColor()
	: m_ClearColor{0.2f, 0.3f, 0.8f, 1.0f}
{
}

test::TestClearColor::~TestClearColor()
{
}

void test::TestClearColor::OnUpdate(float deltaTime)
{
}

void test::TestClearColor::OnRender()
{
}

void test::TestClearColor::OnImGuiRender()
{
	ImGui::ColorEdit4("Clear color", m_ClearColor);
}
