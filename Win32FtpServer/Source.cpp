#include "Headers.h"
#include "Controls.h"
#include "Settings.h"
#include "Account.h"
#include "Server.h"


// Параметры окна
const int WindowWidth = 750;
const int WindowHeight = 460;
// Заголовок окна
char szWindowName[MAX_PATH] = "FTP test Server";
// Название класса приложения
char szClassName[MAX_PATH] = "FTP test Server Class";
// Буфер для вывода ошибок
char szErrorBuff[MAX_PATH] = {0};
// Настройки сервера
Settings srvSetup;
// Объект сервера
Server ftpServer;

// Оконная процедура
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
// Калбек процедура диалогового окна акаунтов
BOOL CALLBACK AccDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
// Калбек процедура диалогового окна добавления аккаунта
BOOL CALLBACK AccAddDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AccEditDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
// Калбек процедура диалогового окна настройки сервера
BOOL CALLBACK SetupDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Хендлы элментов
HWND hWndMain = 0, hBtnStart = 0, hBtnAccounts = 0, hBtnSettings = 0, hBtnHelp = 0, hBtnExit = 0, 
	hTextArea = 0, hStatusBar = 0;

HWND hUsers = 0;
HWND hAccountList = 0;

// Главная функция
int WINAPI WinMain(HINSTANCE hInstance, 
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{
	// Получаем дескриптор приложения
	hInstance				= GetModuleHandle(NULL);
		
	if((hWndMain = ServerCreateWindow(hInstance, WndProc, 
								szClassName, szWindowName,
								WindowWidth, WindowHeight)) == 0)
	{
		MessageBox( NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION );
		// Ошибка создания окна
		return false;
	}
	// Показать окно
	ShowWindow(hWndMain, SW_SHOW);
	// Слегка повысим приоритет
	SetForegroundWindow(hWndMain);
	// Установить фокус клавиатуры на наше окно
	SetFocus(hWndMain);
	///////////////////////////////// Создание компонентов ///////////////////////////////////
	if(!ServerCreateButtons(hInstance, hWndMain, hBtnStart, hBtnAccounts, hBtnSettings, hBtnHelp, hBtnExit) ||
		!ServerCreateTextArea(hInstance, hWndMain, hTextArea) ||
		!ServerCreateStatusBar(hInstance, hWndMain, 0, 2, hStatusBar))
	{
		MessageBox(0, "Error Create Server Components", "Error initialize components", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	// Сообщение
	MSG Msg;
	// Цикл выборки сообщений
	while(GetMessage(&Msg, 0, 0, 0))
	{
		
		switch (Msg.message)
        {
            case WM_KEYDOWN:
				if(LOWORD(Msg.wParam) == VK_ESCAPE)
					PostQuitMessage(0);    
            break;	
		case WM_SYSCHAR:
		case WM_SYSKEYUP:
		case WM_SYSKEYDOWN:
			if(LOWORD(Msg.wParam) == VK_ESCAPE)
			{
				PostQuitMessage(0);
				return 0;	
			}
        }
		
		if(!IsDialogMessage(hWndMain, &Msg))
        {
            TranslateMessage( &Msg );   
            DispatchMessage( &Msg );
		}
	}
	return Msg.wParam;
}


// Оконная процедура
LRESULT CALLBACK WndProc(HWND hWnd, 
						 UINT uMsg, 
						 WPARAM wParam, 
						 LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_SYSCHAR:
	case WM_SYSKEYUP:
	case WM_SYSKEYDOWN:
		if(LOWORD(wParam) == VK_ESCAPE)
		{
			if(ftpServer.IsStart())
				ftpServer.Stop();
			PostQuitMessage(0);
		}
		return 0;
	case WM_INITDIALOG:	
		return 0;
	case WM_CONTEXTMENU:
		{
			HMENU hMenu = CreatePopupMenu();
			AppendMenu( hMenu, MFT_STRING, IDM_CUST_1, "&Очистить логи" );
			//AppendMenu( hMenu, MFT_STRING, IDM_CUST_2, "&Справка" );
			//AppendMenu( hMenu, MFT_SEPARATOR, 0, NULL );
			//AppendMenu( hMenu, MFT_STRING, IDM_EXIT, "В&ыход" );
			
			TrackPopupMenu( hMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, 
				LOWORD( lParam ), HIWORD( lParam ), 0, hWnd, NULL ); 

			DestroyMenu( hMenu );
		}
		break;
	case WM_ACTIVATE:
		return 0;
	case WM_COMMAND:
		{
			// Обработка пункта меню
			if(wParam == IDM_CUST_1)
			{
				// Чистим список логов
				ftpServer.ClearLogs();
				return 0;
			}
			// Обработка пункта меню
			if(wParam == IDM_CUST_2)
			{
				// Чистим список логов
				MessageBox(hTextArea, "Справка", "Информация", MB_OK | MB_ICONINFORMATION);
				return 0;
			}

			if((HWND)lParam == hBtnStart)
			{
				ftpServer.SetControls(hTextArea, hStatusBar, hWndMain);
				if(ftpServer.IsStart())
				{
					ftpServer.Stop();
					SetWindowText(hBtnStart, "Старт");
				}
				else
				{
					if(!srvSetup.Load())
					{
						MessageBox(0, "Ошибка загрузки файла настроек.\r\n Созданы настройки по умолчанию", "Ошибка чтения файла конфигурации", MB_OK | MB_ICONEXCLAMATION);
						srvSetup.Default();
						srvSetup.Save();
						return 0;
					}
					if(ftpServer.Start(srvSetup))
						SetWindowText(hBtnStart, "Стоп");
					else
						MessageBox(0, "Ошибка запуска сервера.", "Ошибка", MB_OK | MB_ICONEXCLAMATION);
				}
			}
			if((HWND)lParam == hBtnAccounts)
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_ACC_DIALOG), hWndMain, AccDlgProc);
			}
			if((HWND)lParam == hBtnSettings)
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_SETTINGS_DIALOG), hWndMain, SetupDlgProc);
			}
			if((HWND)lParam == hBtnHelp)
			{
				MessageBox(hWnd, "Справка", "Справка", MB_OK | MB_ICONASTERISK);
			}
			if((HWND)lParam == hBtnExit)
			{
				if(ftpServer.IsStart())
					ftpServer.Stop();
				PostQuitMessage(0);
			}
		}
		return 0;
	case WM_NOTIFY:
        {
		}
		return 0;
	case WM_DESTROY:
		{
			if(ftpServer.IsStart())
				ftpServer.Stop();
			PostQuitMessage(0);
		}
		return  0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
// Калбек процедура диалогового окна акаунтов
BOOL CALLBACK AccDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Обрабатываем сообщение
	switch(uMsg) 
	{
		case WM_COMMAND:
		{
			// Добавить акаунт
			if((HWND)lParam == GetDlgItem(hWnd, IDC_BTN_ADD))
				DialogBox(0, MAKEINTRESOURCE(IDD_ACC_ADD), hWnd, AccAddDlgProc);
			// Редактировать
			if((HWND)lParam == GetDlgItem(hWnd, IDC_BTN_EDIT))
			{
				int ItemIndex = SendMessage(hAccountList, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
				if(ItemIndex == -1)
				{
					MessageBox(hWnd, "Выберите аккаунт для редактирования", "Ошибка выделения", MB_OK | MB_ICONEXCLAMATION);
					return 0;
				}
				TCHAR BUFF[MAX_PATH] = {0};
				ListView_GetItemText(hAccountList, ItemIndex, 0, BUFF, MAX_PATH);

				DialogBox(0, MAKEINTRESOURCE(IDD_ACC_ADD), hWnd, AccEditDlgProc);
			}
			// Удалить
			if((HWND)lParam == GetDlgItem(hWnd, IDC_BTN_DEL))
			{
				int ItemIndex = SendMessage(hAccountList, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
				if(ItemIndex == -1)
				{
					MessageBox(hWnd, "Выберите аккаунт для удаления", "Ошибка выделения", MB_OK | MB_ICONEXCLAMATION);
					return 0;
				}
				TCHAR BUFF[MAX_PATH] = {0};
				LVITEM lvitem;
				ZeroMemory(&lvitem, sizeof(lvitem));
				lvitem.iItem = ItemIndex;
				lvitem.cchTextMax = MAX_PATH;
				lvitem.pszText = BUFF;
				
				
				ListView_GetItemText(hAccountList, ItemIndex, 0, BUFF, MAX_PATH);
				ListView_DeleteItem(hAccountList, ItemIndex);
				ftpServer.accounts.erase(BUFF);
				// Выделяем первый элемент в списке
				ListView_SetItemState(hAccountList, 0, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
				// Снимаем выделение
				ListView_SetItemState(hAccountList, 0, LVIS_FOCUSED & LVIS_SELECTED, 0x000F);
			}
			if((HWND)lParam == GetDlgItem(hWnd, IDC_BTN_CANCEL))
			{
				if(!ftpServer.SaveAccounts())	
				{
					sprintf(szErrorBuff, "Ошибка сохранения файла пользователей.\r\n Код ошибки: %d", GetLastError());
					MessageBox(0, szErrorBuff, "Ошибка сохранения", MB_OK | MB_ICONEXCLAMATION);
				}
				// Завершаяем диалог
				EndDialog(hWnd, 0);
			}
			// Отмена
		}
		return 0;
		case WM_INITDIALOG:
		{
			INITCOMMONCONTROLSEX InitCtrlEx;
			InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
			InitCtrlEx.dwICC  = ICC_PROGRESS_CLASS;
			InitCommonControlsEx(&InitCtrlEx);	
			hAccountList = GetDlgItem(hWnd, IDC_ACC_LIST);
			// Загружаем акаунты из файла
			if(!ftpServer.LoadAccounts())
			{
				MessageBox(0, "Создан логин anonymous", "Пользователь по умолчанию", MB_OK | MB_ICONEXCLAMATION);
				// Путь к файлу настроек
				char cmdLine[MAX_PATH] = {0};
				// Получаем путь
				GetModuleFileName(GetModuleHandle(NULL), cmdLine, MAX_PATH);
				// Отсекаем лишние символы
				*strrchr(cmdLine, '\\') = NULL;
				ftpServer.accounts["anonymous"] = Account("anonymous", "anonymous@mail.com", cmdLine);
			}
			// Добавляем в список
			SendMessage(GetDlgItem(hWnd, IDC_ACC_LIST), WM_LISTREFRESH, 0, 0);
		}
		case WM_LISTREFRESH:
			{
				// Структура элемент списка
				LVITEM lvItem;
				// Очищаем сисок
				ListView_DeleteAllItems(hAccountList);
				ListView_SetExtendedListViewStyleEx(hAccountList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
				map<string, Account>::iterator it;
				int i = 0;
				for(it = ftpServer.accounts.begin(); it != ftpServer.accounts.end(); ++it)
				{
					// Обнуляем
					memset(&lvItem, 0, sizeof(LVITEM));
					// Заполняем поля информацией
					lvItem.mask = LVIF_IMAGE|LVIF_TEXT;
					lvItem.state = 0;
					lvItem.stateMask = 0 ;
					lvItem.iItem = i;
					lvItem.iSubItem = 0;
					lvItem.pszText = (LPSTR)it->first.c_str();
					lvItem.cchTextMax = it->first.size();
					// Добавляем в список
					ListView_InsertItem(hAccountList, &lvItem);
				}
			}
			break;
		return TRUE;
		case WM_CLOSE:
			ftpServer.SaveAccounts();
			// Завершаяем диалог
			EndDialog(hWnd, 0);
			break;
		default:
			return 0;
	}	
	return 0;
}
// Калбек процедура диалогового окна добавления аккаунта
BOOL CALLBACK AccAddDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Обрабатываем сообщение
	switch(uMsg) 
	{
		case WM_COMMAND:
		{
			if((HWND)lParam == GetDlgItem(hWnd, IDC_BTN_ADD_PATH))
			{
				TCHAR path[MAX_PATH] = {0};
				BROWSEINFO bi;
				ZeroMemory(&bi, sizeof(bi));
				bi.lpszTitle = "Укажите домашний каталог";
				LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
				if(pidl != 0)
				{
					SHGetPathFromIDList(pidl, path);
					SetWindowText(GetDlgItem(hWnd, IDC_ACC_ADD_PATH_STATIC), path);
				}
			}
			// Добавить акаунт
			if((HWND)lParam == GetDlgItem(hWnd, IDC_BTN_ADD_OK))
			{
				TCHAR login[MAX_PATH] = {0};
				TCHAR pass[MAX_PATH] = {0};
				TCHAR homePath[MAX_PATH] = {0};
				// Получаем данные о новом пользователе
				GetWindowText(GetDlgItem(hWnd, IDC_EDIT_LOGIN), login, MAX_PATH);
				GetWindowText(GetDlgItem(hWnd, IDC_EDIT_PASS), pass, MAX_PATH);
				GetWindowText(GetDlgItem(hWnd, IDC_ACC_ADD_PATH_STATIC), homePath, MAX_PATH);
				if(strlen(login) == 0 || strlen(pass) == 0 || strlen(homePath) == 0)
				{
					MessageBox(hWnd, "Неверно заполнены поля", "Ошибка добавления", MB_OK | MB_ICONINFORMATION);
					return 0;
				}
				for(size_t i = 0; i < strlen(login); i++)
					login[i] = tolower(login[i]);

				// Если аккаунта с таким логином нет - добавляем
				if(ftpServer.accounts.count(login) == 0)
				{
					ftpServer.accounts[login] = Account(login, pass, homePath);
					// Добавляем в список
					// Структура элемент списка
					LVITEM lvItem;
					// Очищаем сисок
					ListView_DeleteAllItems(hAccountList);
					ListView_SetExtendedListViewStyleEx(hAccountList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
					map<string, Account>::iterator it;
					int i = 0;
					for(it = ftpServer.accounts.begin(); it != ftpServer.accounts.end(); ++it)
					{
						// Обнуляем
						memset(&lvItem, 0, sizeof(LVITEM));
						// Заполняем поля информацией
						lvItem.mask = LVIF_IMAGE|LVIF_TEXT;
						lvItem.state = 0;
						lvItem.stateMask = 0 ;
						lvItem.iItem = i;
						lvItem.iSubItem = 0;
						lvItem.pszText = (LPSTR)it->first.c_str();
						lvItem.cchTextMax = it->first.size();
						// Добавляем в список
						ListView_InsertItem(hAccountList, &lvItem);
					}
					// Завершаяем диалог
					EndDialog(hWnd, 0);
				}
				else
				{
					MessageBox(hWnd, "Такой аккаунт уже есть в базе", "Повторное добавление", MB_OK | MB_ICONINFORMATION);
				}
			}
			if((HWND)lParam == GetDlgItem(hWnd, IDC_BTN_ADD_CANCEL))
			{
				// Завершаяем диалог
				EndDialog(hWnd, 0);
			}
			// Отмена
		}
		return 0;
		case WM_INITDIALOG:
		{
			INITCOMMONCONTROLSEX InitCtrlEx;
			InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
			InitCtrlEx.dwICC  = ICC_PROGRESS_CLASS;
			InitCommonControlsEx(&InitCtrlEx);	
			
		}
		return TRUE;
		case WM_CLOSE:
			// Завершаяем диалог
			EndDialog(hWnd, 0);
			break;
		default:
			return FALSE;
	}	
	return FALSE;
}
// Калбек процедура диалогового окна добавления аккаунта
BOOL CALLBACK AccEditDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR INIT_LOGIN[MAX_PATH] = {0};
	// Обрабатываем сообщение
	switch(uMsg) 
	{
		case WM_COMMAND:
		{
			if((HWND)lParam == GetDlgItem(hWnd, IDC_BTN_ADD_PATH))
			{
				TCHAR path[MAX_PATH] = {0};
				BROWSEINFO bi;
				ZeroMemory(&bi, sizeof(bi));
				bi.lpszTitle = "Укажите домашний каталог";
				LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
				if(pidl != 0)
				{
					SHGetPathFromIDList(pidl, path);
					SetWindowText(GetDlgItem(hWnd, IDC_ACC_ADD_PATH_STATIC), path);
				}
			}
			// Применить
			if((HWND)lParam == GetDlgItem(hWnd, IDC_BTN_ADD_OK))
			{
				int ItemIndex = SendMessage(hAccountList, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
				if(ItemIndex == -1)
					SendMessage(hWnd, WM_CLOSE, 0, 0);
			
				memset(INIT_LOGIN, 0, sizeof(TCHAR) * MAX_PATH);
				ListView_GetItemText(hAccountList, ItemIndex, 0, INIT_LOGIN, MAX_PATH);
				for(size_t i = 0; i < strlen(INIT_LOGIN); i++)
					INIT_LOGIN[i] = tolower(INIT_LOGIN[i]);

				TCHAR login[MAX_PATH] = {0};
				TCHAR pass[MAX_PATH] = {0};
				TCHAR homePath[MAX_PATH] = {0};
				// Получаем данные о новом пользователе
				GetWindowText(GetDlgItem(hWnd, IDC_EDIT_LOGIN), login, MAX_PATH);
				GetWindowText(GetDlgItem(hWnd, IDC_EDIT_PASS), pass, MAX_PATH);
				GetWindowText(GetDlgItem(hWnd, IDC_ACC_ADD_PATH_STATIC), homePath, MAX_PATH);
				if(strlen(login) == 0 || strlen(pass) == 0 || strlen(homePath) == 0)
				{
					MessageBox(hWnd, "Неверно заполнены поля", "Ошибка редактирования", MB_OK | MB_ICONINFORMATION);
					return 0;
				}
				for(size_t i = 0; i < strlen(login); i++)
					login[i] = tolower(login[i]);
				// Сохраняем запись о редактируемом аккаунте
				Account temp = ftpServer.accounts[INIT_LOGIN];
				// Удаляем из списка аккаунтов
				ftpServer.accounts.erase(INIT_LOGIN);
				// Если аккаунта с таким логином нет - добавляем
				if(ftpServer.accounts.count(login) == 0)
				{
					ftpServer.accounts[login] = Account(login, pass, homePath);
					// Добавляем в список
					// Структура элемент списка
					LVITEM lvItem;
					// Очищаем сисок
					ListView_DeleteAllItems(hAccountList);
					ListView_SetExtendedListViewStyleEx(hAccountList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
					map<string, Account>::iterator it;
					int i = 0;
					for(it = ftpServer.accounts.begin(); it != ftpServer.accounts.end(); ++it)
					{
						// Обнуляем
						memset(&lvItem, 0, sizeof(LVITEM));
						// Заполняем поля информацией
						lvItem.mask = LVIF_IMAGE|LVIF_TEXT;
						lvItem.state = 0;
						lvItem.stateMask = 0 ;
						lvItem.iItem = i;
						lvItem.iSubItem = 0;
						lvItem.pszText = (LPSTR)it->first.c_str();
						lvItem.cchTextMax = it->first.size();
						// Добавляем в список
						ListView_InsertItem(hAccountList, &lvItem);
					}
				}
				else
				{
					// Изменение не удалось - вернем аккаунт в список
					ftpServer.accounts[INIT_LOGIN] = temp;
					MessageBox(hWnd, "Ошибка задания имени аккаунта", "Повторное добавление", MB_OK | MB_ICONINFORMATION);
					return 0;
				}
				// Завершаяем диалог
				EndDialog(hWnd, 0);	
			}
			if((HWND)lParam == GetDlgItem(hWnd, IDC_BTN_ADD_CANCEL))
			{
				// Завершаяем диалог
				EndDialog(hWnd, 0);
			}
			// Отмена
		}
		return 0;
		case WM_INITDIALOG:
		{
			INITCOMMONCONTROLSEX InitCtrlEx;
			InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
			InitCtrlEx.dwICC  = ICC_PROGRESS_CLASS;
			InitCommonControlsEx(&InitCtrlEx);
			int ItemIndex = SendMessage(hAccountList, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
			if(ItemIndex == -1)
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			
			ListView_GetItemText(hAccountList, ItemIndex, 0, INIT_LOGIN, MAX_PATH);

			SetWindowText(GetDlgItem(hWnd, IDC_EDIT_LOGIN), ftpServer.accounts[INIT_LOGIN].GetLogin().c_str());
			SetWindowText(GetDlgItem(hWnd, IDC_EDIT_PASS), ftpServer.accounts[INIT_LOGIN].GetPassword().c_str());
			SetWindowText(GetDlgItem(hWnd, IDC_ACC_ADD_PATH_STATIC), ftpServer.accounts[INIT_LOGIN].GetHome().c_str());
		}
		return TRUE;
		case WM_CLOSE:
			// Завершаяем диалог
			EndDialog(hWnd, 0);
			break;
		default:
			return FALSE;
	}	
	return FALSE;
}
// Калбек процедура диалогового окна настройки сервера
BOOL CALLBACK SetupDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Обрабатываем сообщение
	switch(uMsg) 
	{
		case WM_COMMAND:
		{
			// Применение настроек
			if((HWND)lParam == GetDlgItem(hWnd, IDC_BUTTON_APPLY))
			{
				char buff[MAX_PATH] = {0};
				// Получаем сообщение приветствия
				SendMessage(GetDlgItem(hWnd, IDC_EDIT_WELCMSG), WM_GETTEXT, MAX_PATH, LPARAM(srvSetup.msgWelcome.c_str()));
				// Порт
				SendMessage(GetDlgItem(hWnd, IDC_EDIT_PORT), WM_GETTEXT, MAX_PATH, LPARAM(buff));
				srvSetup.servPort = atoi(buff);
				memset(buff, 0, sizeof(char) * MAX_PATH);
				SendMessage(GetDlgItem(hWnd, IDC_EDIT_MAX_CON), WM_GETTEXT, MAX_PATH, LPARAM(buff));
				srvSetup.maxConnections = atoi(buff);
				srvSetup.fileLog = (BOOL)IsDlgButtonChecked(hWnd, IDC_CHECK_LOG);
				if(!srvSetup.Save())
				{
					srvSetup.Default();
					srvSetup.Save();
				}
				// Завершаяем диалог
				EndDialog(hWnd, 0);
			}
			if((HWND)lParam == GetDlgItem(hWnd, IDC_BUTTON_CANCEL))
			{
				// Завершаяем диалог
				EndDialog(hWnd, 0);
			}
		}
		return 0;
		case WM_INITDIALOG:
		{
			if(!srvSetup.Load())
			{
				MessageBox(hWnd, "Файл настроек не найден!\r\nСоздаем файл настроек по умолчанию...", "Ошибка чтения настроек", MB_OK | MB_ICONEXCLAMATION);
				srvSetup.Default();
				srvSetup.Save();
			}
			char buff[MAX_PATH] = {0};			
			// Записываем настройки в поля
			SendMessage(GetDlgItem(hWnd, IDC_EDIT_WELCMSG), WM_SETTEXT, srvSetup.msgWelcome.size(), LPARAM(srvSetup.msgWelcome.c_str()));
			sprintf(buff, "%d", srvSetup.servPort);
			SendMessage(GetDlgItem(hWnd, IDC_EDIT_PORT), WM_SETTEXT, strlen(buff), LPARAM(buff));
			sprintf(buff, "%d", srvSetup.maxConnections);
			SendMessage(GetDlgItem(hWnd, IDC_EDIT_MAX_CON), WM_SETTEXT, strlen(buff), LPARAM(buff));

			CheckDlgButton(hWnd, IDC_CHECK_LOG, srvSetup.fileLog ? BST_CHECKED : BST_UNCHECKED);
			

			SendMessage(GetDlgItem(hWnd, IDC_EDIT_MAX_CON), WM_SETTEXT, strlen(buff), LPARAM(buff));
		}
		return TRUE;
		case WM_CLOSE:
			// Завершаяем диалог
			EndDialog(hWnd, 0);
			break;
		default:
			return FALSE;
	}	
	return FALSE;
}