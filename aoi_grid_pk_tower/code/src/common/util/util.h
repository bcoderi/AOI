#pragma once

#include <vector>
#include <set>

//比较两个int类型vector的元素是否相同
bool IntEqual(const std::vector<int>& vec1, const std::vector<int>& vec2);

//获取两个int类型vector的交集
void IntIntersect(const std::vector<int>& vec1, const std::vector<int>& vec2, std::vector<int>& out);

//获取两个int类型vector的差集
void IntExept(const std::vector<int>& vec1, const std::vector<int>& vec2, std::vector<int>& out);

void PlayerIntersect(const std::set<int>& set1, const std::set<int>& set2, std::set<int>& out);

void PlayerExcept(const std::set<int>& set1, const std::set<int>& set2, std::set<int>& out);
