#pragma once

/*
	��ͼ��С25x25��x���귶Χ[0,25]��y���귶Χ[0,25]
	������С5x5
	�����Ұ��Χ10x10-s_visible_area
	���������������-s_player_num
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
