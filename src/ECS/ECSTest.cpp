//#include "ECS/ECS.hpp"
//#include "ResourceManager.h"
//struct ComponentA
//{
//	int A;
//};
//struct ComponentB
//{
//	int B;
//};
//int main(void)
//{
//	auto size = sizeof(ComponentB);
//	Registry& registry = Registry::GetInstance();
//	Entity entity1 = registry.CreateEntity();
//	Entity entity2 = registry.CreateEntity();
//	registry.AssignComponent<ComponentA>(entity1, ComponentA{1});
//	registry.AssignComponent<ComponentB>(entity1, ComponentB{2});
//	registry.AssignComponent<ComponentB>(entity2, ComponentB{3});
//	View view = registry.GetView<ComponentA, ComponentB>();
//	auto e = view[0];
//	return 0;
//}