
#include "tower.h"
#include <utility>
#include <algorithm>
#include "../common/player.h"
#include "../common/util/util.h"
#include "../common/common.h"

CTowerAOIManager::CTowerAOIManager(CPlayerManager* player_mgr)
	: m_player_mgr(player_mgr)
	, m_log_file("tower_test_file.txt")
{
	if (!m_player_mgr)
		return;

	for (int y = 0; y < s_grid_count_y; ++y)
	{
		for (int x = 0; x < s_grid_count_x; ++x)
		{
			int tower_id = y * s_grid_count_y + x;
			CTower tower(tower_id, (float)(s_grid_width * 0.5 + s_map_min_x + x * s_grid_width), (float)(s_grid_height * 0.5 + s_map_min_y + y * s_grid_height));
			m_towers.insert(std::make_pair(tower_id, tower));
		}
	}
}

CTowerAOIManager::~CTowerAOIManager()
{
	m_log_file.close();
}

void CTowerAOIManager::Enter(int player_id)
{
	if (!m_player_mgr)
		return;

	CPlayer* player = m_player_mgr->GetPlayer(player_id);
	if (!player)
		return;

	//获取灯塔ID
	int tower_id = GetTowerID(player->m_x, player->m_y);
	//获取玩家可见范围内的灯塔列表
	std::vector<int> tower_ids;
	GetTowerIDs(player->m_x, player->m_y, tower_ids);
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
			if (*itr_watch == player->m_id)
				continue;

			std::set<int>::const_iterator itr_player_find = player->m_players.find(player->m_id);
			if (itr_player_find == player->m_players.end())
			{
				player->m_players.insert(player->m_id);
				CPlayer* watch_player = m_player_mgr->GetPlayer(*itr_watch);
				if (watch_player)
				{
					watch_player->ReceiveEnterMessage(player); 
					if (s_log)
					{
						m_log_file << "玩家 id:" << watch_player->m_id << ",坐标:" << watch_player->m_x << "," << watch_player->m_y << ",收到消息："
							<< "玩家 id:" << player->m_id << ",坐标:" << player->m_x << "," << player->m_y << ",进入视野" << std::endl;
					}
					watch_player->m_players.insert(player->m_id);
				}
			}
		}
		itr_tower->second.m_watchers.insert(player->m_id);
	}
}

void CTowerAOIManager::Move(int player_id, int tx, int ty)
{
	if (!m_player_mgr)
		return;

	CPlayer* player = m_player_mgr->GetPlayer(player_id);
	if (!player)
		return;

	std::vector<int> f_tower_ids;
	GetTowerIDs(player->m_x, player->m_y, f_tower_ids);
	std::vector<int> t_tower_ids;
	GetTowerIDs(tx, ty, t_tower_ids);

	int f_tower_id = GetTowerID(player->m_x, player->m_y);
	int t_tower_id = GetTowerID(tx, ty);

	player->m_x = tx;
	player->m_y = ty;

	//玩家视野范围内灯塔没变，仅做移动操作
	if (IntEqual(f_tower_ids, t_tower_ids))
	{
		std::set<int>& player_ids = player->m_players;
		std::set<int>::const_iterator itr_player_id = player_ids.begin();
		for (; itr_player_id != player_ids.end(); ++itr_player_id)
		{
			CPlayer* notify_player = m_player_mgr->GetPlayer(*itr_player_id);
			if (notify_player)
			{
				notify_player->ReceiveMoveMessage(player);
				if (s_log)
				{
					m_log_file << "玩家 id:" << notify_player->m_id << ",坐标:" << notify_player->m_x << "," << notify_player->m_y << ",收到消息："
						<< "玩家 id:" << player->m_id << ",移动到坐标:" << player->m_x << "," << player->m_y << std::endl;
				}
			}
		}
	}

	std::set<int> f_player_ids;
	GetTowerWatchers(f_tower_ids, f_player_ids);
	std::set<int> t_player_ids;
	GetTowerWatchers(t_tower_ids, t_player_ids);

	std::vector<int> new_tower_ids;
	IntExept(f_tower_ids, t_tower_ids, new_tower_ids);

	std::vector<int>::const_iterator itr_new_tower_id = new_tower_ids.begin();
	for (; itr_new_tower_id != new_tower_ids.end(); ++itr_new_tower_id)
	{
		std::map<int, CTower>::iterator itr_find = m_towers.find(*itr_new_tower_id);
		if (itr_find != m_towers.end())
		{
			//将玩家从灯塔观察者移除
			itr_find->second.m_watchers.erase(player->m_id);
		}
	}

	std::set<int> new_player_ids;
	PlayerExcept(f_player_ids, t_player_ids, new_player_ids);
	std::set<int>::const_iterator itr_new_player_id = new_player_ids.begin();
	for (; itr_new_player_id != new_player_ids.end(); ++itr_new_player_id)
	{
		//从玩家列表移除
		CPlayer* tower_player = m_player_mgr->GetPlayer(*itr_new_player_id);
		if (tower_player)
		{
			tower_player->m_players.erase(player_id);
			if (player->m_players.find(*itr_new_player_id) != player->m_players.end())
			{
				player->m_players.erase(*itr_new_player_id);
				tower_player->ReceiveLeaveMessage(player);
				if (s_log)
				{
					m_log_file << "玩家 id:" << tower_player->m_id << ",坐标:" << tower_player->m_x << "," << tower_player->m_y << ",收到消息："
						<< "玩家 id:" << player->m_id << ",坐标:" << player->m_x << "," << player->m_y << ",离开视野" << std::endl;
				}
			}
		}
	}

	std::set<int>::const_iterator itr_player = player->m_players.begin();
	for (; itr_player != player->m_players.end(); ++itr_player)
	{
		CPlayer* player_player = m_player_mgr->GetPlayer(*itr_player);
		if (player_player)
		{
			player_player->ReceiveMoveMessage(player);
			if (s_log)
			{
				m_log_file << "玩家 id:" << player_player->m_id << ",坐标:" << player_player->m_x << "," << player_player->m_y << ",收到消息："
					<< "玩家 id:" << player->m_id << ",移动到坐标:" << player->m_x << "," << player->m_y << std::endl;
			}
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
			watchers.insert(player->m_id);

			//将灯塔观察者交给玩家维护
			std::set<int>::iterator itr_watch = watchers.begin();
			for (; itr_watch != watchers.end(); ++itr_watch)
			{
				if (*itr_watch == player->m_id)
					continue;

				std::set<int>::const_iterator itr_player_find = player->m_players.find(*itr_watch);
				if (itr_player_find == player->m_players.end())
				{
					player->m_players.insert(*itr_watch);
					CPlayer* watch_player = m_player_mgr->GetPlayer(*itr_watch);
					if (watch_player)
					{
						watch_player->ReceiveEnterMessage(player);
						if (s_log)
						{
							m_log_file << "玩家 id:" << watch_player->m_id << ",坐标:" << watch_player->m_x << "," << watch_player->m_y << ",收到消息："
								<< "玩家 id:" << player->m_id << ",坐标:" << player->m_x << "," << player->m_y << ",进入视野" << std::endl;
						}
						watch_player->m_players.insert(player->m_id);
					}
				}
			}
		}
	}
}

void CTowerAOIManager::Leave(int player_id)
{
	if (!m_player_mgr)
		return;

	CPlayer* player = m_player_mgr->GetPlayer(player_id);
	if (!player)
		return;

	//获取灯塔ID
	int tower_id = GetTowerID(player->m_x, player->m_y);
	//获取玩家可见范围内的灯塔列表
	std::vector<int> tower_ids;
	GetTowerIDs(player->m_x, player->m_y, tower_ids);
	std::vector<int>::const_iterator itr_tower = tower_ids.begin();
	for (; itr_tower != tower_ids.end(); ++itr_tower)
	{
		std::map<int, CTower>::iterator itr_one_tower = m_towers.find(*itr_tower);
		if (itr_one_tower != m_towers.end())
		{
			//将玩家从灯塔观察者移除
			itr_one_tower->second.m_watchers.erase(player->m_id);
		}
	}

	//从玩家列表移除
	std::set<int>::iterator itr_player_player = player->m_players.begin();
	for (; itr_player_player != player->m_players.end(); ++itr_player_player)
	{
		CPlayer* noify_player = m_player_mgr->GetPlayer(*itr_player_player);
		if (noify_player && noify_player->m_id != player->m_id)
		{
			noify_player->m_players.erase(player->m_id);
			noify_player->ReceiveLeaveMessage(player);
			if (s_log)
			{
				m_log_file << "玩家 id:" << noify_player->m_id << ",坐标:" << noify_player->m_x << "," << noify_player->m_y << ",收到消息："
					<< "玩家 id:" << player->m_id << ",坐标:" << player->m_x << "," << player->m_y << ",离开视野" << std::endl;
			}
		}
	}
}

std::ofstream& CTowerAOIManager::GetLogFile()
{
	return m_log_file;
}

int CTowerAOIManager::GetTowerID(int x, int y)
{
	if (x == s_map_max_x)
	{
		x = x - 1;
	}

	if (y == s_map_max_y)
	{
		y = y - 1;
	}

	return (y - s_map_min_y) / s_grid_height * s_grid_count_x + (x - s_map_min_y) / s_grid_width;
}

void CTowerAOIManager::GetTowerIDs(int x, int y, std::vector<int>& tower_ids)
{
	int tower_id = GetTowerID(x, y);
	tower_ids.push_back(tower_id);

	int id_x = tower_id % s_grid_count_x;
	int id_y = tower_id % s_grid_count_y;

	if (id_x - 1 >= 0)
	{
		if (InTower(tower_id - 1, x, y))
		{
			tower_ids.push_back(tower_id - 1);
		}

		if (id_y - 1 >= 0 && InTower(tower_id -1 - s_grid_count_x, x, y))
		{
			tower_ids.push_back(tower_id - 1 + s_grid_count_x);
		}

		if (id_y + 1 < s_grid_count_y && InTower(tower_id - 1 + s_grid_count_x, x, y))
		{
			tower_ids.push_back(tower_id - 1 + s_grid_count_x);
		}
	}

	if (id_x + 1 < s_grid_count_x)
	{
		if (InTower(tower_id + 1, x, y))
		{
			tower_ids.push_back(tower_id + 1);
		}

		if (id_y - 1 >= 0 && InTower(tower_id + 1 - s_grid_count_x, x , y))
		{
			tower_ids.push_back(tower_id + 1 - s_grid_count_x);
		}

		if (id_y + 1 < s_grid_count_y && InTower(tower_id + 1 + s_grid_count_x, x, y))
		{
			tower_ids.push_back(tower_id + 1 + s_grid_count_x);
		}
	}

	if (id_y - 1 >= 0 && InTower(tower_id - s_grid_count_x, x, y))
	{
		tower_ids.push_back(tower_id - s_grid_count_x);
	}

	if (id_y + 1 < s_grid_count_y && InTower(tower_id + s_grid_count_x, x, y))
	{
		tower_ids.push_back(tower_id + s_grid_count_x);
	}
}

bool CTowerAOIManager::InTower(int id, int x, int y)
{
	std::map<int, CTower>::const_iterator itr = m_towers.find(id);
	if (itr == m_towers.end())
		return false;

	float min_x = std::max((float)(x - s_visible_area), (float)s_map_min_x);
	float max_x = std::max((float)(x + s_visible_area), (float)s_map_max_x);
	float min_y = std::max((float)(y - s_visible_area), (float)s_map_min_y);
	float max_y = std::max((float)(y + s_visible_area), (float)s_map_max_y);

	return min_x <= m_towers[id].m_x
		&& max_x >= m_towers[id].m_x
		&& min_y <= m_towers[id].m_y
		&& max_y >= m_towers[id].m_y;
}

void CTowerAOIManager::GetTowerWatchers(const std::vector<int>& tower_ids, std::set<int>& player_ids)
{
	player_ids.clear();
	std::vector<int>::const_iterator itr_tower_id = tower_ids.begin();
	for (; itr_tower_id != tower_ids.end(); ++itr_tower_id)
	{
		std::map<int, CTower>::const_iterator itr_find = m_towers.find(*itr_tower_id);
		if (itr_find != m_towers.end())
		{
			const CTower& tower = itr_find->second;
			player_ids.insert(tower.m_watchers.begin(), tower.m_watchers.end());
		}
	}
}
