#pragma once

#include "Headers.h"

// �������� ����
HWND ServerCreateWindow(HINSTANCE, WNDPROC, LPCSTR, LPCSTR, long, long);
// �������� ������
BOOL ServerCreateButtons(HINSTANCE, HWND, HWND &, HWND &, HWND &, HWND &, HWND &);
// �������� ���� ������ �������� ������� � ������� ������� (�����������)
BOOL ServerCreateTextArea(HINSTANCE, HWND, HWND &);
// ������ ���
BOOL ServerCreateStatusBar(HINSTANCE, HWND, int, int, HWND &);
