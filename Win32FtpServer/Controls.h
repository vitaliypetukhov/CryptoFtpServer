#pragma once

#include "Headers.h"

// Создание окна
HWND ServerCreateWindow(HINSTANCE, WNDPROC, LPCSTR, LPCSTR, long, long);
// Создание кнопок
BOOL ServerCreateButtons(HINSTANCE, HWND, HWND &, HWND &, HWND &, HWND &, HWND &);
// Создание поля вывода запросов клиента и ответов сервера (Логирование)
BOOL ServerCreateTextArea(HINSTANCE, HWND, HWND &);
// Статус бар
BOOL ServerCreateStatusBar(HINSTANCE, HWND, int, int, HWND &);
