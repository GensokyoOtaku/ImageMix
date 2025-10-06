#include "lib_cv.h"
#include "log.h"

inline ll read();
inline LPCWSTR stringToLPCWSTR(std::string orig)
{
    size_t origsize = orig.length() + 1;
    // const size_t newsize = 100;
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

#define IDR_DLC 200
#define IDR_COCKROACH 201

#define IDC_SLIDER_MIX_NUM 31
#define IDC_SLIDER_MIX_OFFSET 32

int excItem_0[100] = {IDR_START, IDR_PAUSE, IDR_TANK, IDR_COCKROACH};
int excNum_0 = 4;
//==========================================
bool start = true;
LPCTSTR szAppClassName = TEXT("ImgMix");
LPCTSTR szAppWindowName = TEXT("ImgMix");
HMENU hmenu;    // �˵����
HMENU hsubmode; // �Ӳ˵�
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
    POINT pt; // ���ڽ����������
    int id;   // ���ڽ��ղ˵�ѡ���ֵ
    // ��Ҫ�޸�TaskbarCreated������ϵͳ�������Զ������Ϣ
    WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));
    switch (message)
    {
    case WM_CREATE: // ���ڴ���ʱ�����Ϣ.
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
        hmenu = CreatePopupMenu(); // ���ɲ˵�
        hsubmode = CreatePopupMenu();
        AppendMenu(hmenu, MF_STRING, IDR_START, "�����滻"); // Ϊ�˵����ѡ��
        AppendMenu(hmenu, MF_STRING, IDR_TANK, "��Ӱ̹��");
        // AppendMenu(hmenu, MF_POPUP | MF_STRING, IDR_DLC, L"����ģʽ");
        AppendMenu(hmenu, MF_STRING, IDR_COCKROACH, "���������");
        //=======================================================================================
        AppendMenu(hmenu, MF_SEPARATOR, NULL, NULL);
        AppendMenu(hmenu, MF_STRING, IDR_ROTATE, "˳ʱ����ת 0��");
        AppendMenu(hmenu, MF_STRING, IDR_RESET_ANGLE, "������ת�Ƕ�");
        //=======================================================================================
        AppendMenu(hmenu, MF_SEPARATOR, NULL, NULL);
        switch (berlinEnable)
        {
        case 0:
            AppendMenu(hmenu, MF_STRING, IDR_BERLIN_NOISE, "��������(�ѽ���)");
            break;
        case 1:
            AppendMenu(hmenu, MF_STRING, IDR_BERLIN_NOISE, "��������(������)");
            break;
        default:
            break;
        }
        //=======================================================================================
        AppendMenu(hmenu, MF_SEPARATOR, NULL, NULL);
        switch (output_mode)
        {
        case OUTPUT_JPG:
            AppendMenu(hmenu, MF_STRING, IDR_OUTPUT_MODE, "�����ʽ��JPEG");
            break;
        case OUTPUT_PNG:
            AppendMenu(hmenu, MF_STRING, IDR_OUTPUT_MODE, "�����ʽ��PNG");
            break;
        default:
            break;
        }
        //=======================================================================================
        AppendMenu(hmenu, MF_SEPARATOR, NULL, NULL);
        AppendMenu(hmenu, MF_STRING, IDR_PAUSE, "ֹͣ����");
        //=======================================================================================
        AppendMenu(hmenu, MF_SEPARATOR, NULL, NULL);
        AppendMenu(hmenu, MF_STRING, IDR_LOADCFG, "���ػ�������");
        //=======================================================================================
        AppendMenu(hmenu, MF_SEPARATOR, NULL, NULL);
        AppendMenu(hmenu, MF_STRING, IDR_ABOUT, "����");
        AppendMenu(hmenu, MF_STRING, IDR_EXIT, "�˳�");

        // AppendMenu(hsubmode, MF_STRING, IDR_COCKROACH, L"���������");
        break;
    case WM_CHANGECBCHAIN: // �������ı�
        if ((HWND)wParam == hwndNextViewer)
            hwndNextViewer = (HWND)lParam;
        else if (hwndNextViewer)
            SendMessage(hwndNextViewer, message, wParam, lParam);
        break;
    case WM_DRAWCLIPBOARD: // ���������ݸı�
                           // case WM_PASTE:
        if (start)
        {
            SendMessage(hwndNextViewer, message, wParam, lParam);
            getImgFormClipboard(hwnd);
        }

    case WM_USER: // ����ʹ�øó���ʱ�����Ϣ.
        if (lParam == WM_RBUTTONDOWN)
        {
            GetCursorPos(&pt);           // ȡ�������
            ::SetForegroundWindow(hwnd); // ����ڲ˵��ⵥ������˵�����ʧ������

            updateMenuItem_Mix(hmenu);

            RECT rect;
            GetWindowRect(hwnd, &rect);
            id = TrackPopupMenu(hmenu, TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, NULL, hwnd, NULL); // ��ʾ�˵�����ȡѡ��ID
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
                ModifyMenu(hmenu, 7, MF_STRING | MF_BYPOSITION, IDR_OUTPUT_MODE, "�����ʽ��PNG");
                LOG("Enable mirage tank mode");
            }
            if (id == IDR_LOADCFG)
            {
                loadConfig();
                MessageBox(hwnd, TEXT("���������Ѹ���"), szAppClassName, MB_OK);
            }
            if (id == IDR_DLC)
            {
                // GetCursorPos(&pt);//ȡ�������
                //::SetForegroundWindow(hwnd);//����ڲ˵��ⵥ������˵�����ʧ������
                id = TrackPopupMenu(hsubmode, TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, NULL, hwnd, NULL);
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
                    ModifyMenu(hmenu, 4, MF_STRING | MF_BYPOSITION, IDR_ROTATE, "˳ʱ����ת 0��");
                    break;
                case 90:
                    ModifyMenu(hmenu, 4, MF_STRING | MF_BYPOSITION, IDR_ROTATE, "˳ʱ����ת 90��");
                    break;
                case 180:
                    ModifyMenu(hmenu, 4, MF_STRING | MF_BYPOSITION, IDR_ROTATE, "˳ʱ����ת 180��");
                    break;
                case 270:
                    ModifyMenu(hmenu, 4, MF_STRING | MF_BYPOSITION, IDR_ROTATE, "˳ʱ����ת 270��");
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
                    ModifyMenu(hmenu, 6, MF_STRING | MF_BYPOSITION, IDR_BERLIN_NOISE, "��������(�ѽ���)");
                    break;
                case 1:
                    ModifyMenu(hmenu, 6, MF_STRING | MF_BYPOSITION, IDR_BERLIN_NOISE, "��������(������)");
                    break;
                default:
                    break;
                }
            }
            if (id == IDR_RESET_ANGLE)
            {
                angle = 0;
                ModifyMenu(hmenu, 4, MF_STRING | MF_BYPOSITION, IDR_ROTATE, "˳ʱ����ת 0��");
            }
            if (id == IDR_OUTPUT_MODE)
            {
                if (MIX_MODE != TANK_MODE)
                {
                    output_mode = (output_mode + 1) % OUTPUT_TOT;
                    switch (output_mode)
                    {
                    case OUTPUT_JPG:
                        ModifyMenu(hmenu, 7, MF_STRING | MF_BYPOSITION, IDR_OUTPUT_MODE, "�����ʽ��JPEG");
                        break;
                    case OUTPUT_PNG:
                        ModifyMenu(hmenu, 7, MF_STRING | MF_BYPOSITION, IDR_OUTPUT_MODE, "�����ʽ��PNG");
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    MessageBox(hwnd, TEXT("��Ӱ̹��ģʽ�޷�ʹ��PNG�����ʽ���"), szAppClassName, MB_OK);
                }
            }
            if (id == IDR_ABOUT)
                MessageBox(hwnd, TEXT("�Զ������������е�ͼƬ\n��ע��gif�ᱻ�޸�Ϊ��̬ͼƬ"), szAppClassName, MB_OK);
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
    case WM_DESTROY: // ��������ʱ�����Ϣ.
        ChangeClipboardChain(hwnd, hwndNextViewer);
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    default:
        /*
         * ��ֹ��Explorer.exe �����Ժ󣬳�����ϵͳϵͳ�����е�ͼ�����ʧ
         *
         * ԭ��Explorer.exe �����������ؽ�ϵͳ����������ϵͳ������������ʱ�����ϵͳ������
         * ע�����TaskbarCreated ��Ϣ�Ķ������ڷ���һ����Ϣ������ֻ��Ҫ��׽�����Ϣ�����ؽ�ϵ
         * ͳ���̵�ͼ�꼴�ɡ�
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

    // �˴�ʹ��WS_EX_TOOLWINDOW ������������ʾ���������ϵĴ��ڳ���ť
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

    // FreeConsole();
    // ��Ϣѭ��
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
