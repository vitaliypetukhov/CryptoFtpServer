#pragma once
#include <string>
using namespace std;

// Сессия соединения
class Session
{
public:
	Session()
	{
		clientSocket = 0;
		cmd_user = 0;
		cmd_pass = 0;
		cmd_port = 0;
		curDir = "/";
		Login = "";
		IP = "qwe";
		PORT = "";
		remoteIP = "";
		remotePort = 0;
	}
	Session(const Session &href)
	{
		clientSocket = href.clientSocket;
		cmd_user = href.cmd_user;
		cmd_pass = href.cmd_pass;
		cmd_port = href.cmd_port;
		curDir = href.curDir;
		Login = href.Login;
		IP = href.IP;
		PORT = href.PORT;			
		remoteIP = href.remoteIP;
		remotePort = href.remotePort;
	}
	Session &operator = (const Session &href)
	{
		if(&href == this)
			return *this;
		clientSocket = href.clientSocket;
		cmd_user = href.cmd_user;
		cmd_pass = href.cmd_pass;
		cmd_port = href.cmd_port;
		curDir = href.curDir;
		Login = href.Login;
		IP = href.IP;
		PORT = href.PORT;			
		remoteIP = href.remoteIP;
		remotePort = href.remotePort;
		return *this;
	}
	// Клиентский сокет
	SOCKET clientSocket;
	// Маркеры команд
	bool cmd_user, cmd_pass, cmd_port;
	// Текущий каталог на сервере
	string curDir;
	// Имя пользователя
	string Login;
	// Адрес и порт клиента
	string IP, PORT;
	// Параметры соединения полученные после команды "PORT"
	string remoteIP;
	int remotePort;
};