
#define BOOST_TEST_MODULE test_cross
#define BOOST_TEST_DYN_LINK
#include <unordered_map>
#include <iostream>
#include <vector>
#include <cmath>
#include <boost/test/included/unit_test.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/timer/timer.hpp>
#include <boost/range/irange.hpp>
#include "../src/common/nuid.h"
#include "../src/common/silence_unused.h"
#include "../src/cross/cross.h"
#include <corecrt_math_defines.h>

BOOST_AUTO_TEST_SUITE(test_cross)

class Player;

class CLogMgr
{
public:
	CLogMgr()
		: m_log_file("test_grid_pk_cross_cross_file.txt")
	{
	}

	~CLogMgr()
	{
		m_log_file.close();
	}

public:
	std::ofstream& GetLogFile()
	{
		return m_log_file;
	}

private:
	std::ofstream m_log_file;
};

class CrossAoiTest : public CrossAoi
{
public:
	CrossAoiTest() : CrossAoi(0, 0, 0, 0, 0, 0, 0) {}

	CrossAoiTest(float map_bound_xmin, float map_bound_xmax, float map_bound_zmin,float map_bound_zmax, size_t beacon_x, size_t beacon_z, float beacon_radius)
		: CrossAoi(map_bound_xmin, map_bound_xmax, map_bound_zmin, map_bound_zmax, beacon_x, beacon_z, beacon_radius)
	{}

	friend class Player;
};


class Player
{
public:
	Player() : m_guid(GenNuid()), m_pos(0, 0, 0) {}
	Player(Nuid nuid, CrossPos pos) : m_guid(nuid), m_pos(pos) {}

public:
	inline Nuid AddSensor(float radius, bool log = false)
	{
		auto sensor_id = GenNuid();
		m_aoi->AddSensor(m_guid, sensor_id, radius);
		if (log)
		{
			printf("Player %lu Add Sensor %lu, radius %f\n", m_guid, sensor_id, radius);
		}
		return sensor_id;
	}

	inline void AddToAoi(CrossAoiTest* aoi, bool log = false)
	{
		m_aoi = aoi;
		m_aoi->AddPlayer(m_guid, m_pos.x, m_pos.y, m_pos.z);
		m_player_aoi = m_aoi->GetPlayerMap().find(m_guid)->second.get();

		if (log)
		{
			printf("Add Player: %lu, Pos(%f, %f, %f)\n",
				m_guid, m_pos.x, m_pos.y, m_pos.z);
		}
	}

	inline void RemoveFromAoi(bool log = false)
	{
		if (log)
		{
			printf("Remove Player: %lu, Pos(%f, %f, %f)\n",
				m_guid, m_pos.x, m_pos.y, m_pos.z);
		}
		m_aoi->RemovePlayer(m_guid);
		m_aoi = nullptr;
		m_player_aoi = nullptr;
	}

	inline void MoveDelta(float x, float y, float z)
	{
		MoveTo(m_pos.x + x, m_pos.y + y, m_pos.z + z);
	}

	inline void MoveTo(float x, float y, float z, bool log = false)
	{
		m_pos.Set(x, y, z);
		if (m_aoi)
		{
			m_aoi->UpdatePos(m_guid, x, y, z);
		}

		if (log)
		{
			printf("Player %lu MoveTo (%f, %f, %f)\n",
				m_guid, x, y, z);
		}
	}

public:
	Nuid m_guid;
	CrossPos m_pos;
	CrossPlayerAoi *m_player_aoi;
	CrossAoiTest *m_aoi;
};


std::vector<Player> GenPlayers(const size_t player_num, const float map_size)
{
	std::vector<Player> players(player_num);

	boost::random::mt19937 random_generator(std::time(0));
	boost::random::uniform_real_distribution<float> pos_generator(-map_size, map_size);

	for (int i : boost::irange(player_num))
	{
		Player& player = players[i];
		player.m_pos.Set(pos_generator(random_generator), 0, pos_generator(random_generator));
	}

	BOOST_TEST_REQUIRE((players.size() == player_num));
	return players;
}


std::vector<CrossPos> GenMovements(const size_t player_num, const float length)
{
	std::vector<CrossPos> movements;
	movements.reserve(player_num);

	boost::random::mt19937 random_generator(std::time(0));
	boost::random::uniform_real_distribution<float> angle_gen(0, 360);
	for (int UNUSED(i) : boost::irange(player_num))
	{
		float angle = angle_gen(random_generator);
		float radian = 2 * M_PI * angle / 360;
		movements.emplace_back(std::cos(radian) * length, 0, std::sin(radian) * length);
	}

	return movements;
}

void TestOneMilestone(std::vector<Player> *players, const size_t player_num,
	const float map_size, CLogMgr& log_mgr)
{
	printf("\n===Begin Milestore: player_num = %lu, map_size = (%f, %f)\n",
		player_num, -map_size, map_size);

	log_mgr.GetLogFile() << "\nBegin Test cross, player_num:" << player_num << ",map_size:" << map_size << std::endl;

	boost::timer::cpu_timer run_timer;
	int times = 1;
	std::vector<CrossAoiTest> cross_aois;
	for (auto UNUSED(i) : boost::irange(times))
	{
		cross_aois.emplace_back(-map_size, map_size, -map_size, map_size, 3, 3, 100);
	}
	
	int add_begin_tick = GetTickCount();
	for (auto &cross_aoi : cross_aois)
	{
		for (auto &player : *players)
		{
			player.AddToAoi(&cross_aoi);
			player.AddSensor(100);
		}
		BOOST_TEST_REQUIRE((cross_aoi.GetPlayerMap().size() == player_num + 9));
	}

	run_timer.stop();
	printf("Add Player (%i times)", times);
	log_mgr.GetLogFile() << "Add Player times:" << times << ",use_time:" << GetTickCount() - add_begin_tick << std::endl;
	std::cout << run_timer.format();

	for (auto &cross_aoi : cross_aois)
	{
		cross_aoi.Tick();
	}

	run_timer.start();
	for (auto &cross_aoi : cross_aois)
	{
		cross_aoi.Tick();
	}
	run_timer.stop();

	printf("Tick (%i times)", times);
	std::cout << run_timer.format();

	float speed = 6;
	float delta_time = 0.001;
	auto movements = GenMovements(player_num, delta_time * speed);
	times = 1 / delta_time;
	run_timer.start();
	int move_begin_tick = GetTickCount();
	for (int UNUSED(t) : boost::irange(times))
	{
		for (int i : boost::irange(player_num))
		{
			auto &player = players->at(i);
			auto &move = movements[i];
			player.MoveDelta(move.x, move.y, move.z);
		}
	}
	run_timer.stop();
	printf("Update Pos (%i times)", times);
	log_mgr.GetLogFile() << "Update Pos times:" << times << ",use_time:" << GetTickCount() - move_begin_tick << std::endl;
	log_mgr.GetLogFile() << "End Test Grid" << std::endl;
	std::cout << run_timer.format();

	printf("===End Milestore\n");
}

BOOST_AUTO_TEST_CASE(test_milestone)
{
	CLogMgr log_mgr;
	for (size_t player_num : {20, 50, 100, 200})
	{
		for (float map_size : {50, 100, 200, 500})
		{
			auto players = GenPlayers(player_num, map_size);
			TestOneMilestone(&players, player_num, map_size, log_mgr);
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()
