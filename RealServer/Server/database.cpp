#include "database.h"

SQLHENV henv;
SQLHDBC hdbc;
SQLHSTMT hstmt = 0;

void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[100];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle! n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage, (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			wprintf(L"[%s] : ", wszState);
			wprintf(L"%s", wszMessage);
			wprintf(L" - %d\n", iError);
		}
	}
}

void Initialise_DB()
{
	SQLRETURN retcode;

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"SSU_Project", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);
				
				// Allocate statement handle  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					cout << "ODBC Connection Success" << endl;
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
				}
				else {
					cout << "연결 실패" << endl;
					HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
				}
			}
		}
	}
}

bool Search_Id(Player* pl, char* login_id)
{
	// cout << atoi(login_id) << endl;
	SQLRETURN retcode;
	SQLINTEGER c_id, c_hp, c_exp, c_maxhp, c_mp, c_maxmp;
	SQLWCHAR c_name[MAX_NAME_SIZE];
	SQLSMALLINT c_x, c_y, c_z, c_lv, c_job;
	SQLLEN cbP_name = 0, cbP_id = 0, cbP_x = 0, cbP_y = 0, cbP_z = 0,
		cbP_hp = 0, cbP_lv = 0, cbP_exp = 0, cbP_maxhp = 0, cbP_job = 0, 
		cbP_mp = 0, cbP_maxmp = 0;
	char temp[50];
	sprintf_s(temp, sizeof(temp), "EXEC search_player %s", login_id);
	//cout << exec << endl;
	wchar_t* exec;
	int strSize = MultiByteToWideChar(CP_ACP, 0, temp, -1, NULL, NULL);
	exec = new WCHAR[strSize];
	MultiByteToWideChar(CP_ACP, 0, temp, sizeof(temp) + 1, exec, strSize);
	//wprintf(L"%s", exec);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)exec, SQL_NTS);
	//패스워드 넣어야하고 비교해서 접속하게 해야한다. 

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  //있는  ID면 불러오고 
		retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &c_id, 100, &cbP_id);
		retcode = SQLBindCol(hstmt, 2, SQL_C_WCHAR, c_name, MAX_NAME_SIZE, &cbP_name);
		retcode = SQLBindCol(hstmt, 3, SQL_C_SHORT, &c_x, 100, &cbP_x);
		retcode = SQLBindCol(hstmt, 4, SQL_C_SHORT, &c_y, 100, &cbP_y);
		retcode = SQLBindCol(hstmt, 5, SQL_C_SHORT, &c_z, 100, &cbP_z);
		retcode = SQLBindCol(hstmt, 6, SQL_C_LONG, &c_hp, 100, &cbP_hp);
		retcode = SQLBindCol(hstmt, 7, SQL_C_SHORT, &c_lv, 100, &cbP_lv);
		retcode = SQLBindCol(hstmt, 8, SQL_C_LONG, &c_exp, 100, &cbP_exp);
		retcode = SQLBindCol(hstmt, 9, SQL_C_LONG, &c_maxhp, 100, &cbP_maxhp);
		retcode = SQLBindCol(hstmt, 10, SQL_C_SHORT, &c_job, 100, &cbP_job);
		retcode = SQLBindCol(hstmt, 11, SQL_C_LONG, &c_mp, 100, &cbP_mp);
		retcode = SQLBindCol(hstmt, 12, SQL_C_LONG, &c_maxmp, 100, &cbP_maxmp);

		// Fetch and print each row of data. On an error, display a message and exit.
		retcode = SQLFetch(hstmt);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			//strcmp(clients[array_id].name, (const char*)c_name);
			
			//pl->set_login_id(c_id); -> 현재 오류때문에 주석 넣음, DB처리할때는 풀어야함
			strSize = WideCharToMultiByte(CP_ACP, 0, c_name, -1, NULL, 0, NULL, NULL);
			WideCharToMultiByte(CP_ACP, 0, c_name, -1, pl->get_name(), strSize, 0, 0);
			cout << pl->get_name() << endl;
			//여기에  직업 등 필요한 걸 위에랑 아래 추가하자 
			
			pl->set_x(c_x);
			pl->set_y(c_y);
			pl->set_z(c_z);
			pl->set_hp(c_hp);
			pl->set_lv(c_lv);
			pl->set_exp(c_exp);
			pl->set_maxhp(c_maxhp);
			pl->set_job((JOB)c_job);
			pl->set_mp(c_mp);
			pl->set_maxmp(c_maxmp);

			cout << pl->get_id() << "," << pl->get_name() << "," << pl->get_x() << "," << pl->get_y() << ","
				<< pl->get_hp() << "," << pl->get_lv() << "," << pl->get_exp() << "," << pl->get_maxhp() << pl->get_job() << endl;
			SQLCancel(hstmt);
			delete exec;
			return true;
		}
		else {   //없으면 만들어야지 
			SQLCancel(hstmt);
			cout << "id 생성으로 간다" << endl;
			// exec 다시 설정
			char temp2[100];
			sprintf_s(temp2, sizeof(temp2), "EXEC create_player %s, insert_%d", login_id, atoi(login_id));
			int strSize2 = MultiByteToWideChar(CP_ACP, 0, temp2, -1, NULL, NULL);
			wchar_t* exec2 = new WCHAR[strSize2];
			cout << temp2 << endl;
			MultiByteToWideChar(CP_ACP, 0, temp2, sizeof(temp2) + 1, exec2, strSize2);
			wprintf(L"%s", exec2);
			retcode = SQLExecDirect(hstmt, (SQLWCHAR*)exec2, SQL_NTS);
			
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				delete exec2;
				if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO) {
					HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
					return false;
				}
				else {	// 생성이 되었으니 다시 읽자
					retcode = SQLExecDirect(hstmt, (SQLWCHAR*)exec, SQL_NTS);

					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &c_id, 100, &cbP_id);
						retcode = SQLBindCol(hstmt, 2, SQL_C_WCHAR, c_name, MAX_NAME_SIZE, &cbP_name);
						retcode = SQLBindCol(hstmt, 3, SQL_C_SHORT, &c_x, 100, &cbP_x);
						retcode = SQLBindCol(hstmt, 4, SQL_C_SHORT, &c_y, 100, &cbP_y);
						retcode = SQLBindCol(hstmt, 5, SQL_C_SHORT, &c_z, 100, &cbP_z);
						retcode = SQLBindCol(hstmt, 6, SQL_C_LONG, &c_hp, 100, &cbP_hp);
						retcode = SQLBindCol(hstmt, 7, SQL_C_SHORT, &c_lv, 100, &cbP_lv);
						retcode = SQLBindCol(hstmt, 8, SQL_C_LONG, &c_exp, 100, &cbP_exp);
						retcode = SQLBindCol(hstmt, 9, SQL_C_LONG, &c_maxhp, 100, &cbP_maxhp);
						retcode = SQLBindCol(hstmt, 10, SQL_C_SHORT, &c_job, 100, &cbP_job);
						retcode = SQLBindCol(hstmt, 11, SQL_C_LONG, &c_mp, 100, &cbP_mp);
						retcode = SQLBindCol(hstmt, 12, SQL_C_LONG, &c_maxmp, 100, &cbP_maxmp);

						// Fetch and print each row of data. On an error, display a message and exit.
						retcode = SQLFetch(hstmt);
						if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
						{
							//strcmp(clients[array_id].name, (const char*)c_name);
							// pl->set_login_id(c_id); -> 현재 오류때문에 주석 넣음, DB처리할때는 풀어야함
							strSize = WideCharToMultiByte(CP_ACP, 0, c_name, -1, NULL, 0, NULL, NULL);
							WideCharToMultiByte(CP_ACP, 0, c_name, -1, pl->get_name(), strSize, 0, 0);
							cout << pl->get_name() << endl;

							pl->set_x(c_x);
							pl->set_y(c_y);
							pl->set_z(c_z);
							pl->set_hp(c_hp);
							pl->set_lv(c_lv);
							pl->set_exp(c_exp);
							pl->set_maxhp(c_maxhp);
							pl->set_job((JOB)c_job);
							pl->set_mp(c_mp);
							pl->set_maxmp(c_maxmp);

							cout << pl->get_id() << endl;
							SQLCancel(hstmt);
							delete exec;
							return true;
						}
						else {
							cout << "id 생성 실패" << endl;
							delete exec;
							return false;
						}
					}
				}
			}
		}
	}
	else {
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
		return false;
	}
}

void Save_position(Player* pl)
{
	SQLRETURN retcode;

	char temp[100];
	//여기도 패스워드, Z좌표, 직업,MP,MAXMP 를 추가로 넣어서 저장해야한다. 
	sprintf_s(temp, sizeof(temp), "EXEC save_player_info %d, %d, %d, %d, %d, %d, %d", 
		pl->get_login_id(), pl->get_x(), pl->get_y(), pl->get_hp(),
		pl->get_lv(), pl->get_exp(), pl->get_maxhp());
	wchar_t* exec;
	int strSize = MultiByteToWideChar(CP_ACP, 0, temp, -1, NULL, NULL);
	exec = new WCHAR[strSize];
	MultiByteToWideChar(CP_ACP, 0, temp, sizeof(temp) + 1, exec, strSize);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)exec, SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		// Fetch and print each row of data. On an error, display a message and exit.
		// retcode = SQLFetch(hstmt);
		SQLLEN* pcrow = new SQLLEN;
		retcode = SQLRowCount(hstmt, pcrow);
		if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO) {

			HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
		}
	}
	else {
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	}
	//SQLCancel(hstmt);
}

void Disconnect_DB()
{
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}


//추가 
void Update_DB(wstring id, short posX, short posY, short level, int exp, short HP) //인자를 player로 하고  저것들 외에 다른 스탯들도 업데이트하자 
{
	wstring qu{};
	SQLRETURN retcode;
	qu += L"EXEC update_DB ";
	qu += id;
	qu += L", ";
	qu += to_wstring(posX);
	qu += L", ";
	qu += to_wstring(posY);
	qu += L", ";
	qu += to_wstring(level);
	qu += L", ";
	qu += to_wstring(exp);
	qu += L", ";
	qu += to_wstring(HP);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)qu.c_str(), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		cout << "update 완료!" << endl;

	}
	else
		cout << "update 실패" << endl;

}