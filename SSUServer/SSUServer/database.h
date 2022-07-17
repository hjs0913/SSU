#pragma once
#include <WS2tcpip.h>
#include <sqlext.h>
#include <string>
#include "Player.h"

void	HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
void	Initialize_DB();
bool	Search_Id(Player* pl, char* login_id, char* password);
void	Save_position(Player* pl);
void	Disconnect_DB();
bool    Add_DB(char* login_id, char* password, Player* pl, char* nick_name, int job, int element);
extern bool    DB_On;
