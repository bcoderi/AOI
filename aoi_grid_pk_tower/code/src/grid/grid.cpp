
#include "grid.h"
#include "../common/common.h"
#include "../common/player.h"
#include "../common/util/util.h"

GridAOIManager::GridAOIManager(CPlayerManager* player_mgr)
	: m_player_mgr(player_mgr)
	, m_log_file("grid_test_file.txt")
{
	//生成格子ID
	for (int y = 0; y < s_grid_count_y; ++y)
	{
		for (int x = 0; x < s_grid_count_x; ++x)
		{
			int grid_id = y * s_grid_count_x + x;
			CGrid grid(grid_id, s_map_min_x + x * s_grid_width, s_map_min_x + (x + 1) * s_grid_width, s_map_min_y + y * s_grid_height, s_map_min_y + (y + 1) * s_grid_height);
			m_grids.insert(std::make_pair(grid_id, grid));
		}
	}

 	const std::map<int, CPlayer*>& all_players = m_player_mgr->GetPlayers();
 	for (std::map<int, CPlayer*>::const_iterator itr = all_players.begin(); itr != all_players.end(); ++itr)
 	{
 		if (itr->second)
 		{
 			int grid = GetGird(itr->second->m_x, itr->second->m_y);
 			std::map<int, CGrid>::iterator itr_find = m_grids.find(grid);
 			if (itr_find != m_grids.end())
 			{
 				itr_find->second.m_players.insert(itr->second->m_id);
 			}
 		}
 	}
}

GridAOIManager::~GridAOIManager()
{

}

void GridAOIManager::Enter(int player_id)
{
	if (!m_player_mgr)
		return;

	CPlayer* player = m_player_mgr->GetPlayer(player_id);
	if (!player)
		return;

	int grid = GetGird(player->m_x, player->m_y);
	std::map<int, CGrid>::iterator itr_find = m_grids.find(grid);
	if (itr_find != m_grids.end())
	{
		itr_find->second.m_players.insert(player->m_id);
	}

	std::set<int> player_ids;
	GetPlayers(player, player_ids);
	std::set<int>::const_iterator itr = player_ids.begin();
	for (; itr != player_ids.end(); ++itr)
	{
		CPlayer* notify_player = m_player_mgr->GetPlayer(*itr);
		if (notify_player)
		{
			notify_player->ReceiveEnterMessage(player);
			if (s_log)
			{
				m_log_file << "玩家 id:" << notify_player->m_id << ",坐标:" << notify_player->m_x << "," << notify_player->m_y << ",收到消息："
					<< "玩家 id:" << player->m_id << ",坐标:" << player->m_x << "," << player->m_y << ",进入视野" << std::endl;
			}
		}
	}
}

void GridAOIManager::Move(int player_id, int tx, int ty)
{
	if (!m_player_mgr)
		return;

	CPlayer* player = m_player_mgr->GetPlayer(player_id);
	if (!player)
		return;

	//获取移动前的所属格子
	int f_grid = GetGird(player->m_x, player->m_y);
	//获取移动后的所属格子
	int t_grid = GetGird(tx, ty);

	//获取移动前的格子列表
	std::set<int> f_player_ids;
	GetPlayers(player, f_player_ids);

	player->m_x = tx;
	player->m_y = ty;

	//获取移动后的格子列表
	std::set<int> t_player_ids;
	GetPlayers(player, t_player_ids);

	//前后所属格子不同，删除后添加
	if (f_grid != t_grid)
	{
		std::map<int, CGrid>::iterator itr_find_f = m_grids.find(f_grid);
		if (itr_find_f != m_grids.end())
		{
			itr_find_f->second.m_players.erase(player->m_id);
		}

		std::map<int, CGrid>::iterator itr_find_t = m_grids.find(t_grid);
		if (itr_find_t != m_grids.end())
		{
			itr_find_t->second.m_players.insert(player->m_id);
		}
	}

	/*
		此处获取差集交集可以优化，此处是对玩家列表处理，可以调整成对格子列表处理
		差集，发送离开视野消息
	*/
	std::set<int> except_player_ids;
	PlayerExcept(f_player_ids, t_player_ids, except_player_ids);
	std::set<int>::const_iterator itr_except_player_id = except_player_ids.begin();
	for (; itr_except_player_id != except_player_ids.end(); ++itr_except_player_id)
	{
		CPlayer* notify_player = m_player_mgr->GetPlayer(*itr_except_player_id);
		if (notify_player)
		{
			notify_player->ReceiveLeaveMessage(player);
			if (s_log)
			{
				m_log_file << "玩家 id:" << notify_player->m_id << ",坐标:" << notify_player->m_x << "," << notify_player->m_y << ",收到消息："
					<< "玩家 id:" << player->m_id << ",坐标:" << player->m_x << "," << player->m_y << ",离开视野" << std::endl;
			}
		}
	}

	//交集，发送移动消息
	std::set<int> intersect_player_ids;
	PlayerIntersect(f_player_ids, t_player_ids, intersect_player_ids);
	std::set<int>::const_iterator itr_intersect_player_id = intersect_player_ids.begin();
	for (; itr_intersect_player_id != intersect_player_ids.end(); ++itr_intersect_player_id)
	{
		CPlayer* notify_player = m_player_mgr->GetPlayer(*itr_intersect_player_id);
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

	//差集，发送进入游戏消息
	std::set<int> except_player_ids2;
	PlayerExcept(t_player_ids, f_player_ids, except_player_ids2);
	std::set<int>::const_iterator itr_except_player_id2 = except_player_ids2.begin();
	for (; itr_except_player_id2 != except_player_ids2.end(); ++itr_except_player_id2)
	{
		CPlayer* notify_player = m_player_mgr->GetPlayer(*itr_except_player_id2);
		if (notify_player)
		{
			notify_player->ReceiveEnterMessage(player);
			if (s_log)
			{
				m_log_file << "玩家 id:" << notify_player->m_id << ",坐标:" << notify_player->m_x << "," << notify_player->m_y << ",收到消息："
					<< "玩家 id:" << player->m_id << ",坐标:" << player->m_x << "," << player->m_y << ",进入视野" << std::endl;
			}
		}
	}
}

void GridAOIManager::Leave(int player_id)
{
	if (!m_player_mgr)
		return;

	CPlayer* player = m_player_mgr->GetPlayer(player_id);
	if (!player)
		return;

	int grid = GetGird(player->m_x, player->m_y);
	std::map<int, CGrid>::iterator itr_find = m_grids.find(grid);
	if (itr_find != m_grids.end())
	{
		itr_find->second.m_players.erase(player->m_id);
	}

	std::set<int> player_ids;
	GetPlayers(player, player_ids);
	std::set<int>::const_iterator itr = player_ids.begin();
	for (; itr != player_ids.end(); ++itr)
	{
		CPlayer* notify_player = m_player_mgr->GetPlayer(*itr);
		if (notify_player)
		{
			notify_player->ReceiveLeaveMessage(player);
			if (s_log)
			{
				m_log_file << "玩家 id:" << notify_player->m_id << ",坐标:" << notify_player->m_x << "," << notify_player->m_y << ",收到消息："
					<< "玩家 id:" << player->m_id << ",坐标:" << player->m_x << "," << player->m_y << ",离开视野" << std::endl;
			}
		}
	}
}

std::ofstream& GridAOIManager::GetLogFile()
{
	return m_log_file;
}

int GridAOIManager::GetGird(int x, int y)
{
	//如果坐标处于地图边界 特殊处理,可以定义玩家可移动区域，在这种情况下，玩家就无法到达边界
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

void GridAOIManager::GetPlayers(CPlayer* player, std::set<int>& player_ids)
{
	if (!player)
		return;

	player_ids.clear();
	
	int grid = GetGird(player->m_x, player->m_y);
	std::vector<CGrid> grids;
	std::map<int, CGrid>::iterator itr_find = m_grids.find(grid);
	if (itr_find != m_grids.end())
	{
		grids.push_back(itr_find->second);
	}

	//获取x方向格子索引
	int idx = grid % s_grid_count_x;
	//获取y方向格子索引
	int idy = grid / s_grid_count_x;

	/*
		左边界判断
		如果格子左边有格子，判断该格子上方和下方是否有格子
	*/
	if (idx - 1 >= 0)
	{
		if (IsGridExist(grid - 1))
		{
			grids.push_back(m_grids[grid - 1]);
		}

		if (idy - 1 >= 0)
		{
			if (IsGridExist(grid - 1 - s_grid_count_x))
			{
				grids.push_back(m_grids[grid - 1 - s_grid_count_x]);
			}
		}

		if (idy + 1 < s_grid_count_y)
		{
			if (IsGridExist(grid - 1 + s_grid_count_x))
			{
				grids.push_back(m_grids[grid - 1 + s_grid_count_x]);
			}
		}
	}

	/*
		右边界判断
		如果格子右边有格子，判断该格子上方和下方是否有格子
	*/

	if (idx + 1 < s_grid_count_x)
	{
		if (IsGridExist(grid + 1))
		{
			grids.push_back(m_grids[grid + 1]);

		}

		if (idy - 1 >= 0)
		{
			if (IsGridExist(grid + 1 - s_grid_count_x))
			{
				grids.push_back(m_grids[grid + 1 - s_grid_count_x]);
			}
		}

		if (idy + 1 < s_grid_count_y)
		{
			if (IsGridExist(grid + 1 + s_grid_count_x))
			{
				grids.push_back(m_grids[grid + 1 + s_grid_count_x]);
			}
		}
	}

	//下边界判断
	if (idy - 1 >= 0)
	{
		if (IsGridExist(grid - s_grid_count_x))
		{
			grids.push_back(m_grids[grid - s_grid_count_x]);
		}
	}

	//上边界判断
	if (idy + 1 < s_grid_count_y)
	{
		if (IsGridExist(grid + s_grid_count_x))
		{
			grids.push_back(m_grids[grid + s_grid_count_x]);
		}
	}

	//获取格子内的所有玩家
	std::vector<CGrid>::const_iterator itr = grids.begin();
	for (; itr != grids.end(); ++itr)
	{
		const CGrid& real_grid = *itr;
		std::set<int>::const_iterator itr_grid_player = real_grid.m_players.begin();
		for (; itr_grid_player != real_grid.m_players.end(); ++itr_grid_player)
		{
			if (*itr_grid_player != player->m_id)
			{
				player_ids.insert(*itr_grid_player);
			}
		}
	}
}

bool GridAOIManager::IsGridExist(int id)
{
	std::map<int, CGrid>::const_iterator itr = m_grids.find(id);
	if (itr != m_grids.end())
	{
		return true;
	}
	return false;
}
