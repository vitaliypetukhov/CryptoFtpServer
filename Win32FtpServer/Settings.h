#pragma once

#include "Headers.h"

class Settings
{
public:
	// �����������
	Settings();
	// ����������� �����
	Settings(const Settings&);
	// ����������
	~Settings();
	// �������� �������� �� ����� ������������
	BOOL Load();
	// ���������� �������� � ���� ������������
	BOOL Save();
	// ��������� �������� �� ���������
	void Default();
	// �������� ������������
	Settings &Settings::operator=(const Settings &href);
	// ������ �����������
	string msgWelcome;
	// ���� ������� ������� ������
	int servPort;
	// ������������ ����� ����������
	int maxConnections;
	// ���������� � ����
	bool fileLog;
	const char * getPath() const { return cmdLine; }
private:
	// ���� � ����������� ��������
	char *cmdLine;
};