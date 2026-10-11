/*主程序源文件，包含：WinMain() 入口点、程序初始化逻辑、
核心业务流程控制。功能：协调数据库、用户界面和业务逻辑。*/
#include "framework.h"
#include "FlightBookingSystem.h"
#include <iostream>
#include <stdio.h>

// 全局变量定义
HINSTANCE hInst;   // 应用程序实例句柄
WCHAR szTitle[100] = L"航空订票管理系统";   // 窗口标题
WCHAR szWindowClass[100] = L"FlightBookingSystem";    // 窗口类名
DatabaseConnection dbConn = { 0 };         // 数据库连接结构体
BOOL isAdmin = FALSE;        // 管理员身份标识
BOOL isLoggedIn = FALSE;      // 登录状态标识
HWND g_hLoginDlg = NULL;      // 登录对话框句柄
BOOL g_loginSuccess = FALSE;  // 登录成功标识
int currentUserID = 0;        // 当前用户ID
wchar_t currentUserName[100] = L"";   // 当前用户名

// 添加 CreateLoginDialog 函数声明
extern void CreateLoginDialog(HWND hWnd);

// 窗口注册函数
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;        // 窗口类扩展结构
    wcex.cbSize = sizeof(WNDCLASSEX);     // 结构体大小
    wcex.style = CS_HREDRAW | CS_VREDRAW;   // 窗口样式
    wcex.lpfnWndProc = WndProc;  //窗口过程函数指针, 这里使用了WndProc，所以需要提前声明
    wcex.cbClsExtra = 0;        // 额外类内存
    wcex.cbWndExtra = 0;        // 额外窗口内存
    wcex.hInstance = hInstance;    // 应用程序实例
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);     // 默认图标
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);       // 箭头光标
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);     // 窗口背景色
    wcex.lpszMenuName = nullptr;          // 无菜单
    wcex.lpszClassName = szWindowClass;      // 窗口类名
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);  // 小图标
    
    // 先保存注册结果
    ATOM result = RegisterClassExW(&wcex);

    // === 添加调试信息 ===
    printf("窗口类注册: %s, 类名: %ls\n",
        result ? "成功" : "失败", szWindowClass);

    // 然后返回结果
    return result;
}

// 实例初始化函数
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;          // 保存实例句柄

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 1024, 768, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd)
    {
        printf("!!! 窗口创建失败 !!!\n");
        return FALSE;
    }

    // 调试信息
    printf("主窗口创建成功: %p\n", hWnd);
    printf("窗口过程: %p\n", (void*)GetWindowLongPtr(hWnd, GWLP_WNDPROC));

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}


// 主程序入口
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,      // 当前实例句柄
    _In_ LPWSTR    lpCmdLine,        // 命令行参数
    _In_ int       nCmdShow)         // 窗口显示方式
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 添加控制台用于调试
    AllocConsole();        // 分配控制台
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);    // 重定向标准输出到控制台
    printf("=== 航空订票系统启动 ===\n");

    // 初始化全局变量
    hInst = hInstance;

    // 1. 注册窗口类
    printf("1. 注册窗口类...\n");
    if (!MyRegisterClass(hInstance))
    {
        printf("!!! 窗口类注册失败 !!!\n");
        MessageBox(NULL, L"窗口类注册失败!", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    printf("窗口类注册成功\n");

    // 2. 初始化通用控件
    printf("2. 初始化通用控件...\n");
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES;
    InitCommonControlsEx(&icex);
    printf("通用控件初始化完成\n");

    // 3. 连接数据库
    printf("3. 连接数据库...\n");
    if (!ConnectToDatabase())
    {
        printf("数据库连接失败 - 程序无法正常运行\n");
        MessageBox(NULL,
            L"数据库连接失败! 请确保SQL Server运行且FlightBookingSystem数据库存在。",
            L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    else
    {
        printf("数据库连接成功\n");
    }

    // 4. 创建主窗口
    printf("4. 创建主窗口...\n");
    HWND hWnd = CreateWindowW(
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MAXIMIZEBOX | WS_SIZEBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1400,  // 宽度
        1300,  // 高度
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!hWnd)
    {
        printf("!!! 窗口创建失败 !!!\n");
        MessageBox(NULL, L"窗口创建失败!", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    printf("主窗口创建成功\n");

    // 5. 显示窗口
    printf("5. 显示窗口...\n");
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // 6. 显示登录对话框
    printf("6. 显示登录对话框...\n");
    CreateLoginDialog(hWnd);

    // 7. 进入主消息循环
    printf("7. 进入主消息循环...\n");
    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessage(&msg, nullptr, 0, 0)) != 0) {
        if (bRet == -1) {
            printf("!!! GetMessage 返回错误 !!!\n");
            break;
        }
        else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    printf("=== 程序正常退出 ===\n");

    if (f) fclose(f);
    FreeConsole();
    return 0;
}


// 窗口过程函数
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:           // 窗口创建消息 - 进行初始化工作
        printf("收到 WM_CREATE 消息\n");
        break;

    case WM_NOTIFY:
    {
        LPNMHDR lpnmh = (LPNMHDR)lParam;          // 通知消息 - 处理控件通知
        if (lpnmh->code == TCN_SELCHANGE && lpnmh->idFrom == IDC_MAIN_TAB) {
            // 处理选项卡切换
            HWND hTab = lpnmh->hwndFrom;
            int selectedTab = TabCtrl_GetCurSel(hTab);       // 获取当前选中的选项卡

            // 显示/隐藏对应的页面
            for (int i = 0; i < 3; i++) {
                HWND hPage = GetDlgItem(hTab, IDC_FLIGHT_PAGE + i);
                if (hPage) {
                    ShowWindow(hPage, (i == selectedTab) ? SW_SHOW : SW_HIDE);
                }
            }
        }
        break;
    }

    // 命令消息 - 处理菜单、按钮点击等
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);     // 命令ID
        int wmEvent = HIWORD(wParam);  // 事件类型
        HWND hControl = (HWND)lParam;    // 控件句柄


        printf("=== 收到 WM_COMMAND ===\n");
        printf("命令ID: %d, 事件: %d\n", wmId, wmEvent);
        printf("控件类型: %s\n", (wmEvent == BN_CLICKED) ? "按钮点击" : "其他事件");
        printf("控件句柄: %p\n", hControl);

        // 获取控件详细信息用于调试
        if (hControl) {
            wchar_t className[256];
            GetClassNameW(hControl, className, 256);
            printf("控件类名: %ls\n", className);

            wchar_t text[256];
            GetWindowTextW(hControl, text, 256);
            printf("控件文本: %ls\n", text);

            HWND parent = GetParent(hControl);
            printf("控件父窗口: %p\n", parent);

        }

        // 获取主选项卡 
        HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
        HWND hActivePage = NULL;
        int selectedTab = -1;

        if (hTab) {
            selectedTab = TabCtrl_GetCurSel(hTab);
            printf("当前选中的选项卡: %d\n", selectedTab);

            // 根据选项卡索引获取对应的页面
            switch (selectedTab) {
            case 0: hActivePage = GetDlgItem(hTab, IDC_FLIGHT_PAGE); break;        // 航班查询页面
            case 1: hActivePage = GetDlgItem(hTab, IDC_BOOKING_PAGE); break;        // 我的订单页面
            case 2: hActivePage = GetDlgItem(hTab, IDC_ADMIN_PAGE); break;          // 系统管理页面
            }
        }

        if (!hActivePage) {
            printf("没有找到活动页面\n");
            break;
        }

        printf("活动页面: %p\n", hActivePage);

        // 使用 hActivePage 作为父窗口
        HWND hParent = hActivePage;

        // 直接处理按钮点击
        switch (wmId) {
        case IDC_SEARCH_FLIGHTS:
            printf("=== IDC_SEARCH_FLIGHTS 被点击 ===\n");
            {
                printf("开始处理航班查询...\n");
                HWND hFlightList = GetDlgItem(hActivePage, IDC_FLIGHT_LIST);
                printf("航班列表: %p\n", hFlightList);

                if (hFlightList) {
                    printf("开始刷新航班列表...\n");
                    RefreshEnhancedFlightList(hFlightList);
                    printf("查询完成\n");
                }
                else {
                    printf("!!! 航班列表控件未找到 !!!\n");
                }
            }
            break;

        case IDC_RESET_SEARCH:
            printf("=== IDC_RESET_SEARCH 被点击 ===\n");
            ResetSearchConditions(hWnd);
            break;

        case IDC_BOOK_FLIGHT:
            printf("=== IDC_BOOK_FLIGHT 被点击 ===\n");
            HandleBookFlight(hWnd);
            break;

        case IDC_WAITLIST_BOOKING:
            printf("=== IDC_WAITLIST_BOOKING 被点击 ===\n");
            HandleWaitlistBooking(hWnd);
            break;

        case IDC_REFRESH_BOOKINGS:
            printf("=== IDC_REFRESH_BOOKINGS 被点击 ===\n");
            HandleRefreshBookings(hWnd);
            break;

        case IDC_CANCEL_BOOKING:
            printf("=== IDC_CANCEL_BOOKING 被点击 ===\n");
            HandleCancelBooking(hWnd);
            break;

        case IDC_REFRESH_ADMIN_DATA:
            printf("=== IDC_REFRESH_ADMIN_DATA 被点击 ===\n");
            HandleRefreshAdminData(hWnd);
            break;

        case IDC_ADD_FLIGHT:
            printf("=== IDC_ADD_FLIGHT 被点击 ===\n");
            HandleAddFlight(hWnd);
            break;

        case IDC_DELETE_FLIGHT:
            printf("=== IDC_DELETE_FLIGHT 被点击 ===\n");
            HandleDeleteFlight(hWnd);
            break;

        case IDC_REFRESH_FLIGHTS:
            printf("=== IDC_REFRESH_FLIGHTS 被点击 ===\n");
            HandleRefreshFlights(hWnd);
            break;

        case IDC_ADD_PASSENGER:
            printf("=== IDC_ADD_PASSENGER 被点击 ===\n");
            HandleAddPassenger(hWnd);
            break;

        case IDC_DELETE_PASSENGER:
            printf("=== IDC_DELETE_PASSENGER 被点击 ===\n");
            HandleDeletePassenger(hWnd);
            break;

        case IDC_ADD_WAITLIST:
            printf("=== IDC_ADD_WAITLIST 被点击 ===\n");
            HandleAddWaitlist(hWnd);
            break;

        case IDC_DELETE_WAITLIST:
            printf("=== IDC_DELETE_WAITLIST 被点击 ===\n");
            HandleDeleteWaitlist(hWnd);
            break;

        case IDC_UPDATE_FLIGHT:
            printf("=== IDC_UPDATE_FLIGHT 被点击 ===\n");
            HandleUpdateFlight(hWnd);
            break;

        case 105: // IDM_EXIT
            DestroyWindow(hWnd);
            break;

        default:
            printf("未知命令ID: %d\n", wmId);
            break;
        }
        break;
    }

    case WM_PAINT:
    {
        // 窗口绘制消息
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // 绘制背景
        RECT rect;
        GetClientRect(hWnd, &rect);
        FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));

        // 如果未登录，显示提示信息
        if (!isLoggedIn) {
            DrawText(hdc, L"请先登录系统", -1, &rect,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        // 窗口销毁消息 - 程序退出
        printf("收到 WM_DESTROY 消息\n");
        DisconnectDatabase();
        PostQuitMessage(0);
        break;

    default:
        // 其他未处理的消息交给默认窗口过程处理
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}