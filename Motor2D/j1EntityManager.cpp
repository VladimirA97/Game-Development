#include "j1EntityManager.h"
#include "j1Charger.h"
#include "j1Bat.h"
#include "p2Log.h"
#include "j1Render.h"
#include "j1Map.h"
#include "j1App.h"
#include "j1Collision.h"
#include "Brofiler\Brofiler.h"
#include "j1Input.h"

j1EntityManager::j1EntityManager()
{
	name.create("entityManager");
}

j1EntityManager::~j1EntityManager()
{}

bool j1EntityManager::Awake(pugi::xml_node& config)
{
	this->config = config;
	for (p2List_item<Entity*>* entity = entities.start; entity; entity = entity->next)
	{
		entity->data->Awake(config.child(entity->data->name.GetString()));
	}

	return true;
}

bool j1EntityManager::Start()
{
	path_marker = App->tex->Load("maps/non_walkable_tile.png");

	return true;
}

bool j1EntityManager::Update(float dt)
{
	BROFILER_CATEGORY("EntityManager Update", Profiler::Color::Yellow);
	for (p2List_item<Entity*>* entity = entities.start; entity; entity = entity->next)
	{
		entity->data->Entity_Update(dt);
		entity->data->Update(dt);
	}

	if (App->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN)
		draw_path = !draw_path;

	return true;
}

bool j1EntityManager::PostUpdate(float dt)
{
	BROFILER_CATEGORY("EntityManager PostUpdate", Profiler::Color::Yellow);
	for (p2List_item<Entity*>* entity = entities.start; entity; entity = entity->next)
	{
		entity->data->PostUpdate(dt);
		int i = 0;
		if (draw_path)
		{
			while (i < entity->data->entityPath.Count())
			{
				iPoint coords = App->map->MapToWorld(entity->data->entityPath.At(i)->x, entity->data->entityPath.At(i)->y);
				App->render->Blit(path_marker, coords.x, coords.y);
				i++;
			}
		}
		App->render->Blit(entity->data->graphics, entity->data->position.x, entity->data->position.y, &entity->data->animation->GetCurrentFrame(dt), entity->data->scale);
	}

	return true;
}

bool j1EntityManager::CleanUp()
{
	p2List_item<Entity*>* item;
	item = entities.start;

	while (item != NULL)
	{
		RELEASE(item->data);
		item = item->next;
	}
	entities.clear();
	return true;
}

void j1EntityManager::DeleteEntity(Entity* entity_to_delete)
{
	p2List_item<Entity*>* entity_finder = entities.start; 
	while (entity_finder != NULL)
	{
		if (entity_finder->data == entity_to_delete)
		{
			entities.del(entity_finder);
			RELEASE(entity_finder->data);
			break;
		}
		entity_finder = entity_finder->next;
	}
}

Entity* j1EntityManager::createEntity(entity_type type, int x, int y)
{
	Entity* ret = nullptr;
	
	switch (type)
	{
	case CHARGER:
		ret = new Charger();
		break;
	case BAT:
		ret = new Bat();
		ret->flying = true;
		break;
	}
	ret->type = type;
	ret->virtualPosition.x = ret->position.x = x;
	ret->virtualPosition.y = ret->position.y = y;
	ret->animation = ret->idle_left;

	float time = ret->jump_force / -gravity;
	int max_height = (ret->jump_force * time) + ((gravity / 2) * time * time);
	ret->max_jump_value = (max_height / App->map->data.tile_height) * 2;

	entities.add(ret);

	return ret;
}

bool j1EntityManager::Load(pugi::xml_node& data)
{
	CleanUp();
	for (pugi::xml_node charger = data.child("charger"); charger; charger = charger.next_sibling("charger"))
	{
		createEntity(CHARGER, charger.attribute("position_x").as_int(), charger.attribute("position_y").as_int());
	}

	for (pugi::xml_node bat = data.child("bat"); bat; bat = bat.next_sibling("bat"))
	{
		createEntity(BAT, bat.attribute("position_x").as_int(), bat.attribute("position_y").as_int());
	}
	
	return true;
}

bool j1EntityManager::Save(pugi::xml_node& data) const
{
	for (p2List_item<Entity*>* entity = entities.start; entity; entity = entity->next)
	{
		pugi::xml_node child = data.append_child(entity->data->name.GetString());
		child.append_attribute("position_x") = entity->data->position.x;
		child.append_attribute("position_y") = entity->data->position.y;
	}

	return true;
}