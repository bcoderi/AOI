
#include <iostream>
#include "common/player.h"
#include "grid/grid.h"
#include "common/common.h"
#include <windows.h>
#include "tower/tower.h"
#include <time.h>

int main()
{
	CPlayerManager* player_mgr = new CPlayerManager();
	for (int i = 0; i < s_player_num; ++i)
	{
		player_mgr->NewPlayer(i, (i + 1), (i + 1));
	}
	const std::map<int, CPlayer*>& all_players = player_mgr->GetPlayers();

	/*
		九宫格
	*/
	GridAOIManager* grid_mgr = new GridAOIManager(player_mgr);
	
	//进入
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::cout << "test_grid_enter_begin-----------------" << std::endl;
		grid_mgr->GetLogFile() << "test_grid_enter_begin-----------------" << std::endl;
		int grid_enter_time_begin = GetTickCount();
		std::map<int, CPlayer*>::const_iterator itr_all_grid_player = all_players.begin();
		for (; itr_all_grid_player != all_players.end(); ++itr_all_grid_player)
		{
			if (itr_all_grid_player->second)
			{
				grid_mgr->Enter(itr_all_grid_player->second->m_id);
			}
		}
		std::cout << "test_grid_enter_end-----------------" << std::endl;
		std::cout << "test_grid_enter_use_time:" << GetTickCount() - grid_enter_time_begin << std::endl;
		grid_mgr->GetLogFile() << "test_grid_enter_end-----------------" << std::endl;
		grid_mgr->GetLogFile() << "test_grid_enter_use_time:" << GetTickCount() - grid_enter_time_begin << std::endl;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//移动-(1-50)的范围移动
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	{
 		std::srand(time(0));
 		std::cout << "test_grid_move_begin-----------------" << std::endl;
		grid_mgr->GetLogFile() << "test_grid_move_begin-----------------" << std::endl;
 		int grid_move_time_begin = GetTickCount();
 		std::map<int, CPlayer*>::const_iterator itr_all_grid_player = all_players.begin();
 		for (; itr_all_grid_player != all_players.end(); ++itr_all_grid_player)
 		{
 			if (itr_all_grid_player->second)
 			{
 				int x_rand = std::rand() % (50 - 1 + 1) + 1;
 				int y_rand = std::rand() % (50 - 1 + 1) + 1;
 				grid_mgr->Move(itr_all_grid_player->second->m_id, itr_all_grid_player->second->m_x + x_rand, itr_all_grid_player->second->m_y + y_rand);
 			}
 		}
 		std::cout << "test_grid_move_end-----------------" << std::endl;
 		std::cout << "test_grid_move_use_time:" << GetTickCount() - grid_move_time_begin << std::endl;
		grid_mgr->GetLogFile() << "test_grid_move_end-----------------" << std::endl;
		grid_mgr->GetLogFile() << "test_grid_move_use_time:" << GetTickCount() - grid_move_time_begin << std::endl;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//离开
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::cout << "test_grid_leave_begin-----------------" << std::endl;
		grid_mgr->GetLogFile() << "test_grid_leave_begin-----------------" << std::endl;
		int grid_leave_time_begin = GetTickCount();
		std::map<int, CPlayer*>::const_iterator itr_all_grid_player = all_players.begin();
		for (; itr_all_grid_player != all_players.end(); ++itr_all_grid_player)
		{
			if (itr_all_grid_player->second)
			{
				grid_mgr->Leave(itr_all_grid_player->second->m_id);
			}
		}
		std::cout << "test_grid_leave_end-----------------" << std::endl;
		std::cout << "test_grid_leave_use_time:" << GetTickCount() - grid_leave_time_begin << std::endl;
		grid_mgr->GetLogFile() << "test_grid_leave_end-----------------" << std::endl;
		grid_mgr->GetLogFile() << "test_grid_leave_use_time:" << GetTickCount() - grid_leave_time_begin << std::endl;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	for (int i = 0; i < s_player_num; ++i)
	{
		player_mgr->ResetPlayer(i, (i + 1), (i + 1));
	}

	/*
		灯塔
	*/
	CTowerAOIManager* tower_mgr = new CTowerAOIManager(player_mgr);
	//进入
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::cout << "test_tower_enter_begin-----------------" << std::endl;
		tower_mgr->GetLogFile() << "test_tower_enter_begin----------------------------------------------------------------------------" << std::endl;
		int enter_time_begin = GetTickCount();
		std::map<int, CPlayer*>::const_iterator itr_all_tower_player = all_players.begin();
		for (; itr_all_tower_player != all_players.end(); ++itr_all_tower_player)
		{
			if (itr_all_tower_player->second)
			{
				tower_mgr->Enter(itr_all_tower_player->second->m_id);
			}
		}
		std::cout << "test_tower_enter_end-----------------" << std::endl;
		std::cout << "test_tower_enter_use_time:" << GetTickCount() - enter_time_begin << std::endl;
		tower_mgr->GetLogFile() << "test_tower_enter_end-----------------------------------------------------------------------------" << std::endl;
		tower_mgr->GetLogFile() << "test_tower_enter_use_time:" << GetTickCount() - enter_time_begin << std::endl;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//移动-(1-50)的范围移动
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::srand(time(0));
		std::cout << "test_tower_move_begin-----------------" << std::endl;
		tower_mgr->GetLogFile() << "test_tower_move_begin-----------------" << std::endl;
		int tower_move_time_begin = GetTickCount();
		std::map<int, CPlayer*>::const_iterator itr_all_tower_player = all_players.begin();
		for (; itr_all_tower_player != all_players.end(); ++itr_all_tower_player)
		{
			if (itr_all_tower_player->second)
			{
				int x_rand = std::rand() % (50 - 1 + 1) + 1;
				int y_rand = std::rand() % (50 - 1 + 1) + 1;
				tower_mgr->Move(itr_all_tower_player->second->m_id, itr_all_tower_player->second->m_x + x_rand, itr_all_tower_player->second->m_y + y_rand);
			}
		}
		std::cout << "test_tower_move_end-----------------" << std::endl;
		std::cout << "test_tower_move_use_time:" << GetTickCount() - tower_move_time_begin << std::endl;

		tower_mgr->GetLogFile() << "test_tower_move_end-----------------" << std::endl;
		tower_mgr->GetLogFile() << "test_tower_move_use_time:" << GetTickCount() - tower_move_time_begin << std::endl;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//离开
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::cout << "test_tower_leave_begin-----------------" << std::endl;
		tower_mgr->GetLogFile() << "test_tower_leave_begin-----------------" << std::endl;
		int leave_time_begin = GetTickCount();
		std::map<int, CPlayer*>::const_iterator itr_all_tower_player = all_players.begin();
		for (; itr_all_tower_player != all_players.end(); ++itr_all_tower_player)
		{
			if (itr_all_tower_player->second)
			{
				tower_mgr->Leave(itr_all_tower_player->second->m_id);
			}
		}
		std::cout << "test_tower_leave_end-----------------" << std::endl;
		std::cout << "test_tower_leave_use_time:" << GetTickCount() - leave_time_begin << std::endl;
		tower_mgr->GetLogFile() << "test_tower_leave_end-----------------" << std::endl;
		tower_mgr->GetLogFile() << "test_tower_leave_use_time:" << GetTickCount() - leave_time_begin << std::endl;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (player_mgr)
	{
		delete player_mgr;
		player_mgr = NULL;
	}

	if (grid_mgr)
	{
		delete grid_mgr;
		player_mgr = NULL;
	}

	if (tower_mgr)
	{
		delete tower_mgr;
		tower_mgr = NULL;
	}

	getchar();

	return 0;
}
