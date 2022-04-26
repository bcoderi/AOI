#pragma once

#include "common/tower_common.h"

class ICenterNotifier;
class CTowerMapInfo;

class ISceneManager;

class ITower
{
public:
	/*
		��ʼ������AOI
		scene�����賡��������
		map_info����ͼ�����Ϣ
	*/
	virtual bool Init(ISceneManager *scene, const CTowerMapInfo& map_info) = 0;

	/*
		ʵ����볡��
		entity_id��ʵ���ΨһID
	*/
	virtual void EntityEnter(int64 entity_id) = 0;

	/*
		ʵ���ڳ������ƶ�
		entity_id��ʵ���ΨһID
		tx��Ŀ�ĵص�X����
		ty��Ŀ�ĵص�Y����
	*/
	virtual void EntityMove(int64 entity_id, int tx, int ty) = 0;

	/*
		ʵ���뿪����
		entity_id��ʵ���ΨһID
	*/
	virtual void EntityLeave(int64 entity_id) = 0;
public:
	virtual ~ITower() {}
};
