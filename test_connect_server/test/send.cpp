#include "stdafx.h"
#include "CNet.h"

void CNet::send_login_packet(char* id, char* name)
{
	cs_packet_login packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_LOGIN;
	strcpy_s(packet.id, id);
	strcpy_s(packet.name, name);
	do_send(sizeof(packet), &packet);
}

void CNet::send_attack_packet(int skill)
{
	cs_packet_attack packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_ATTACK;
	//packet.skill = (char)skill;
	do_send(sizeof(packet), &packet);
}

void CNet::send_move_packet(XMFLOAT3 position)
{
	cs_packet_move packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_MOVE;
	//packet.direction = (char)direction;
	packet.x = position.x;
	packet.y = position.y;
	packet.z = position.z;
	do_send(sizeof(packet), &packet);
}

void CNet::send_look_packet(XMFLOAT3 look, XMFLOAT3 right)
{
	cs_packet_look packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_LOOK;
	packet.x = look.x;
	packet.y = look.y;
	packet.z = look.z;
	packet.right_x = right.x;
	packet.right_y = right.y;
	packet.right_z = right.z;
	do_send(sizeof(packet), &packet);
}

void CNet::send_skill_packet(int sk_t, int sk_n)
{
	cs_packet_skill packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_SKILL;

	packet.skill_type = sk_t;
	packet.skill_num = sk_n;
	do_send(sizeof(packet), &packet);
}

void CNet::send_picking_skill_packet(int sk_t, int sk_n, int target)
{
	cs_packet_picking_skill packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PICKING_SKILL;
	packet.target = target;
	packet.skill_type = sk_t;
	packet.skill_num = sk_n;
	do_send(sizeof(packet), &packet);

}

void CNet::send_change_job_packet(JOB my_job)
{
	cs_packet_change_job packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_CHANGE_JOB;
	packet.job = my_job;
	do_send(sizeof(packet), &packet);

}

void CNet::send_change_element_packet(ELEMENT my_element)
{
	cs_packet_change_element packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_CHANGE_ELEMENT;
	packet.element = my_element;
	do_send(sizeof(packet), &packet);

}

void CNet::send_chat_packet(const char* send_str)
{
	cs_packet_chat packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_CHAT;
	strcpy_s(packet.message, send_str);
	do_send(sizeof(packet), &packet);
}

void CNet::send_party_room_packet()
{
	cs_packet_party_room packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_ROOM;
	do_send(sizeof(packet), &packet);
}

void CNet::send_raid_rander_ok_packet()
{
	cs_packet_raid_rander_ok packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_RAID_RANDER_OK;
	do_send(sizeof(packet), &packet);
}

void CNet::send_party_room_make()
{
	cs_packet_party_room_make packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_ROOM_MAKE;
	do_send(sizeof(packet), &packet);
}

void CNet::send_party_room_info_request(int party_index_id)
{
	cs_packet_party_room_info_request packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_ROOM_INFO_REQUEST;
	packet.room_id = m_party[party_id_index_vector[party_index_id]]->get_party_id();
	do_send(sizeof(packet), &packet);
}

void CNet::send_party_room_enter_request()
{
	cs_packet_party_room_enter_request packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_ROOM_ENTER_REQUEST;
	packet.room_id = m_party_info->get_party_id();
	do_send(sizeof(packet), &packet);
}

void CNet::send_party_room_quit_request()
{
	cs_packet_party_room_quit_request packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_ROOM_QUIT_REQUEST;
	packet.room_id = party_enter_room_id;
	do_send(sizeof(packet), &packet);
}

void CNet::send_party_invite(char* user)
{
	cs_packet_party_invite packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_INVITE;
	packet.room_id = party_enter_room_id;
	strcpy_s(packet.user_name, user);
	do_send(sizeof(packet), &packet);
}

void CNet::send_party_add_partner(JOB j)
{
	cs_packet_party_add_partner packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_ADD_PARTNER;
	packet.room_id = party_enter_room_id;
	packet.job = j;
	do_send(sizeof(packet), &packet);
}

void CNet::send_party_invitation_reply(int accept)
{
	cs_packet_party_invitation_reply packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PARTY_INVITATION_REPLY;
	packet.room_id = InvitationRoomId;
	packet.invite_user_id = InvitationUser;
	packet.accept = accept;
	do_send(sizeof(packet), &packet);
}