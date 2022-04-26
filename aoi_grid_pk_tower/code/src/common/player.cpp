
#include <iostream>
#include "player.h"
#include "common.h"

void CPlayer::ReceiveEnterMessage(CPlayer* player)
{
	if (s_log)
		return;

	if (!player)
		return;

	std::cout << "��� id:" << m_id << ",����:" << m_x << "," << m_y << ",�յ���Ϣ��"
		<< "��� id:" << player->m_id << ",����:" << player->m_x << "," << player->m_y << ",������Ұ" << std::endl;
}

void CPlayer::ReceiveMoveMessage(CPlayer* player)
{
	if (s_log)
		return;

	if (!player)
		return;

	std::cout << "��� id:" << m_id << ",����:" << m_x << "," << m_y << ",�յ���Ϣ��"
		<< "��� id:" << player->m_id << ",�ƶ�������:" << player->m_x << "," << player->m_y << std::endl;
}

void CPlayer::ReceiveLeaveMessage(CPlayer* player)
{
	if (s_log)
		return;

	if (!player)
		return;

	std::cout << "��� id:" << m_id << ",����:" << m_x << "," << m_y << ",�յ���Ϣ��"
		<< "��� id:" << player->m_id << ",����:" << player->m_x << "," << player->m_y << ",�뿪��Ұ" << std::endl;
}

CPlayerManager::CPlayerManager()
{

}

CPlayerManager::~CPlayerManager()
{
	std::map<int, CPlayer*>::iterator itr = m_players.begin();
	for (; itr != m_players.end(); ++itr)
	{
		if (itr->second != NULL)
		{
			delete itr->second;
			itr->second = NULL;
		}
	}

	m_players.clear();
}

void CPlayerManager::NewPlayer(int id, int x, int y)
{
	m_players[id] = new CPlayer(id, x, y);
}

void CPlayerManager::ResetPlayer(int id, int x, int y)
{
	std::map<int, CPlayer*>::iterator itr_find = m_players.find(id);
	if (itr_find != m_players.end())
	{
		itr_find->second->m_id = id;
		itr_find->second->m_x = x;
		itr_find->second->m_y = y;
		itr_find->second->m_players.clear();
	}
}

CPlayer* CPlayerManager::GetPlayer(int id)
{
	std::map<int, CPlayer*>::const_iterator itr = m_players.find(id);
	if (itr == m_players.end())
	{
		return NULL;
	}
	else
	{
		return itr->second;
	}
}

const std::map<int, CPlayer*>& CPlayerManager::GetPlayers()
{
	return m_players;
}
