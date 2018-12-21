#pragma once

#include "Headers.h"
#include "Settings.h"
#include "Account.h"
#include "Session.h"

class Server
{
public:
	// Конструктор по умолчанию
	Server();
	// Деструктор
	~Server();
	// Добавить текст в поле вывода
	void AddText (const TCHAR* text, bool same_row = false);
	// Задать идентификаторы контролов
	void SetControls(HWND _hTextArea, HWND _hStatusBar, HWND _hWndMain);
	// Загрузить аккаунты
	BOOL LoadAccounts();
	// Сохранить аккаунты
	BOOL SaveAccounts();
	// Запустить сервер
	BOOL Start(Settings &setup);
	// Остановить сервер
	void Stop();
	// Запущен ли сервер
	bool IsStart();	
	// Очистить список логов
	void ClearLogs();
	// Преобразовать из UTF8 в CP1251
	string Utf8_to_cp1251(const char *str);
	// Преобразовать из CP1251 в UTF8
	string cp1251_to_utf8(const char *str);
	// Аккаунты
	map<string, Account> accounts;
	// Сессии клиентских соединений
	map<SOCKET, Session> sessions;
private:
	// Структура библиотеки WinSock
	WSAData wsaData;
	// Идентификаторы элементов управления
	HWND hWndMain, hTextArea, hStatusBar;	
	// Настройки сервера
	Settings serverSettings;
	// Запущен сервер или остановлен
	bool serverStatus;	
	// Буфер для ошибок
	char szErrorBuff[MAX_PATH];
	// Буфер для сообщений
	char szMsgBuff[MAX_PATH];
	// Серверная потоковая функция
	friend DWORD WINAPI ServerThread(LPVOID lpParam);
	// Клиентская потоковая функция
	friend DWORD WINAPI ClientThread(LPVOID lpParam);
public:
	HANDLE serverThread;
	HANDLE stopEvent, startEvent;
	// Хендл файла логов
	HANDLE LogToFile;
	// Хендл файла ключей
	HANDLE KeysFile;
	// Количество клиентов
	int userOnline;
	// Количество строк записей сервера
	long curRow;
};