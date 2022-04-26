#pragma once

#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>

class CPlayer;
class CPlayerManager;

class CGrid
{
public:
	CGrid()
		: m_id(0)
		, m_min_x(0)
		, m_max_x(0)
		, m_min_y(0)
		, m_max_y(0)
	{
	}

	CGrid(int id, int min_x, int max_x, int min_y, int max_y)
		: m_id(id)
		, m_min_x(min_x)
		, m_max_x(max_x)
		, m_min_y(min_y)
		, m_max_y(max_y)
	{
	}

public:
	int m_id;
	int m_min_x;
	int m_max_x;
	int m_min_y;
	int m_max_y;
	std::set<int> m_players;
};

class GridAOIManager
{
public:
	GridAOIManager(CPlayerManager* player_mgr);
	~GridAOIManager();
public:
	void Enter(int player_id);
	void Move(int player_id, int tx, int ty);
	void Leave(int player_id);
public:
	std::ofstream& GetLogFile();
private:
	/*
		根据坐标获取格子ID
		格子边界上的坐标只会在一个格子内
		计算方式：(y-地图最小Y坐标)/格子高度*X方向格子数量+(x-地图最小X坐标)/格子宽度
	*/
	int GetGird(int x, int y);
	
	/*
		获取九宫格内的所有玩家
		获取顺序为玩家所属格子(8)-7-2-12-9-4-14-3-13
	*/
	void GetPlayers(CPlayer* player, std::set<int>& player_ids);
	bool IsGridExist(int id);
private:
	CPlayerManager* m_player_mgr;
	std::map<int, CGrid> m_grids;

	std::ofstream m_log_file;
};
