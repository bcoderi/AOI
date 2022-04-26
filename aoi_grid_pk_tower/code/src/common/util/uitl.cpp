
#include "util.h"

bool IntEqual(const std::vector<int>& vec1, const std::vector<int>& vec2)
{
	if (vec1.size() != vec2.size())
		return false;

	if (vec1 == vec2)
		return true;

	return false;
}

void IntIntersect(const std::vector<int>& vec1, const std::vector<int>& vec2, std::vector<int>& out)
{
	out.clear();
	std::vector<int>::const_iterator itr1 = vec1.begin();
	for (; itr1 != vec1.end(); ++itr1)
	{
		bool find = false;
		for (std::vector<int>::const_iterator itr2 = vec2.begin(); itr2 != vec2.end(); ++itr2)
		{
			if (*itr1 == *itr2)
			{
				find = true;
				break;
			}
		}

		if (find)
		{
			out.push_back(*itr1);
		}
	}
}

void IntExept(const std::vector<int>& vec1, const std::vector<int>& vec2, std::vector<int>& out)
{
	out.clear();
	std::vector<int>::const_iterator itr1 = vec1.begin();
	for (; itr1 != vec1.end(); ++itr1)
	{
		bool find = false;
		for (std::vector<int>::const_iterator itr2 = vec2.begin(); itr2 != vec2.end(); ++itr2)
		{
			if (*itr1 == *itr2)
			{
				find = true;
				break;
			}
		}

		if (!find)
		{
			out.push_back(*itr1);
		}
	}
}

void PlayerIntersect(const std::set<int>& set1, const std::set<int>& set2, std::set<int>& out)
{
	out.clear();
	std::set<int>::const_iterator itr1 = set1.begin();
	for (; itr1 != set1.end(); ++itr1)
	{
		bool find = false;
		for (std::set<int>::const_iterator itr2 = set2.begin(); itr2 != set2.end(); ++itr2)
		{
			if (*itr1 == *itr2)
			{
				find = true;
				break;
			}
		}

		if (find)
		{
			out.insert(*itr1);
		}
	}
}

void PlayerExcept(const std::set<int>& set1, const std::set<int>& set2, std::set<int>& out)
{
	out.clear();
	std::set<int>::const_iterator itr1 = set1.begin();
	for (; itr1 != set1.end(); ++itr1)
	{
		bool find = false;
		for (std::set<int>::const_iterator itr2 = set2.begin(); itr2 != set2.end(); ++itr2)
		{
			if (*itr1 == *itr2)
			{
				find = true;
				break;
			}
		}

		if (!find)
		{
			out.insert(*itr1);
		}
	}
}
