#pragma once

typedef long long int64;

class CTowerMapInfo
{
public:
	CTowerMapInfo()
		: m_grid_count_y(0)
		, m_grid_count_x(0)
		, m_grid_width(0)
		, m_grid_height(0)
		, m_map_min_x(0)
		, m_map_max_x(0)
		, m_map_min_y(0)
		, m_map_max_y(0)
	{
	}
public:
	int m_grid_count_y;
	int m_grid_count_x;
	int m_grid_width;
	int m_grid_height;
	int m_map_min_x;
	int m_map_max_x;
	int m_map_min_y;
	int m_map_max_y;
};
