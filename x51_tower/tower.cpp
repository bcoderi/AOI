
#include "tower.h"
#include <utility>
#include <algorithm>
#include "common/util/tower_util.h"
#include "common/tower_common.h"

CTowerAOIManager::CTowerAOIManager()
	: m_scene_mgr(NULL)
{
}

CTowerAOIManager::~CTowerAOIManager()
{
	
}

bool CTowerAOIManager::Init(ISceneManager* scene, const CTowerMapInfo& map_info)
{
	m_scene_mgr = scene;
	m_tower_map_info = map_info;

	for (int y = 0; y < map_info.m_grid_count_y; ++y)
	{
		for (int x = 0; x < map_info.m_grid_count_x; ++x)
		{
			int tower_id = y * map_info.m_grid_count_y + x;
			CTower tower(tower_id, (float)(map_info.m_grid_width * 0.5 + map_info.m_map_min_x + x * map_info.m_grid_width)
				, (float)(map_info.m_grid_height * 0.5 + map_info.m_map_min_y + y * map_info.m_grid_height));
			m_towers.insert(std::make_pair(tower_id, tower));
		}
	}
}

void CTowerAOIManager::EntityEnter(int64 entity_id)
{
	if (!m_scene_mgr)
		return;

	IEntity* entity = m_scene_mgr->FindEntity(entity_id);
	if (!entity)
		return;

	//获取灯塔ID
	int tower_id = GetTowerID(entity->m_x, entity->m_y);
	//获取实体可见范围内的灯塔列表
	std::vector<int> tower_ids;
	GetTowerIDs(entity->m_x, entity->m_y, tower_ids);
	std::vector<int>::const_iterator itr_tower_id = tower_ids.begin();
	for (; itr_tower_id != tower_ids.end(); ++itr_tower_id)
	{
		std::map<int, CTower>::iterator itr_tower = m_towers.find(*itr_tower_id);
		if (itr_tower == m_towers.end())
			continue;

		std::set<int>& watchers = itr_tower->second.m_watchers;
		std::set<int>::iterator itr_watch = watchers.begin();
		for (; itr_watch != watchers.end(); ++itr_watch)
		{
			if (*itr_watch == entity->m_id)
				continue;

			std::set<int>::const_iterator itr_entity_find = entity->m_entitys.find(entity->m_id);
			if (itr_entity_find == entity->m_entitys.end())
			{
				entity->m_entitys.insert(entity->m_id);
				IEntity* watch_entity = m_scene_mgr->FindEntity(*itr_watch);
				if (watch_entity)
				{
					watch_entity->BroadcastEvent();
					watch_entity->m_entitys.insert(entity->m_id);
				}
			}
		}
		itr_tower->second.m_watchers.insert(entity->m_id);
	}
}

void CTowerAOIManager::EntityMove(int64 entity_id, int tx, int ty)
{
	if (!m_scene_mgr)
		return;

	IEntity* entity = m_scene_mgr->FindEntity(entity_id);
	if (!entity)
		return;

	std::vector<int> f_tower_ids;
	GetTowerIDs(entity->m_x, entity->m_y, f_tower_ids);
	std::vector<int> t_tower_ids;
	GetTowerIDs(tx, ty, t_tower_ids);

	int f_tower_id = GetTowerID(entity->m_x, entity->m_y);
	int t_tower_id = GetTowerID(tx, ty);

	entity->m_x = tx;
	entity->m_y = ty;

	//玩家视野范围内灯塔没变，仅做移动操作
	if (IntEqual(f_tower_ids, t_tower_ids))
	{
		std::set<int>& entity_ids = entity->m_entitys;
		std::set<int>::const_iterator itr_entity_id = entity_ids.begin();
		for (; itr_entity_id != entity_ids.end(); ++itr_entity_id)
		{
			IEntity* notify_enity = m_scene_mgr->FindEntity(*itr_entity_id);
			if (notify_enity)
			{
				notify_enity->BroadcastEvent();
			}
		}
	}

	std::set<int> f_entity_ids;
	GetTowerWatchers(f_tower_ids, f_entity_ids);
	std::set<int> t_entity_ids;
	GetTowerWatchers(t_tower_ids, t_entity_ids);

	std::vector<int> new_tower_ids;
	IntExept(f_tower_ids, t_tower_ids, new_tower_ids);

	std::vector<int>::const_iterator itr_new_tower_id = new_tower_ids.begin();
	for (; itr_new_tower_id != new_tower_ids.end(); ++itr_new_tower_id)
	{
		std::map<int, CTower>::iterator itr_find = m_towers.find(*itr_new_tower_id);
		if (itr_find != m_towers.end())
		{
			//将玩家从灯塔观察者移除
			itr_find->second.m_watchers.erase(entity->m_id);
		}
	}

	std::set<int> new_entity_ids;
	EntityExcept(f_entity_ids, t_entity_ids, new_entity_ids);
	std::set<int>::const_iterator itr_new_entity_id = new_entity_ids.begin();
	for (; itr_new_entity_id != new_entity_ids.end(); ++itr_new_entity_id)
	{
		//从玩家列表移除
		IEntity* tower_entity = m_scene_mgr->FindEntity(*itr_new_entity_id);
		if (tower_entity)
		{
			tower_entity->m_entitys.erase(entity_id);
			if (entity->m_entitys.find(*itr_new_entity_id) != entity->m_entitys.end())
			{
				entity->m_entitys.erase(*itr_new_entity_id);
				tower_entity->BroadcastEvent();
			}
		}
	}

	std::set<int>::const_iterator itr_entity = entity->m_entitys.begin();
	for (; itr_entity != entity->m_entitys.end(); ++itr_entity)
	{
		IEntity* entity_entity = m_scene_mgr->FindEntity(*itr_entity);
		if (entity_entity)
		{
			entity_entity->BroadcastEvent();
		}
	}

	std::vector<int> new_enter_tower_ids;
	IntExept(t_tower_ids, f_tower_ids, new_enter_tower_ids);
	std::vector<int>::const_iterator itr_new_tower_id_enter = new_enter_tower_ids.begin();
	for (; itr_new_tower_id_enter != new_enter_tower_ids.end(); ++itr_new_tower_id_enter)
	{
		std::map<int, CTower>::iterator itr_find = m_towers.find(*itr_new_tower_id_enter);
		if (itr_find != m_towers.end())
		{
			//将玩家添加到灯塔观察者
			std::set<int>& watchers = itr_find->second.m_watchers;
			watchers.insert(entity->m_id);

			//将灯塔观察者交给玩家维护
			std::set<int>::iterator itr_watch = watchers.begin();
			for (; itr_watch != watchers.end(); ++itr_watch)
			{
				if (*itr_watch == entity->m_id)
					continue;

				std::set<int>::const_iterator itr_entity_find = entity->m_entitys.find(*itr_watch);
				if (itr_entity_find == entity->m_entitys.end())
				{
					entity->m_entitys.insert(*itr_watch);
					IEntity* watch_entity = m_scene_mgr->FindEntity(*itr_watch);
					if (watch_entity)
					{
						watch_entity->BroadcastEvent();
						watch_entity->m_entitys.insert(entity->m_id);
					}
				}
			}
		}
	}
}

void CTowerAOIManager::EntityLeave(int64 entity_id)
{
	if (!m_scene_mgr)
		return;

	IEntity* entity = m_scene_mgr->FindEntity(entity_id);
	if (!entity)
		return;

	//获取灯塔ID
	int tower_id = GetTowerID(entity->m_x, entity->m_y);
	//获取玩家可见范围内的灯塔列表
	std::vector<int> tower_ids;
	GetTowerIDs(entity->m_x, entity->m_y, tower_ids);
	std::vector<int>::const_iterator itr_tower = tower_ids.begin();
	for (; itr_tower != tower_ids.end(); ++itr_tower)
	{
		std::map<int, CTower>::iterator itr_one_tower = m_towers.find(*itr_tower);
		if (itr_one_tower != m_towers.end())
		{
			//将玩家从灯塔观察者移除
			itr_one_tower->second.m_watchers.erase(entity->m_id);
		}
	}

	//从玩家列表移除
	std::set<int>::iterator itr_entity_entity = entity->m_entitys.begin();
	for (; itr_entity_entity != entity->m_entitys.end(); ++itr_entity_entity)
	{
		IEntity* noify_entity = m_scene_mgr->FindEntity(*itr_entity_entity);
		if (noify_entity && noify_entity->m_id != entity->m_id)
		{
			noify_entity->m_entitys.erase(entity->m_id);
			noify_entity->BroadcastEvent();
		}
	}
}


int CTowerAOIManager::GetTowerID(int x, int y)
{
	if (x == m_tower_map_info.m_map_max_x)
	{
		x = x - 1;
	}

	if (y == m_tower_map_info.m_map_max_y)
	{
		y = y - 1;
	}

	return (y - m_tower_map_info.m_map_min_y) / m_tower_map_info.m_grid_height * m_tower_map_info.m_grid_count_x + (x - m_tower_map_info.m_map_min_y) / m_tower_map_info.m_grid_width;
}

void CTowerAOIManager::GetTowerIDs(int x, int y, std::vector<int>& tower_ids)
{
	int tower_id = GetTowerID(x, y);
	tower_ids.push_back(tower_id);

	int id_x = tower_id % m_tower_map_info.m_grid_count_x;
	int id_y = tower_id % m_tower_map_info.m_grid_count_y;

	if (id_x - 1 >= 0)
	{
		if (InTower(tower_id - 1, x, y))
		{
			tower_ids.push_back(tower_id - 1);
		}

		if (id_y - 1 >= 0 && InTower(tower_id -1 - m_tower_map_info.m_grid_count_x, x, y))
		{
			tower_ids.push_back(tower_id - 1 + m_tower_map_info.m_grid_count_x);
		}

		if (id_y + 1 < m_tower_map_info.m_grid_count_y && InTower(tower_id - 1 + m_tower_map_info.m_grid_count_x, x, y))
		{
			tower_ids.push_back(tower_id - 1 + m_tower_map_info.m_grid_count_x);
		}
	}

	if (id_x + 1 < m_tower_map_info.m_grid_count_x)
	{
		if (InTower(tower_id + 1, x, y))
		{
			tower_ids.push_back(tower_id + 1);
		}

		if (id_y - 1 >= 0 && InTower(tower_id + 1 - m_tower_map_info.m_grid_count_x, x , y))
		{
			tower_ids.push_back(tower_id + 1 - m_tower_map_info.m_grid_count_x);
		}

		if (id_y + 1 < m_tower_map_info.m_grid_count_y && InTower(tower_id + 1 + m_tower_map_info.m_grid_count_x, x, y))
		{
			tower_ids.push_back(tower_id + 1 + m_tower_map_info.m_grid_count_x);
		}
	}

	if (id_y - 1 >= 0 && InTower(tower_id - m_tower_map_info.m_grid_count_x, x, y))
	{
		tower_ids.push_back(tower_id - m_tower_map_info.m_grid_count_x);
	}

	if (id_y + 1 < m_tower_map_info.m_grid_count_y && InTower(tower_id + m_tower_map_info.m_grid_count_x, x, y))
	{
		tower_ids.push_back(tower_id + m_tower_map_info.m_grid_count_x);
	}
}

bool CTowerAOIManager::InTower(int id, int x, int y)
{
	std::map<int, CTower>::const_iterator itr = m_towers.find(id);
	if (itr == m_towers.end())
		return false;

	float min_x = std::max((float)(x - m_tower_map_info.m_grid_width), (float)m_tower_map_info.m_map_min_x);
	float max_x = std::max((float)(x + m_tower_map_info.m_grid_width), (float)m_tower_map_info.m_map_max_x);
	float min_y = std::max((float)(y - m_tower_map_info.m_grid_width), (float)m_tower_map_info.m_map_min_y);
	float max_y = std::max((float)(y + m_tower_map_info.m_grid_width), (float)m_tower_map_info.m_map_max_y);

	return min_x <= m_towers[id].m_x
		&& max_x >= m_towers[id].m_x
		&& min_y <= m_towers[id].m_y
		&& max_y >= m_towers[id].m_y;
}

void CTowerAOIManager::GetTowerWatchers(const std::vector<int>& tower_ids, std::set<int>& entity_ids)
{
	entity_ids.clear();
	std::vector<int>::const_iterator itr_tower_id = tower_ids.begin();
	for (; itr_tower_id != tower_ids.end(); ++itr_tower_id)
	{
		std::map<int, CTower>::const_iterator itr_find = m_towers.find(*itr_tower_id);
		if (itr_find != m_towers.end())
		{
			const CTower& tower = itr_find->second;
			entity_ids.insert(tower.m_watchers.begin(), tower.m_watchers.end());
		}
	}
}
