#pragma once

#include <set>
#include <map>

class CPlayer
{
public:
	CPlayer(int id, int x, int y)
		: m_id(id)
		, m_x(x)
		, m_y(y)
	{
	}
public:
	void ReceiveEnterMessage(CPlayer* player);
	void ReceiveMoveMessage(CPlayer* player);
	void ReceiveLeaveMessage(CPlayer* player);
public:
	int m_id;
	int m_x;
	int m_y;
	std::set<int> m_players;	//需要通知的玩家列表
};

class CPlayerManager
{
public:
	CPlayerManager();
	~CPlayerManager();
public:
	void NewPlayer(int id, int x, int y);
	void ResetPlayer(int id, int x, int y);
	CPlayer* GetPlayer(int id);
	const std::map<int, CPlayer*>& GetPlayers();
private:
	std::map<int, CPlayer*> m_players;
};
