#pragma once

#include <unordered_map>
#include <map>
#include <vector>
#include <cmath>
#include <limits>
#include <memory>

#include "../common/base_types.h"

class SquarePlayerAoi;
typedef std::unordered_map<Nuid, std::shared_ptr<SquarePlayerAoi>> CrossPlayerMap;
typedef std::unordered_map<Nuid, SquarePlayerAoi*> CrossPlayerPtrMap;
typedef std::vector<Nuid> PlayerNuids;
typedef std::vector<SquarePlayerAoi*> PlayerPtrList;
typedef Uint64 SquareId;
typedef std::vector<SquarePlayerAoi*> SquarePlayers;
typedef std::unordered_map<SquareId, SquarePlayers> SquareList;
constexpr int kSquareIdShift = sizeof(SquareId) * 4;

#define AOI_FLOAT_MAX FLT_MAX
#define AOI_INF_POS AOI_FLOAT_MAX, AOI_FLOAT_MAX, AOI_FLOAT_MAX

struct SquarePos {
  SquarePos(float _x, float _y, float _z)
      : x(_x), y(_y), z(_z) {}

  void Set(float _x, float _y, float _z) {
    x = _x;
    y = _y;
    z = _z;
  }

  float x, y, z;
};


struct SquareSensor {
  SquareSensor(Nuid _sensor_id, float _radius)
      : sensor_id(_sensor_id), radius(_radius), radius_square(_radius * _radius) {}

  Nuid sensor_id;
  float radius;
  float radius_square;
  PlayerPtrList aoi_players[2];
};


struct SquarePlayerAoi
{
  SquarePlayerAoi(Uint64 _nuid, float _x, float _y, float _z)
      : nuid(_nuid), pos(_x, _y, _z), last_pos(AOI_INF_POS), flags(0) {}

  AOI_CLASS_ADD_FLAG(Removed, 0, flags);
  AOI_CLASS_ADD_FLAG(New, 1, flags);

  Nuid nuid;
  SquareId square_id;
  int square_index;
  SquarePos pos;
  SquarePos last_pos;
  Uint32 flags;
  std::vector<SquareSensor> sensors;
};


struct SquareSensorUpdateInfo {
  Nuid sensor_id;
  PlayerNuids enters;
  PlayerNuids leaves;
};


struct SquareAoiUpdateInfo {
  Nuid nuid;
  std::vector<SquareSensorUpdateInfo> sensor_update_list;
};

typedef std::unordered_map<Nuid, SquareAoiUpdateInfo> AoiUpdateInfos;


inline int CoordToId(float coord, float inverse_square_size) {
  return static_cast<int>(std::floor(coord * inverse_square_size));
}


inline SquareId GenSquareId(int xi, int zi) {
  return (static_cast<SquareId>(xi) << kSquareIdShift) | static_cast<Uint32>(zi);
}


inline SquareId PosToId(float x, float z, float inverse_square_size) {
  int left = CoordToId(x, inverse_square_size);
  int right = CoordToId(z, inverse_square_size);
  return GenSquareId(left, right);
}


class SquareAoi {
 public:
  explicit SquareAoi(float square_size = 200);

  void AddPlayer(Nuid nuid, float x, float y, float z);
  void RemovePlayer(Nuid nuid);
  void AddSensor(Nuid nuid, Nuid sensor_id, float radius);
  void UpdatePos(Nuid nuid, float x, float y, float z);
  AoiUpdateInfos Tick();
  const SquareList& GetSquares() const {
    return squares_;
  }
  const CrossPlayerMap& GetPlayerMap() const {
    return player_map_;
  }

 protected:
  void _AddToSquare(Nuid nuid, SquarePlayerAoi*);
  void _RemoveFromSquare(Nuid nuid, SquarePlayerAoi*);
  SquareAoiUpdateInfo _UpdatePlayerAoi(Uint32 cur_aoi_map_idx, SquarePlayerAoi* player);
  void _CalcAoiPlayers(const SquarePlayerAoi& player, const SquareSensor& sensor, PlayerPtrList* aoi_map);
  inline void _GetSquaresAndPlayerNum(const SquarePos& pos, float radius,
                                      std::vector<SquarePlayers*> *squares, size_t* player_num, int player_map_size);
  void _CheckLeave(SquarePlayerAoi* pptr, float radius_square,
                    const PlayerPtrList &aoi_players, PlayerNuids *leaves);
  void _CheckEnter(SquarePlayerAoi* pptr, float radius_square,
                    const PlayerPtrList &aoi_players, PlayerNuids *leaves);

 protected:
  float square_size_;
  float inverse_square_size_;
  Uint32 cur_aoi_map_idx_;

  SquareList squares_;
  CrossPlayerMap player_map_;
};

inline void SquareAoi::_GetSquaresAndPlayerNum(const SquarePos& pos, float radius,
                                        std::vector<SquarePlayers*> *squares,
                                        size_t* player_num, int player_map_size)
{
  float pos_x = pos.x;
  float pos_z = pos.z;
  int minxi = CoordToId(pos_x - radius, inverse_square_size_);
  int maxxi = CoordToId(pos_x + radius, inverse_square_size_);
  int minzi = CoordToId(pos_z - radius, inverse_square_size_);
  int maxzi = CoordToId(pos_z + radius, inverse_square_size_);

  if (0)
  {
	  squares->clear();
  }

  *player_num = 0;
  for (int xi = minxi; xi <= maxxi; ++xi)
  {
    for (int zi = minzi; zi <= maxzi; ++zi)
	{
      auto square_id = GenSquareId(xi, zi);
      auto square_iter = squares_.find(square_id);
      if (square_iter == squares_.end())
        continue;

      squares->push_back(&(square_iter->second));
      *player_num += square_iter->second.size();
    }
  }

  /*if (player_map_size < (int)player_num)
  {
	  int i = 0;
  }*/
}
