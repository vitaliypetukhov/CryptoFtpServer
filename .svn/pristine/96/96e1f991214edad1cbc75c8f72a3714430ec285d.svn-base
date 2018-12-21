#pragma once

#include "Settings.h"
// Конструктор
Settings::Settings()
{
	msgWelcome = "FTP test server";
	servPort = 21;
	maxConnections = 10;
	fileLog = true;
	// Путь к файлу настроек
	cmdLine = new char[MAX_PATH];
	// Получаем путь
	GetModuleFileName(GetModuleHandle(NULL), cmdLine, MAX_PATH);
	// Отсекаем лишние символы
    *strrchr(cmdLine, '\\') = NULL;
}
// Конструктор копии
Settings::Settings(const Settings &href)
{
	msgWelcome = href.msgWelcome;
	servPort = href.servPort;
	maxConnections = href.maxConnections;
	fileLog = href.fileLog;
	// Путь к файлу настроек
	cmdLine = new char[MAX_PATH];
	strcpy(cmdLine, href.cmdLine);
}
// Деструктор
Settings::~Settings()
{
	delete []cmdLine;
}
// Оператор присваивания
Settings &Settings::operator=(const Settings &href)
{
	if(this == &href)
		return *this;
	msgWelcome = href.msgWelcome;
	servPort = href.servPort;
	maxConnections = href.maxConnections;
	fileLog = href.fileLog;
	delete []cmdLine;
	// Путь к файлу настроек
	cmdLine = new char[MAX_PATH];
	strcpy(cmdLine, href.cmdLine);
	return *this;
}
// Загрузка настроек из файла
BOOL Settings::Load()
{
	char path[MAX_PATH] = {0};
	char buff[MAX_PATH] = {0};
	// Формируем строку пути к файлу настроек
	strcpy(path, cmdLine);
	strcat(path, "\\server.ini");
	// Читаем строку приветствия
	GetPrivateProfileString("server settings", "welcome_message", "fail", buff, MAX_PATH, path);
	// Проверяем чтение строки
	if(strcmp(buff, "fail") == 0)
		return FALSE;
	// Записываем строку приветствия сервера
	strcpy((char*)msgWelcome.c_str(), buff);
	// Очищаем буферную строку
	memset(buff, 0, sizeof(char) * MAX_PATH);
	// Читаем серверный порт
	GetPrivateProfileString("server settings", "server_port", "fail", buff, MAX_PATH, path);
	// Проверяем чтение строки
	if(strcmp(buff, "fail") == 0)
		return FALSE;
	// Получаем порт для прослушки сервером
	servPort = atoi(buff);
	if(servPort == 0)
		return FALSE;
	// Очищаем буферную строку
	memset(buff, 0, sizeof(char) * MAX_PATH);
	// Читаем макс количество подключений
	GetPrivateProfileString("server settings", "max_connections", "fail", buff, MAX_PATH, path);
	// Проверяем чтение строки
	if(strcmp(buff, "fail") == 0)
		return FALSE;
	// Задаем максимальное количество соединений
	maxConnections = atoi(buff);
	if(maxConnections == 0)
		return FALSE;
	// Читаем
	GetPrivateProfileString("server settings", "log_to_file", "fail", buff, MAX_PATH, path);
	if(strcmp(buff, "fail") == 0)
		return FALSE;
	if(strcmp(buff, "true") == 0)
		fileLog = true;
	else if(strcmp(buff, "false") == 0)
		fileLog = false;
	else
		return FALSE;
	// Все настройки считаны
	return TRUE;
}
// Сохранение настроек в файл
BOOL Settings::Save()
{
	char path[MAX_PATH] = {0};
	char buff[MAX_PATH] = {0};
	// Формируем строку пути к файлу настроек
	strcpy(path, cmdLine);
	strcat(path, "\\server.ini");
	// Записываем в файл строку приветствия
	if(!WritePrivateProfileString("server settings", "welcome_message", msgWelcome.c_str(), path))
		return FALSE;
	// Формируем строку номера порта
	sprintf(buff, "%d", servPort);
	// Записываем порт сервера
	if(!WritePrivateProfileString("server settings", "server_port", buff, path))
		return FALSE;
	// Формируем строку макс. соединений
	sprintf(buff, "%d", maxConnections);
	// Записываем макс. количество в файл настроек
	if(!WritePrivateProfileString("server settings", "max_connections", buff, path))
		return FALSE;
	// Формируем строку маркера дублирования логов в файл
	sprintf(buff, "%s", fileLog ? "true" : "false");
	// Записываем макс. количество в файл настроек
	if(!WritePrivateProfileString("server settings", "log_to_file", buff, path))
		return FALSE;
	// Все прошло успешно
	return TRUE;
}
// Применение настроек по умолчанию
void Settings::Default()
{
	msgWelcome = "FTP test server";
	servPort = 21;
	maxConnections = 10;
	fileLog = true;
}