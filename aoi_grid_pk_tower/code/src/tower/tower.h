#pragma once

#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>

class CPlayerManager;
class CPlayer;

class CTower
{
public:
	CTower()
		: m_id(0)
		, m_x(0.f)
		, m_y(0.f)
	{
	}

	CTower(int id, float x, float y)
		: m_id(id)
		, m_x(x)
		, m_y(y)
	{
	}

public:
	int m_id;
	float m_x;
	float m_y;
	std::set<int> m_watchers;
	std::set<int> m_markers;
};

class CTowerAOIManager
{
public:
	CTowerAOIManager(CPlayerManager* player_mgr);
	~CTowerAOIManager();
public:
	void Enter(int player_id);
	void Move(int player_id, int tx, int ty);
	void Leave(int player_id);
public:
	std::ofstream& GetLogFile();
private:
	int GetTowerID(int x, int y);
	void GetTowerIDs(int x, int y, std::vector<int>& tower_ids);
	bool InTower(int id, int x, int y);
	//根据灯塔坐标获取灯塔上的所有观察者
	void GetTowerWatchers(const std::vector<int>& tower_ids, std::set<int>& player_ids);
private:
	CPlayerManager* m_player_mgr;
	std::map<int, CTower> m_towers;

	std::ofstream m_log_file;
};
