#pragma once

#include <vector>
#include <set>

//�Ƚ�����int����vector��Ԫ���Ƿ���ͬ
bool IntEqual(const std::vector<int>& vec1, const std::vector<int>& vec2);

//��ȡ����int����vector�Ľ���
void IntIntersect(const std::vector<int>& vec1, const std::vector<int>& vec2, std::vector<int>& out);

//��ȡ����int����vector�Ĳ
void IntExept(const std::vector<int>& vec1, const std::vector<int>& vec2, std::vector<int>& out);

void PlayerIntersect(const std::set<int>& set1, const std::set<int>& set2, std::set<int>& out);

void PlayerExcept(const std::set<int>& set1, const std::set<int>& set2, std::set<int>& out);
