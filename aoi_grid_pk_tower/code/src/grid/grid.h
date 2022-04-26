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
		���������ȡ����ID
		���ӱ߽��ϵ�����ֻ����һ��������
		���㷽ʽ��(y-��ͼ��СY����)/���Ӹ߶�*X�����������+(x-��ͼ��СX����)/���ӿ��
	*/
	int GetGird(int x, int y);
	
	/*
		��ȡ�Ź����ڵ��������
		��ȡ˳��Ϊ�����������(8)-7-2-12-9-4-14-3-13
	*/
	void GetPlayers(CPlayer* player, std::set<int>& player_ids);
	bool IsGridExist(int id);
private:
	CPlayerManager* m_player_mgr;
	std::map<int, CGrid> m_grids;

	std::ofstream m_log_file;
};
