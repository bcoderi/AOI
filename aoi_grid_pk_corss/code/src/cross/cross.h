#pragma once

#include <limits>
#include <vector>
#include <memory>
#include <boost/unordered_map.hpp>
#include "../common/khash.h"
#include "../common/nuid.h"
#include "../common/base_types.h"

#define COORD_TYPE_PLAYER   1
#define COORD_TYPE_GUARD_LEFT   2
#define COORD_TYPE_GUARD_RIGHT  3
#define AOI_FLOAT_LOWEST FLT_MIN
#define AOI_INF_POS AOI_FLOAT_LOWEST, AOI_FLOAT_LOWEST, AOI_FLOAT_LOWEST

struct CrossPlayerAoi;
struct CrossSensor;

#define AOI_HASH_MAP boost::unordered_map
typedef AOI_HASH_MAP<Nuid, std::shared_ptr<CrossPlayerAoi>> CrossPlayerMap;
typedef AOI_HASH_MAP<Nuid, CrossPlayerAoi*> CrossPlayerPtrMap;
typedef std::vector<Nuid> CrossPlayerNuids;
typedef std::vector<CrossPlayerAoi*> CrossPlayerPtrList;


struct CrossPos
{
	CrossPos(float _x, float _y, float _z)
		: x(_x)
		, y(_y)
		, z(_z)
	{}

	void Set(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	float x, y, z;
};


struct CrossCoordNode
{
	CrossCoordNode(char coord_type, float coord_value, CrossPlayerAoi *pplayer = NULL, CrossSensor *psensor = NULL)
		: m_type(coord_type)
		, m_value(coord_value)
		, m_pplayer(pplayer)
		, m_psensor(psensor)
		, m_prev(NULL)
		, m_next(NULL)
	{}

	char m_type;
	float m_value;
	CrossCoordNode* m_prev;
	CrossCoordNode* m_next;
	CrossPlayerAoi* m_pplayer;
	CrossSensor* m_psensor;

	void PrintLog();
};


KHASH_MAP_INIT_INT64(SensorHashMap, CrossPlayerAoi*);

struct CrossSensor
{
	CrossSensor(Nuid sensor_id, float radius, CrossPlayerAoi *pplayer);
	inline void AddCandidate(CrossPlayerAoi* other_pplayer);
	inline void RemoveCandidate(CrossPlayerAoi* other_pplayer);
	inline void PrintCandidate();

	Nuid m_sensor_id;
	float m_radius;
	float m_radius_square;
	CrossPlayerAoi *m_pplayer;
	CrossCoordNode m_left_x;
	CrossCoordNode m_right_x;
	CrossCoordNode m_left_z;
	CrossCoordNode m_right_z;
	CrossPlayerPtrList m_aoi_players[2];

	std::shared_ptr<khash_t(SensorHashMap)> m_aoi_player_candidates;
};


struct CrossPlayerAoi
{
	CrossPlayerAoi(Uint64 nuid, float x, float y, float z)
		: m_nuid(nuid), m_pos(x, y, z), m_last_pos(AOI_INF_POS), flags(0),
		m_node_x(COORD_TYPE_PLAYER, AOI_FLOAT_LOWEST, this),
		m_node_z(COORD_TYPE_PLAYER, AOI_FLOAT_LOWEST, this)
	{}

	AOI_CLASS_ADD_FLAG(Removed, 0, flags);
	AOI_CLASS_ADD_FLAG(Dirty, 1, flags);
	AOI_CLASS_ADD_FLAG(New, 2, flags);
	AOI_CLASS_ADD_FLAG(Beacon, 3, flags);

	Nuid m_nuid;
	CrossPos m_pos;
	CrossPos m_last_pos;
	Uint32 flags;
	CrossCoordNode m_node_x;
	CrossCoordNode m_node_z;
	std::vector<CrossSensor> m_sensors;
	std::shared_ptr<boost::unordered_map<Nuid, std::vector<Nuid>>> m_detected_by;
};


struct CrossSensorUpdateInfo
{
	Nuid m_sensor_id;
	CrossPlayerNuids m_enters;
	CrossPlayerNuids m_leaves;
};


struct CrossAoiUpdateInfo
{
	Nuid m_nuid;
	std::vector<CrossSensorUpdateInfo> m_sensor_update_list;
};

typedef AOI_HASH_MAP<Nuid, CrossAoiUpdateInfo> CrossAoiUpdateInfos;

class CrossAoi
{
public:
	CrossAoi(float map_bound_xmin, float map_bound_xmax, float map_bound_zmin, float map_bound_zmax, size_t beacon_x, size_t beacon_z, float beacon_radius);
	void AddPlayer(Nuid nuid, float x, float y, float z);
	void AddPlayerNoBeacon(Nuid nuid, float x, float y, float z);
	void RemovePlayer(Nuid nuid);
	void AddSensor(Nuid nuid, Nuid sensor_id, float radius);
	void AddSensorNoBeacon(Nuid nuid, Nuid sensor_id, float radius);
	void RemoveSensor(Nuid nuid, Nuid sensor_id);
	void UpdatePos(Nuid nuid, float x, float y, float z);
	CrossAoiUpdateInfos Tick();
	const CrossPlayerMap& GetPlayerMap() const
	{
		return m_player_map;
	}

public:
	void PrintNodeList(CrossCoordNode *list);
	void PrintAllNodeList();

private:
	void _RemovePlayer(Nuid nuid);
	void UpdateSensorPos(const CrossPlayerAoi &player, CrossSensor *sensor);
	void MovePlayerNode(CrossCoordNode **list, CrossCoordNode *pnode);
	CrossAoiUpdateInfo UpdatePlayerAoi(Uint32 cur_aoi_map_idx, CrossPlayerAoi* player);
	void CalcAoiPlayers(const CrossPlayerAoi& player, const CrossSensor& sensor, CrossPlayerPtrList* aoi_map);
	void CheckLeave(CrossPlayerAoi* pptr, float radius_square,
		const CrossPlayerPtrList &aoi_players, CrossPlayerNuids *leaves);
	void CheckEnter(CrossPlayerAoi* pptr, float radius_square,
		const CrossPlayerPtrList &aoi_players, CrossPlayerNuids *leaves);

private:
	CrossCoordNode* m_coord_list_x;
	CrossCoordNode* m_coord_list_z;
	CrossPlayerMap m_player_map;
	Uint32 m_cur_aoi_map_idx;
	std::vector<CrossPlayerAoi*> m_beacons;
};
