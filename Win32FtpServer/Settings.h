#pragma once

#include "Headers.h"

class Settings
{
public:
	// Конструктор
	Settings();
	// Конструктор копии
	Settings(const Settings&);
	// Деструктор
	~Settings();
	// Загрузка настроек из файла конфигурации
	BOOL Load();
	// Сохранение настроек в файл конфигурации
	BOOL Save();
	// Установка настроек по умолчанию
	void Default();
	// Оператор присваивания
	Settings &Settings::operator=(const Settings &href);
	// Строка приветствия
	string msgWelcome;
	// Порт который слушает сервер
	int servPort;
	// Максимальное число соединений
	int maxConnections;
	// Логировать в файл
	bool fileLog;
	const char * getPath() const { return cmdLine; }
private:
	// Путь к раположению настроек
	char *cmdLine;
};