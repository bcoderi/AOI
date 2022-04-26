#pragma once

/*
	地图大小25x25：x坐标范围[0,25]，y坐标范围[0,25]
	灯塔大小5x5
	玩家视野范围10x10-s_visible_area
	场景中玩家总数量-s_player_num
*/

static const bool s_log = true;

static const int s_grid_count_x = 30;
static const int s_grid_count_y = 30;

static const int s_map_min_x = 0;
static const int s_map_min_y = 0;
static const int s_map_max_x = 300;
static const int s_map_max_y = 300;

static const int s_grid_width = 10;        //(s_map_max_x - s_map_min_x) / s_grid_count_x
static const int s_grid_height = 10;        //(s_map_max_y - s_map_min_y) / s_grid_count_y

static const float s_visible_area = 10;

static const int s_player_num = 900;

enum EOutType
{
	EOT_Grid,
	EOT_Tower,
};
