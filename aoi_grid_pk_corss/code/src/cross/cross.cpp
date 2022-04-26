
#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <utility>
#include <boost/range/irange.hpp>

#include "cross.h"

#define MOVE_DIRECTION_LEFT 0
#define MOVE_DIRECTION_RIGHT 1

#define IfInXZRadiusSquare(dx, dz, x0, z0, x1, z1, radius_square) \
  dx = x0 - x1; \
  dz = z0 - z1; \
  if (dx * dx + dz * dz <= radius_square) \

#define IfNotInXZRadiusSquare(dx, dz, x0, z0, x1, z1, radius_square) \
  dx = x0 - x1; \
  dz = z0 - z1; \
  if (dx * dx + dz * dz > radius_square) \

//--------------------------------------------------------------------------------------------------
class KHashDeleter 
{
public:
	void operator()(khash_t(SensorHashMap) *ptr) {
		kh_destroy(SensorHashMap, ptr);
	}
};

CrossSensor::CrossSensor(Nuid _sensor_id, float _radius, CrossPlayerAoi *pplayer)
	: m_sensor_id(_sensor_id), m_radius(_radius),
	m_radius_square(_radius * _radius), m_pplayer(pplayer),
	m_left_x(COORD_TYPE_GUARD_LEFT, pplayer->m_pos.x - _radius, pplayer, this),
	m_right_x(COORD_TYPE_GUARD_RIGHT, pplayer->m_pos.x + _radius, pplayer, this),
	m_left_z(COORD_TYPE_GUARD_LEFT, pplayer->m_pos.z - m_radius, pplayer, this),
	m_right_z(COORD_TYPE_GUARD_RIGHT, pplayer->m_pos.z + m_radius, pplayer, this),
	m_aoi_player_candidates(kh_init(SensorHashMap), KHashDeleter()) {
	kh_resize(SensorHashMap, m_aoi_player_candidates.get(), 100);
}

inline void CrossSensor::AddCandidate(CrossPlayerAoi* other_pplayer)
{
	if (m_pplayer->m_nuid == other_pplayer->m_nuid) return;

	auto candidates = m_aoi_player_candidates.get();
	if (kh_get(SensorHashMap, candidates, other_pplayer->m_nuid) == kh_end(candidates))
	{
		int ret;
		auto k = kh_put(SensorHashMap, candidates, other_pplayer->m_nuid, &ret);
		assert(ret != -1);
		kh_value(candidates, k) = other_pplayer;

		if (other_pplayer->m_detected_by)
		{
			(*other_pplayer->m_detected_by)[m_pplayer->m_nuid].push_back(m_sensor_id);
		}
	}
}

inline void CrossSensor::RemoveCandidate(CrossPlayerAoi* other_pplayer)
{
	auto candidates = m_aoi_player_candidates.get();
	auto k = kh_get(SensorHashMap, candidates, other_pplayer->m_nuid);
	if (k != kh_end(candidates))
	{
		kh_del(SensorHashMap, candidates, k);

		if (other_pplayer->m_detected_by)
		{
			auto &sensor_ids = (*other_pplayer->m_detected_by)[m_pplayer->m_nuid];
			sensor_ids.erase(std::find(sensor_ids.begin(), sensor_ids.end(), m_sensor_id));
		}
	}
}

inline void CrossSensor::PrintCandidate() {
	printf("Player %lu Sensor %lu candidates: ", m_pplayer->m_nuid, m_sensor_id);
	CrossPlayerAoi *val;
	kh_foreach_value(m_aoi_player_candidates.get(), val,
		printf("%lu ", val->m_nuid);
	)
		printf("\n");
}
//--------------------------------------------------------------------------------------------------
void CrossCoordNode::PrintLog()
{
	printf("(");
	printf("%p, ", this);
	switch (m_type)
	{
	case COORD_TYPE_PLAYER:
		printf("player");
		break;
	case COORD_TYPE_GUARD_LEFT:
		printf("left");
		break;
	case COORD_TYPE_GUARD_RIGHT:
		printf("right");
		break;
	default:
		break;
	}
	printf(" %lu, %f", m_pplayer->m_nuid, m_value);
	printf("), ");
}

//--------------------------------------------------------------------------------------------------
inline void ListInsertBefore(CrossCoordNode **list, CrossCoordNode *pos, CrossCoordNode *ptr)
{
	if (!(*list)) 
	{
		ptr->m_next = nullptr;
		ptr->m_prev = nullptr;
		*list = ptr;
	}
	else 
	{
		if (pos->m_prev) 
		{
			pos->m_prev->m_next = ptr;
		}
		ptr->m_prev = pos->m_prev;
		pos->m_prev = ptr;
		ptr->m_next = pos;
		if (*list == pos) *list = ptr;
	}
}

//--------------------------------------------------------------------------------------------------
inline void ListInsertAfter(CrossCoordNode **list, CrossCoordNode *pos, CrossCoordNode *ptr)
{
	if (!(*list)) 
	{
		ptr->m_next = nullptr;
		ptr->m_prev = nullptr;
		*list = ptr;
	}
	else
	{
		if (pos->m_next)
		{
			pos->m_next->m_prev = ptr;
		}
		ptr->m_next = pos->m_next;
		pos->m_next = ptr;
		ptr->m_prev = pos;
	}
}

//--------------------------------------------------------------------------------------------------
inline void ListRemove(CrossCoordNode **list, CrossCoordNode *pos)
{
	if (!pos->m_prev && !pos->m_next)
	{
		*list = nullptr;
	}
	else 
	{
		if (pos->m_prev)
		{
			pos->m_prev->m_next = pos->m_next;
		}
		if (pos->m_next) 
		{
			pos->m_next->m_prev = pos->m_prev;
		}
		if (*list == pos) *list = pos->m_next;
	}
}

//--------------------------------------------------------------------------------------------------
inline void MoveIn(CrossCoordNode *player_node, CrossCoordNode *sensor_node) 
{
	if (player_node->m_pplayer->m_nuid == sensor_node->m_pplayer->m_nuid) return;
	const auto &pos = player_node->m_pplayer->m_pos;
	const auto &other_pos = sensor_node->m_pplayer->m_pos;
	auto radius = sensor_node->m_psensor->m_radius;

	if (fabs(pos.x - other_pos.x) < radius && fabs(pos.z - other_pos.z) < radius)
	{
		sensor_node->m_psensor->AddCandidate(player_node->m_pplayer);
	}
}

inline void MoveOut(CrossCoordNode *player_node, CrossCoordNode *sensor_node)
{
	sensor_node->m_psensor->RemoveCandidate(player_node->m_pplayer);
}

#define MOVE_CROSS_ID(dir, type1, type2) ((dir << 16) + (type1 << 8) + type2)

inline void MoveCross(Uint32 dir, CrossCoordNode *moving_node, CrossCoordNode *static_node) 
{
	Uint32 cross_id = MOVE_CROSS_ID(dir, (Uint32)moving_node->m_type, (Uint32)static_node->m_type);

	switch (cross_id) 
	{
	case MOVE_CROSS_ID(MOVE_DIRECTION_LEFT, COORD_TYPE_PLAYER, COORD_TYPE_GUARD_RIGHT):
	case MOVE_CROSS_ID(MOVE_DIRECTION_RIGHT, COORD_TYPE_PLAYER, COORD_TYPE_GUARD_LEFT):
		MoveIn(moving_node, static_node);
		break;
	case MOVE_CROSS_ID(MOVE_DIRECTION_LEFT, COORD_TYPE_GUARD_LEFT, COORD_TYPE_PLAYER):
	case MOVE_CROSS_ID(MOVE_DIRECTION_RIGHT, COORD_TYPE_GUARD_RIGHT, COORD_TYPE_PLAYER):
		MoveIn(static_node, moving_node);
		break;
	case MOVE_CROSS_ID(MOVE_DIRECTION_LEFT, COORD_TYPE_PLAYER, COORD_TYPE_GUARD_LEFT):
	case MOVE_CROSS_ID(MOVE_DIRECTION_RIGHT, COORD_TYPE_PLAYER, COORD_TYPE_GUARD_RIGHT):
		MoveOut(moving_node, static_node);
		break;
	case MOVE_CROSS_ID(MOVE_DIRECTION_LEFT, COORD_TYPE_GUARD_RIGHT, COORD_TYPE_PLAYER):
	case MOVE_CROSS_ID(MOVE_DIRECTION_RIGHT, COORD_TYPE_GUARD_LEFT, COORD_TYPE_PLAYER):
		MoveOut(static_node, moving_node);
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------------------------------------------
void ListUpdateNode(CrossCoordNode **list, CrossCoordNode *pnode)
{
	float value = pnode->m_value;

	if (pnode->m_next && pnode->m_next->m_value < value)
	{
		// move right
		auto cur_node = pnode->m_next;
		while (1) 
		{
			MoveCross(MOVE_DIRECTION_RIGHT, pnode, cur_node);
			if (!cur_node->m_next || cur_node->m_next->m_value >= value) break;
			cur_node = cur_node->m_next;
		}

		ListRemove(list, pnode);
		ListInsertAfter(list, cur_node, pnode);

	}
	else if (pnode->m_prev && pnode->m_prev->m_value > value)
	{
		// move left
		auto cur_node = pnode->m_prev;
		while (1)
		{
			MoveCross(MOVE_DIRECTION_LEFT, pnode, cur_node);
			if (!cur_node->m_prev || cur_node->m_prev->m_value <= value) break;
			cur_node = cur_node->m_prev;
		}

		ListRemove(list, pnode);
		ListInsertBefore(list, cur_node, pnode);
	}
}

//--------------------------------------------------------------------------------------------------
CrossAoi::CrossAoi(float map_bound_xmin, float map_bound_xmax, float map_bound_zmin, float map_bound_zmax, size_t beacon_x, size_t beacon_z, float beacon_radius)
	: m_coord_list_x(NULL)
	, m_coord_list_z(NULL)
	, m_cur_aoi_map_idx(0)
{
	if (beacon_x == 0 && beacon_z == 0)
		return;

	assert(map_bound_xmax > map_bound_xmin);
	assert(map_bound_zmax > map_bound_zmin);
	float step_x = (map_bound_xmax - map_bound_xmin) / beacon_x / 2;
	float step_z = (map_bound_zmax - map_bound_zmin) / beacon_z / 2;
	for (auto x : boost::irange(beacon_x)) 
	{
		for (auto z : boost::irange(beacon_z))
		{
			float pos_x = map_bound_xmin + step_x * (x * 2 + 1);
			float pos_z = map_bound_zmin + step_z * (z * 2 + 1);
			auto nuid = GenNuid();
			AddPlayerNoBeacon(nuid, pos_x, 0, pos_z);
			AddSensorNoBeacon(nuid, GenNuid(), beacon_radius);
			auto &beacon = *m_player_map[nuid];
			beacon.SetFlag_Beacon();
			beacon.m_detected_by.reset(new boost::unordered_map<Nuid, std::vector<Nuid>>());
			m_beacons.push_back(&beacon);
		}
	}
}

//--------------------------------------------------------------------------------------------------
void CrossAoi::AddPlayer(Nuid nuid, float x, float y, float z) 
{
	if (m_beacons.empty()) 
	{
		AddPlayerNoBeacon(nuid, x, y, z);
		return;
	}

	auto piter = m_player_map.find(nuid);
	if (piter != m_player_map.end())
	{
		AddPlayerNoBeacon(nuid, x, y, z);
		return;
	}

	auto ret = m_player_map.emplace(nuid, new CrossPlayerAoi(nuid, x, y, z));
	auto &player = *ret.first->second;
	player.SetFlag_New();

	// 找最近的 beacon
	CrossPlayerAoi* best_beacon = nullptr;
	float min_dist = std::numeric_limits<float>::max();
	for (auto pbeacon : m_beacons) 
	{
		float dx = pbeacon->m_pos.x - x;
		float dz = pbeacon->m_pos.z - z;
		float dist = dx * dx + dz * dz;
		if (dist < min_dist) {
			min_dist = dist;
			best_beacon = pbeacon;
		}
	}
	assert(best_beacon);

	// 复制 detected_by
	for (auto &elem : *best_beacon->m_detected_by)
	{
		auto other_nuid = elem.first;
		auto &other_player = *m_player_map[other_nuid];
		for (auto sensor_id : elem.second) {
			for (auto &sensor : other_player.m_sensors)
			{
				if (sensor.m_sensor_id == sensor_id) 
				{
					sensor.AddCandidate(&player);
				}
			}
		}
	}
	if (!best_beacon->m_sensors.empty())
	{
		for (auto &sensor : best_beacon->m_sensors)
		{
			sensor.AddCandidate(&player);
		}
	}

	ListInsertBefore(&m_coord_list_x, &best_beacon->m_node_x, &player.m_node_x);
	ListInsertBefore(&m_coord_list_z, &best_beacon->m_node_z, &player.m_node_z);
	UpdatePos(nuid, x, y, z);
}

//--------------------------------------------------------------------------------------------------
void CrossAoi::AddPlayerNoBeacon(Nuid nuid, float x, float y, float z)
{
	auto piter = m_player_map.find(nuid);

	if (piter == m_player_map.end())
	{
		auto ret = m_player_map.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(nuid),
			std::forward_as_tuple(new CrossPlayerAoi(nuid, x, y, z)));
		auto &player = *ret.first->second;
		player.SetFlag_New();
		ListInsertBefore(&m_coord_list_x, m_coord_list_x, &player.m_node_x);
		ListInsertBefore(&m_coord_list_z, m_coord_list_z, &player.m_node_z);
	}
	else
	{
		piter->second->UnsetFlag_Removed();
	}
	UpdatePos(nuid, x, y, z);
}

//--------------------------------------------------------------------------------------------------
void CrossAoi::RemovePlayer(Nuid nuid) 
{
	auto piter = m_player_map.find(nuid);
	if (piter == m_player_map.end()) return;

	auto &player = *piter->second;
	player.SetFlag_Removed();
}

//--------------------------------------------------------------------------------------------------
void CrossAoi::_RemovePlayer(Nuid nuid)
{
	auto piter = m_player_map.find(nuid);
	if (piter == m_player_map.end()) return;

	auto &player = *piter->second;
	std::vector<Nuid> sensor_ids;
	for (auto siter = player.m_sensors.rbegin(); siter != player.m_sensors.rend(); ++siter)
	{
		sensor_ids.push_back(siter->m_sensor_id);
	}

	for (auto sensor_id : sensor_ids)
	{
		RemoveSensor(nuid, sensor_id);
	}

	ListRemove(&m_coord_list_x, &player.m_node_x);
	ListRemove(&m_coord_list_z, &player.m_node_z);
	m_player_map.erase(nuid);
}

//--------------------------------------------------------------------------------------------------
void CrossAoi::AddSensor(Nuid nuid, Nuid sensor_id, float radius)
{
	if (m_beacons.empty())
	{
		return AddSensorNoBeacon(nuid, sensor_id, radius);
	}

	auto piter = m_player_map.find(nuid);
	if (piter == m_player_map.end()) return;

	auto &player = *piter->second;
	// 找最近的 beacon
	CrossPlayerAoi* best_beacon = nullptr;
	float min_dist = std::numeric_limits<float>::max();
	for (auto pbeacon : m_beacons)
	{
		float dx = pbeacon->m_pos.x - player.m_pos.x;
		float dz = pbeacon->m_pos.z - player.m_pos.z;
		float dist = dx * dx + dz * dz;
		if (dist < min_dist) {
			min_dist = dist;
			best_beacon = pbeacon;
		}
	}
	assert(best_beacon);

	auto &best_sensor = best_beacon->m_sensors[0];
	float dr = best_sensor.m_radius - radius;
	if (dr * dr + min_dist > radius) 
	{
		return AddSensorNoBeacon(nuid, sensor_id, radius);
	}

	player.m_sensors.emplace_back(sensor_id, radius, &player);
	auto &sensor = player.m_sensors[player.m_sensors.size() - 1];
	auto size = kh_size(best_sensor.m_aoi_player_candidates.get());
	kh_resize(SensorHashMap, sensor.m_aoi_player_candidates.get(), size);
	CrossPlayerAoi *val;
	kh_foreach_value(best_sensor.m_aoi_player_candidates.get(), val,
		sensor.AddCandidate(val);
	)
		sensor.AddCandidate(best_beacon);

	ListInsertBefore(&m_coord_list_x, &best_sensor.m_left_x, &sensor.m_left_x);
	ListInsertAfter(&m_coord_list_x, &best_sensor.m_right_x, &sensor.m_right_x);

	ListInsertBefore(&m_coord_list_z, &best_sensor.m_left_z, &sensor.m_left_z);
	ListInsertAfter(&m_coord_list_z, &best_sensor.m_right_z, &sensor.m_right_z);

	UpdateSensorPos(player, &sensor);
}

//--------------------------------------------------------------------------------------------------
void CrossAoi::AddSensorNoBeacon(Nuid nuid, Nuid sensor_id, float radius) 
{
	auto piter = m_player_map.find(nuid);
	if (piter == m_player_map.end()) return;

	auto &player = *piter->second;
	player.m_sensors.emplace_back(sensor_id, radius, &player);
	auto &sensor = player.m_sensors[player.m_sensors.size() - 1];

	ListInsertBefore(&m_coord_list_x, &player.m_node_x, &sensor.m_left_x);
	ListInsertAfter(&m_coord_list_x, &player.m_node_x, &sensor.m_right_x);

	ListInsertBefore(&m_coord_list_z, &player.m_node_z, &sensor.m_left_z);
	ListInsertAfter(&m_coord_list_z, &player.m_node_z, &sensor.m_right_z);

	UpdateSensorPos(player, &sensor);
}

//--------------------------------------------------------------------------------------------------
void CrossAoi::RemoveSensor(Nuid nuid, Nuid sensor_id)
{
	auto piter = m_player_map.find(nuid);
	if (piter == m_player_map.end()) return;

	auto &player = *piter->second;
	if (player.m_sensors.size() > 1)
	{
		auto &last_sensor = player.m_sensors[player.m_sensors.size() - 1];
		if (last_sensor.m_sensor_id != sensor_id)
		{
			for (auto &sensor : player.m_sensors)
			{
				if (sensor.m_sensor_id == sensor_id)
				{
					std::swap(sensor, last_sensor);
					break;
				}
			}
		}
	}

	auto &last_sensor = player.m_sensors[player.m_sensors.size() - 1];
	if (last_sensor.m_sensor_id != sensor_id) return;

	ListRemove(&m_coord_list_x, &last_sensor.m_left_x);
	ListRemove(&m_coord_list_x, &last_sensor.m_right_x);
	ListRemove(&m_coord_list_z, &last_sensor.m_left_z);
	ListRemove(&m_coord_list_z, &last_sensor.m_right_z);
	player.m_sensors.pop_back();
}

//--------------------------------------------------------------------------------------------------
void CrossAoi::UpdatePos(Nuid nuid, float x, float y, float z)
{
	auto piter = m_player_map.find(nuid);
	if (piter == m_player_map.end()) return;

	auto &player = *piter->second;
	player.m_pos.Set(x, y, z);
	player.SetFlag_Dirty();

	player.m_node_x.m_value = player.m_pos.x;
	ListUpdateNode(&m_coord_list_x, &player.m_node_x);

	player.m_node_z.m_value = player.m_pos.z;
	ListUpdateNode(&m_coord_list_z, &player.m_node_z);

	if (!player.m_sensors.empty())
	{
		for (auto &sensor : player.m_sensors) {
			UpdateSensorPos(player, &sensor);
		}
	}
}

//--------------------------------------------------------------------------------------------------
void CrossAoi::UpdateSensorPos(const CrossPlayerAoi &player, CrossSensor *psensor)
{
	auto radius = psensor->m_radius;
	psensor->m_right_x.m_value = player.m_pos.x + radius;
	ListUpdateNode(&m_coord_list_x, &psensor->m_right_x);

	psensor->m_left_x.m_value = player.m_pos.x - radius;
	ListUpdateNode(&m_coord_list_x, &psensor->m_left_x);

	psensor->m_right_z.m_value = player.m_pos.z + radius;
	ListUpdateNode(&m_coord_list_z, &psensor->m_right_z);

	psensor->m_left_z.m_value = player.m_pos.z - radius;
	ListUpdateNode(&m_coord_list_z, &psensor->m_left_z);
}

//--------------------------------------------------------------------------------------------------
CrossAoiUpdateInfos CrossAoi::Tick() 
{
	// 全量做一遍 aoi
	CrossAoiUpdateInfos update_infos;
	CrossPlayerPtrList remove_list;

	for (auto& elem : m_player_map)
	{
		auto& player = *elem.second;
		if (player.GetFlag_Beacon()) continue;

		if (player.GetFlag_Removed()) 
		{
			remove_list.push_back(&player);
			continue;
		}

		if (!player.m_sensors.empty())
		{
			auto update_info = UpdatePlayerAoi(m_cur_aoi_map_idx, &player);
			if (!update_info.m_sensor_update_list.empty()) 
			{
				update_infos.emplace(update_info.m_nuid, std::move(update_info));
			}
		}

		player.UnsetFlag_New();
	}

	for (auto pptr : remove_list) 
	{
		RemovePlayer(pptr->m_nuid);
	}

	for (auto& elem : m_player_map)
	{
		auto& player = *elem.second;
		player.m_last_pos = player.m_pos;
	}
	m_cur_aoi_map_idx = 1 - m_cur_aoi_map_idx;
	return update_infos;
}

//--------------------------------------------------------------------------------------------------
CrossAoiUpdateInfo CrossAoi::UpdatePlayerAoi(Uint32 cur_aoi_map_idx,
	CrossPlayerAoi* pptr) {
	CrossAoiUpdateInfo aoi_update_info;
	aoi_update_info.m_nuid = pptr->m_nuid;
	Uint32 new_aoi_map_idx = 1 - cur_aoi_map_idx;

	for (auto& sensor : pptr->m_sensors) 
	{
		auto& old_aoi = sensor.m_aoi_players[cur_aoi_map_idx];
		auto& new_aoi = sensor.m_aoi_players[new_aoi_map_idx];
		CalcAoiPlayers(*pptr, sensor, &new_aoi);

		CrossSensorUpdateInfo update_info;

		auto& enters = update_info.m_enters;
		auto& leaves = update_info.m_leaves;
		float radius_square = sensor.m_radius_square;

		CheckLeave(pptr, radius_square, old_aoi, &leaves);
		CheckEnter(pptr, radius_square, new_aoi, &enters);

		if (enters.empty() && leaves.empty())
		{
			continue;
		}

		update_info.m_sensor_id = sensor.m_sensor_id;
		aoi_update_info.m_sensor_update_list.push_back(std::move(update_info));
	}

	return aoi_update_info;
}

//--------------------------------------------------------------------------------------------------
void CrossAoi::CalcAoiPlayers(const CrossPlayerAoi& player, const CrossSensor& sensor,
	CrossPlayerPtrList* aoi_map)
{
	aoi_map->clear();
	auto candidates = sensor.m_aoi_player_candidates.get();
	aoi_map->reserve(kh_size(candidates));

	auto pos = player.m_pos;
	auto radius = sensor.m_radius;
	auto radius_suqare = radius * radius;
	float dx, dz;

	CrossPlayerAoi* other_ptr;
	std::vector<CrossPlayerAoi*> remove_players;
	kh_foreach_value(candidates, other_ptr,
		if (other_ptr->GetFlag_Beacon()) continue;
	if (other_ptr->GetFlag_Removed()) 
	{
		remove_players.push_back(other_ptr);
		continue;
	}
	IfInXZRadiusSquare(dx, dz, other_ptr->m_pos.x, other_ptr->m_pos.z, pos.x, pos.z, radius_suqare) 
	{
		aoi_map->emplace_back(other_ptr);
	}
	)
}

//--------------------------------------------------------------------------------------------------
void CrossAoi::CheckLeave(CrossPlayerAoi* pptr, float radius_square,
	const CrossPlayerPtrList &aoi_players, CrossPlayerNuids *leaves)
{
	const auto &player_pos = pptr->m_pos;
	float dx, dz;
	float pos_x = player_pos.x;
	float pos_z = player_pos.z;
	for (auto old_player_ptr : aoi_players)
	{
		if (old_player_ptr->GetFlag_Removed()) 
		{
			leaves->push_back(old_player_ptr->m_nuid);
		}
		else
		{
			IfNotInXZRadiusSquare(dx, dz, old_player_ptr->m_pos.x, old_player_ptr->m_pos.z,
				pos_x, pos_z, radius_square)
			{
				leaves->push_back(old_player_ptr->m_nuid);
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------
void CrossAoi::CheckEnter(CrossPlayerAoi* pptr, float radius_square,
	const CrossPlayerPtrList &aoi_players, CrossPlayerNuids *enters)
{
	const auto &player_last_pos = pptr->m_last_pos;
	float pos_x = player_last_pos.x;
	float pos_z = player_last_pos.z;

	if (pptr->GetFlag_New())
	{
		enters->reserve(aoi_players.size());
		for (auto new_player_ptr : aoi_players)
		{
			enters->push_back(new_player_ptr->m_nuid);
		}
		return;
	}

	float dx, dz;
	for (auto new_player_ptr : aoi_players) 
	{
		IfNotInXZRadiusSquare(dx, dz, new_player_ptr->m_last_pos.x, new_player_ptr->m_last_pos.z,
			pos_x, pos_z, radius_square)
		{
			enters->push_back(new_player_ptr->m_nuid);
		}
	}
}

//--------------------------------------------------------------------------------------------------
void CrossAoi::PrintNodeList(CrossCoordNode *list) 
{
	printf("[");
	auto cur_node = list;
	while (cur_node) 
	{
		cur_node->PrintLog();
		cur_node = cur_node->m_next;
	}
	printf("]");
}


void CrossAoi::PrintAllNodeList() 
{
	printf("x_list: ");
	PrintNodeList(m_coord_list_x);
	printf("\nz_list: ");
	PrintNodeList(m_coord_list_z);
	printf("\n");
}