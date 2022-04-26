#pragma once

#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include "common/tower_common.h"
#include "tower_interface.h"

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

class CTowerAOIManager : public ITower
{
public:
	CTowerAOIManager();
	~CTowerAOIManager();
public:
	virtual bool Init(ISceneManager *scene, const CTowerMapInfo& map_info);
	virtual void EntityEnter(int64 entity_id);
	virtual void EntityMove(int64 entity_id, int tx, int ty);
	virtual void EntityLeave(int64 entity_id);
private:
	int GetTowerID(int x, int y);
	void GetTowerIDs(int x, int y, std::vector<int>& tower_ids);
	bool InTower(int id, int x, int y);
	//根据灯塔坐标获取灯塔上的所有观察者
	void GetTowerWatchers(const std::vector<int>& tower_ids, std::set<int>& entity_ids);
private:
	CTowerMapInfo m_tower_map_info;
	ISceneManager* m_scene_mgr;
	std::map<int, CTower> m_towers;
};
