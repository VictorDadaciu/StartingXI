#include <iostream>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>
#include <string>
#include <vector>
#include <tuple>
#include <span>

#include "SXIMath/Vec.h"

#include "SXIRenderer/Renderer.h"
#include "SXICore/File.h"
#include "SXICore/Timing.h"

#include "SXICore/MPL/TypeList.h"
#include "SXICore/MPL/Contains.h"
#include "SXICore/MPL/Count.h"
#include "SXICore/MPL/IndexOf.h"
#include "SXICore/MPL/Repeat.h"
#include "SXICore/MPL/Tuple.h"
#include "SXICore/MPL/Filter.h"
#include "SXICore/MPL/TypeListOperations.h"
#include "SXICore/MPL/Macros.h"

const std::string MODELS_PATH = "../../MysteriousGame/models/";
const std::string TEXTURES_PATH = "../../MysteriousGame/textures/";
const std::string SHADERS_PATH = "../../MysteriousGame/shaders/";
const std::string SHADERS_GEN_PATH = "../../MysteriousGame/shaders/generated/";

static void loop()
{
	sxi::Time time{};
	SDL_Event e;
	SDL_zero(e);
	bool minimized = false;
	bool running = true;
	while (running)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_EVENT_WINDOW_MINIMIZED)
				minimized = true;

			if (e.type == SDL_EVENT_WINDOW_RESTORED)
				minimized = false;

			if (e.type == SDL_EVENT_QUIT)
				return;

			if (e.type == SDL_EVENT_KEY_DOWN)
			{
				if (e.key.key == SDLK_ESCAPE)
					return;
			}
		}
		// if (!minimized)
		// 	renderer->render(time);

		sxi::renderer::render(time);
		time.refresh();
	}
}

template <typename... Ts> using Signature = sxi::mpl::typelist<Ts...>;
template <typename... Ts> using SignatureList = sxi::mpl::typelist<Ts...>;

template <typename... Ts> using ComponentList = sxi::mpl::typelist<Ts...>;

template <typename... Ts> using Archetype = sxi::mpl::typelist<Ts...>;
template <typename... Ts> using ArchetypeList = sxi::mpl::typelist<Ts...>;

struct PositionComponent
{
	glm::vec3 pos;
};
struct RenderComponent
{
	bool render{ true };
};
using Components = ComponentList<PositionComponent, RenderComponent>;

using Soldier = Archetype<PositionComponent>;
using Tank = Archetype<PositionComponent, RenderComponent>;
using Archetypes = ArchetypeList<Soldier, Tank>;

template <typename TArchetype>
struct Entity final
{
	size_t index;
	bool alive;
};

namespace detail
{
	template <typename TArchetype>
	struct ArchetypeStorage final
	{
		ArchetypeStorage(size_t initialCapacity=1) : capacity(initialCapacity)
		{
			assert(initialCapacity > 0);

			sxi::mpl::forTuple([initialCapacity](auto& c){
				c.resize(initialCapacity);
			}, components);
		}

		template <typename... Ts>
    	using TupleOfVectors = std::tuple<std::vector<Ts>...>;
		sxi::mpl::Rename<TupleOfVectors, TArchetype> components;

		void reserve(size_t newCapacity)
		{
			assert(newCapacity > capacity);
			
			sxi::mpl::forTuple([newCapacity](auto& c){
				c.resize(newCapacity);
			}, components);

			capacity = newCapacity;
		}

		void reserveIfNeeded()
		{
			if (capacity > size)
				return;

			reserve(2 * capacity);
		}

		Entity<TArchetype> createEntity()
		{
			reserveIfNeeded();
			return Entity<TArchetype>{ size++, true };
		}
		
	private:
		size_t size{};
		size_t capacity{};
	};
}

class Manager final
{
	template <typename... Ts>
	using TupleOfArchetypeStorages = std::tuple<detail::ArchetypeStorage<Ts>...>;
	sxi::mpl::Rename<TupleOfArchetypeStorages, Archetypes> archetypes;

	template <typename T>
	static constexpr bool isArchetype() noexcept
	{
		return sxi::mpl::Contains<T, Archetypes>::value;
	}
	template <typename T>
	static constexpr bool isComponent() noexcept
	{
		return sxi::mpl::Contains<T, Components>::value;
	}

public:
	template <typename TArchetype>
	Entity<TArchetype> createEntity()
	{
		static_assert(isArchetype<TArchetype>(), "TArchetype must be an archetype");

		return std::get<detail::ArchetypeStorage<TArchetype>>(archetypes).createEntity();
	}

	template <typename TArchetype>
	auto& components() noexcept
	{
		static_assert(isArchetype<TArchetype>(), "TArchetype must be an archetype");

		return std::get<detail::ArchetypeStorage<TArchetype>>(archetypes).components;
	}

	template <typename TComponent, typename TArchetype>
	auto& component(const Entity<TArchetype>& entity) noexcept
	{
		static_assert(isArchetype<TArchetype>(), "TArchetype must be an archetype");
		static_assert(isComponent<TComponent>(), "TComponent must be a component");

		return std::get<std::vector<TComponent>>(
			std::get<detail::ArchetypeStorage<TArchetype>>(archetypes).components)[entity.index];
	}
};

// struct T0{};
// struct T1{};
// struct T2{};
// using S0 = sxi::ecs::Signature<>;
// using S1 = sxi::ecs::Signature<C0, T0>;
// using S2 = sxi::ecs::Signature<C1, T1>;
// using S3 = sxi::ecs::Signature<C0, T1, T2>;

// using Components = sxi::ecs::ComponentList<C0, C1>;
// using Tags = sxi::ecs::TagList<T0, T1, T2>;
// using Signatures = sxi::ecs::SignatureList<S0, S1, S2, S3>;

// using ECSSettings = sxi::ecs::Settings<Components, Tags, Signatures>;
// using ECSManager = sxi::ecs::Manager<ECSSettings>;

int main(int argc, char* args[])
{
	// sxi::renderer::init(1600, 900);
	// sxi::renderer::addGraphicsPipeline(
	// 	sxi::file::readFileAsBytes(SHADERS_GEN_PATH + "basic_lighting.vert.spv"),
	// 	sxi::file::readFileAsBytes(SHADERS_GEN_PATH + "basic_lighting.frag.spv"));
	// sxi::renderer::addTexture(TEXTURES_PATH + "table_basecolor.png");
	// sxi::renderer::addTexture(TEXTURES_PATH + "chair_basecolor.png");
	// sxi::renderer::addModel(MODELS_PATH + "Coffee_Table.obj");
	// sxi::renderer::addModel(MODELS_PATH + "Rocking_Chair.obj");
	// loop();
	// sxi::renderer::destroy();

	// ECSManager manager(2);
	// sxi::ecs::EntityIndex index = manager.createIndex();
	// manager.addTag<T0>(index);
	// manager.kill(index);
	// manager.refresh();
	// manager.print();

	Manager mgr;

	Entity<Soldier> soldier1 = mgr.createEntity<Soldier>();
	PositionComponent& ps1 = mgr.component<PositionComponent>(soldier1);
	ps1.pos = glm::vec3(1, 2, 3);

	Entity<Soldier> soldier2 = mgr.createEntity<Soldier>();
	PositionComponent& ps2 = mgr.component<PositionComponent>(soldier2);
	ps2.pos = glm::vec3(4, 5, 6);

	Entity<Soldier> soldier3 = mgr.createEntity<Soldier>();
	Entity<Soldier> soldier4 = mgr.createEntity<Soldier>();

	Entity<Tank> tank1 = mgr.createEntity<Tank>();
	RenderComponent& rt1 = mgr.component<RenderComponent>(tank1);
	rt1.render = false;

	Entity<Tank> tank2 = mgr.createEntity<Tank>();

	return 0;
}