#include "lib_cv.h"
#include "log.h"

inline ll read();
inline LPCWSTR stringToLPCWSTR(std::string orig)
{
    size_t origsize = orig.length() + 1;
    size_t convertedChars = 0;
    wchar_t *wcstring = (wchar_t *)malloc(sizeof(wchar_t) * (orig.length() - 1));
    mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);

    return wcstring;
}
inline void loadConfig()
{
    freopen("./config.cfg", "r", stdin);

    MAXN_MIX = read();
    MIX_OFFSET = read();
    R_OffsetFactor = (read() > 0 ? 1 : -1);
    G_OffsetFactor = (read() > 0 ? 1 : -1);
    B_OffsetFactor = (read() > 0 ? 1 : -1);
    berlinSize = read();
    if (berlinSize < 0)
    {
        berlinSize *= -1;
    }
    berlinEnable = (read() != 0);

    fclose(stdin);

    if (MAXN_MIX < 1 || MAXN_MIX > 1e9)
        MAXN_MIX = DEFAULT_MAXN_MIX;
    if (MIX_OFFSET < 0 || MIX_OFFSET > 255)
        MIX_OFFSET = DEFAULT_MIX_OFFSET;

    freopen("./config.cfg", "w", stdout);

    puts("parameters:");
    printf("max mixed pixel: %d\n", MAXN_MIX);
    printf("RGB offset: %d\n", MIX_OFFSET);
    printf("R mix option: %d\n", (R_OffsetFactor + 1) / 2);
    printf("G mix option: %d\n", (G_OffsetFactor + 1) / 2);
    printf("B mix option: %d\n", (B_OffsetFactor + 1) / 2);
    printf("berlin size: %d\n", berlinSize);
    printf("berlin enable: %d\n", berlinEnable);

    fclose(stdout);
}
inline void _init()
{
    srand(time(NULL));
    loadConfig();
    R_CODE[90] = ROTATE_90_CLOCKWISE;
    R_CODE[180] = ROTATE_180;
    R_CODE[270] = ROTATE_90_COUNTERCLOCKWISE;
}
//==========================================
HWND hwnd;
MSG msg;
WNDCLASS wndclass;
//==========================================
#define IDR_PAUSE 101
#define IDR_START 102
#define IDR_ABOUT 103
#define IDR_EXIT 104
#define IDR_LOADCFG 105
#define IDR_TANK 106
#define IDR_ROTATE 107
#define IDR_RESET_ANGLE 108
#define IDR_OUTPUT_MODE 109
#define IDR_BERLIN_NOISE 110
#define IDR_OPENCFG 111
#define IDR_OPENLOG 112

#define IDR_DLC 200
#define IDR_COCKROACH 201

#define IDC_SLIDER_MIX_NUM 31
#define IDC_SLIDER_MIX_OFFSET 32

#define POS_ROTATE 4
#define POS_BERLIN_NOISE 7
#define POS_OUTPUT_MODE 9

int excItem_0[100] = {IDR_START, IDR_PAUSE, IDR_TANK, IDR_COCKROACH};
int excNum_0 = 4;
//==========================================
bool start = true;
LPCTSTR szAppClassName = TEXT("ImgMix");
LPCTSTR szAppWindowName = TEXT("ImgMix");
HMENU hmenu;    // 菜单句柄
HMENU hsubmode; // 子菜单
HWND hwndNextViewer = NULL;
HWND hWndMixNum = NULL;
HWND hWndMixOffset = NULL;
//==========================================
inline void updateMenuItem(HMENU hmenu, int IDR)
{
    for (int idr = 0; idr < excNum_0; idr++)
        EnableMenuItem(hmenu, excItem_0[idr], MF_ENABLED);
    EnableMenuItem(hmenu, IDR, MF_GRAYED);
}
inline void updateMenuItem_Mix(HMENU hmenu)
{
    if (MIX_MODE == DEFAULT_MIX_MODE)
    {
        if (start)
            updateMenuItem(hmenu, IDR_START);
        else
            updateMenuItem(hmenu, IDR_PAUSE);
    }
    else if (MIX_MODE == COCKROACH_MODE)
        updateMenuItem(hmenu, IDR_COCKROACH);
    else if (MIX_MODE == TANK_MODE)
        updateMenuItem(hmenu, IDR_TANK);
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    NOTIFYICONDATA nid;
    UINT WM_TASKBARCREATED;
    POINT pt; // 用于接收鼠标坐标
    int id;   // 用于接收菜单选项返回值
    // 不要修改TaskbarCreated，这是系统任务栏自定义的消息
    WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));
    switch (message)
    {
    case WM_CREATE: // 窗口创建时候的消息.
        LOG("");
        LOG("--------------- ImgMix Start ------------------");
        hwndNextViewer = SetClipboardViewer(hwnd);
        nid.cbSize = sizeof(nid);
        nid.hWnd = hwnd;
        nid.uID = 0;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_USER;
        nid.hIcon = (HICON)::LoadImage(
            NULL,
            TEXT("./icons/icon.ico"),
            IMAGE_ICON,
            200, 200,
            LR_DEFAULTCOLOR | LR_CREATEDIBSECTION | LR_LOADFROMFILE);
        lstrcpy(nid.szTip, szAppClassName);
        Shell_NotifyIcon(NIM_ADD, &nid);
        hmenu = CreatePopupMenu(); // 生成菜单
        hsubmode = CreatePopupMenu();
        AppendMenuW(hmenu, MF_STRING, IDR_START, L"像素替换"); // 为菜单添加选项
        AppendMenuW(hmenu, MF_STRING, IDR_TANK, L"幻影坦克");
        AppendMenuW(hmenu, MF_STRING, IDR_COCKROACH, L"绝世大蟑螂");
        //=======================================================================================
        AppendMenuW(hmenu, MF_SEPARATOR, NULL, NULL);
        AppendMenuW(hmenu, MF_STRING, IDR_ROTATE, L"顺时针旋转 0°");
        AppendMenuW(hmenu, MF_STRING, IDR_RESET_ANGLE, L"重置旋转角度");
        //=======================================================================================
        AppendMenuW(hmenu, MF_SEPARATOR, NULL, NULL);
        switch (berlinEnable)
        {
        case 0:
            AppendMenuW(hmenu, MF_STRING, IDR_BERLIN_NOISE, L"柏林噪声(已禁用)");
            break;
        case 1:
            AppendMenuW(hmenu, MF_STRING, IDR_BERLIN_NOISE, L"柏林噪声(已启用)");
            break;
        default:
            break;
        }
        //=======================================================================================
        AppendMenuW(hmenu, MF_SEPARATOR, NULL, NULL);
        switch (output_mode)
        {
        case OUTPUT_JPG:
            AppendMenuW(hmenu, MF_STRING, IDR_OUTPUT_MODE, L"输出格式：JPEG");
            break;
        case OUTPUT_PNG:
            AppendMenuW(hmenu, MF_STRING, IDR_OUTPUT_MODE, L"输出格式：PNG");
            break;
        default:
            break;
        }
        //=======================================================================================
        AppendMenuW(hmenu, MF_SEPARATOR, NULL, NULL);
        AppendMenuW(hmenu, MF_STRING, IDR_PAUSE, L"停止混淆");
        //=======================================================================================
        AppendMenuW(hmenu, MF_SEPARATOR, NULL, NULL);
        AppendMenuW(hmenu, MF_STRING, IDR_LOADCFG, L"重载混淆参数");
        //=======================================================================================
        AppendMenuW(hmenu, MF_SEPARATOR, NULL, NULL);
        AppendMenuW(hmenu, MF_STRING, IDR_OPENCFG, L"打开配置文件");
        //=======================================================================================
        AppendMenuW(hmenu, MF_SEPARATOR, NULL, NULL);
        AppendMenuW(hmenu, MF_STRING, IDR_OPENLOG, L"打开日志文件");
        //=======================================================================================
        AppendMenuW(hmenu, MF_SEPARATOR, NULL, NULL);
        AppendMenuW(hmenu, MF_STRING, IDR_ABOUT, L"关于");
        AppendMenuW(hmenu, MF_STRING, IDR_EXIT, L"退出");
        break;
    case WM_CHANGECBCHAIN: // 监听链改变
        if ((HWND)wParam == hwndNextViewer)
            hwndNextViewer = (HWND)lParam;
        else if (hwndNextViewer)
            SendMessage(hwndNextViewer, message, wParam, lParam);
        break;
    case WM_DRAWCLIPBOARD: // 剪贴板内容改变
                           // case WM_PASTE:
        if (start)
        {
            SendMessage(hwndNextViewer, message, wParam, lParam);
            getImgFormClipboard(hwnd);
        }

    case WM_USER: // 连续使用该程序时候的消息.
        if (lParam == WM_RBUTTONDOWN)
        {
            GetCursorPos(&pt);           // 取鼠标坐标
            ::SetForegroundWindow(hwnd); // 解决在菜单外单击左键菜单不消失的问题

            updateMenuItem_Mix(hmenu);

            RECT rect;
            GetWindowRect(hwnd, &rect);
            id = TrackPopupMenu(hmenu, TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, NULL, hwnd, NULL); // 显示菜单并获取选项ID
            if (id == IDR_PAUSE)
            {
                start = false;
                MIX_MODE = DEFAULT_MIX_MODE;
                LOG("Stop mix service");
            }
            if (id == IDR_START)
            {
                start = true;
                MIX_MODE = DEFAULT_MIX_MODE;
                LOG("Start mix service");
            }
            if (id == IDR_TANK)
            {
                MIX_MODE = TANK_MODE;
                output_mode = OUTPUT_PNG;
                ModifyMenuW(hmenu, POS_OUTPUT_MODE, MF_STRING | MF_BYPOSITION, IDR_OUTPUT_MODE, L"输出格式：PNG");
                LOG("Enable mirage tank mode");
            }
            if (id == IDR_LOADCFG)
            {
                loadConfig();
                MessageBox(hwnd, TEXT("config updated"), szAppClassName, MB_OK);
            }
            if (id == IDR_OPENCFG)
            {
                ShellExecute(NULL, TEXT("open"), TEXT("notepad.exe"), TEXT("config.cfg"), NULL, SW_SHOWNORMAL);
            }
            if (id == IDR_OPENLOG)
            {
                ShellExecute(NULL, TEXT("open"), TEXT("notepad.exe"), TEXT("log.txt"), NULL, SW_SHOWNORMAL);
            }
            if (id == IDR_COCKROACH)
            {
                MIX_MODE = COCKROACH_MODE;
                LOG("Enable cockroach mode");
            }
            if (id == IDR_ROTATE)
            {
                angle = (angle + 90) % 360;
                switch (angle)
                {
                case 0:
                    ModifyMenuW(hmenu, POS_ROTATE, MF_STRING | MF_BYPOSITION, IDR_ROTATE, L"顺时针旋转 0°");
                    break;
                case 90:
                    ModifyMenuW(hmenu, POS_ROTATE, MF_STRING | MF_BYPOSITION, IDR_ROTATE, L"顺时针旋转 90°");
                    break;
                case 180:
                    ModifyMenuW(hmenu, POS_ROTATE, MF_STRING | MF_BYPOSITION, IDR_ROTATE, L"顺时针旋转 180°");
                    break;
                case 270:
                    ModifyMenuW(hmenu, POS_ROTATE, MF_STRING | MF_BYPOSITION, IDR_ROTATE, L"顺时针旋转 270°");
                    break;
                default:
                    break;
                }
            }
            if (id == IDR_BERLIN_NOISE)
            {
                berlinEnable = 1 - berlinEnable;
                switch (berlinEnable)
                {
                case 0:
                    ModifyMenuW(hmenu, POS_BERLIN_NOISE, MF_STRING | MF_BYPOSITION, IDR_BERLIN_NOISE, L"柏林噪声(已禁用)");
                    break;
                case 1:
                    ModifyMenuW(hmenu, POS_BERLIN_NOISE, MF_STRING | MF_BYPOSITION, IDR_BERLIN_NOISE, L"柏林噪声(已启用)");
                    break;
                default:
                    break;
                }
            }
            if (id == IDR_RESET_ANGLE)
            {
                angle = 0;
                ModifyMenuW(hmenu, POS_ROTATE, MF_STRING | MF_BYPOSITION, IDR_ROTATE, L"顺时针旋转 0°");
            }
            if (id == IDR_OUTPUT_MODE)
            {
                if (MIX_MODE != TANK_MODE)
                {
                    output_mode = (output_mode + 1) % OUTPUT_TOT;
                    switch (output_mode)
                    {
                    case OUTPUT_JPG:
                        ModifyMenuW(hmenu, POS_OUTPUT_MODE, MF_STRING | MF_BYPOSITION, IDR_OUTPUT_MODE, L"输出格式：JPEG");
                        break;
                    case OUTPUT_PNG:
                        ModifyMenuW(hmenu, POS_OUTPUT_MODE, MF_STRING | MF_BYPOSITION, IDR_OUTPUT_MODE, L"输出格式：PNG");
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    MessageBox(hwnd, TEXT("miracle tank only support png"), szAppClassName, MB_OK);
                }
            }
            if (id == IDR_ABOUT)
                MessageBox(hwnd, TEXT("QWQ"), szAppClassName, MB_OK);
            if (id == IDR_EXIT)
            {
                SendMessage(hwnd, WM_CLOSE, wParam, lParam);
                LOG("--------------- ImgMix Exit ------------------\n");
                exit(0);
            }
            if (id == 0)
                PostMessage(hwnd, WM_LBUTTONDOWN, NULL, NULL);
        }
        break;
    case WM_DESTROY: // 窗口销毁时候的消息.
        ChangeClipboardChain(hwnd, hwndNextViewer);
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    default:
        /*
         * 防止当Explorer.exe 崩溃以后，程序在系统系统托盘中的图标就消失
         *
         * 原理：Explorer.exe 重新载入后会重建系统任务栏。当系统任务栏建立的时候会向系统内所有
         * 注册接收TaskbarCreated 消息的顶级窗口发送一条消息，我们只需要捕捉这个消息，并重建系
         * 统托盘的图标即可。
         */
        if (message == WM_TASKBARCREATED)
            SendMessage(hwnd, WM_CREATE, wParam, lParam);
        break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}
//==========================================
int main()
{
    _init();

    HWND handle = FindWindow(NULL, szAppWindowName);
    if (handle != NULL)
    {
        MessageBox(NULL, TEXT("Application is already running"), szAppClassName, MB_ICONERROR);
        return 0;
    }

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = NULL;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppClassName;

    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, TEXT("This program requires Windows NT!"), szAppClassName, MB_ICONERROR);
        return 0;
    }

    // 此处使用WS_EX_TOOLWINDOW 属性来隐藏显示在任务栏上的窗口程序按钮
    hwnd = CreateWindowEx(WS_EX_TOOLWINDOW,
                          szAppClassName, szAppWindowName,
                          WS_POPUP,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          NULL, NULL, NULL, NULL);

    ShowWindow(hwnd, SW_HIDE);
    UpdateWindow(hwnd);

    // 消息循环
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
inline ll read()
{
    ll p = 1, ans = 0;
    char ch = getchar();
    while (ch < 48 || ch > 57)
    {
        if (ch == '-')
            p = -1;
        if (ch == -1)
            return 0x3f3f3f3f;
        ch = getchar();
    }
    while (ch >= 48 && ch <= 57)
    {
        if (ch == -1)
            return 0x3f3f3f3f;
        ans = (ans << 3) + (ans << 1) + ch - 48;
        ch = getchar();
    }

    return ans * p;
}
