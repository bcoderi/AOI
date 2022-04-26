#pragma once

#include "common/tower_common.h"

class ICenterNotifier;
class CTowerMapInfo;

class ISceneManager;

class ITower
{
public:
	/*
		初始化灯塔AOI
		scene，炫舞场景管理器
		map_info，地图相关信息
	*/
	virtual bool Init(ISceneManager *scene, const CTowerMapInfo& map_info) = 0;

	/*
		实体加入场景
		entity_id，实体的唯一ID
	*/
	virtual void EntityEnter(int64 entity_id) = 0;

	/*
		实体在场景中移动
		entity_id，实体的唯一ID
		tx，目的地的X坐标
		ty，目的地的Y坐标
	*/
	virtual void EntityMove(int64 entity_id, int tx, int ty) = 0;

	/*
		实体离开场景
		entity_id，实体的唯一ID
	*/
	virtual void EntityLeave(int64 entity_id) = 0;
public:
	virtual ~ITower() {}
};
