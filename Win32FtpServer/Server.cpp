#include "Server.h"

// ��������� �����
#define localhost "127.0.0.1"
// �������� ��� ������ ����� ��������
struct ThreadPackage
{
	Server *serv;
	SOCKET clientSocket;
};

// ������ ����������� ������
CRITICAL_SECTION locker;

// �����������
Server::Server()
{
	// ������ �� �������
	serverStatus = false;
	// �������� ���
	userOnline = 0;
	// ������� ������ �����
	curRow = 0;
	// �������������� ��������� ����������� ������
	InitializeCriticalSection(&locker);
}
// ����������
Server::~Server()
{
	// ���� ������ ������� - �������������
	if(serverStatus)
		Stop();
	// ������� ����������� ������
	DeleteCriticalSection(&locker);
}
// ������ �������������� ���������
void Server::SetControls(HWND _hTextArea, HWND _hStatusBar, HWND _hWndMain)
{
	// ������������� ����
	hWndMain = _hWndMain;
	hTextArea = _hTextArea;
	hStatusBar = _hStatusBar;
}
// �������� ����� � ���� ������
void Server::AddText(const TCHAR* Text, bool same_row)
{
	TCHAR BUFF[MAX_PATH] = {0};
	if(same_row)
	{
		LVITEM lvitem;
		ZeroMemory(&lvitem, sizeof(lvitem));
		lvitem.iItem = curRow;
		lvitem.cchTextMax = MAX_PATH;
		lvitem.pszText = BUFF;
		ListView_GetItemText(hTextArea, curRow - 1, 0, BUFF, MAX_PATH);
		strcat(BUFF, Text);
		ListView_SetItemText(hTextArea, curRow - 1, 0, BUFF);
		if(serverSettings.fileLog)
		{
			DWORD writted;
			WriteFile(LogToFile, (LPVOID)BUFF, sizeof(TCHAR) * strlen(BUFF), &writted, 0);
		}
	}
	else
	{
		LVITEM lvItem;
		ZeroMemory(&lvItem, sizeof(lvItem));
		// ��������� ���� �����������
		lvItem.mask = LVIF_TEXT;
		lvItem.state = 0;
		lvItem.stateMask = 0 ;
		lvItem.iItem = curRow;
		lvItem.iSubItem = 0;
		strcpy(BUFF, Text);
		lvItem.pszText = (LPSTR)BUFF;
		lvItem.cchTextMax = MAX_PATH;
		// ��������� � ������
		ListView_InsertItem(hTextArea, &lvItem);
		// ������� ����� �����������
		curRow++;
		if(serverSettings.fileLog)
		{
			DWORD writted;
			WriteFile(LogToFile, (LPVOID)BUFF, sizeof(TCHAR) * strlen(BUFF), &writted, 0);
		}
	}
}
// ��������� ��������
BOOL Server::LoadAccounts()
{
	// ������� ������ ���������
	accounts.clear();
	// ���� � ����� ��������
	char cmdLine[MAX_PATH] = {0};
	// �������� ����
	GetModuleFileName(GetModuleHandle(NULL), cmdLine, MAX_PATH);
	// �������� ������ �������
    *strrchr(cmdLine, '\\') = NULL;
	// ��������� ������ ���� � ����� ��������
	strcat(cmdLine, "\\accounts.dat");
	// �����
	string line;
	// ��������� �����
	ifstream inf(cmdLine);

	if(!inf)
	{
		MessageBox(0, "������ �������� �����", "������", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	while(getline(inf, line))
	{
		istringstream ss(line);
		string str;
		vector<string> tokens;
		while(getline(ss, str, '\t'))
			tokens.push_back(str);
		
		// ���� ������ �� ������� �������
		if(tokens.size() != 3)
		{
			MessageBox(0, "�������� ������ �����", "������", MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
		// ��������� ������� � ������ ���������
		accounts[tokens[0]] = Account(tokens);
	}

	// ��������� �����
	inf.close();
	// ������ ����� �������
	return TRUE;
}
// ��������� ��������
BOOL Server::SaveAccounts()
{
	// ���� � ����� ��������
	char cmdLine[MAX_PATH] = {0};
	// �������� ����
	GetModuleFileName(GetModuleHandle(NULL), cmdLine, MAX_PATH);
	// �������� ������ �������
    *strrchr(cmdLine, '\\') = NULL;
	// ��������� ������ ���� � ����� ��������
	strcat(cmdLine, "\\accounts.dat");
	// ��������� �������� �����
	ofstream outf(cmdLine, ios::trunc);
	// ���� ����� �� ������
	if(!outf)
	{
		// �������� �� ������
		sprintf(szErrorBuff, "������ �������� ����� ��������� (User.dat)\n");
		MessageBox(0, szErrorBuff, "������ ������������� �������� ����� ���������", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	map<string, Account>::iterator it;
	int i = 0;
	for(it = accounts.begin(); it != accounts.end(); ++it)
	{
		if(it != accounts.begin() && it != accounts.end())
			outf << endl;
		outf << it->second.GetLogin() << "\t" << it->second.GetPassword() << "\t" << it->second.GetHome();
	}
	outf.close();
	return TRUE;
}
// ��������� ������
BOOL Server::Start(Settings &setup)
{
	TCHAR path[MAX_PATH] = {0};
	// ���������� ������ �� �������
	serverStatus = false;
	// ������ ���������
	serverSettings = setup;
	// ��������� ������ � ���������
	if(!LoadAccounts())
	{
		MessageBox(0, "������ ����� anonymous", "������������ �� ���������", MB_OK | MB_ICONEXCLAMATION);
		// ���� � ����� ��������
		char cmdLine[MAX_PATH] = {0};
		// �������� ����
		GetModuleFileName(GetModuleHandle(NULL), cmdLine, MAX_PATH);
		// �������� ������ �������
		*strrchr(cmdLine, '\\') = NULL;
		accounts["anonymous"] = Account("anonymous", "anonymous@mail.com", cmdLine);
		SaveAccounts();
	}
	// ������������� Winsock
	if(WSAStartup(0x0202, &wsaData))
	{
		sprintf(szMsgBuff, "������: %d\n", WSAGetLastError());
		MessageBox(0, szMsgBuff, "������ ������������� WSADATA", MB_OK | MB_ICONEXCLAMATION);
		WSACleanup();
		return FALSE;
	}
	// ���� ����������� ����������� ����������� � ����
	if(serverSettings.fileLog)
	{
		strcpy(path, serverSettings.getPath());
		strcat(path, "\\logs.txt");
		LogToFile = CreateFile(path, GENERIC_WRITE, 
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if(LogToFile == INVALID_HANDLE_VALUE)
		{
			MessageBox(0, "������ �������� ����� �����������", "������ ������� � �����", MB_OK | MB_ICONSTOP);
			serverSettings.fileLog = false;
		}
		else
		{
			SetFilePointer(LogToFile, 0, 0, FILE_END);
		}
	}
	// ������� ������� ���������� ������ �������
	stopEvent = CreateEvent(0, TRUE, FALSE, 0);
	startEvent = CreateEvent(0, TRUE, FALSE, 0);	
	// ������������� ������
    DWORD         dwNetThreadId;
	// ������� ��������� ����� � �������� ��� ��������� ������
	serverThread = CreateThread(NULL, 0, ServerThread, this, 0, &dwNetThreadId);
	// ��������� �������� ������
	if(serverThread == 0)
	{
		sprintf(szErrorBuff, "������ �� �������. \r\n������ �������� ������: %d", GetLastError());
		MessageBox(0, szErrorBuff, "������", MB_OK | MB_ICONEXCLAMATION);
		// ��������� �����������
		serverSettings.fileLog = false;
		CloseHandle(LogToFile);
		// ���������� ������ �������
		return serverStatus;
	}
	AddText("������ �������->");
	// ������� ������� ���������� ������
	DWORD result = WaitForSingleObject(startEvent, 3000);
	if(result != WAIT_OBJECT_0)
	{
		// ��������� �����������
		serverSettings.fileLog = false;
		CloseHandle(LogToFile);
		// ���
		AddText(" ������ ������� !", true);
		CloseHandle(serverThread);
		return serverStatus;
	}
	// ������ ��� - ������ ������� �������
	serverStatus = true;
	// ��������� ���� ������
	sprintf(szMsgBuff, "������ %d", userOnline);
	// ������� ������� ���������� ������������� � ������ ���
	SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)(INT) 0 | 0, (LPARAM) szMsgBuff);
	// ������� �����������
	AddText(" ������ �������. �������� �����������...", true);
	// ���������� ������ �������
	return serverStatus;
}
// ���������� ������
void Server::Stop()
{
	// ���������� ������� ��������� � ���������� ���������
	SetEvent(stopEvent);
	// ��������� ��� ����������
	for(auto it = sessions.begin(); it != sessions.end(); it++)
		// ���������� ������ �� ������ ����������
		shutdown(it->first, SD_BOTH);		
	// ������� �����. ���������� ��� ��� ������������� ���������� ������ ������� ���������� � ��������������� ��������� �������� accept
	SOCKET quitSock = socket(AF_INET, SOCK_STREAM, 0);
	// ��������� ������� �� ������ �����
    if (quitSock == SOCKET_ERROR)
    {
		sprintf(szErrorBuff, "������ �������� ������ ������.\r\n ��� ������: %d", WSAGetLastError());
		MessageBox(0, szErrorBuff, "������", MB_OK | MB_ICONEXCLAMATION);
		CloseHandle(stopEvent);
		CloseHandle(LogToFile);
        return;
    }
	// ��������� ������
	struct sockaddr_in localaddr;
	// ��������� ����
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(serverSettings.servPort);
	// ���� �����
	if(inet_addr(localhost) != INADDR_NONE)
		localaddr.sin_addr.S_un.S_addr = inet_addr(localhost);
	else
	{
		MessageBox(0, "������ ������� ���������� ���� ������.", "������", MB_OK | MB_ICONEXCLAMATION);
		CloseHandle(stopEvent);
		closesocket(quitSock);
		CloseHandle(LogToFile);
		return;
	}
	// �������������� ���� � ���� �� ��� �� ���� ��� ������� ������
	if(connect(quitSock, (sockaddr *)&localaddr, sizeof(localaddr)))
	{
		sprintf(szErrorBuff, "������ ���������� ������������� ������.\r\n ��� ������: %d", GetLastError());
		MessageBox(0, szErrorBuff, 0, 0);
		CloseHandle(stopEvent);
		closesocket(quitSock);
		CloseHandle(LogToFile);
		return;
	}
	// ��������� ����� ������
	closesocket(quitSock);
	/* �.�. ������� ���������� ������������ ��������� ����� ���������� �������� ����������� ��������� ������� ��������� �������
	� ����� ������ �� ������� �����	������������� ���������� ������������ ���������� ������ */
	AddText("��������� �������...");
	// ������� ���� ��������� ����� �����������
	while(WaitForSingleObject(serverThread, 500) != WAIT_OBJECT_0);
	// ��������� ����� ���������� ������
	CloseHandle(serverThread);
	CloseHandle(stopEvent);
	// ������ ���������� �������
	serverStatus = false;
	// ������� ���������
	AddText("������ ����������.", true);
	CloseHandle(LogToFile);
	// ���������������� ����������
	if (WSACleanup())
	{
		MessageBox(0, "������ WSACleanup", "������", MB_OK | MB_ICONEXCLAMATION);
		return;
	}
}
// ������� �� ������
bool Server::IsStart()
{
	return serverStatus;
}
// �������� ������ �����
void Server::ClearLogs()
{
	ListView_DeleteAllItems(hTextArea);
	curRow = 0;
}
// ��������� ���������� �������
DWORD WINAPI ClientThread(LPVOID lpParam)
{
	// �������� ��������
	ThreadPackage *package = (ThreadPackage*)lpParam;
	// ���������� ��������� �� ��������� ������
	Server *server = package->serv;
	// ������ ������
	const int PACK_LEN = 1024;
	// ����� �������
	TCHAR sendBuff[PACK_LEN] = {0};
	TCHAR reciveBuff[PACK_LEN] = {0};
	// ����� ��� ���������
	TCHAR szMsgBuff[MAX_PATH];
	// ������ � ����������� ������
	EnterCriticalSection(&locker);
	// �������� ������ ������
	Session session = server->sessions[package->clientSocket];
	// ��������� �������������� ������
	sprintf(szMsgBuff, "accept ip = %s port = %s\r\n", session.IP.c_str(), session.PORT.c_str());
	// ��������� ��������� � ������ ����� �������
	server->AddText(szMsgBuff);
	// ��������� ������ �����������
	memset(sendBuff, 0, sizeof(sendBuff));
	sprintf(sendBuff, "200 %s\r\n", server->serverSettings.msgWelcome.c_str());
	// ���������� ��������� ����������� �������
	send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
	// ��������� ������ � ����� �������
	sprintf(szMsgBuff, "[%s:%s] <= %s",	session.IP.c_str(), session.PORT.c_str(), sendBuff);
	server->AddText(szMsgBuff);
	// ����������� ������� �������� ����������
	server->userOnline++;
	// ������� ������
	sprintf(szMsgBuff, "������ %d", server->userOnline);
	SendMessage(server->hStatusBar, SB_SETTEXT, (WPARAM)(INT) 0 | 0, (LPARAM) szMsgBuff);
	// �������� ����������� ������
	LeaveCriticalSection(&locker);
	// ������ ��������� ����� ����������� �������
	bool cmdComplete = false, r = false, n = false;
	// �������
	string cmd = "";
	// ���������� �������� ����
	int bytes_recive = 0;
	// ���� ��������� �������������� � ��������
	while((bytes_recive = recv(session.clientSocket, reciveBuff, sizeof(reciveBuff), 0)) && bytes_recive != SOCKET_ERROR && 
		WaitForSingleObject(server->stopEvent, 0) != WAIT_OBJECT_0)
    {
		// ���� ������ �� ������� - ����������
		if(cmdComplete)
		{
			cmd.clear();
			cmdComplete = false;
		}
		// �������� ����������� ���������
		for (int i = 0; i < bytes_recive && !cmdComplete; i++)
		{
			//printf("0x%02x ", reciveBuff[i]);
			if(reciveBuff[i] != '\r' && reciveBuff[i] != '\n')
				cmd += reciveBuff[i];
			if(reciveBuff[i] == '\r')
				r = true;
			else if(reciveBuff[i] == '\n')
				n = true;
			if(r && n)
			{
				cmdComplete = true;
				r = !r;
				n = !n;
			}
		}
		// ������� ������ � ���������
		if(cmdComplete)
		{
			// ��������������� ������� � ������ �������
			transform(cmd.begin(), cmd.end(), cmd.begin(), tolower);
			// ��������� �������������� ������
			sprintf(szMsgBuff, "[%s:%s] => %s\r\n", session.IP.c_str(), session.PORT.c_str(), cmd.c_str());
			// ������� ������� �� ������� � ����
			EnterCriticalSection(&locker);
			server->AddText(szMsgBuff);
			LeaveCriticalSection(&locker);
			// ������������ �������
			////////////////////////////////////////////// USER //////////////////////////////////////////////
			if(cmd.find("user") == 0)
			{
				// ������������ �� �����������
				session.cmd_user = false;
				session.cmd_pass = false;
				session.cmd_port = false;
				// ������ ����������� ���������� �� ����������
				session.remoteIP.clear();
				session.remotePort = 0;
				// ������ � ����. ������
				EnterCriticalSection(&locker);
				// ���� ������ ������� ������� "user " � ������ � �������
				if(cmd.size() > 5)
				{	
					// �������� �����
					string User = cmd.substr(5);
					// ����� �������� � ������ ������������������ �� �������
					if(server->accounts.find(User) != server->accounts.end())
					{
						// ���������� ����� ������� ������
						session.Login = User;
						// ������������ ������
						session.cmd_user = true;
						// ��������� � ���������� �������� ���������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "331 User name okay, need password.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ��������� �������������� ������ ���������� ������
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						// ��������� � ��������� ����
						server->AddText(szMsgBuff);
					}
					else
					{
						// ��������� � ���������� �������� ���������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "530 Invalid user name.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ��������� �������������� ������ ���������� ������
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						// ��������� � ��������� ����
						server->AddText(szMsgBuff);
					}
				}
				else
				{
					// ��������� � ���������� �������� ���������
					memset(sendBuff, 0, sizeof(sendBuff));
					strcpy(sendBuff, "530 Invalid user name.\r\n");
					send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
					// ��������� �������������� ������ ���������� ������
					sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
					// ��������� � ��������� ����
					server->AddText(szMsgBuff);
				}
				// ������� �� ����. ������
				LeaveCriticalSection(&locker);
			}
			////////////////////////////////////////////// PASS //////////////////////////////////////////////
			else if(cmd.find("pass") == 0)
			{
				// ������ � ����. ������
				EnterCriticalSection(&locker);
				// ���� ����� ������ ��� ������� ������
				if(session.cmd_user)
				{
					if(cmd.size() > 5)
					{
						// �������� �����
						string Pass = cmd.substr(5);
						// ���� ����� �� ��� ���������� �������� - ��������� ������
						if(session.Login == "anonymous" || 
							server->accounts[session.Login].GetPassword() == Pass)
						{	
							// ������ ������
							session.cmd_pass = true;
							// ���������� ������ �������� �����������
							memset(sendBuff, 0, sizeof(sendBuff));
							strcpy(sendBuff, "230 User logged in, proceed.\r\n");
							send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
							// ��������� �������������� ������ ���������� ������
							sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
							// ��������� � ��������� ����
							server->AddText(szMsgBuff);
						}
						else
						{
							// ������ �� ������
							session.cmd_pass = false;
							// ���������� ������ ��������� �����������
							memset(sendBuff, 0, sizeof(sendBuff));
							strcpy(sendBuff, "530 Not logged in.\r\n");
							send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
							// ��������� �������������� ������ ���������� ������
							sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
							// ��������� � ��������� ����
							server->AddText(szMsgBuff);
						}
					}
				}
				else
				{
					session.cmd_pass = false;
					// ��������� � ���������� �������� ���������
					memset(sendBuff, 0, sizeof(sendBuff));
					strcpy(sendBuff, "503 command user must be before command pass.\r\n");
					send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
					// ��������� �������������� ������ ���������� ������
					sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
					// ��������� � ��������� ����
					server->AddText(szMsgBuff);
				}
				// ������� �� ����. ������
				LeaveCriticalSection(&locker);
			}
			////////////////////////////////////////////// TYPE I //////////////////////////////////////////////
			else if(cmd.find("type i") == 0)
			{
				// ������ � ����. ������
				EnterCriticalSection(&locker);
				// ��������� � ���������� �������� ���������
				memset(sendBuff, 0, sizeof(sendBuff));
				strcpy(sendBuff, "200 Switching to Binary.\r\n");
				send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
				// ��������� �������������� ������ ���������� ������
				sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
				// ��������� � ��������� ����
				server->AddText(szMsgBuff);
				// ������� �� ����. ������
				LeaveCriticalSection(&locker);
			}
			////////////////////////////////////////////// TYPE A //////////////////////////////////////////////
			else if(cmd.find("type a") == 0)
			{
				// ������ � ����. ������
				EnterCriticalSection(&locker);
				// ��������� � ���������� �������� ���������
				memset(sendBuff, 0, sizeof(sendBuff));
				strcpy(sendBuff, "200 Switching to ASCII.\r\n");
				send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
				// ��������� �������������� ������ ���������� ������
				sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
				// ��������� � ��������� ����
				server->AddText(szMsgBuff);
				// ������� �� ����. ������
				LeaveCriticalSection(&locker);
			}
			////////////////////////////////////////////// SYST //////////////////////////////////////////////
			else if(cmd.find("syst") == 0)
			{
				// ������ � ����. ������
				EnterCriticalSection(&locker);
				// ��������� � ���������� �������� ���������
				memset(sendBuff, 0, sizeof(sendBuff));
				strcpy(sendBuff, "200 UNIX Type: L8.\r\n");
				send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
				// ��������� �������������� ������ ���������� ������
				sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
				// ��������� � ��������� ����
				server->AddText(szMsgBuff);		
				// ������� �� ����. ������
				LeaveCriticalSection(&locker);
			}
			////////////////////////////////////////////// QUIT //////////////////////////////////////////////
			else if(cmd.find("quit") == 0
				|| cmd.find("close") == 0)
			{
				// ������ � ����. ������
				EnterCriticalSection(&locker);
				// ���������� ��������� �������� ������� �� �����
				memset(sendBuff, 0, sizeof(sendBuff));
				strcpy(sendBuff, "221 Goodbye, closing session.\r\n");
				send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
				// ��������� �������������� ������ ���������� ������
				sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
				// ��������� � ��������� ����
				server->AddText(szMsgBuff);
				// ������� �� ����. ������
				LeaveCriticalSection(&locker);
				// ����������� ������������
				break;
			}
			//////////////////////////////////// ������� ��������� ����������� //////////////////////////////
			else if(session.cmd_user
				&& session.cmd_pass)
			{
				////////////////////////////////////////////// PORT //////////////////////////////////////////////
				if(cmd.find("port") == 0)
				{
					session.cmd_port = true;
					session.remoteIP.clear();
					session.remotePort = 0;
					// �������� ������� �������
					replace_if(cmd.begin(), cmd.end(), 
						[] (const char ch) -> bool {return ch == ',';}, '.');
					// �������� ��������� ������� PORT
					long pos = cmd.find(' ');
					if(pos == string::npos)
					{
						// ��������� � ���������� �������� ���������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "451 Local error, PORT fail.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ��������� ������ � ���� �������			
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						continue;
					}
					string remoteAdress = cmd.substr(pos + 1);
					// �������� ���� ������ � ����
					string remoteIP = "", remotePort = "";
					int dots = 0;
					for(size_t i = 0; i < remoteAdress.size(); i++)
					{
						if(remoteAdress.at(i) == '.')
							dots++;
						if(dots < 4)
							remoteIP +=	remoteAdress.at(i);
						else
							remotePort += remoteAdress.at(i);
					}
					// �������� ����
					// �������� ������� �����
					string H1 = remotePort.substr(1, remotePort.rfind('.'));
					// �������� ������� �����
					string H2 = remotePort.substr(remotePort.rfind('.') + 1);
					// ���������� �����
					session.remoteIP = remoteIP;
					// �������� ���� �������
					session.remotePort = atoi(H1.c_str()) * 256 + atoi(H2.c_str());
					// ��������� � ���������� �������� ���������
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "200 PORT {%s %d}, command successful.\r\n", session.remoteIP.c_str(), session.remotePort);
					send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
					// ��������� ������ � ���� �������			
					sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
					EnterCriticalSection(&locker);
					server->AddText(szMsgBuff);
					LeaveCriticalSection(&locker);
				}
				////////////////////////////////////////////// LIST //////////////////////////////////////////////
				else if(cmd.find("list") == 0)
				{
					// ����� �������� LIST ������ ���� ��������� ������� PORT
					if(!session.cmd_port)
					{
						// ����������� �� ���� �������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "503 Data connection not specified.  A PORT/EPRT or PASV/EPSV command must be issued before executing this operation.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ��������� ����� � ���� �������
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						// ��������� � ��������� ����. �������
						continue;
					}
					// ���� ������� PORT �������������� ������� LIST
					string param;
					long pos = cmd.find(' ');
					// ���� ������� �������� ��������� - �������� ��
					if(pos != string::npos)
					{
						param = cmd.substr(pos + 1, cmd.size());
						// ��������� ����������� ���� ���� ��� ���
						if(param.at(param.size() - 1) != '/')
							param += '/';
					}
					else
						param = "";
					// ��������� ������ �������, �.�. �� ������ ������������� ������� � ���������� ��������� ������� PORT
					session.cmd_port = false;
					// ������� ����� ���������� � ��������
					SOCKET dataConnection = socket(AF_INET, SOCK_STREAM, 0);
					// ����� �� ������
					if(dataConnection == SOCKET_ERROR)
					{
						// ���������� ����� ������ �������� ������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "451 Local error. Data socket create failed.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ������ � ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						// ��������� � ��������� ����. �������
						continue;
					}
					// ��������� ��������� ������
					sockaddr_in dest_addr;
					ZeroMemory((void*)&dest_addr, sizeof(dest_addr));
					dest_addr.sin_family		=	AF_INET;
					dest_addr.sin_port			=	htons(session.remotePort);
					// �������� ���� ���� ������
					if(inet_addr(session.remoteIP.c_str()) != INADDR_NONE)
						dest_addr.sin_addr.S_un.S_addr	=	inet_addr(session.remoteIP.c_str());
					else
					{
						// ��������� ����� ������
						closesocket(dataConnection);
						EnterCriticalSection(&locker);
						// ��������� ������ � ����
						sprintf(szMsgBuff, "Error parse remote ip %s", session.remoteIP.c_str());
						server->AddText(szMsgBuff);
						// ���������� ����� ������ �������� ������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "425 Open data connection error. Remote addr parse fail.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ��������� ������ � ���
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						// ��������� ��������� �������
						continue;
					}
					// ����������� � ��������
					if(connect(dataConnection, (sockaddr *)&dest_addr, sizeof(dest_addr)))
					{
						EnterCriticalSection(&locker);
						// ��������� ������ � ���
						sprintf(szMsgBuff, "Connect error: %d\r\n",WSAGetLastError());
						server->AddText(szMsgBuff);
						// ���������� ����� �������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "425 Open data connection error. Connect fail.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ������ � ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						// ��������� ��������� �������
						continue;
					}
					EnterCriticalSection(&locker);
					sprintf(szMsgBuff, "Connect to remote client success.\r\n");
					server->AddText(szMsgBuff);
					
					// ��������� ���������
					WIN32_FIND_DATA findData;
					// ��������� ����
					string serverPath;
					// ��������� ����
					string localPath;
					// �������� ���� � ��������� �������� ������������
					string home = server->accounts[session.Login].GetHome();
					// ������� ���� ���� �����
					if(home.at(home.size() - 1) == '\\')
						home.resize(home.size() - 1);
					// �������� ������� ������� � ������� ���������� ������ ������������
					string dir = session.curDir;
					// ��������� ����������� ���� ���� ��� ���
					if(dir.at(dir.size() - 1) != '/')
						dir += '/';					
					// ���� ���������� ������� LIST ���
					if(param == "")
					{
						// ��������� ����
						serverPath = dir;
						localPath = home + dir;
					}
					// ��������� ��������� ������� LIST
					else
					{
						// ���� ����� ���������
						if(param.at(0) == '/')
						{
							serverPath = param;
							localPath = home + param;
						}
						// ����� ���� �� �������� ��������
						else
						{
							serverPath = dir + param;
							localPath = home + dir + param;
						}
					}
					// ������������� �����
					for(size_t i = 0; i < localPath.size(); i++)
					{
						if(localPath.at(i) == '/')
							localPath.at(i) = '\\';
					}
					// ��������� ������� ��������� ��������
					if(!PathFileExists(localPath.c_str()))
					{
						// ����� �������
						memset(sendBuff, 0, sizeof(sendBuff));			
						sprintf(sendBuff, "550 Directory %s not found.\r\n", serverPath.c_str());
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ������ � ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						continue;
					}
					// ��������� ������������ ������
					HANDLE hFind = FindFirstFile((localPath + "*.*").c_str(), &findData);
					// ���� �������� ������
					if (hFind == INVALID_HANDLE_VALUE)
					{
						// ������ � ����
						sprintf(szMsgBuff, "Open %s fail error code: %d\r\n", localPath.c_str(), GetLastError());
						server->AddText(szMsgBuff);
						// ���������� ��������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "550 Error list files.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ������ � ���
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						// ��������� ���������� ������
						closesocket(dataConnection);						
						continue;
					}
					// �������� � ������ �������� ������
					memset(sendBuff, 0, sizeof(TCHAR) * PACK_LEN);
					strcpy(sendBuff, "150 Opening ASCII mode data connection.\r\n");
					send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
					// ��������� � ������ ��� 
					sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
					server->AddText(szMsgBuff);
					// ����������� �������� �������
					string month[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
					// �������������� ������ � ������� �����
					string permstr, username, groupname, length, filename;
					// ����� ���������� ������ ������
					do
					{
						/*Txxxxxxxxx[ ]uk[ ]user[ ]group[ ]size[ ]mm[ ]dd[ ]yytt[ ]name CR, LF ���,
						T � ��� �������� (�d� � �������, �-� � ����, �l� � ������ � �.�.);
						xxxxxxxxx � �������� ������ �����;
						user � ������������, �������� �����;
						group � ������ ���������;
						size � ������ ��������;
						mm � ����� �������� �������� � ��������� ����, �������� �jul�;
						dd � ���� ������ �������� ��������;
						yytt � ����� ����� ���� ��� ��� ����� �������� ��������;
						name � ��� �������� (�����, ��������, ������);
						[ ] � ���� ��� ����� ��������. */
						
						// �������� ������ � ������� �����
						if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) && strcmp(findData.cFileName, ".") != 0 && (
							(session.curDir == "/") ? (strcmp(findData.cFileName, "..") != 0) : true) )
						{
							permstr.clear();username.c_str();groupname.c_str();length.c_str();filename.c_str();
							// ������ �����
							LONGLONG fileSize;
							LARGE_INTEGER sz;
							// �������� ������ �����
							sz.HighPart = findData.nFileSizeHigh;
							sz.LowPart = findData.nFileSizeLow;
							fileSize = sz.QuadPart;
							// ������ � ���� �����
							SYSTEMTIME sysTime;
							memset(&sysTime, 0, sizeof(SYSTEMTIME));
							FileTimeToSystemTime(&findData.ftCreationTime, &sysTime);
							// ��������� ������ � ������ ����/����������
							if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
								permstr = "drwxrwxrwx";
							else
								permstr = "-rwxrwxrwx";
							// ���������
							memset(sendBuff, 0, sizeof(TCHAR) * PACK_LEN);
							stringstream ss;
							sprintf(sendBuff, "%s   1 %-10s %-10s %10llu %3.3s %2d %02d:%02d %s\r\n",
								permstr.c_str(), "user", "group", fileSize,
								month[sysTime.wMonth - 1].c_str(), sysTime.wDay, sysTime.wHour, sysTime.wMinute, findData.cFileName);
							// ���������� ������
							send(dataConnection, sendBuff, strlen(sendBuff), 0);
						}
						memset(&findData, 0, sizeof(findData));
						// ��������� � ����������
					}while(FindNextFile(hFind, &findData));
					// ���������� ��������
					memset(sendBuff, 0, sizeof(sendBuff));
					strcpy(sendBuff, "226 Transfer complete.\r\n");
					send(session. clientSocket, sendBuff, strlen(sendBuff), 0);
					// ������ � ���
					sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
					server->AddText(szMsgBuff);
					LeaveCriticalSection(&locker);
					// ��������� ���������� ����������
					closesocket(dataConnection);					
				}
				////////////////////////////////////////////// CDUP //////////////////////////////////////////////
				else if(cmd.find("cdup") == 0)
				{
					string dir = session.curDir;
					// ���������� ��������� ����
					if(dir.at(dir.size() - 1) == '/')
						dir.resize(dir.size() - 1);
					// ���� ���������� ����
					long i = dir.rfind("/");
					// �������� ����� ����
					if(i != string::npos)
						dir = dir.substr(0, i + 1);
					else
						dir = "/";
					// ����� ���� ��� �������� ������������
					session.curDir = dir;
					// ��������� �������� ���������
					string msg = "257 \"" + session.curDir + "\"" + " is current directory\r\n";
					// ����� �������
					memset(sendBuff, 0, sizeof(sendBuff));			
					strcpy(sendBuff, msg.c_str());
					send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
					// ������ � ����
					sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
					EnterCriticalSection(&locker);
					server->AddText(szMsgBuff);
					LeaveCriticalSection(&locker);
				}
				////////////////////////////////////////////// SIZE //////////////////////////////////////////////
				else if(cmd.find("size") == 0)
				{
					// �������� ��������� �������
					string param = cmd.substr(cmd.find(' ') + 1);
					// ��������� ���� ���� ��� ���
					if(param.at(0) != '/')
						param = '/' + param;
					// �������� �����
					for(size_t i = 0; i < param.size(); i++)
					{
						if(param[i] == '/')
							param.at(i) = '\\';
					}
					EnterCriticalSection(&locker);
					// �������� ���� � ��������� �������� ������������
					string home = server->accounts[session.Login].GetHome();
					// ������� ���� ���� �����
					if(home.at(home.size() - 1) == '\\')
						home.resize(home.size() - 1);
					// ��������� ���� � �����
					string path = home + param;
					// ��������� ����
					HANDLE hFile = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
					// ���� ��������
					if(hFile != INVALID_HANDLE_VALUE)
					{
						LARGE_INTEGER fileSize = {0};
						// �������� ������ �����
						if(!GetFileSizeEx(hFile, &fileSize))
						{
							// �������� ������
							sprintf(szMsgBuff, "GetFileSizeEx fail, errorCode: %d\r\n", GetLastError());
							server->AddText(szMsgBuff);
							// ��������� ����� �� ������
							memset(sendBuff, 0, sizeof(sendBuff));
							strcpy(sendBuff, "413 error read file size\r\n");
							send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
							// ����
							sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
							server->AddText(szMsgBuff);
						}
						// ���� ������� ���� � �������� ��� ������
						else
						{
							stringstream ss;
							// ��������� �����
							ss << "213 " << fileSize.QuadPart << "\r\n";
							memset(sendBuff, 0, sizeof(sendBuff));
							strcpy(sendBuff, ss.str().c_str());
							send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
							// ������ � ���
							sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
							server->AddText(szMsgBuff);
						}
						// ��������� �������� ����������
						CloseHandle(hFile);
					}
					// ���� �� ��������
					else
					{
						// ����� �������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "413 error open file\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						server->AddText(szMsgBuff);
					}
					LeaveCriticalSection(&locker);
				}
				////////////////////////////////////////////// STOR //////////////////////////////////////////////
				else if(cmd.find("stor") == 0)
				{
					if(!session.cmd_port)
					{
						// ����������� �� ���� �������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "426 Data connection not specified.  A PORT/EPRT or PASV/EPSV command must be issued before executing this operation.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ��������� ����� � ���� �������
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						continue;
					}
					// PORT ������������
					session.cmd_port = false;
					// ���� ������� PORT �������������� ������� STOR
					string param;
					long pos = cmd.find(' ');
					// ���� ������� �������� ��������� - �������� ��
					if(pos != string::npos)
						param = cmd.substr(pos + 1, cmd.size());
					else
						param = "";
					// ��������� ����
					string serverPath;
					// ��������� ����
					string localPath;
					EnterCriticalSection(&locker);
					// �������� ���� � ��������� �������� ������������
					string home = server->accounts[session.Login].GetHome();
					// ������� ���� ���� �����
					if(home.at(home.size() - 1) == '\\')
						home.resize(home.size() - 1);
					// �������� ������� ������� � ������� ���������� ������ ������������
					string dir = session.curDir;
					// ��������� ����������� ���� ���� ��� ���
					if(dir.at(dir.size() - 1) != '/')
						dir += '/';
					// ���� ���������� ������� STOR ���
					if(param == "")
					{
						// ���������� ����� �������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "553 Error file name.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ������ � ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						server->AddText(szMsgBuff);						
						LeaveCriticalSection(&locker);
						continue;
					}
					LeaveCriticalSection(&locker);
					long separator = param.rfind('\t');
					string key = "";
					if(separator != string::npos)
					{
						// �������� ����
						key = param.substr(separator + 1);
						// �������� �������� � ����� �����
						param = param.substr(0, separator);
					}
					// ���� ����� ���������
					if(param.at(0) == '/')
					{
						serverPath = param;
						localPath = home + param;
					}
					// ����� ���� �� �������� ��������
					else
					{
						serverPath = dir + param;
						localPath = home + dir + param;
					}
					// ������������� �����
					for(size_t i = 0; i < localPath.size(); i++)
						if(localPath.at(i) == '/')
							localPath.at(i) = '\\';
					// ��������� ����
					HANDLE hFile = CreateFile(localPath.c_str(), 
						GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
						0, CREATE_ALWAYS, FILE_ATTRIBUTE_ENCRYPTED, 0);
					
					// ���� ��������
					if(hFile == INVALID_HANDLE_VALUE)
					{
						// ����� �������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "413 error create local file\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						CloseHandle(hFile);
						continue;
					}
					

					////// connection start //////
					// ������� ����� ���������� � ��������
					SOCKET dataConnection = socket(AF_INET, SOCK_STREAM, 0);
					if(dataConnection == SOCKET_ERROR)
					{
						// ���������� ����� ������ �������� ������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "451 Local error. Data socket create failed.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						CloseHandle(hFile);
						continue;
					}
					// ���� ����� ������
					sockaddr_in dest_addr;
					ZeroMemory((void*)&dest_addr, sizeof(dest_addr));
					// ��������� ��������� ������
					dest_addr.sin_family		=	AF_INET;
					dest_addr.sin_port			=	htons(session.remotePort);
					// ������� �������� ���������� ���� �����
					if(inet_addr(session.remoteIP.c_str()) != INADDR_NONE)
						dest_addr.sin_addr.S_un.S_addr	=	inet_addr(session.remoteIP.c_str());
					else
					{
						// ��������� ����� ������
						closesocket(dataConnection);
						// ��������� ������ � ����
						sprintf(szMsgBuff, "Error parse remote ip %s\r\n", session.remoteIP.c_str());
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						// ���������� ����� ������ �������� ������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "451 Local error. Remote addr parse fail.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ��������� ������ � ���
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						CloseHandle(hFile);
						// ��������� ��������� �������
						continue;
					}
					// ����������� � ��������
					if(connect(dataConnection, (sockaddr *)&dest_addr, sizeof(dest_addr)))
					{
						// ��������� ������ � ���
						sprintf(szMsgBuff, "Connect error: %d\r\n",WSAGetLastError());
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						// ���������� ����� �������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "425 Open data connection error. Connect fail.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ������ � ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						CloseHandle(hFile);
						closesocket(dataConnection);
						continue;
					}

					EnterCriticalSection(&locker);
					sprintf(szMsgBuff, "Connect to remote client success.\r\n");
					server->AddText(szMsgBuff);
					////// connection end //////
					// �������� � ������ ������ ������
					memset(sendBuff, 0, sizeof(sendBuff));
					strcpy(sendBuff, "150 Opening ASCII mode data connection.\r\n");
					send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
					// ��������� � ������ ��� 
					sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
					server->AddText(szMsgBuff);
					LeaveCriticalSection(&locker);

					// ��������� ���� ��� �����
					if(key.size() != 0)
					{
						HANDLE hKeyFile = CreateFile((localPath + ".crypt_key").c_str(), 
							GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							0, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM/*FILE_ATTRIBUTE_NORMAL*/, 0);
						// ���� ��������
						if(hKeyFile == INVALID_HANDLE_VALUE)
						{
							// ����� �������
							memset(sendBuff, 0, sizeof(sendBuff));
							strcpy(sendBuff, "413 error create local key file\r\n");
							send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
							// ����
							sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
							EnterCriticalSection(&locker);
							server->AddText(szMsgBuff);
							LeaveCriticalSection(&locker);
							CloseHandle(hFile);
							closesocket(dataConnection);
							continue;
						}
						DWORD dwBytesWritten = -1;
						WriteFile(hKeyFile, key.c_str(), sizeof(char) * key.size(), &dwBytesWritten, 0);
						CloseHandle(hKeyFile);
					}
					// ������� ������
					TCHAR readBuffer[PACK_LEN] = {0};
					DWORD dwBytesWritten = -1;
					DWORD dwRead = -1;

					try
					{
						// ������ ���� ������� �� 1024 ����
						while ((dwRead = recv(dataConnection, readBuffer, sizeof(TCHAR) * PACK_LEN, 0)) != INVALID_SOCKET
							&& dwBytesWritten
							&& WaitForSingleObject(server->stopEvent, 0) != WAIT_OBJECT_0)
						{	
							// ����������
							WriteFile(hFile, readBuffer, sizeof(TCHAR) * dwRead, &dwBytesWritten, 0);
							memset(readBuffer, 0, sizeof(TCHAR) * PACK_LEN);
						}
						closesocket(dataConnection);
						CloseHandle(hFile);
					}
					catch(...)
					{
						MessageBox(0, "catch", 0, 0);
					}
					// ���������� ��������
					memset(sendBuff, 0, sizeof(sendBuff));
					strcpy(sendBuff, "226 File recive OK.\r\n");
					send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
					// ����
					sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
					EnterCriticalSection(&locker);
					server->AddText(szMsgBuff);
					LeaveCriticalSection(&locker);
				}
				////////////////////////////////////////////// RETR //////////////////////////////////////////////
				else if(cmd.find("retr") == 0)
				{
					if(!session.cmd_port)
					{
						// ����������� �� ���� �������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "426 Data connection not specified.  A PORT/EPRT or PASV/EPSV command must be issued before executing this operation.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ��������� ����� � ���� �������
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						continue;
					}
					// PORT ������������
					session.cmd_port = false;
					// �������� ���������
					string param;
					long pos = cmd.find(' ');
					// ���� ������� �������� ��������� - �������� ��
					if(pos != string::npos)
						param = cmd.substr(pos + 1, cmd.size());
					else
						param = "";
					// ��������� ����
					string serverPath;
					// ��������� ����
					string localPath;
					EnterCriticalSection(&locker);
					// �������� ���� � ��������� �������� ������������
					string home = server->accounts[session.Login].GetHome();
					// ������� ���� ���� �����
					if(home.at(home.size() - 1) == '\\')
						home.resize(home.size() - 1);
					// �������� ������� ������� � ������� ���������� ������ ������������
					string dir = session.curDir;
					// ��������� ����������� ���� ���� ��� ���
					if(dir.at(dir.size() - 1) != '/')
						dir += '/';
					// ���� ���������� ������� RETR ���
					if(param == "")
					{
						// ���������� ����� �������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "553 Error file name.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ������ � ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						server->AddText(szMsgBuff);						
						LeaveCriticalSection(&locker);
						continue;
					}
					LeaveCriticalSection(&locker);
					// ���� ����� ���������
					if(param.at(0) == '/')
					{
						serverPath = param;
						localPath = home + param;
					}
					// ����� ���� �� �������� ��������
					else
					{
						serverPath = dir + param;
						localPath = home + dir + param;
					}
					// ������������� �����
					for(size_t i = 0; i < localPath.size(); i++)
						if(localPath.at(i) == '/')
							localPath.at(i) = '\\';
					// ��������� ���������� �� ����
					if(!PathFileExists(localPath.c_str()))
					{
						// ����� �������
						memset(sendBuff, 0, sizeof(sendBuff));			
						sprintf(sendBuff, "550 File %s not found.\r\n", serverPath.c_str());
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ������ � ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						continue;
					}
					// ��������� ����
					HANDLE hFile = CreateFile(localPath.c_str(), GENERIC_READ, 
						FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
					// ���� ��������
					if(hFile == INVALID_HANDLE_VALUE)
					{
						// ����� �������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "413 error open local file\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						CloseHandle(hFile);
						continue;
					}

					// �������� ���� ��� �����
					string curKey = "";
					HANDLE hKeyFile = CreateFile((localPath + ".crypt_key").c_str(), GENERIC_READ, 
						FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN, 0);
					if(hKeyFile != INVALID_HANDLE_VALUE)
					{
						TCHAR buff[1024] = {0};
						DWORD dwRead;
						ReadFile(hKeyFile, buff, 1024, &dwRead, 0);
						curKey.resize(strlen(buff));
						strcpy((char*)curKey.c_str(), buff);
						CloseHandle(hKeyFile);
						//MessageBox(0, curKey.c_str(), 0, 0);
					}


					
					////// connection start //////
					// ������� ����� ���������� � ��������
					SOCKET dataConnection = socket(AF_INET, SOCK_STREAM, 0);
					if(dataConnection == SOCKET_ERROR)
					{
						// ���������� ����� ������ �������� ������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "451 Local error. Data socket create failed.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						CloseHandle(hFile);
						continue;
					}
					// ���� ����� ������
					sockaddr_in dest_addr;
					ZeroMemory((void*)&dest_addr, sizeof(dest_addr));
					// ��������� ��������� ������
					dest_addr.sin_family		=	AF_INET;
					dest_addr.sin_port			=	htons(session.remotePort);
					// ������� �������� ���������� ���� �����
					if(inet_addr(session.remoteIP.c_str()) != INADDR_NONE)
						dest_addr.sin_addr.S_un.S_addr	=	inet_addr(session.remoteIP.c_str());
					else
					{
						// ��������� ����� ������
						closesocket(dataConnection);
						// ��������� ������ � ����
						sprintf(szMsgBuff, "Error parse remote ip %s\r\n", session.remoteIP.c_str());
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						// ���������� ����� ������ �������� ������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "451 Local error. Remote addr parse fail.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ��������� ������ � ���
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						CloseHandle(hFile);
						// ��������� ��������� �������
						continue;
					}
					// ����������� � ��������
					if(connect(dataConnection, (sockaddr *)&dest_addr, sizeof(dest_addr)))
					{
						// ��������� ������ � ���
						sprintf(szMsgBuff, "Connect error: %d\r\n",WSAGetLastError());
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						// ���������� ����� �������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "425 Open data connection error. Connect fail.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ������ � ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						CloseHandle(hFile);
						closesocket(dataConnection);
						continue;
					}
					// ����. ������
					EnterCriticalSection(&locker);
					// ����
					sprintf(szMsgBuff, "Connect to remote client success.\r\n");
					server->AddText(szMsgBuff);
					// �������� � ������ �������� ������
					memset(sendBuff, 0, sizeof(sendBuff));
					strcpy(sendBuff, "150 Opening ASCII mode data connection.\r\n");
					send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
					// ��������� � ������ ��� 
					sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
					server->AddText(szMsgBuff);
					LeaveCriticalSection(&locker);
					////// Send file start //////
					TCHAR readBuffer[PACK_LEN] = {0};
					DWORD dwBytesWritten = 0;
					DWORD dwRead = -1;
					// ������ ���� ������� �� 1024 ����
					int keyPos = 0;

					sprintf(szMsgBuff, "file->%s, decryptKey->%s", localPath.c_str(), ((curKey != "") ? curKey.c_str() : "empty"));
					EnterCriticalSection(&locker);
					server->AddText(szMsgBuff);
					LeaveCriticalSection(&locker);

					while (dwRead && dwBytesWritten != SOCKET_ERROR
						&& WaitForSingleObject(server->stopEvent, 0) != WAIT_OBJECT_0)
					{
						memset(readBuffer, 0, sizeof(TCHAR) * PACK_LEN);
						// ������ ����
						ReadFile(hFile, readBuffer, sizeof(TCHAR) * PACK_LEN, &dwRead, 0);
						if(curKey != "")
						{
							////// decrypt section start //////	
							for(size_t i = 0; i < dwRead; i++, keyPos++)
							{
								if(keyPos == curKey.size())
									keyPos = 0;
								readBuffer[i] ^= curKey.at(keyPos);
							}
							////// decrypt section end //////
						}
						// ����������
						dwBytesWritten = send(dataConnection, readBuffer, sizeof(TCHAR) * dwRead, 0);
					}
					closesocket(dataConnection);
					CloseHandle(hFile);
					////// Send file end //////
					// ���������� ��������
					memset(sendBuff, 0, sizeof(sendBuff));
					strcpy(sendBuff, "226 File send OK.\r\n");
					send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
					// ����
					sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
					EnterCriticalSection(&locker);
					server->AddText(szMsgBuff);
					LeaveCriticalSection(&locker);					
				}
				////////////////////////////////////////////// MKD //////////////////////////////////////////////
				else if(cmd.find("mkd") == 0)
				{
					// �������� ���������
					string param;
					long pos = cmd.find(' ');
					// ���� ������� �������� ��������� - �������� ��
					if(pos != string::npos)
						param = cmd.substr(pos + 1, cmd.size());
					// ���� ���������� ������� MKD ���
					else
					{
						// ���������� ����� �������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "553 Error directory name.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ������ � ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);						
						LeaveCriticalSection(&locker);
						continue;
					}
					// ��������� ����
					string serverPath;
					// ��������� ����
					string localPath;
					// �������� ���� � ��������� �������� ������������
					EnterCriticalSection(&locker);
					string home = server->accounts[session.Login].GetHome();
					LeaveCriticalSection(&locker);
					// ������� ���� ���� �����
					if(home.at(home.size() - 1) == '\\')
						home.resize(home.size() - 1);
					// �������� ������� ������� � ������� ���������� ������ ������������
					string dir = session.curDir;
					// ��������� ����������� ���� ���� ��� ���
					if(dir.at(dir.size() - 1) != '/')
						dir += '/';
					// ���� ����� ���������
					if(param.at(0) == '/')
					{
						serverPath = param;
						localPath = home + param;
					}
					// ����� ���� �� �������� ��������
					else
					{
						serverPath = dir + param;
						localPath = home + dir + param;
					}
					// ������������� �����
					for(size_t i = 0; i < localPath.size(); i++)
						if(localPath.at(i) == '/')
							localPath.at(i) = '\\';

					if(!CreateDirectory(localPath.c_str(), 0) && GetLastError() != ERROR_ALREADY_EXISTS)
					{
						// ���������� ����� �������
						memset(sendBuff, 0, sizeof(sendBuff));
						strcpy(sendBuff, "550 Directory create fail.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ������ � ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);						
						LeaveCriticalSection(&locker);
						continue;
					}
					// ���������� ����� �������
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "257 Directory \"%s\" was created.\r\n", serverPath.c_str());
					send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
					// ������ � ����
					sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
					EnterCriticalSection(&locker);
					server->AddText(szMsgBuff);						
					LeaveCriticalSection(&locker);
				}
				////////////////////////////////////////////// CWD //////////////////////////////////////////////
				else if(cmd.find("cwd") == 0)
				{
					long pos = cmd.find(' ');
					// ���������� ���
					if(pos == string::npos)
					{
						// ����� �������
						memset(sendBuff, 0, sizeof(sendBuff));			
						strcpy(sendBuff, "550 No such file or directory.\r\n");
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ������ � ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						continue;
					}
					// �������� ��������� �������
					string param = cmd.substr(pos + 1);
					// ������� �� ������� ����
					if(param.find("..") != string::npos)
					{
						string dir = session.curDir;
						// ���������� ��������� ����
						if(dir.at(dir.size() - 1) == '/')
							dir.resize(dir.size() - 1);
						// ���� ���������� ����
						long i = dir.rfind("/");
						// �������� ����� ����
						if(i != string::npos)
							dir = dir.substr(0, i + 1);
						else
							dir = "/";
						// ����� ���� ��� �������� ������������
						session.curDir = dir;
						// ��������� �������� ���������
						string msg = "257 \"" + session.curDir + "\"" + " is current directory\r\n";
						// ����� �������
						memset(sendBuff, 0, sizeof(sendBuff));			
						strcpy(sendBuff, msg.c_str());
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ������ � ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						continue;
					}
					// ��������� ����
					string path;
					// �������� ������� ������� � ������� ���������� ������ ������������
					string dir = session.curDir;
					// ��������� ����������� ���� ���� �����
					if(dir.at(dir.size() - 1) != '/')
						dir += '/';
					// ��������� ��������� ������� CWD
					// ���� ����� ���������
					if(param.at(0) == '/')
					{
						path = param;
						if(path.at(path.size() - 1) != '/')
							path += '/';
					}
					// ����� ���� �� �������� ��������
					else
						path = dir + param;
					
					// ������� ������� ����� ���� ����� ����
					string temp = "";
					for(size_t i = 0; i < path.size(); i++)
					{
						if(temp.size() != 0)
						{
							if(path.at(i) == '/' && temp.at(temp.size() - 1) == '/')
								continue;
							else
								temp += path.at(i);
						}
						else
						{
							temp += path.at(i);
						}
					}
					path = temp;
					// �������� ���� � ��������� ��������
					EnterCriticalSection(&locker);
					string home = server->accounts[session.Login].GetHome();
					LeaveCriticalSection(&locker);
					// ������� ����
					if(home.at(home.size() - 1) == '\\')
						home.resize(home.size() - 1);					
					// 
					string testPath = home;
					for(size_t i = 0; i < path.size(); i++)
					{
						if(path.at(i) == '/')
							testPath += '\\';
						else
							testPath += path.at(i);
					}
					// �������� ������������� ����������
					DWORD dwAttrib = GetFileAttributes(testPath.c_str());
					// ��������� ������� ��������� ��������
					if( !(dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) )
					{
						// ����� �������
						memset(sendBuff, 0, sizeof(sendBuff));			
						sprintf(sendBuff, "550 Directory %s not found.\r\n", path.c_str());
						send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
						// ������ � ����
						sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
						EnterCriticalSection(&locker);
						server->AddText(szMsgBuff);
						LeaveCriticalSection(&locker);
						continue;
					}
					session.curDir = path;
					// ��������� �������� ���������
					string msg = "257 \"" + session.curDir + "\"" + " is current directory\r\n";
					// ����� �������
					memset(sendBuff, 0, sizeof(sendBuff));			
					strcpy(sendBuff, msg.c_str());
					send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
					// ������ � ����
					sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
					EnterCriticalSection(&locker);
					server->AddText(szMsgBuff);
					LeaveCriticalSection(&locker);
				}
				////////////////////////////////////////////// PWD //////////////////////////////////////////////
				else if(cmd.find("pwd") == 0 )
				{
					// ��������� ����� �������
					string msg = "257 \"" + session.curDir + "\"" + " is current directory\r\n";
					memset(sendBuff, 0, sizeof(sendBuff));
					strcpy(sendBuff, msg.c_str());
					send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
					// �����������
					sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
					EnterCriticalSection(&locker);
					server->AddText(szMsgBuff);
					LeaveCriticalSection(&locker);
				}
				// ������� �� ��������������
				else
				{
					// ����� �������
					memset(sendBuff, 0, sizeof(sendBuff));
					strcpy(sendBuff, "202 Command not support.\r\n");
					send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
					// ������ � ����
					sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
					EnterCriticalSection(&locker);
					server->AddText(szMsgBuff);
					LeaveCriticalSection(&locker);
				}
			}
			// ��� ���������� ������� ��������� �����������
			else if(!session.cmd_user || !session.cmd_pass)
			{
				// ���������� ����� ���������� �����������
				memset(sendBuff, 0, sizeof(sendBuff));
				strcpy(sendBuff, "530 not logged in.\r\n");
				send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
				// �����������
				sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
				EnterCriticalSection(&locker);
				server->AddText(szMsgBuff);
				LeaveCriticalSection(&locker);
			}
			// ������� �� ����������
			else 
			{
				// ������� �� ��������������
				memset(sendBuff, 0, sizeof(sendBuff));
				strcpy(sendBuff, "502 Command not support.\r\n");
				send(session.clientSocket, sendBuff, strlen(sendBuff), 0);
				// �����������
				sprintf(szMsgBuff, "[%s:%s] <= %s", session.IP.c_str(), session.PORT.c_str(), sendBuff);
				EnterCriticalSection(&locker);
				server->AddText(szMsgBuff);
				LeaveCriticalSection(&locker);
			}
			cmd.clear();
			cmdComplete = false;
			memset(reciveBuff, 0, sizeof(TCHAR) * PACK_LEN);
			memset(sendBuff, 0, sizeof(TCHAR) * PACK_LEN);
		}
		
    }
	// ������ � ����������� ������
	EnterCriticalSection(&locker);
	// ��������� ���������� �������� �������������
	server->userOnline--;
	// ��������� ���� ������
	sprintf(szMsgBuff, "������ %d", server->userOnline);
	// ������� ������� ���������� ������������� � ������ ���
	SendMessage(server->hStatusBar, SB_SETTEXT, (WPARAM)(INT) 0 | 0, (LPARAM) szMsgBuff);
	// ��������� �������������� ������
	sprintf(szMsgBuff, "Disconnected ip = %s port = %s\r\n", session.IP.c_str(), session.PORT.c_str());
	// ��������� ��������� � ������� ���������� � ����
	server->AddText(szMsgBuff);
	// ������� ������
	server->sessions.erase(session.clientSocket);
	// �������� ����������� ������
	LeaveCriticalSection(&locker);
	// ��������� ���������� �����
	closesocket(session.clientSocket);
    return 0;
}
// ��������� ��������� �������
DWORD WINAPI ServerThread(LPVOID lpParam)
{
	// �������� ��������� �� ��������� ������
	Server *server = (Server*)lpParam;
	// ����� ��� ���������
	TCHAR szMsgBuff[MAX_PATH];
    SOCKET        serverSocket,
                  clientSocket;
    struct sockaddr_in localaddr,
                       clientaddr;
    HANDLE        hThread;
    DWORD         dwThreadId;
    int           iSize;
	EnterCriticalSection(&locker);
	int port = server->serverSettings.servPort;
	LeaveCriticalSection(&locker);
	// ������� ��������� �����
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	// ��������� ��������
    if (serverSocket == SOCKET_ERROR)
    {
		sprintf(szMsgBuff, "Socket error %d\n", WSAGetLastError());
		MessageBox(0, szMsgBuff, "Error", MB_OK | MB_ICONEXCLAMATION);
		WSACleanup();
		return -1;
    }
	// ��������� ���� ���������
    localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(port);
	// ������ �����
    if (bind(serverSocket, (struct sockaddr *)&localaddr, 
            sizeof(localaddr)) == SOCKET_ERROR)
    {
		sprintf(szMsgBuff, "Bind socket error %d\n", WSAGetLastError());
		MessageBox(0, szMsgBuff, "Error", MB_OK | MB_ICONEXCLAMATION);
		closesocket(serverSocket);
		WSACleanup();
		return -1;
    }
    // ����������� ���������
	if(listen(serverSocket, 4))
	{
		sprintf(szMsgBuff, "Error listen %d\n", WSAGetLastError());
		MessageBox(0, szMsgBuff, "Error", MB_OK | MB_ICONEXCLAMATION);
		closesocket(serverSocket);
		WSACleanup();
		return -1;
	}
	// ������������� ������ ���������
	SetEvent(server->startEvent);
	// ������� ����
    while (WaitForSingleObject(server->stopEvent, 0) != WAIT_OBJECT_0)
    {
        iSize = sizeof(clientaddr);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientaddr,
                        &iSize);  

		if(WaitForSingleObject(server->stopEvent, 10) == WAIT_OBJECT_0)
		{
			closesocket(clientSocket);
			break;
		}
		// ��������� ���������� ������ ����������
		if(server->userOnline >= server->serverSettings.maxConnections)
		{
			// ���������� ���������� ������������� ������ �����
			if(clientSocket != INVALID_SOCKET)
				closesocket(clientSocket);
		}
		else
		{
			// ��������� �������� ������
			if (clientSocket == INVALID_SOCKET)
			{        
				MessageBox(0, "�������� ������ ���������� �������� � �������.\r\n ��� ������: %d", "������", GetLastError());
				break;
			}
			// ������ � ����������� ������
			EnterCriticalSection(&locker);
			// ������� ������ ��� �������� �����������
			Session session;			
			// ��������� ���� �����������
			session.IP = inet_ntoa(*(in_addr*)(&clientaddr.sin_addr));
			session.PORT = to_string(ntohs(clientaddr.sin_port));
			session.clientSocket = clientSocket;
			// ��������� � ������
			server->sessions[clientSocket] = session;
			// ����������� ������ ������� � ���������� �����
			ThreadPackage package;
			package.serv = server;
			package.clientSocket = clientSocket;
			// �������� ����������� ������
			LeaveCriticalSection(&locker);
			// ������� ����� ���������������� �����
			hThread = CreateThread(0, 0, ClientThread, &package, 0, &dwThreadId);
			// ��������� �������� ������
			if (hThread == NULL)
			{
				MessageBox(0, "������ �������� ����������������� ������.\r\n ��� ������: %d", "������", GetLastError());
				break;
			}
			// ���������� �� ��������
			CloseHandle(hThread);
		}
    }
	// ��������� ��������� �����
    closesocket(serverSocket);
    return 0;
}