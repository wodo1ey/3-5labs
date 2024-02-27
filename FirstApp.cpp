#include <iostream>
#include <Windows.h>
#include <windowsx.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Структура конфига настроек
struct ConfigSettings {
	int windowWidth = 320;
	int windowHeight = 240;
	int N = 3;
	int BGcolorR = 0;
	int BGcolorG = 0;
	int BGcolorB = 255;
	int LineColorR = 0;
	int LineColorG = 0;
	int LineColorB = 0;
};
ConfigSettings settings;

// Методы для работы с файлом при помощи отображения на память
void ReadConfigSettingsMem(ConfigSettings& settings) {
	HANDLE hFile = CreateFileA("config.txt", FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		HANDLE hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (hMapFile != NULL) {
			LPVOID pData = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
			if (pData != NULL) {
				memcpy(&settings, pData, sizeof(ConfigSettings));
				UnmapViewOfFile(pData);
			}
			CloseHandle(hMapFile);
		}
		CloseHandle(hFile);
	}
}

void WriteConfigSettingsMem(const ConfigSettings& settings) {
	HANDLE hFile = CreateFileA("config.txt", FILE_GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD ByresWritten;
		BOOL result = WriteFile(hFile, &settings, sizeof(ConfigSettings), &ByresWritten, NULL);
	}
	CloseHandle(hFile);
}

// Методы для работы с файлом при помощи файловых переменных
void ReadConfigSettingsVar(ConfigSettings& settings) {
	FILE* file;
	fopen_s(&file, "config.txt", "rb");
	if (file != NULL) {
		fread(&settings, sizeof(ConfigSettings), 1, file);
		fclose(file);
	}
}

void WriteConfigSettingsVar(const ConfigSettings& settings) {
	FILE* file;
	fopen_s(&file, "config.txt", "wb");
	if (file != NULL) {
		fwrite(&settings, sizeof(ConfigSettings), 1, file);
		fclose(file);
	}
}

// Методы для работы с файлом при помощи потоков ввода-вывода
void ReadConfigSettingsIO(ConfigSettings& settings) {
	ifstream file("config.txt", std::ios::binary);
	if (file.is_open()) {
		file.read(reinterpret_cast<char*>(&settings), sizeof(ConfigSettings));
		file.close();
	}
}

void WriteConfigSettingsIO(const ConfigSettings& settings) {
	ofstream file("config.txt", std::ios::binary);
	if (file.is_open()) {
		file.write(reinterpret_cast<const char*>(&settings), sizeof(ConfigSettings));
		file.close();
	}
}

// Методы для работы с файлом при помощи файловых функций WinAPI/NativeAPI
void ReadConfigSettingsWinAPI(ConfigSettings& settings) {
	HANDLE hFile = CreateFile(L"config.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD bytesRead;
		ReadFile(hFile, &settings, sizeof(ConfigSettings), &bytesRead, NULL);
		CloseHandle(hFile);
	}
}

void WriteConfigSettingsWinAPI(const ConfigSettings& settings) {
	HANDLE hFile = CreateFile(L"config.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD bytesWritten;
		WriteFile(hFile, &settings, sizeof(ConfigSettings), &bytesWritten, NULL);
		CloseHandle(hFile);
	}
}

HANDLE hMapFile;
int* pData;
static int* cells;
int N = 3;
int method = 1;
HBRUSH hBrush;

UINT WM_CELL_CHANGED;

LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
WNDCLASS NewWindowClass(HBRUSH BGColor, HCURSOR Cursor, HINSTANCE hInst, HICON Icon, LPCWSTR Name, WNDPROC Procedure);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow) {
	// Инициализация генератора случайных чисел
	srand(time(0));
	// Создание оконного класса 
	WNDCLASS SoftwareMainClass = NewWindowClass((HBRUSH)COLOR_WINDOW, LoadCursor(NULL, IDC_ARROW), hInst, LoadIcon(NULL, IDI_QUESTION), L"MainWndClass", SoftwareMainProcedure);
	// Регистрация класса окна
	if (!RegisterClassW(&SoftwareMainClass)) { return -1; }
	MSG SoftwareMainMessage = { 0 };
	// Выбор метода чтения/записи конф. файла с помощью консольного ввода
	AllocConsole();
	HANDLE setStdin = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE setStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD set_written;
	WriteConsole(setStdout, L"Выберите метод чтения/зпаиси конф. файла (1 - Mem, 2 - Var, 3 - IO, 4 - API): ", 80, &set_written, NULL);
	char set_buffer[256];
	DWORD set_read;
	ReadConsole(setStdin, set_buffer, sizeof(set_buffer), &set_read, NULL);
	switch (atoi(set_buffer))
	{
	case 1:
		ReadConfigSettingsMem(settings);
		method = 1;
		break;
	case 2:
		ReadConfigSettingsVar(settings);
		method = 2;
		break;
	case 3:
		ReadConfigSettingsIO(settings);
		method = 3;
		break;
	case 4:
		ReadConfigSettingsWinAPI(settings);
		method = 4;
		break;
	default:
		ReadConfigSettingsMem(settings);
		method = 1;
		break;
	}
	// Если не найдено окно с назвнием "First c++ window", то считывается с консольного ввода размер игрового поля (если такое окно уже существует, то новое создаётся с тем же размером)
	HWND hWnd1 = FindWindow(NULL, L"Fitst c++ window");
	if (hWnd1 == NULL) {
		HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD written;
		WriteConsole(hStdout, L"Введите желаемое количество клеток (при некорректном вводе будет использовано значение по умолчанию): ", 101, &written, NULL);
		char buffer[256];
		DWORD read;
		ReadConsole(hStdin, buffer, sizeof(buffer), &read, NULL);
		if (atoi(buffer) > 0) {
			N = atoi(buffer);
			settings.N = N;
		}
		else N = settings.N;
	}
	else N = settings.N;
	FreeConsole();

	WM_CELL_CHANGED = RegisterWindowMessage(L"WM_CELL_CHANGED");
	// Создание File Mapping объекта для обмена данными между процессмаи
	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,   // Идентификатор файла
		NULL,                   // Защита
		PAGE_READWRITE,         // Доступ
		0,                      // Размер файла (0 - динамический)
		sizeof(int) * N * N,    // Размер fileMapping
		L"MyMappingObject");    // Имя fileMapping

	if (hMapFile == NULL) {
		cout << "Не удалось создать File Mapping объект" << std::endl;
		return 1;
	}

	pData = (int*)MapViewOfFile(hMapFile, FILE_MAP_WRITE, 0, 0, sizeof(cells) * N * N);

	if (pData == NULL) {
		std::cout << "Не удалось получить указатель на представленную область памяти" << std::endl;
		CloseHandle(hMapFile);
		return 1;
	}

	cells = new int[N * N];
	memset(cells, 0, N * N * sizeof(int));

	Sleep(30);
	// Создание окна с именем "First c++ window" и размером из конфигурации
	CreateWindow(L"MainWndClass", L"Fitst c++ window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 300, 300, settings.windowWidth, settings.windowHeight, NULL, NULL, NULL, NULL);
	// Запуск цикла обработки сообщений
	while (GetMessage(&SoftwareMainMessage, NULL, NULL, NULL)) {
		TranslateMessage(&SoftwareMainMessage);
		DispatchMessage(&SoftwareMainMessage);
	}
	// По завершению цикла освобождаются ресурсы
	UnmapViewOfFile(pData);
	CloseHandle(hMapFile);

	return 0;
}

WNDCLASS NewWindowClass(HBRUSH BGColor, HCURSOR Cursor, HINSTANCE hInst, HICON Icon, LPCWSTR Name, WNDPROC Procedure) {
	WNDCLASS NWC = { 0 };

	NWC.hIcon = Icon;
	NWC.hCursor = Cursor;
	NWC.hInstance = hInst;
	NWC.lpszClassName = Name;
	NWC.hbrBackground = BGColor;
	NWC.lpfnWndProc = Procedure;

	return NWC;
}

LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	// Инициализация статических переменных для цветов линий и заднего фона
	static int LineColorR = settings.LineColorR;
	static int LineColorG = settings.LineColorG;
	static int LineColorB = settings.LineColorB;
	static int deltaColor = 5;
	static int BGcolorR = settings.BGcolorR;
	static int BGcolorG = settings.BGcolorG;
	static int BGcolorB = settings.BGcolorB;
	// Инициализация переменных для размеров окна и ячеек + массив для хранения днных о содержимом ячеек
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	static int cellSize = min(width / N, height / N);
	static int* cells;
	// При получении сообщения WM_CELL_CHANGED содержимое массива cells обновляется данными из массива pData, содержащего информацию о ячейках
	if (msg == WM_CELL_CHANGED) {
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				cells[i * N + j] = pData[i * N + j];
			}
		}
		InvalidateRect(hWnd, NULL, TRUE);
	}

	switch (msg)
	{
	case WM_CREATE: // Создание окна
		hBrush = CreateSolidBrush(RGB(settings.BGcolorR, settings.BGcolorG, settings.BGcolorB)); // Кисть для заднего фона
		// Иницилизация массива cells значениями из мссиваа pData
		cells = new int[N * N];
		memset(cells, 0, N * N * sizeof(int));
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				if (pData[i * N + j] != 0) {
					cells[i * N + j] = pData[i * N + j];
				}
			}
		}
		break;
	case WM_KEYDOWN:
		switch (wp)
		{
		case VK_RETURN: // Enter - Изменение цвета заднего фона на случайный
		{
			if (hBrush) DeleteObject(hBrush);
			BGcolorR = rand() % 256;
			BGcolorG = rand() % 256;
			BGcolorB = rand() % 256;
			hBrush = CreateSolidBrush(RGB(BGcolorR, BGcolorG, BGcolorB));
			SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		}
		case VK_ESCAPE: DestroyWindow(hWnd); // ESC - Закрытие окна
		case 'Q': // CTRL + Q - Закрытие окна
			if (GetAsyncKeyState(VK_CONTROL) < 0) {
				DestroyWindow(hWnd);
			}
		case 'C': // CTRL + C - Открытие блокнота
			if (GetAsyncKeyState(VK_SHIFT) < 0) {
				HANDLE hProcess = NULL;
				HANDLE hThread = NULL;
				LPCWSTR notepadPath = L"C:\\Windows\\notepad.exe";
				STARTUPINFO si;
				PROCESS_INFORMATION pi;
				DWORD dwProcessId = 0;
				DWORD dwThreadId = 0;
				ZeroMemory(&si, sizeof(si));
				ZeroMemory(&pi, sizeof(pi));
				BOOL bCreateProcess = NULL;
				bCreateProcess = CreateProcess(notepadPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

				WaitForSingleObject(pi.hProcess, INFINITE);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}
		default:
			break;
		}
	case WM_MOUSEWHEEL: // Кручение колеса мыши - плавное изменение цвета линий
	{
		int delta = GET_WHEEL_DELTA_WPARAM(wp);
		if (LineColorR == 255) {
			LineColorR = 0;
		}
		else {
			LineColorR += deltaColor;
		}

		if (LineColorG == 255) {
			LineColorG = 0;
		}
		else {
			LineColorG += deltaColor;
		}

		if (LineColorB >= 255) {
			LineColorB = 0;
		}
		else {
			LineColorB += deltaColor;
		}

		COLORREF newColor = RGB(LineColorR, LineColorG, LineColorB);
		HDC hdc = GetDC(hWnd); // Получение контекста устройства
		SetDCPenColor(hdc, newColor);
		ReleaseDC(hWnd, hdc); // Освобождение контекста устройства
		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		break;
	}
	case WM_SIZE: // При изменении размеров окна обновляется размер ячеек и происходит перерисовка содержимого
	{
		cellSize = min(width / N, height / N);

		InvalidateRect(hWnd, NULL, TRUE);
		UpdateWindow(hWnd);
		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		FillRect(hdc, &ps.rcPaint, hBrush);

		HPEN hPen = CreatePen(PS_SOLID, 1, RGB(LineColorR, LineColorG, LineColorB));
		HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
		// Рисование вертикальных и горизонтальных лииний
		for (int x = 0; x < ps.rcPaint.right; x += (ps.rcPaint.right / N))
		{
			MoveToEx(hdc, x, 0, NULL);
			LineTo(hdc, x, ps.rcPaint.bottom);
		}
		for (int y = 0; y < ps.rcPaint.bottom; y += (ps.rcPaint.bottom / N))
		{
			MoveToEx(hdc, 0, y, NULL);
			LineTo(hdc, ps.rcPaint.right, y);
		}
		SelectObject(hdc, hOldPen);
		DeleteObject(hPen);
		// Проход по каждой ячейке массива cells 
		for (int i = 0; i < N * N; ++i) {
			int x = (ps.rcPaint.right / N) * (i % N);
			int y = (ps.rcPaint.bottom / N) * (i / N);
			if (cells[i] == 1) { // Если 1, то рисуется эллипс
				HBRUSH eBrush = CreateSolidBrush(RGB(255, 0, 0));
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, eBrush);
				Ellipse(hdc, x, y, x + (ps.rcPaint.right / N), y + (ps.rcPaint.bottom / N));
				SelectObject(hdc, hOldBrush);
				DeleteObject(eBrush);
			}
			else if (cells[i] == 2) { // Если 2, то рисуется крестик
				HPEN cPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
				HPEN hOldPen = (HPEN)SelectObject(hdc, cPen);
				MoveToEx(hdc, x, y, NULL);
				LineTo(hdc, x + (ps.rcPaint.right / N), y + (ps.rcPaint.bottom / N));
				MoveToEx(hdc, x + (ps.rcPaint.right / N), y, NULL);
				LineTo(hdc, x, y + (ps.rcPaint.bottom / N));
				SelectObject(hdc, hOldPen);
				DeleteObject(cPen);
			}
		}
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_LBUTTONDOWN: // ЛКМ - Нолик
	{
		// Координаты клика
		int x = LOWORD(lp);
		int y = HIWORD(lp);
		int xCell = x / (width / N);
		int yCell = y / (height / N);
		if (cells[yCell * N + xCell] != 2) { // Если содержимое ячейки не = 2, то ей присваивается значение 1
			cells[yCell * N + xCell] = 1;
			pData[yCell * N + xCell] = 1;
		}
		// Обновление содержимого окна, отправка сообщения об изменении ячейки + перерисовка
		InvalidateRect(hWnd, NULL, TRUE);
		UpdateWindow(hWnd);
		SendMessage(HWND_BROADCAST, WM_CELL_CHANGED, 0, 0);
		break;
	}
	case WM_RBUTTONDOWN: // ПКМ - Крестик
	{
		// Координаты клика
		int x = LOWORD(lp);
		int y = HIWORD(lp);
		int xCell = x / (width / N);
		int yCell = y / (height / N);
		if (cells[yCell * N + xCell] != 1) { // Если содержимое ячейки не = 1, то ей присваивается значение 2
			cells[yCell * N + xCell] = 2;
			pData[yCell * N + xCell] = 2;
		}
		// Обновление содержимого окна, отправка сообщения об изменении ячейки + перерисовка
		InvalidateRect(hWnd, NULL, TRUE);
		UpdateWindow(hWnd);
		SendMessage(HWND_BROADCAST, WM_CELL_CHANGED, 0, 0);
		break;
	}
	case WM_DESTROY: // Закрытие окна
		// Сохранение настроек в зависимости от метода
		settings.windowWidth = width;
		settings.windowHeight = height;
		settings.N = N;
		settings.BGcolorR = BGcolorR;
		settings.BGcolorG = BGcolorG;
		settings.BGcolorB = BGcolorB;
		settings.LineColorR = LineColorR;
		settings.LineColorG = LineColorG;
		settings.LineColorB = LineColorB;
		switch (method)
		{
		case 1:
			WriteConfigSettingsMem(settings);
			break;
		case 2:
			WriteConfigSettingsVar(settings);
			break;
		case 3:
			WriteConfigSettingsIO(settings);
			break;
		case 4:
			WriteConfigSettingsWinAPI(settings);
			break;
		default:
			WriteConfigSettingsMem(settings);
			break;
		}
		// Освобождение ресурсов
		DeleteObject(hBrush);
		delete[] cells;
		PostQuitMessage(0); // Сообение о завершении работы приложения
		break; // Другие сообщения
	default: return DefWindowProc(hWnd, msg, wp, lp); // Стандартная обработка сообщения DefWindowProc
		break;
	}
}
