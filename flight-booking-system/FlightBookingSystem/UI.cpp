/*用户界面源文件，实现：控制台、用户输入处理、结果显示（航班列表、预订确认）*/
#include "framework.h"
#include "FlightBookingSystem.h"
#include <commctrl.h>

// 页面窗口过程函数 ，处理选项卡页面的窗口消息
LRESULT CALLBACK PageWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SHOWWINDOW:      //页面显示时创建相应内容
        if (wParam) { // 窗口显示时
            printf("=== 页面显示: %p ===\n", hWnd);

            // 获取页面ID
            int pageID = (int)GetWindowLongPtr(hWnd, GWLP_ID);
            printf("页面ID: %d\n", pageID);

            // 清除旧内容
            HWND hChild = GetWindow(hWnd, GW_CHILD);
            while (hChild) {
                DestroyWindow(hChild);
                hChild = GetWindow(hWnd, GW_CHILD);
            }

            // 根据页面ID创建相应内容
            switch (pageID) {
            case IDC_FLIGHT_PAGE:
                CreateEnhancedFlightSearchTab(hWnd);
                break;
            case IDC_BOOKING_PAGE:
                CreateEnhancedMyBookingsTab(hWnd);
                break;
            case IDC_ADMIN_PAGE:
                CreateEnhancedAdminTab(hWnd);
                break;
            }
        }
        break;

    case WM_COMMAND:         //将命令消息转发给主窗口处理
    {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        printf("=== 页面窗口收到 WM_COMMAND ===\n");
        printf("命令ID: %d, 事件: %d\n", wmId, wmEvent);
        printf("页面句柄: %p\n", hWnd);

        // 将消息转发给主窗口
        HWND hTab = GetParent(hWnd);
        HWND hMainWnd = hTab ? GetParent(hTab) : NULL;

        if (hMainWnd) {
            printf("转发消息到主窗口: %p\n", hMainWnd);
            // 使用 SendMessage 确保同步处理
            return SendMessage(hMainWnd, WM_COMMAND, wParam, lParam);
        }
        break;
    }

    case WM_NOTIFY:        //将通知消息转发给主窗口
    {
        // 如果需要处理列表视图等通知消息，也转发给主窗口
        HWND hTab = GetParent(hWnd);
        HWND hMainWnd = hTab ? GetParent(hTab) : NULL;
        if (hMainWnd) {
            return SendMessage(hMainWnd, WM_NOTIFY, wParam, lParam);
        }
        break;
     }
   }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

// 工具函数 
void MyGetDlgItemText(HWND hDlg, int nID, wchar_t* buffer, int bufferSize) {
    //安全获取对话框控件文本内容
    HWND hItem = GetDlgItem(hDlg, nID);
    if (hItem) {
        GetWindowTextW(hItem, buffer, bufferSize);
    }
    else {
        buffer[0] = L'\0';
    }
}

int MyGetDlgItemInt(HWND hDlg, int nID) {
    wchar_t buffer[32];
    MyGetDlgItemText(hDlg, nID, buffer, 32);
    return _wtoi(buffer);
}

void ShowMessage(const wchar_t* message, const wchar_t* title) {
    MessageBoxW(NULL, message, title, MB_OK | MB_ICONINFORMATION);
}

// 创建主界面
void CreateMainInterface(HWND hWnd) {
    // 设置窗口标题，区分管理员和普通用户
    wchar_t title[256];
    if (isAdmin) {
        swprintf(title, 256, L"航空订票管理系统 - 管理员: %s", currentUserName);
    }
    else {
        swprintf(title, 256, L"航空订票管理系统 - 用户: %s", currentUserName);
    }
    SetWindowTextW(hWnd, title);

    // 获取主窗口客户区大小
    RECT rcMain;
    GetClientRect(hWnd, &rcMain);
    int mainWidth = rcMain.right - rcMain.left;
    int mainHeight = rcMain.bottom - rcMain.top;

    printf("主窗口客户区: 宽=%d, 高=%d\n", mainWidth, mainHeight);

    // 创建选项卡控件 - 使用主窗口的整个客户区
    HWND hTab = CreateWindowW(WC_TABCONTROLW, L"",
        WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
        10, 10, mainWidth - 20, mainHeight - 20,  // 使用整个客户区减去边距
        hWnd, (HMENU)IDC_MAIN_TAB, hInst, NULL);

    printf("选项卡控件大小: 宽=%d, 高=%d\n", mainWidth - 20, mainHeight - 20);


    // 添加选项卡
    TCITEMW tie;
    tie.mask = TCIF_TEXT;

    if (isAdmin) {
        const wchar_t* adminTabs[] = { L"航班查询", L"我的订单", L"系统管理" };
        for (int i = 0; i < 3; i++) {
            tie.pszText = (wchar_t*)adminTabs[i];
            TabCtrl_InsertItem(hTab, i, &tie);
        }
    }
    else {
        const wchar_t* tabs[] = { L"航班查询", L"我的订单" };
        for (int i = 0; i < 2; i++) {
            tie.pszText = (wchar_t*)tabs[i];
            TabCtrl_InsertItem(hTab, i, &tie);
        }
    }

    // 创建各选项卡内容
    CreateTabPages(hTab);
}

// 创建选项卡页面
void CreateTabPages(HWND hTab) {
    RECT rc;
    GetClientRect(hTab, &rc);

    // 调整页面区域
    TabCtrl_AdjustRect(hTab, FALSE, &rc);

    // 大幅增加页面高度，确保能显示所有内容
    rc.bottom += 100; 

    printf("选项卡页面区域: 左=%d, 上=%d, 宽=%d, 高=%d\n",
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);

    // 注册页面窗口类
    static BOOL classRegistered = FALSE;
    if (!classRegistered) {
        WNDCLASSW wc = { 0 };
        wc.lpfnWndProc = PageWndProc;
        wc.hInstance = hInst;
        wc.lpszClassName = L"FlightPageClass";
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.style = CS_HREDRAW | CS_VREDRAW;

        if (RegisterClassW(&wc)) {
            classRegistered = TRUE;
            printf("页面窗口类注册成功\n");
        }
        else {
            printf("页面窗口类注册失败\n");
        }
    }

    // 创建所有页面
    HWND hFlightPage = CreateWindowW(L"FlightPageClass", L"",
        WS_CHILD | WS_BORDER,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        hTab, (HMENU)IDC_FLIGHT_PAGE, hInst, NULL);

    HWND hBookingPage = CreateWindowW(L"FlightPageClass", L"",
        WS_CHILD | WS_BORDER,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        hTab, (HMENU)IDC_BOOKING_PAGE, hInst, NULL);

    // 管理员页面
    if (isAdmin) {
        HWND hAdminPage = CreateWindowW(L"FlightPageClass", L"",
            WS_CHILD | WS_BORDER,
            rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
            hTab, (HMENU)IDC_ADMIN_PAGE, hInst, NULL);
    }

    printf("选项卡页面创建完成\n");

    // 显示第一个页面
    ShowWindow(hFlightPage, SW_SHOW);
}

// 创建增强的航班查询选项卡
void CreateEnhancedFlightSearchTab(HWND hParent) {
    printf("创建航班查询界面 - 父窗口: %p\n", hParent);

    // 获取主窗口句柄
    HWND hMainWnd = GetParent(GetParent(hParent));  // hParent -> hTab -> hMainWnd

    // 清除可能存在的旧控件
    HWND hChild = GetWindow(hParent, GW_CHILD);
    while (hChild) {
        DestroyWindow(hChild);
        hChild = GetWindow(hParent, GW_CHILD);
    }

    int y = 10;
    int labelWidth = 80;
    int editWidth = 120;
    int buttonWidth = 100;

    // 使用清晰的列布局
    int col1 = 20;    // 第一列标签
    int col2 = 110;   // 第一列输入框
    int col3 = 250;   // 第二列标签  
    int col4 = 340;   // 第二列输入框

    // === 查询条件区域 ===
    CreateWindowW(L"STATIC", L"查询条件:",
        WS_CHILD | WS_VISIBLE, col1, y, 80, 25, hParent, NULL, hInst, NULL);
    y += 35;

    // 第一行：航班号和航空公司
    CreateWindowW(L"STATIC", L"航班号:", WS_CHILD | WS_VISIBLE, col1, y, labelWidth, 25, hParent, NULL, hInst, NULL);
    CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, col2, y, editWidth, 25, hParent, (HMENU)IDC_FLIGHT_NUMBER, hInst, NULL);

    CreateWindowW(L"STATIC", L"航空公司:", WS_CHILD | WS_VISIBLE, col3, y, labelWidth, 25, hParent, NULL, hInst, NULL);
    CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, col4, y, editWidth, 25, hParent, (HMENU)IDC_AIRLINE, hInst, NULL);
    y += 35;

    // 第二行：出发和到达城市
    CreateWindowW(L"STATIC", L"出发城市:", WS_CHILD | WS_VISIBLE, col1, y, labelWidth, 25, hParent, NULL, hInst, NULL);
    CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, col2, y, editWidth, 25, hParent, (HMENU)IDC_DEPARTURE_CITY, hInst, NULL);

    CreateWindowW(L"STATIC", L"到达城市:", WS_CHILD | WS_VISIBLE, col3, y, labelWidth, 25, hParent, NULL, hInst, NULL);
    CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, col4, y, editWidth, 25, hParent, (HMENU)IDC_ARRIVAL_CITY, hInst, NULL);
    y += 35;

    // 第三行：出发时间和到达时间
    CreateWindowW(L"STATIC", L"出发时间:", WS_CHILD | WS_VISIBLE, col1, y, labelWidth, 25, hParent, NULL, hInst, NULL);
    CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, col2, y, editWidth, 25, hParent, (HMENU)IDC_DEPARTURE_DATE, hInst, NULL);

    CreateWindowW(L"STATIC", L"到达时间:", WS_CHILD | WS_VISIBLE, col3, y, labelWidth, 25, hParent, NULL, hInst, NULL);
    CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, col4, y, editWidth, 25, hParent, (HMENU)IDC_ARRIVAL_DATE, hInst, NULL);
    y += 35;

    // 第四行：舱位等级和查询按钮
    CreateWindowW(L"STATIC", L"舱位等级:", WS_CHILD | WS_VISIBLE, col1, y, labelWidth, 25, hParent, NULL, hInst, NULL);
    HWND hClassCombo = CreateWindowW(L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER,
        col2, y, editWidth, 100, hParent, (HMENU)IDC_CLASS_LEVEL, hInst, NULL);
    SendMessageW(hClassCombo, CB_ADDSTRING, 0, (LPARAM)L"所有舱位");
    SendMessageW(hClassCombo, CB_ADDSTRING, 0, (LPARAM)L"经济舱");
    SendMessageW(hClassCombo, CB_ADDSTRING, 0, (LPARAM)L"商务舱");
    SendMessageW(hClassCombo, CB_ADDSTRING, 0, (LPARAM)L"头等舱");
    SendMessageW(hClassCombo, CB_SETCURSEL, 0, 0);

    // 查询按钮行 
    HWND hSearchBtn = CreateWindowW(L"BUTTON", L"查询航班",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        col3, y, buttonWidth, 30, hParent, (HMENU)IDC_SEARCH_FLIGHTS, hInst, NULL);

    HWND hResetBtn = CreateWindowW(L"BUTTON", L"重置条件",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        col4, y, buttonWidth, 30, hParent, (HMENU)IDC_RESET_SEARCH, hInst, NULL);

    printf("创建的按钮: 查询=%p, 重置=%p\n", hSearchBtn, hResetBtn);
    printf("按钮ID: 查询=%d, 重置=%d\n", IDC_SEARCH_FLIGHTS, IDC_RESET_SEARCH);
    printf("查询按钮父窗口: %p, 主窗口: %p\n", hParent, hMainWnd);

    // === 航班列表区域 ===
    CreateWindowW(L"STATIC", L"航班列表:",
        WS_CHILD | WS_VISIBLE, col1, y, 80, 25, hParent, NULL, hInst, NULL);
    y += 30;

    HWND hFlightList = CreateWindowW(WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,  // 移除 LVS_OWNERDATA
        col1, y, 940, 200, hParent, (HMENU)IDC_FLIGHT_LIST, hInst, NULL);
    ListView_SetExtendedListViewStyle(hFlightList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // 设置列 - 显示更多航班信息
    const wchar_t* cols[] = {
     L"航班号", L"航空公司", L"出发城市", L"到达城市",
     L"出发时间", L"到达时间", L"余票", L"经济舱价", L"商务舱价", L"头等舱价", L"飞行日期", L"飞机型号"
    };
    int widths[] = { 100, 120, 100, 100, 150, 150, 80, 80, 80, 80, 100, 120 };

    // 确保列被正确添加
    LVCOLUMNW lvc = { 0 };
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
    lvc.fmt = LVCFMT_LEFT;

    for (int i = 0; i < 12; i++) {
        LVCOLUMNW lvc = { 0 };
        lvc.mask = LVCF_TEXT | LVCF_WIDTH;
        lvc.pszText = const_cast<wchar_t*>(cols[i]);
        lvc.cx = widths[i];
        ListView_InsertColumn(hFlightList, i, &lvc);
    }
    y += 220;

    // === 预订信息区域 === - 重新设计为两列布局，增加间距
    CreateWindowW(L"STATIC", L"预订信息:",
        WS_CHILD | WS_VISIBLE, col1, y, 80, 25, hParent, NULL, hInst, NULL);
    y += 35;

    // 第一行：乘客姓名和证件号码 - 增加列间距
    int infoCol1 = col1;      // 第一列标签
    int infoCol2 = col2;      // 第一列输入框  
    int infoCol3 = col3 + 50; // 第二列标签（增加间距）
    int infoCol4 = col4 + 50; // 第二列输入框（增加间距）

    CreateWindowW(L"STATIC", L"乘客姓名:", WS_CHILD | WS_VISIBLE, infoCol1, y, 80, 25, hParent, NULL, hInst, NULL);
    CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, infoCol2, y, 150, 25, hParent, (HMENU)IDC_PASSENGER_NAME, hInst, NULL);

    CreateWindowW(L"STATIC", L"证件号码:", WS_CHILD | WS_VISIBLE, infoCol3, y, 80, 25, hParent, NULL, hInst, NULL);
    CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, infoCol4, y, 150, 25, hParent, (HMENU)IDC_ID_CARD, hInst, NULL);
    y += 40;

    // 第二行：性别和订票数量
    CreateWindowW(L"STATIC", L"性别:", WS_CHILD | WS_VISIBLE, infoCol1, y, 60, 25, hParent, NULL, hInst, NULL);
    HWND hGenderCombo = CreateWindowW(L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER,
        infoCol2, y, 80, 100, hParent, (HMENU)IDC_GENDER, hInst, NULL);
    SendMessageW(hGenderCombo, CB_ADDSTRING, 0, (LPARAM)L"男");
    SendMessageW(hGenderCombo, CB_ADDSTRING, 0, (LPARAM)L"女");
    SendMessageW(hGenderCombo, CB_SETCURSEL, 0, 0);

    CreateWindowW(L"STATIC", L"订票数量:", WS_CHILD | WS_VISIBLE, infoCol3, y, 80, 25, hParent, NULL, hInst, NULL);
    CreateWindowW(L"EDIT", L"1", WS_CHILD | WS_VISIBLE | WS_BORDER, infoCol4, y, 60, 25, hParent, (HMENU)IDC_TICKET_COUNT, hInst, NULL);
    y += 40;

    // 第三行：联系电话和邮箱 - 增加间距避免重叠
    CreateWindowW(L"STATIC", L"联系电话:", WS_CHILD | WS_VISIBLE, infoCol1, y, 80, 25, hParent, NULL, hInst, NULL);
    CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, infoCol2, y, 180, 25, hParent, (HMENU)IDC_PASSENGER_PHONE, hInst, NULL);

    CreateWindowW(L"STATIC", L"邮箱:", WS_CHILD | WS_VISIBLE, infoCol3, y, 60, 25, hParent, NULL, hInst, NULL);
    CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, infoCol4, y, 180, 25, hParent, (HMENU)IDC_PASSENGER_EMAIL, hInst, NULL);
    y += 50;

    // === 操作按钮区域 === - 确保按钮可见

    HWND hBookBtn = CreateWindowW(L"BUTTON", L"预订选中航班",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        infoCol2, y, 160, 40, hParent, (HMENU)IDC_BOOK_FLIGHT, hInst, NULL);

    HWND hWaitlistBtn = CreateWindowW(L"BUTTON", L"候补选中航班",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        infoCol4, y, 160, 40, hParent, (HMENU)IDC_WAITLIST_BOOKING, hInst, NULL);

    printf("创建的按钮: 预订=%p, 候补=%p\n", hBookBtn, hWaitlistBtn);
    printf("按钮ID: 预订=%d, 候补=%d\n", IDC_BOOK_FLIGHT, IDC_WAITLIST_BOOKING);
    printf("按钮父窗口: %p, 主窗口: %p\n", hParent, hMainWnd);

    // 测试按钮是否可见和启用
    if (hBookBtn) {
        printf("预订按钮状态: 可见=%d, 启用=%d\n",
            IsWindowVisible(hBookBtn), IsWindowEnabled(hBookBtn));
    }

    printf("航班查询界面创建完成\n");

    // 初始加载航班数据
    RefreshEnhancedFlightList(hFlightList);
}

// 创建航班查询选项卡（原有的实现）
void CreateFlightSearchTab(HWND hParent) {
    // 使用增强版本的函数
    CreateEnhancedFlightSearchTab(hParent);
}

// 创建机票预订选项卡
void CreateEnhancedBookingTab(HWND hParent) {
    // 乘客信息
    CreateWindowW(L"STATIC", L"乘客姓名:",
        WS_CHILD | WS_VISIBLE,
        20, 50, 80, 20, hParent, NULL, hInst, NULL);

    CreateWindowW(L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        100, 50, 120, 20, hParent, (HMENU)IDC_PASSENGER_NAME, hInst, NULL);

    CreateWindowW(L"STATIC", L"订票数量:",
        WS_CHILD | WS_VISIBLE,
        240, 50, 80, 20, hParent, NULL, hInst, NULL);

    CreateWindowW(L"EDIT", L"1",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        320, 50, 60, 20, hParent, (HMENU)IDC_TICKET_COUNT, hInst, NULL);

    CreateWindowW(L"STATIC", L"舱位等级:",
        WS_CHILD | WS_VISIBLE,
        400, 50, 80, 20, hParent, NULL, hInst, NULL);

    HWND hClassCombo = CreateWindowW(L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_DROPDOWNLIST,
        480, 50, 100, 100, hParent, (HMENU)IDC_CLASS_LEVEL, hInst, NULL);

    // 添加舱位选项
    SendMessageW(hClassCombo, CB_ADDSTRING, 0, (LPARAM)L"经济舱");
    SendMessageW(hClassCombo, CB_ADDSTRING, 0, (LPARAM)L"商务舱");
    SendMessageW(hClassCombo, CB_ADDSTRING, 0, (LPARAM)L"头等舱");
    SendMessageW(hClassCombo, CB_SETCURSEL, 0, 0);

    CreateWindowW(L"BUTTON", L"预订选中航班",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        600, 50, 100, 25, hParent, (HMENU)IDC_BOOK_FLIGHT, hInst, NULL);

    CreateWindowW(L"STATIC", L"请先在航班查询中选择要预订的航班，然后填写乘客信息进行预订",
        WS_CHILD | WS_VISIBLE,
        20, 90, 500, 20, hParent, NULL, hInst, NULL);
}

// 创建我的订单选项卡
void CreateEnhancedMyBookingsTab(HWND hParent) {
    printf("=== 创建我的订单页面 ===\n");
    printf("订单页面父窗口: %p\n", hParent);

    // 清除可能存在的旧控件
    HWND hChild = GetWindow(hParent, GW_CHILD);
    while (hChild) {
        DestroyWindow(hChild);
        hChild = GetWindow(hParent, GW_CHILD);
    }

    CreateWindowW(L"STATIC", L"我的订单列表:",
        WS_CHILD | WS_VISIBLE,
        20, 50, 100, 20, hParent, NULL, hInst, NULL);

    HWND hBookingList = CreateWindowW(WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        20, 80, 940, 400, hParent, (HMENU)IDC_BOOKING_LIST, hInst, NULL);
    ListView_SetExtendedListViewStyle(hBookingList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_ONECLICKACTIVATE);

    printf("订单列表控件: %p\n", hBookingList);
    printf("订单列表可见: %d, 启用: %d\n", IsWindowVisible(hBookingList), IsWindowEnabled(hBookingList));

    // 设置列表列 - 修复字符串常量问题，添加机票状态列
    LVCOLUMNW lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    // 使用临时变量 - 添加机票状态列
    wchar_t col0[] = L"订单号";
    wchar_t col1[] = L"乘客姓名";
    wchar_t col2[] = L"航班号";
    wchar_t col3[] = L"出发城市";
    wchar_t col4[] = L"到达城市";
    wchar_t col5[] = L"票数";
    wchar_t col6[] = L"舱位";
    wchar_t col7[] = L"总金额";
    wchar_t col8[] = L"预订时间";
    wchar_t col9[] = L"机票状态";

    lvc.pszText = col0;
    lvc.iSubItem = 0;
    lvc.cx = 80;
    ListView_InsertColumn(hBookingList, 0, &lvc);
    printf("插入列0: 订单号\n");

    lvc.pszText = col1;
    lvc.iSubItem = 1;
    lvc.cx = 100;
    ListView_InsertColumn(hBookingList, 1, &lvc);
    printf("插入列1: 乘客姓名\n");

    lvc.pszText = col2;
    lvc.iSubItem = 2;
    lvc.cx = 100;
    ListView_InsertColumn(hBookingList, 2, &lvc);
    printf("插入列2: 航班号\n");

    lvc.pszText = col3;
    lvc.iSubItem = 3;
    lvc.cx = 80;
    ListView_InsertColumn(hBookingList, 3, &lvc);
    printf("插入列3: 出发城市\n");

    lvc.pszText = col4;
    lvc.iSubItem = 4;
    lvc.cx = 80;
    ListView_InsertColumn(hBookingList, 4, &lvc);
    printf("插入列4: 到达城市\n");

    lvc.pszText = col5;
    lvc.iSubItem = 5;
    lvc.cx = 50;
    ListView_InsertColumn(hBookingList, 5, &lvc);
    printf("插入列5: 票数\n");

    lvc.pszText = col6;
    lvc.iSubItem = 6;
    lvc.cx = 60;
    ListView_InsertColumn(hBookingList, 6, &lvc);
    printf("插入列6: 舱位\n");

    lvc.pszText = col7;
    lvc.iSubItem = 7;
    lvc.cx = 80;
    ListView_InsertColumn(hBookingList, 7, &lvc);
    printf("插入列7: 总金额\n");

    lvc.pszText = col8;
    lvc.iSubItem = 8;
    lvc.cx = 120;
    ListView_InsertColumn(hBookingList, 8, &lvc);
    printf("插入列8: 预订时间\n");

    lvc.pszText = col9;
    lvc.iSubItem = 9;
    lvc.cx = 80;
    ListView_InsertColumn(hBookingList, 9, &lvc);
    printf("插入列9: 机票状态\n");

    // 创建按钮 - 添加详细调试
    HWND hRefreshBtn = CreateWindowW(L"BUTTON", L"刷新订单",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, 490, 80, 25, hParent, (HMENU)IDC_REFRESH_BOOKINGS, hInst, NULL);

    HWND hCancelBtn = CreateWindowW(L"BUTTON", L"取消选中订单",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        120, 490, 100, 25, hParent, (HMENU)IDC_CANCEL_BOOKING, hInst, NULL);

    printf("创建的订单页面按钮:\n");
    printf("  刷新按钮: 句柄=%p, ID=%d, 可见=%d\n",
        hRefreshBtn, IDC_REFRESH_BOOKINGS, IsWindowVisible(hRefreshBtn));
    printf("  取消按钮: 句柄=%p, ID=%d, 可见=%d\n",
        hCancelBtn, IDC_CANCEL_BOOKING, IsWindowVisible(hCancelBtn));

    // 初始加载订单数据
    printf("开始加载订单数据，用户ID: %d\n", currentUserID);
    RefreshBookingListWithStatus(hBookingList, currentUserID);
    printf("订单页面内容创建完成\n");
}

// 创建系统管理选项卡
void CreateAdminTab(HWND hParent) {
    CreateWindowW(L"STATIC", L"航班管理:",
        WS_CHILD | WS_VISIBLE,
        20, 50, 80, 20, hParent, NULL, hInst, NULL);

    CreateWindowW(L"BUTTON", L"添加航班",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        120, 50, 80, 25, hParent, (HMENU)IDC_ADD_FLIGHT, hInst, NULL);

    CreateWindowW(L"BUTTON", L"修改航班",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        220, 50, 80, 25, hParent, (HMENU)IDC_UPDATE_FLIGHT, hInst, NULL);

    CreateWindowW(L"BUTTON", L"删除航班",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        320, 50, 80, 25, hParent, (HMENU)IDC_DELETE_FLIGHT, hInst, NULL);

    CreateWindowW(L"STATIC", L"管理员功能:\n"
        L"1. 添加新航班信息\n"
        L"2. 修改现有航班信息\n"
        L"3. 删除航班信息\n"
        L"4. 查看所有预订记录",
        WS_CHILD | WS_VISIBLE,
        20, 90, 300, 100, hParent, NULL, hInst, NULL);
}

// 刷新航班列表 - 修复价格字段
void RefreshFlightList(HWND hList, const wchar_t* departure, const wchar_t* arrival) {
    // 清空列表
    ListView_DeleteAllItems(hList);

    FlightInfo* flights = NULL;
    int count = 0;

    printf("开始刷新航班列表，出发: %ls, 到达: %ls\n", departure, arrival);

    if (GetFlightsByRoute(departure, arrival, &flights, &count)) {
        if (count > 0) {
            for (int i = 0; i < count; i++) {
                LVITEMW lvi;
                lvi.mask = LVIF_TEXT;
                lvi.iItem = i;
                lvi.iSubItem = 0;
                lvi.pszText = flights[i].flightNumber;
                ListView_InsertItem(hList, &lvi);

                ListView_SetItemText(hList, i, 1, flights[i].airline);
                ListView_SetItemText(hList, i, 2, flights[i].departureCity);
                ListView_SetItemText(hList, i, 3, flights[i].arrivalCity);
                ListView_SetItemText(hList, i, 4, flights[i].departureTime);
                ListView_SetItemText(hList, i, 5, flights[i].arrivalTime);

                wchar_t buffer[32];
                // 舱位等级显示经济舱
                wchar_t cabinClass[] = L"经济舱";
                ListView_SetItemText(hList, i, 6, cabinClass);

                swprintf(buffer, 32, L"%d", flights[i].totalSeats);
                ListView_SetItemText(hList, i, 7, buffer);

                swprintf(buffer, 32, L"%d", flights[i].availableSeats);
                ListView_SetItemText(hList, i, 8, buffer);

                // 使用经济舱价格 - 修复价格字段
                swprintf(buffer, 32, L"%.2f", flights[i].economyPrice);
                ListView_SetItemText(hList, i, 9, buffer);

                ListView_SetItemText(hList, i, 10, flights[i].flightDay);
                ListView_SetItemText(hList, i, 11, flights[i].planeModel);
            }

            wchar_t message[100];
            swprintf(message, 100, L"找到 %d 个航班", count);
            ShowMessage(message, L"查询结果");

            printf("成功显示 %d 个真实航班数据\n", count);
        }
        else {
            ShowMessage(L"没有找到符合条件的航班", L"查询结果");
            printf("未找到符合条件的航班\n");
        }

        if (flights) {
            free(flights);
        }
    }
    else {
        ShowMessage(L"查询失败，请检查数据库连接", L"错误");
        printf("航班查询失败\n");
    }
}

// 添加高级查询函数
void RefreshEnhancedFlightList(HWND hList) {
    printf("=== 开始刷新航班列表 ===\n");

    // 获取查询条件
    wchar_t flightNumber[50] = { 0 };
    wchar_t airline[50] = { 0 };
    wchar_t departure[50] = { 0 };
    wchar_t arrival[50] = { 0 };
    wchar_t depTime[50] = { 0 };
    wchar_t arrTime[50] = { 0 };

    HWND hParent = GetParent(hList);
    printf("父窗口: %p\n", hParent);

    HWND hFlightNum = GetDlgItem(hParent, IDC_FLIGHT_NUMBER);
    HWND hAirline = GetDlgItem(hParent, IDC_AIRLINE);
    HWND hDeparture = GetDlgItem(hParent, IDC_DEPARTURE_CITY);
    HWND hArrival = GetDlgItem(hParent, IDC_ARRIVAL_CITY);
    HWND hDepTime = GetDlgItem(hParent, IDC_DEPARTURE_DATE);
    HWND hArrTime = GetDlgItem(hParent, IDC_ARRIVAL_DATE);

    printf("查询控件: hFlightNum=%p, hAirline=%p, hDeparture=%p, hArrival=%p\n",
        hFlightNum, hAirline, hDeparture, hArrival);

    if (hFlightNum) GetWindowTextW(hFlightNum, flightNumber, 50);
    if (hAirline) GetWindowTextW(hAirline, airline, 50);
    if (hDeparture) GetWindowTextW(hDeparture, departure, 50);
    if (hArrival) GetWindowTextW(hArrival, arrival, 50);
    if (hDepTime) GetWindowTextW(hDepTime, depTime, 50);
    if (hArrTime) GetWindowTextW(hArrTime, arrTime, 50);

    printf("实际查询参数: 航班号='%ls', 航空公司='%ls', 出发='%ls', 到达='%ls'\n",
        flightNumber, airline, departure, arrival);

    // 清空列表
    ListView_DeleteAllItems(hList);
    printf("列表清空完成\n");

    FlightInfo* flights = NULL;
    int count = 0;

    printf("调用 GetFlightsAdvanced...\n");
    
    // 使用空字符串代替NULL
    wchar_t empty[2] = L"";
    BOOL queryResult = GetFlightsAdvanced(
        (flightNumber[0] ? flightNumber : empty),
        (airline[0] ? airline : empty),
        (departure[0] ? departure : empty),
        (arrival[0] ? arrival : empty),
        (depTime[0] ? depTime : empty),
        (arrTime[0] ? arrTime : empty),
        &flights, &count);

    if (queryResult) {
        printf("查询成功，找到 %d 条记录\n", count);
        if (queryResult) {
            printf("查询成功，找到 %d 条记录\n", count);

            if (count > 0) {
                // 确保列表是空的
                ListView_DeleteAllItems(hList);

                for (int i = 0; i < count; i++) {
                    LVITEMW lvi;
                    lvi.mask = LVIF_TEXT | LVIF_PARAM;
                    lvi.iItem = i;
                    lvi.iSubItem = 0;
                    lvi.pszText = flights[i].flightNumber;
                    lvi.lParam = flights[i].flightID;

                    // 插入项目
                    int itemIndex = ListView_InsertItem(hList, &lvi);
                    printf("插入项目 %d: %ls, 索引: %d\n", i, flights[i].flightNumber, itemIndex);

                    if (itemIndex != -1) {
                        // 修正列顺序 - 确保数据正确对应列标题
                        ListView_SetItemText(hList, itemIndex, 1, flights[i].airline);
                        ListView_SetItemText(hList, itemIndex, 2, flights[i].departureCity);
                        ListView_SetItemText(hList, itemIndex, 3, flights[i].arrivalCity);
                        ListView_SetItemText(hList, itemIndex, 4, flights[i].departureTime);
                        ListView_SetItemText(hList, itemIndex, 5, flights[i].arrivalTime);

                        wchar_t buffer[32];

                        // 第6列：余票量
                        swprintf(buffer, 32, L"%d", flights[i].availableSeats);
                        ListView_SetItemText(hList, itemIndex, 6, buffer);

                        // 第7列：经济舱价格
                        swprintf(buffer, 32, L"%.2f", flights[i].economyPrice);
                        ListView_SetItemText(hList, itemIndex, 7, buffer);

                        // 第8列：商务舱价格
                        swprintf(buffer, 32, L"%.2f", flights[i].businessPrice);
                        ListView_SetItemText(hList, itemIndex, 8, buffer);

                        // 第9列：头等舱价格
                        swprintf(buffer, 32, L"%.2f", flights[i].firstClassPrice);
                        ListView_SetItemText(hList, itemIndex, 9, buffer);

                        // 第10列：飞行日期
                        ListView_SetItemText(hList, itemIndex, 10, flights[i].flightDay);

                        // 第11列：飞机型号
                        ListView_SetItemText(hList, itemIndex, 11, flights[i].planeModel);

                        printf("成功设置项目 %d 的数据\n", itemIndex);
                    }
                    else {
                        printf("!!! 插入项目失败: %d !!!\n", i);
                    }
                }
                printf("成功显示 %d 个航班\n", count);

                // 强制重绘列表
                InvalidateRect(hList, NULL, TRUE);
                UpdateWindow(hList);
            }

            wchar_t message[100];
            swprintf(message, 100, L"找到 %d 个符合条件的航班", count);
            ShowMessage(message, L"查询结果");
        }
        else {
            ShowMessage(L"没有找到符合条件的航班", L"查询结果");
            printf("未找到符合条件的航班\n");
        }

        if (flights) {
            free(flights);
        }
    }
    else {
        ShowMessage(L"查询失败，请检查数据库连接", L"错误");
        printf("航班查询失败\n");
    }
}

// 添加候补预订函数
BOOL AddWaitlistBooking(int flightID) {
    // 获取乘客信息
    wchar_t passengerName[100] = { 0 };
    wchar_t idCard[20] = { 0 };
    wchar_t gender[10] = { 0 };
    int ticketCount = 1;
    int classLevel = 1;

    HWND hParent = GetActiveWindow();
    HWND hName = GetDlgItem(hParent, IDC_PASSENGER_NAME);
    HWND hIdCard = GetDlgItem(hParent, IDC_ID_CARD);
    HWND hGender = GetDlgItem(hParent, IDC_GENDER);
    HWND hTicketCount = GetDlgItem(hParent, IDC_TICKET_COUNT);
    HWND hClassLevel = GetDlgItem(hParent, IDC_CLASS_LEVEL);

    if (hName) GetWindowTextW(hName, passengerName, 100);
    if (hIdCard) GetWindowTextW(hIdCard, idCard, 20);
    if (hGender) {
        int sel = (int)SendMessageW(hGender, CB_GETCURSEL, 0, 0);
        if (sel == 0) wcscpy_s(gender, 10, L"男");
        else if (sel == 1) wcscpy_s(gender, 10, L"女");
    }
    if (hTicketCount) {
        wchar_t countText[32];
        GetWindowTextW(hTicketCount, countText, 32);
        ticketCount = _wtoi(countText);
    }
    if (hClassLevel) {
        classLevel = (int)SendMessageW(hClassLevel, CB_GETCURSEL, 0, 0) + 1;
    }

    // 验证输入
    if (wcslen(passengerName) == 0) {
        ShowMessage(L"请输入乘客姓名", L"错误");
        return FALSE;
    }
    if (wcslen(idCard) == 0) {
        ShowMessage(L"请输入证件号码", L"错误");
        return FALSE;
    }
    if (ticketCount <= 0) {
        ShowMessage(L"请输入有效的票数", L"错误");
        return FALSE;
    }

    if (AddToWaitlist(flightID, passengerName, idCard, gender, ticketCount, classLevel)) {
        ShowMessage(L"候补预订成功!", L"成功");
        return TRUE;
    }
    else {
        ShowMessage(L"候补预订失败", L"错误");
        return FALSE;
    }
}

// 刷新订单列表
void RefreshBookingList(HWND hList, int userID) {
    // 清空列表
    ListView_DeleteAllItems(hList);

    BookingInfo* bookings = NULL;
    int count = 0;

    if (GetUserBookings(userID, &bookings, &count)) {
        for (int i = 0; i < count; i++) {
            LVITEMW lvi;
            lvi.mask = LVIF_TEXT;
            lvi.iItem = i;
            lvi.iSubItem = 0;

            wchar_t bookingID[20];
            swprintf(bookingID, 20, L"%d", bookings[i].bookingID);
            lvi.pszText = bookingID;
            ListView_InsertItem(hList, &lvi);

            ListView_SetItemText(hList, i, 1, bookings[i].passengerName);
            ListView_SetItemText(hList, i, 2, bookings[i].flightNumber);
            ListView_SetItemText(hList, i, 3, bookings[i].departureCity);
            ListView_SetItemText(hList, i, 4, bookings[i].arrivalCity);

            wchar_t buffer[32];
            swprintf(buffer, 32, L"%d", bookings[i].ticketCount);
            ListView_SetItemText(hList, i, 5, buffer);

            // 舱位等级显示
            wchar_t classLevel[20];
            switch (bookings[i].classLevel) {
            case 1: swprintf(classLevel, 20, L"经济舱"); break;
            case 2: swprintf(classLevel, 20, L"商务舱"); break;
            case 3: swprintf(classLevel, 20, L"头等舱"); break;
            default: swprintf(classLevel, 20, L"未知"); break;
            }
            ListView_SetItemText(hList, i, 6, classLevel);

            swprintf(buffer, 32, L"%.2f", bookings[i].totalAmount);
            ListView_SetItemText(hList, i, 7, buffer);

            ListView_SetItemText(hList, i, 8, bookings[i].bookingDate);
        }

        if (bookings) {
            free(bookings);
        }
    }
    else {
        ShowMessage(L"获取订单失败", L"错误");
    }
}

// 创建登录对话框
void CreateLoginDialog(HWND hWnd) {
    printf("创建登录对话框\n");

    // 直接显示登录对话框模态
    INT_PTR result = DialogBox(hInst, MAKEINTRESOURCE(IDD_LOGIN_DIALOG), hWnd, LoginDialogProc);

    if (result == IDOK) {
        // 登录成功，创建主界面
        printf("登录成功，创建主界面\n");
        CreateMainInterface(hWnd);
    }
    else {
        // 登录取消或失败，退出程序
        printf("登录取消或失败，退出程序\n");
        PostQuitMessage(0);
    }
}

// 登录对话框过程函数
INT_PTR CALLBACK LoginDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        // 设置默认值
        SetDlgItemText(hDlg, IDC_USERNAME, L"customer1");
        SetDlgItemText(hDlg, IDC_PASSWORD, L"123456");
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_LOGIN:
        {
            wchar_t username[100], password[100];
            GetDlgItemText(hDlg, IDC_USERNAME, username, 100);
            GetDlgItemText(hDlg, IDC_PASSWORD, password, 100);

            if (wcslen(username) == 0 || wcslen(password) == 0) {
                MessageBox(hDlg, L"请输入用户名和密码!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }

            if (LoginUser(username, password)) {
                EndDialog(hDlg, IDOK);
            }
            else {
                MessageBox(hDlg, L"登录失败! 请检查用户名和密码。", L"错误", MB_OK | MB_ICONERROR);
            }
        }
        break;

        case IDC_REGISTER:
        {
            wchar_t username[100], password[100], fullname[100];
            GetDlgItemText(hDlg, IDC_USERNAME, username, 100);
            GetDlgItemText(hDlg, IDC_PASSWORD, password, 100);

            // 使用用户名作为默认全名
            wcscpy_s(fullname, username);

            if (wcslen(username) == 0 || wcslen(password) == 0) {
                MessageBox(hDlg, L"请输入用户名和密码!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }

            if (RegisterUser(username, password, fullname)) {
                MessageBox(hDlg, L"注册成功! 请使用新账号登录。", L"成功", MB_OK | MB_ICONINFORMATION);
            }
            else {
                MessageBox(hDlg, L"注册失败! 用户名可能已存在。", L"错误", MB_OK | MB_ICONERROR);
            }
        }
        break;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        break;
    }
    return FALSE;
}


// 重置查询条件
void ResetSearchConditions(HWND hWnd) {
    printf("=== 重置查询条件 ===\n");

    // 找到航班查询页面
    HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
    if (!hTab) return;

    HWND hFlightPage = GetDlgItem(hTab, IDC_FLIGHT_PAGE);
    if (!hFlightPage) return;

    // 清空所有输入框
    HWND hControls[] = {
        GetDlgItem(hFlightPage, IDC_FLIGHT_NUMBER),
        GetDlgItem(hFlightPage, IDC_AIRLINE),
        GetDlgItem(hFlightPage, IDC_DEPARTURE_CITY),
        GetDlgItem(hFlightPage, IDC_ARRIVAL_CITY),
        GetDlgItem(hFlightPage, IDC_DEPARTURE_DATE),
        GetDlgItem(hFlightPage, IDC_ARRIVAL_DATE),
        GetDlgItem(hFlightPage, IDC_PASSENGER_NAME),
        GetDlgItem(hFlightPage, IDC_ID_CARD),
        GetDlgItem(hFlightPage, IDC_PASSENGER_PHONE),
        GetDlgItem(hFlightPage, IDC_PASSENGER_EMAIL)
    };

    const wchar_t* controlNames[] = {
        L"航班号", L"航空公司", L"出发城市", L"到达城市", L"出发时间",
        L"到达时间", L"乘客姓名", L"证件号码", L"联系电话", L"邮箱"
    };

    for (int i = 0; i < 10; i++) {
        if (hControls[i]) {
            SetWindowTextW(hControls[i], L"");
            printf("清空: %ls\n", controlNames[i]);
        }
        else {
            printf("控件未找到: %ls\n", controlNames[i]);
        }
    }

    // 重置下拉框和票数
    HWND hCabinCombo = GetDlgItem(hFlightPage, IDC_CLASS_LEVEL);
    HWND hGenderCombo = GetDlgItem(hFlightPage, IDC_GENDER);
    HWND hCountEdit = GetDlgItem(hFlightPage, IDC_TICKET_COUNT);

    if (hCabinCombo) {
        SendMessageW(hCabinCombo, CB_SETCURSEL, 0, 0);
        printf("重置舱位等级\n");
    }
    if (hGenderCombo) {
        SendMessageW(hGenderCombo, CB_SETCURSEL, 0, 0);
        printf("重置性别\n");
    }
    if (hCountEdit) {
        SetWindowTextW(hCountEdit, L"1");
        printf("重置票数\n");
    }

    // 刷新显示所有航班
    HWND hFlightList = GetDlgItem(hFlightPage, IDC_FLIGHT_LIST);
    if (hFlightList) {
        RefreshEnhancedFlightList(hFlightList);
        printf("刷新航班列表完成\n");
    }

    printf("重置条件完成\n");
}

// 处理航班查询
void HandleFlightSearch(HWND hWnd) {
    printf("=== 执行航班查询 ===\n");

    HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
    if (!hTab) {
        printf("选项卡未找到\n");
        return;
    }

    HWND hFlightPage = GetDlgItem(hTab, IDC_FLIGHT_PAGE);
    if (!hFlightPage) {
        printf("航班页面未找到\n");
        return;
    }

    HWND hFlightList = GetDlgItem(hFlightPage, IDC_FLIGHT_LIST);
    if (hFlightList) {
        printf("开始刷新航班列表...\n");
        RefreshEnhancedFlightList(hFlightList);
        printf("查询完成\n");
    }
    else {
        printf("!!! 航班列表控件未找到 !!!\n");
    }
}

// 处理机票预订 
void HandleBookFlight(HWND hWnd) {
    printf("=== 执行机票预订 ===\n");

    HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
    if (!hTab) return;

    HWND hFlightPage = GetDlgItem(hTab, IDC_FLIGHT_PAGE);
    if (!hFlightPage) return;

    HWND hFlightList = GetDlgItem(hFlightPage, IDC_FLIGHT_LIST);
    if (!hFlightList) {
        ShowMessage(L"航班列表控件未找到", L"错误");
        return;
    }

    int selectedIndex = ListView_GetNextItem(hFlightList, -1, LVNI_SELECTED);
    if (selectedIndex == -1) {
        ShowMessage(L"请先选择一个航班", L"提示");
        return;
    }

    printf("选中航班索引: %d\n", selectedIndex);

    // 获取航班号
    wchar_t flightNumber[50] = { 0 };
    ListView_GetItemText(hFlightList, selectedIndex, 0, flightNumber, 50);
    printf("选中的航班号: %ls\n", flightNumber);

    int flightID = GetFlightIDByNumber(flightNumber);
    printf("对应的航班ID: %d\n", flightID);

    if (flightID == 0) {
        ShowMessage(L"获取航班信息失败", L"错误");
        return;
    }

    // 获取订票数量
    int ticketCount = 1;
    HWND hCountEdit = GetDlgItem(hFlightPage, IDC_TICKET_COUNT);
    if (hCountEdit) {
        wchar_t countText[32];
        GetWindowTextW(hCountEdit, countText, 32);
        ticketCount = _wtoi(countText);
        if (ticketCount <= 0) ticketCount = 1;
    }

    // 检查余票
    int availableSeats = GetFlightAvailableSeats(flightID);
    printf("航班 %ls 余票: %d\n", flightNumber, availableSeats);

    if (availableSeats <= 0) {
        // 没有余票，询问是否候补
        wchar_t message[256];
        swprintf(message, 256, L"航班 %ls 没有余票！是否加入候补？", flightNumber);

        if (MessageBoxW(hWnd, message, L"没有余票", MB_YESNO | MB_ICONQUESTION) == IDYES) {
            // 用户选择候补
            printf("用户选择候补航班 %ls\n", flightNumber);
            HandleWaitlistBooking(hWnd);
        }
        else {
            printf("用户取消候补\n");
        }
        return;
    }

    // 正常预订流程
    // 获取乘客详细信息
    wchar_t passengerName[100] = { 0 };
    wchar_t idCard[20] = { 0 };
    wchar_t gender[10] = { 0 };
    wchar_t phone[20] = { 0 };
    wchar_t email[100] = { 0 };
    int classLevel = 1;

    // 获取输入控件
    HWND hNameEdit = GetDlgItem(hFlightPage, IDC_PASSENGER_NAME);
    HWND hIdCardEdit = GetDlgItem(hFlightPage, IDC_ID_CARD);
    HWND hGenderCombo = GetDlgItem(hFlightPage, IDC_GENDER);
    HWND hPhoneEdit = GetDlgItem(hFlightPage, IDC_PASSENGER_PHONE);
    HWND hEmailEdit = GetDlgItem(hFlightPage, IDC_PASSENGER_EMAIL);
    HWND hClassCombo = GetDlgItem(hFlightPage, IDC_CLASS_LEVEL);

    // 获取输入值
    if (hNameEdit) GetWindowTextW(hNameEdit, passengerName, 100);
    if (hIdCardEdit) GetWindowTextW(hIdCardEdit, idCard, 20);
    if (hGenderCombo) {
        int sel = (int)SendMessageW(hGenderCombo, CB_GETCURSEL, 0, 0);
        if (sel == 0) wcscpy_s(gender, 10, L"男");
        else if (sel == 1) wcscpy_s(gender, 10, L"女");
    }
    if (hPhoneEdit) GetWindowTextW(hPhoneEdit, phone, 20);
    if (hEmailEdit) GetWindowTextW(hEmailEdit, email, 100);
    if (hClassCombo) {
        classLevel = (int)SendMessageW(hClassCombo, CB_GETCURSEL, 0, 0);
        printf("选择的舱位索引: %d\n", classLevel);

        // 修正舱位等级映射
        switch (classLevel) {
        case 0: classLevel = 1; break; // 所有舱位 -> 经济舱
        case 1: classLevel = 1; break; // 经济舱
        case 2: classLevel = 2; break; // 商务舱  
        case 3: classLevel = 3; break; // 头等舱
        default: classLevel = 1; break;
        }
        printf("实际预订舱位: %d\n", classLevel);
    }

    printf("预订信息: 乘客=%ls, 证件=%ls, 性别=%ls, 电话=%ls, 邮箱=%ls, 票数=%d, 舱位=%d\n",
        passengerName, idCard, gender, phone, email, ticketCount, classLevel);

    // 验证输入
    if (wcslen(passengerName) == 0) {
        ShowMessage(L"请输入乘客姓名", L"错误");
        return;
    }
    if (wcslen(idCard) == 0) {
        ShowMessage(L"请输入证件号码", L"错误");
        return;
    }

    // 执行预订
    if (BookFlightWithDetails(flightID, passengerName, idCard, gender, phone, email, ticketCount, classLevel)) {
        ShowMessage(L"预订成功!", L"成功");
        // 刷新航班列表显示最新余票
        RefreshEnhancedFlightList(hFlightList);

        // 刷新订单列表
        HWND hBookingPage = GetDlgItem(hTab, IDC_BOOKING_PAGE);
        if (hBookingPage) {
            HWND hBookingList = GetDlgItem(hBookingPage, IDC_BOOKING_LIST);
            if (hBookingList) {
                RefreshBookingListWithStatus(hBookingList, currentUserID);
            }
        }
    }
    else {
        ShowMessage(L"预订失败，请检查余票或联系管理员", L"错误");
    }
}

// 处理候补预订 - 修改逻辑
void HandleWaitlistBooking(HWND hWnd) {
    printf("=== 执行候补预订 ===\n");

    HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
    if (!hTab) return;

    HWND hFlightPage = GetDlgItem(hTab, IDC_FLIGHT_PAGE);
    if (!hFlightPage) return;

    HWND hFlightList = GetDlgItem(hFlightPage, IDC_FLIGHT_LIST);
    if (!hFlightList) {
        ShowMessage(L"航班列表控件未找到", L"错误");
        return;
    }

    int selectedIndex = ListView_GetNextItem(hFlightList, -1, LVNI_SELECTED);
    if (selectedIndex == -1) {
        ShowMessage(L"请先选择一个航班", L"提示");
        return;
    }

    // 获取航班号
    wchar_t flightNumber[50] = { 0 };
    ListView_GetItemText(hFlightList, selectedIndex, 0, flightNumber, 50);
    int flightID = GetFlightIDByNumber(flightNumber);

    if (flightID == 0) {
        ShowMessage(L"获取航班信息失败", L"错误");
        return;
    }

    // 检查余票
    int ticketCount = 1;
    HWND hCountEdit = GetDlgItem(hFlightPage, IDC_TICKET_COUNT);
    if (hCountEdit) {
        wchar_t countText[32];
        GetWindowTextW(hCountEdit, countText, 32);
        ticketCount = _wtoi(countText);
        if (ticketCount <= 0) ticketCount = 1;
    }

    int availableSeats = GetFlightAvailableSeats(flightID);
    if (availableSeats >= ticketCount) {
        // 有余票，询问是否预订
        wchar_t message[256];
        swprintf(message, 256, L"航班 %ls 还有余票！当前余票：%d。是否直接预订？",
            flightNumber, availableSeats);

        if (MessageBoxW(hWnd, message, L"有余票", MB_YESNO | MB_ICONQUESTION) == IDYES) {
            // 用户选择预订
            HandleBookFlight(hWnd);
        }
        return;
    }

    // 正常候补流程
    // 获取乘客信息
    wchar_t passengerName[100] = { 0 };
    wchar_t idCard[20] = { 0 };
    wchar_t gender[10] = { 0 };
    int classLevel = 1;

    HWND hNameEdit = GetDlgItem(hFlightPage, IDC_PASSENGER_NAME);
    HWND hIdCardEdit = GetDlgItem(hFlightPage, IDC_ID_CARD);
    HWND hGenderCombo = GetDlgItem(hFlightPage, IDC_GENDER);
    HWND hClassCombo = GetDlgItem(hFlightPage, IDC_CLASS_LEVEL);

    if (hNameEdit) GetWindowTextW(hNameEdit, passengerName, 100);
    if (hIdCardEdit) GetWindowTextW(hIdCardEdit, idCard, 20);
    if (hGenderCombo) {
        int sel = (int)SendMessageW(hGenderCombo, CB_GETCURSEL, 0, 0);
        if (sel == 0) wcscpy_s(gender, 10, L"男");
        else if (sel == 1) wcscpy_s(gender, 10, L"女");
    }
    if (hClassCombo) {
        classLevel = (int)SendMessageW(hClassCombo, CB_GETCURSEL, 0, 0) + 1;
    }

    // 验证输入
    if (wcslen(passengerName) == 0) {
        ShowMessage(L"请输入乘客姓名", L"错误");
        return;
    }
    if (wcslen(idCard) == 0) {
        ShowMessage(L"请输入证件号码", L"错误");
        return;
    }

    // 添加候补预订
    if (AddToWaitlist(flightID, passengerName, idCard, gender, ticketCount, classLevel)) {
        ShowMessage(L"候补预订成功!", L"成功");

        // 刷新订单列表显示候补状态
        HWND hBookingPage = GetDlgItem(hTab, IDC_BOOKING_PAGE);
        if (hBookingPage) {
            HWND hBookingList = GetDlgItem(hBookingPage, IDC_BOOKING_LIST);
            if (hBookingList) {
                RefreshBookingListWithStatus(hBookingList, currentUserID);
            }
        }
    }
    else {
        ShowMessage(L"候补预订失败", L"错误");
    }
}

void HandleRefreshBookings(HWND hWnd) {
    printf("=== 刷新订单列表 ===\n");

    HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
    if (!hTab) {
        printf("!!! 选项卡未找到 !!!\n");
        return;
    }

    HWND hBookingPage = GetDlgItem(hTab, IDC_BOOKING_PAGE);
    if (!hBookingPage) {
        printf("!!! 订单页面未找到 !!!\n");
        return;
    }

    HWND hBookingList = GetDlgItem(hBookingPage, IDC_BOOKING_LIST);
    if (hBookingList) {
        printf("开始刷新订单列表，用户ID: %d\n", currentUserID);
        RefreshBookingList(hBookingList, currentUserID);
        ShowMessage(L"订单列表已刷新", L"提示");
        printf("订单列表刷新完成\n");
    }
    else {
        printf("!!! 订单列表控件未找到 !!!\n");
        ShowMessage(L"订单列表控件未找到", L"错误");
    }
}

void HandleCancelBooking(HWND hWnd) {
    printf("=== 执行取消预订 ===\n");

    HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
    if (!hTab) {
        printf("!!! 选项卡未找到 !!!\n");
        return;
    }

    HWND hBookingPage = GetDlgItem(hTab, IDC_BOOKING_PAGE);
    if (!hBookingPage) {
        printf("!!! 订单页面未找到 !!!\n");
        return;
    }

    HWND hBookingList = GetDlgItem(hBookingPage, IDC_BOOKING_LIST);
    if (!hBookingList) {
        ShowMessage(L"订单列表控件未找到", L"错误");
        printf("!!! 订单列表控件未找到 !!!\n");
        return;
    }

    // 获取选中的订单
    int selectedIndex = ListView_GetNextItem(hBookingList, -1, LVNI_SELECTED);
    printf("选中的订单索引: %d\n", selectedIndex);

    if (selectedIndex == -1) {
        ShowMessage(L"请先选择一个订单", L"提示");
        printf("没有选中的订单\n");
        return;
    }

    // 获取订单ID、状态和金额信息
    wchar_t bookingIDText[50] = { 0 };
    ListView_GetItemText(hBookingList, selectedIndex, 0, bookingIDText, 50);
    int recordID = _wtoi(bookingIDText);

    wchar_t statusText[50] = { 0 };
    ListView_GetItemText(hBookingList, selectedIndex, 9, statusText, 50);

    wchar_t passengerName[100] = { 0 };
    ListView_GetItemText(hBookingList, selectedIndex, 1, passengerName, 100);

    wchar_t amountText[50] = { 0 };
    ListView_GetItemText(hBookingList, selectedIndex, 7, amountText, 50);
    float amount = (float)_wtof(amountText);

    printf("要处理的记录ID: %d, 状态: %ls, 乘客: %ls, 金额: %.2f\n",
        recordID, statusText, passengerName, amount);

    if (recordID == 0) {
        ShowMessage(L"获取订单信息失败", L"错误");
        printf("!!! 记录ID无效 !!!\n");
        return;
    }

    // 根据状态文本判断类型，而不是金额
    BOOL isWaitlist = (wcscmp(statusText, L"候补中") == 0 || wcscmp(statusText, L"部分成功") == 0);

    printf("记录类型判断: 状态=%ls, 是否为候补=%d\n", statusText, isWaitlist);

    // 确认对话框
    wchar_t message[256];
    if (isWaitlist) {
        swprintf(message, 256, L"确定要取消乘客 %ls 的候补申请吗？", passengerName);
    }
    else {
        swprintf(message, 256, L"确定要取消乘客 %ls 的预订吗？", passengerName);
    }

    if (MessageBoxW(hWnd, message, L"确认取消", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        printf("用户确认取消操作\n");

        BOOL success = FALSE;

        // 根据记录类型执行不同的取消逻辑
        if (isWaitlist) {
            // 取消候补申请
            printf("执行取消候补申请，候补ID: %d\n", recordID);
            success = DeleteWaitlistBooking(recordID);
        }
        else {
            // 取消普通预订（包括候补成功的预订）
            printf("执行取消预订，预订ID: %d\n", recordID);
            success = CancelBooking(recordID);
        }

        if (success) {
            wchar_t successMsg[100];
            if (isWaitlist) {
                swprintf(successMsg, 100, L"取消候补成功!");
            }
            else {
                swprintf(successMsg, 100, L"取消预订成功!");
            }
            ShowMessage(successMsg, L"成功");
            printf("取消操作成功\n");

            // 刷新订单列表
            RefreshBookingListWithStatus(hBookingList, currentUserID);
            printf("订单列表已刷新\n");
        }
        else {
            wchar_t errorMsg[100];
            if (isWaitlist) {
                swprintf(errorMsg, 100, L"取消候补失败");
            }
            else {
                swprintf(errorMsg, 100, L"取消预订失败");
            }
            ShowMessage(errorMsg, L"错误");
            printf("取消操作失败\n");
        }
    }
    else {
        printf("用户取消操作\n");
    }
}

void HandleRefreshAdminData(HWND hWnd) {
    printf("=== 开始处理刷新订单 ===\n");
    HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
    if (!hTab) return;

    HWND hAdminPage = GetDlgItem(hTab, IDC_ADMIN_PAGE);
    if (hAdminPage) {
        RefreshAdminData(hAdminPage);
    }
}

void HandleAddFlight(HWND hWnd) {
    printf("=== 执行添加航班 ===\n");

    // 显示添加航班对话框
    INT_PTR result = DialogBox(hInst, MAKEINTRESOURCE(IDD_ADD_FLIGHT_DIALOG), hWnd, AddFlightDialogProc);

    if (result == IDOK) {
        // 刷新管理员航班列表
        HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
        if (hTab) {
            HWND hAdminPage = GetDlgItem(hTab, IDC_ADMIN_PAGE);
            if (hAdminPage) {
                HWND hFlightList = GetDlgItem(hAdminPage, IDC_ADMIN_FLIGHT_LIST);
                if (hFlightList) {
                    RefreshAdminFlightList(hFlightList);
                }
            }
        }
        ShowMessage(L"航班添加成功!", L"成功");
    }
}

void HandleDeleteFlight(HWND hWnd) {
    printf("=== 执行删除航班 ===\n");

    HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
    if (!hTab) return;

    HWND hAdminPage = GetDlgItem(hTab, IDC_ADMIN_PAGE);
    if (!hAdminPage) return;

    HWND hFlightList = GetDlgItem(hAdminPage, IDC_ADMIN_FLIGHT_LIST);
    if (!hFlightList) {
        ShowMessage(L"航班列表控件未找到", L"错误");
        return;
    }

    int selectedIndex = ListView_GetNextItem(hFlightList, -1, LVNI_SELECTED);
    if (selectedIndex == -1) {
        ShowMessage(L"请先选择一个航班", L"提示");
        return;
    }

    // 获取航班号
    wchar_t flightNumber[50] = { 0 };
    ListView_GetItemText(hFlightList, selectedIndex, 0, flightNumber, 50);
    int flightID = GetFlightIDByNumber(flightNumber);

    if (flightID == 0) {
        ShowMessage(L"获取航班信息失败", L"错误");
        return;
    }

    // 确认删除
    wchar_t message[100];
    swprintf(message, 100, L"确定要删除航班 %ls 吗？", flightNumber);

    if (MessageBoxW(hWnd, message, L"确认删除", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        if (DeleteFlight(flightID)) {
            ShowMessage(L"删除航班成功!", L"成功");
            // 刷新航班列表
            RefreshAdminFlightList(hFlightList);
        }
        else {
            ShowMessage(L"删除航班失败! 可能存在相关预订。", L"错误");
        }
    }
}

void HandleRefreshFlights(HWND hWnd) {
    HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
    if (!hTab) return;

    HWND hAdminPage = GetDlgItem(hTab, IDC_ADMIN_PAGE);
    if (!hAdminPage) return;

    HWND hFlightList = GetDlgItem(hAdminPage, IDC_ADMIN_FLIGHT_LIST);
    if (hFlightList) {
        RefreshAdminFlightList(hFlightList);
        ShowMessage(L"航班列表已刷新", L"提示");
    }
}

void CreateEnhancedAdminTab(HWND hParent) {
    printf("创建管理员界面\n");

    // 清除可能存在的旧控件
    HWND hChild = GetWindow(hParent, GW_CHILD);
    while (hChild) {
        DestroyWindow(hChild);
        hChild = GetWindow(hParent, GW_CHILD);
    }

    // 获取父窗口客户区大小
    RECT rcParent;
    GetClientRect(hParent, &rcParent);
    int parentWidth = rcParent.right - rcParent.left;
    int parentHeight = rcParent.bottom - rcParent.top;

    printf("管理员页面实际尺寸: 宽=%d, 高=%d\n", parentWidth, parentHeight);

    // 使用更合理的布局参数
    int y = 10;
    int labelWidth = 100;
    int buttonWidth = 100;
    int buttonHeight = 25;
    int buttonSpacing = 10;
    int sectionSpacing = 20;
    int listHeight = 150;
    int col1 = 20;

    // === 航班管理区域 ===
    // 先创建"航班管理"标签，放在最上面
    CreateWindowW(L"STATIC", L"航班管理:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        col1, y, 200, 20, hParent, NULL, hInst, NULL);
    y += 25;  // 增加间距，确保标签和按钮不重叠

    // 航班管理按钮 - 水平排列，增加间距
    int buttonStartY = y;
    CreateWindowW(L"BUTTON", L"添加航班",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        col1, buttonStartY, buttonWidth, buttonHeight, hParent, (HMENU)IDC_ADD_FLIGHT, hInst, NULL);

    CreateWindowW(L"BUTTON", L"修改航班",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        col1 + buttonWidth + buttonSpacing, buttonStartY, buttonWidth, buttonHeight, hParent, (HMENU)IDC_UPDATE_FLIGHT, hInst, NULL);

    CreateWindowW(L"BUTTON", L"删除航班",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        col1 + (buttonWidth + buttonSpacing) * 2, buttonStartY, buttonWidth, buttonHeight, hParent, (HMENU)IDC_DELETE_FLIGHT, hInst, NULL);

    y += buttonHeight + 15;  // 按钮高度 + 额外间距

    // 航班列表标签
    CreateWindowW(L"STATIC", L"航班列表:",
        WS_CHILD | WS_VISIBLE,
        col1, y, 150, 20, hParent, NULL, hInst, NULL);
    y += 25;

    // 航班列表
    HWND hFlightList = CreateWindowW(WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        col1, y, parentWidth - 40, listHeight, hParent, (HMENU)IDC_ADMIN_FLIGHT_LIST, hInst, NULL);
    ListView_SetExtendedListViewStyle(hFlightList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // 设置航班列表列
    printf("设置航班列表列...\n");
    const wchar_t* flightCols[] = {
        L"航班号", L"航空公司", L"出发城市", L"到达城市", L"出发时间", L"到达时间",
        L"总座位", L"余票", L"经济舱价", L"商务舱价", L"头等舱价", L"飞行日期", L"飞机型号"
    };
    int flightWidths[] = { 100, 120, 100, 100, 150, 150, 80, 80, 80, 80, 80, 100, 120 };

    LVCOLUMNW lvc = { 0 };
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
    lvc.fmt = LVCFMT_LEFT;

    for (int i = 0; i < 13; i++) {
        lvc.pszText = const_cast<wchar_t*>(flightCols[i]);
        lvc.cx = flightWidths[i];
        int result = ListView_InsertColumn(hFlightList, i, &lvc);
        printf("插入航班列 %d: %ls, 结果: %d\n", i, flightCols[i], result);
    }

    y += listHeight + sectionSpacing;

    // === 乘客管理区域 ===
    // 乘客管理标签
    CreateWindowW(L"STATIC", L"乘客管理:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        col1, y, 200, 20, hParent, NULL, hInst, NULL);
    y += 25;

    // 乘客管理按钮 - 水平排列
    buttonStartY = y;
    CreateWindowW(L"BUTTON", L"添加乘客",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        col1, buttonStartY, buttonWidth, buttonHeight, hParent, (HMENU)IDC_ADD_PASSENGER, hInst, NULL);

    CreateWindowW(L"BUTTON", L"删除乘客",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        col1 + buttonWidth + buttonSpacing, buttonStartY, buttonWidth, buttonHeight, hParent, (HMENU)IDC_DELETE_PASSENGER, hInst, NULL);

    y += buttonHeight + 15;

    // 乘客列表标签
    CreateWindowW(L"STATIC", L"所有乘客预订:",
        WS_CHILD | WS_VISIBLE,
        col1, y, 150, 20, hParent, NULL, hInst, NULL);
    y += 25;

    // 乘客列表
    HWND hPassengerList = CreateWindowW(WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
        col1, y, parentWidth - 40, listHeight, hParent, (HMENU)IDC_ADMIN_PASSENGER_LIST, hInst, NULL);
    ListView_SetExtendedListViewStyle(hPassengerList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // 设置乘客列表列
    printf("设置乘客列表列...\n");
    const wchar_t* passengerCols[] = {
        L"订单号", L"用户ID", L"乘客姓名", L"证件号", L"性别", L"航班号",
        L"出发城市", L"到达城市", L"票数", L"舱位", L"总金额", L"预订时间"
    };
    int passengerWidths[] = { 80, 60, 100, 120, 60, 100, 80, 80, 50, 60, 80, 120 };

    for (int i = 0; i < 12; i++) {
        lvc.pszText = const_cast<wchar_t*>(passengerCols[i]);
        lvc.cx = passengerWidths[i];
        int result = ListView_InsertColumn(hPassengerList, i, &lvc);
        printf("插入乘客列 %d: %ls, 结果: %d\n", i, passengerCols[i], result);
    }
    y += listHeight + sectionSpacing;

    // === 候补管理区域 ===
    // 候补管理标签
    CreateWindowW(L"STATIC", L"候补管理:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        col1, y, 200, 20, hParent, NULL, hInst, NULL);
    y += 25;

    // 候补管理按钮 - 水平排列
    buttonStartY = y;
    CreateWindowW(L"BUTTON", L"添加候补",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        col1, buttonStartY, buttonWidth, buttonHeight, hParent, (HMENU)IDC_ADD_WAITLIST, hInst, NULL);

    CreateWindowW(L"BUTTON", L"删除候补",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        col1 + buttonWidth + buttonSpacing, buttonStartY, buttonWidth, buttonHeight, hParent, (HMENU)IDC_DELETE_WAITLIST, hInst, NULL);

    y += buttonHeight + 15;

    // 候补列表标签
    CreateWindowW(L"STATIC", L"候补预订列表:",
        WS_CHILD | WS_VISIBLE,
        col1, y, 150, 20, hParent, NULL, hInst, NULL);
    y += 25;

    // 候补列表
    HWND hWaitlistList = CreateWindowW(WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
        col1, y, parentWidth - 40, listHeight, hParent, (HMENU)IDC_ADMIN_WAITLIST_LIST, hInst, NULL);
    ListView_SetExtendedListViewStyle(hWaitlistList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // 设置候补列表列
    printf("设置候补列表列...\n");
    const wchar_t* waitlistCols[] = {
        L"候补ID", L"用户ID", L"乘客姓名", L"证件号", L"性别", L"航班号",
        L"出发城市", L"到达城市", L"票数", L"舱位", L"状态", L"申请时间"
    };
    int waitlistWidths[] = { 80, 60, 100, 120, 60, 100, 80, 80, 50, 60, 80, 120 };

    for (int i = 0; i < 12; i++) {
        lvc.pszText = const_cast<wchar_t*>(waitlistCols[i]);
        lvc.cx = waitlistWidths[i];
        int result = ListView_InsertColumn(hWaitlistList, i, &lvc);
        printf("插入候补列 %d: %ls, 结果: %d\n", i, waitlistCols[i], result);
    }
    y += listHeight + 20;

    // 刷新数据按钮 - 放在底部
    CreateWindowW(L"BUTTON", L"刷新所有数据",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        col1, y, 150, 30, hParent, (HMENU)IDC_REFRESH_ADMIN_DATA, hInst, NULL);

    printf("管理员界面创建完成，总高度: %d\n", y);

    // 初始加载数据
    RefreshAdminData(hParent);
}

// 刷新候补列表
void RefreshWaitlistList(HWND hList) {
    printf("刷新候补列表\n");

    // 清空列表
    ListView_DeleteAllItems(hList);

    WaitlistBooking* waitlists = NULL;
    int count = 0;

    if (GetWaitlistBookings(&waitlists, &count)) {
        for (int i = 0; i < count; i++) {
            LVITEMW lvi;
            lvi.mask = LVIF_TEXT;
            lvi.iItem = i;
            lvi.iSubItem = 0;

            wchar_t waitlistID[20];
            swprintf(waitlistID, 20, L"%d", waitlists[i].waitlistID);
            lvi.pszText = waitlistID;
            ListView_InsertItem(hList, &lvi);

            // 用户ID
            wchar_t userID[20];
            swprintf(userID, 20, L"%d", waitlists[i].userID);
            ListView_SetItemText(hList, i, 1, userID);

            // 乘客信息
            ListView_SetItemText(hList, i, 2, waitlists[i].passengerName);
            ListView_SetItemText(hList, i, 3, waitlists[i].idCardNumber);
            ListView_SetItemText(hList, i, 4, waitlists[i].gender);

            // 航班信息
            ListView_SetItemText(hList, i, 5, waitlists[i].flightNumber);
            ListView_SetItemText(hList, i, 6, waitlists[i].departureCity);
            ListView_SetItemText(hList, i, 7, waitlists[i].arrivalCity);

            // 票数和舱位
            wchar_t ticketCount[20];
            swprintf(ticketCount, 20, L"%d", waitlists[i].ticketCount);
            ListView_SetItemText(hList, i, 8, ticketCount);

            wchar_t classLevel[20];
            switch (waitlists[i].classLevel) {
            case 1: swprintf(classLevel, 20, L"经济舱"); break;
            case 2: swprintf(classLevel, 20, L"商务舱"); break;
            case 3: swprintf(classLevel, 20, L"头等舱"); break;
            default: swprintf(classLevel, 20, L"未知"); break;
            }
            ListView_SetItemText(hList, i, 9, classLevel);

            // 状态和申请时间
            wchar_t displayStatus[20] = { 0 };
            if (wcscmp(waitlists[i].status, L"waiting") == 0) {
                wcscpy_s(displayStatus, 20, L"候补中");
            }
            else if (wcscmp(waitlists[i].status, L"partially_success") == 0) {
                wcscpy_s(displayStatus, 20, L"部分成功");
            }
            else {
                wcscpy_s(displayStatus, 20, waitlists[i].status);
            }
            ListView_SetItemText(hList, i, 10, displayStatus);
            ListView_SetItemText(hList, i, 11, waitlists[i].waitlistDate);
            printf("候补记录 %d: ID=%d, 乘客=%ls, 状态=%ls->%ls, 票数=%d\n",
                i, waitlists[i].waitlistID, waitlists[i].passengerName,
                waitlists[i].status, displayStatus, waitlists[i].ticketCount);
        }

        if (waitlists) {
            free(waitlists);
        }

        printf("成功显示 %d 条候补记录\n", count);
    }
    else {
        printf("获取候补信息失败\n");
    }
}

// 刷新乘客列表
void RefreshPassengerList(HWND hList) {
    printf("刷新乘客列表\n");

    // 清空列表
    ListView_DeleteAllItems(hList);

    BookingInfo* bookings = NULL;
    int count = 0;

    if (GetAllPassengerBookings(&bookings, &count)) {
        for (int i = 0; i < count; i++) {
            LVITEMW lvi;
            lvi.mask = LVIF_TEXT;
            lvi.iItem = i;
            lvi.iSubItem = 0;

            wchar_t bookingID[20];
            swprintf(bookingID, 20, L"%d", bookings[i].bookingID);
            lvi.pszText = bookingID;
            ListView_InsertItem(hList, &lvi);

            // 用户ID
            wchar_t userID[20];
            swprintf(userID, 20, L"%d", bookings[i].userID);
            ListView_SetItemText(hList, i, 1, userID);

            // 乘客姓名
            ListView_SetItemText(hList, i, 2, bookings[i].passengerName);

            // 获取乘客详细信息
            PassengerDetail* details = NULL;
            int detailCount = 0;
            if (GetPassengerDetails(bookings[i].bookingID, &details, &detailCount) && detailCount > 0) {
                // 证件号
                ListView_SetItemText(hList, i, 3, details[0].idCardNumber);
                // 性别
                ListView_SetItemText(hList, i, 4, details[0].gender);

                if (details) {
                    free(details);
                }
            }
            else {
                // 如果没有详细信息，显示默认值
               // 使用字符串变量
                wchar_t unknown[] = L"未知";
                wchar_t unknownId[] = L"未知";

                // 证件号
                ListView_SetItemText(hList, i, 3, unknownId);
                // 性别
                ListView_SetItemText(hList, i, 4, unknown);
            }

            // 航班信息
            ListView_SetItemText(hList, i, 5, bookings[i].flightNumber);
            ListView_SetItemText(hList, i, 6, bookings[i].departureCity);
            ListView_SetItemText(hList, i, 7, bookings[i].arrivalCity);

            // 票数
            wchar_t ticketCount[20];
            swprintf(ticketCount, 20, L"%d", bookings[i].ticketCount);
            ListView_SetItemText(hList, i, 8, ticketCount);

            // 舱位等级
            wchar_t classLevel[20];
            switch (bookings[i].classLevel) {
            case 1: swprintf(classLevel, 20, L"经济舱"); break;
            case 2: swprintf(classLevel, 20, L"商务舱"); break;
            case 3: swprintf(classLevel, 20, L"头等舱"); break;
            default: swprintf(classLevel, 20, L"未知"); break;
            }
            ListView_SetItemText(hList, i, 9, classLevel);

            // 总金额
            wchar_t totalAmount[32];
            swprintf(totalAmount, 32, L"%.2f", bookings[i].totalAmount);
            ListView_SetItemText(hList, i, 10, totalAmount);

            // 预订时间
            ListView_SetItemText(hList, i, 11, bookings[i].bookingDate);
        }

        if (bookings) {
            free(bookings);
        }

        printf("成功显示 %d 条乘客预订记录\n", count);
    }
    else {
        printf("获取乘客预订信息失败\n");
        ShowMessage(L"获取乘客信息失败", L"错误");
    }
}

// 显示预订详情对话框
void ShowBookingDetailsDialog(int bookingID) {
    // 空实现 - 后续完善
    printf("ShowBookingDetailsDialog 函数暂未实现，bookingID: %d\n", bookingID);
}



// 刷新管理员航班列表
void RefreshAdminFlightList(HWND hList) {
    printf("刷新管理员航班列表\n");

    // 清空列表
    ListView_DeleteAllItems(hList);

    FlightInfo* flights = NULL;
    int count = 0;

    if (GetAllFlights(&flights, &count)) {
        for (int i = 0; i < count; i++) {
            LVITEMW lvi;
            lvi.mask = LVIF_TEXT;
            lvi.iItem = i;
            lvi.iSubItem = 0;
            lvi.pszText = flights[i].flightNumber;
            ListView_InsertItem(hList, &lvi);

            // 设置各列数据
            ListView_SetItemText(hList, i, 1, flights[i].airline);
            ListView_SetItemText(hList, i, 2, flights[i].departureCity);
            ListView_SetItemText(hList, i, 3, flights[i].arrivalCity);
            ListView_SetItemText(hList, i, 4, flights[i].departureTime);
            ListView_SetItemText(hList, i, 5, flights[i].arrivalTime);

            wchar_t buffer[32];
            swprintf(buffer, 32, L"%d", flights[i].totalSeats);
            ListView_SetItemText(hList, i, 6, buffer);

            swprintf(buffer, 32, L"%d", flights[i].availableSeats);
            ListView_SetItemText(hList, i, 7, buffer);

            swprintf(buffer, 32, L"%.2f", flights[i].economyPrice);
            ListView_SetItemText(hList, i, 8, buffer);

            swprintf(buffer, 32, L"%.2f", flights[i].businessPrice);
            ListView_SetItemText(hList, i, 9, buffer);

            swprintf(buffer, 32, L"%.2f", flights[i].firstClassPrice);
            ListView_SetItemText(hList, i, 10, buffer);

            ListView_SetItemText(hList, i, 11, flights[i].flightDay);
            ListView_SetItemText(hList, i, 12, flights[i].planeModel);
        }

        if (flights) {
            free(flights);
        }
    }
}

// 刷新所有管理员数据
void RefreshAdminData(HWND hParent) {
    printf("刷新管理员数据\n");

    // 刷新航班列表
    HWND hFlightList = GetDlgItem(hParent, IDC_ADMIN_FLIGHT_LIST);
    if (hFlightList) {
        RefreshAdminFlightList(hFlightList);
    }

    // 刷新乘客列表
    HWND hPassengerList = GetDlgItem(hParent, IDC_ADMIN_PASSENGER_LIST);
    if (hPassengerList) {
        RefreshPassengerList(hPassengerList);
    }

    // 刷新候补列表
    HWND hWaitlistList = GetDlgItem(hParent, IDC_ADMIN_WAITLIST_LIST);
    if (hWaitlistList) {
        RefreshWaitlistList(hWaitlistList);
    }

    ShowMessage(L"管理员数据刷新完成", L"提示");
}

// 预订详情对话框过程函数
INT_PTR CALLBACK BookingDetailsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    // 空实现
    return FALSE;
}

// 刷新带状态的订单列表
void RefreshBookingListWithStatus(HWND hList, int userID) {
    // 清空列表
    ListView_DeleteAllItems(hList);

    BookingInfo* bookings = NULL;
    int count = 0;

    printf("开始刷新订单列表，用户ID: %d\n", userID);

    if (GetUserBookingsWithStatus(userID, &bookings, &count)) {
        printf("成功获取 %d 条订单记录\n", count);

        for (int i = 0; i < count; i++) {
            LVITEMW lvi;
            lvi.mask = LVIF_TEXT;
            lvi.iItem = i;
            lvi.iSubItem = 0;

            wchar_t bookingID[20];
            swprintf(bookingID, 20, L"%d", bookings[i].bookingID);
            lvi.pszText = bookingID;
            int itemIndex = ListView_InsertItem(hList, &lvi);

            printf("插入订单 %d: ID=%d, 乘客=%ls, 状态=%ls\n",
                i, bookings[i].bookingID, bookings[i].passengerName, bookings[i].status);

            ListView_SetItemText(hList, i, 1, bookings[i].passengerName);
            ListView_SetItemText(hList, i, 2, bookings[i].flightNumber);
            ListView_SetItemText(hList, i, 3, bookings[i].departureCity);
            ListView_SetItemText(hList, i, 4, bookings[i].arrivalCity);

            wchar_t buffer[32];
            swprintf(buffer, 32, L"%d", bookings[i].ticketCount);
            ListView_SetItemText(hList, i, 5, buffer);

            // 舱位等级显示
            wchar_t classLevel[20];
            switch (bookings[i].classLevel) {
            case 1: swprintf(classLevel, 20, L"经济舱"); break;
            case 2: swprintf(classLevel, 20, L"商务舱"); break;
            case 3: swprintf(classLevel, 20, L"头等舱"); break;
            default: swprintf(classLevel, 20, L"未知"); break;
            }
            ListView_SetItemText(hList, i, 6, classLevel);

            swprintf(buffer, 32, L"%.2f", bookings[i].totalAmount);
            ListView_SetItemText(hList, i, 7, buffer);

            // 显示正确的预订时间
            ListView_SetItemText(hList, i, 8, bookings[i].bookingDate);

            // 机票状态 - 直接使用从数据库获取的状态
            ListView_SetItemText(hList, i, 9, bookings[i].status);

            printf("订单 %d 最终显示状态: %ls\n", i, bookings[i].status);
        }

        if (bookings) {
            free(bookings);
        }

        printf("订单列表刷新完成，共显示 %d 条记录\n", count);
    }
    else {
        printf("获取订单失败\n");
        ShowMessage(L"获取订单失败", L"错误");
    }
}

// 添加航班对话框过程函数
INT_PTR CALLBACK AddFlightDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        // 设置默认值 - 但不要设置余票数，让用户输入
        SetDlgItemText(hDlg, IDC_EDIT_TOTAL_SEATS, L"180");
        // 不要设置余票数，让用户单独输入
        SetDlgItemText(hDlg, IDC_EDIT_ECONOMY_PRICE, L"800.00");
        SetDlgItemText(hDlg, IDC_EDIT_BUSINESS_PRICE, L"1200.00");
        SetDlgItemText(hDlg, IDC_EDIT_FIRSTCLASS_PRICE, L"1600.00");
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BUTTON_SAVE_FLIGHT: {
            // 获取输入数据
            FlightInfo flight = { 0 };

            GetDlgItemText(hDlg, IDC_EDIT_FLIGHT_NUMBER, flight.flightNumber, 20);
            GetDlgItemText(hDlg, IDC_EDIT_AIRLINE, flight.airline, 50);
            GetDlgItemText(hDlg, IDC_EDIT_DEPARTURE_CITY, flight.departureCity, 50);
            GetDlgItemText(hDlg, IDC_EDIT_ARRIVAL_CITY, flight.arrivalCity, 50);
            GetDlgItemText(hDlg, IDC_EDIT_DEPARTURE_TIME, flight.departureTime, 50);
            GetDlgItemText(hDlg, IDC_EDIT_ARRIVAL_TIME, flight.arrivalTime, 50);
            GetDlgItemText(hDlg, IDC_EDIT_FLIGHT_DAY, flight.flightDay, 20);
            GetDlgItemText(hDlg, IDC_EDIT_PLANE_MODEL, flight.planeModel, 30);

            wchar_t buffer[32];

            // 获取总座位数
            GetDlgItemText(hDlg, IDC_EDIT_TOTAL_SEATS, buffer, 32);
            flight.totalSeats = _wtoi(buffer);

            // 获取余票数 - 单独获取，不与总座位数绑定
            GetDlgItemText(hDlg, IDC_EDIT_AVAILABLE_SEATS, buffer, 32);
            flight.availableSeats = _wtoi(buffer);

            // 验证余票数不超过总座位数
            if (flight.availableSeats > flight.totalSeats) {
                MessageBox(hDlg, L"余票数不能超过总座位数!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }

            GetDlgItemText(hDlg, IDC_EDIT_ECONOMY_PRICE, buffer, 32);
            flight.economyPrice = (float)_wtof(buffer);

            GetDlgItemText(hDlg, IDC_EDIT_BUSINESS_PRICE, buffer, 32);
            flight.businessPrice = (float)_wtof(buffer);

            GetDlgItemText(hDlg, IDC_EDIT_FIRSTCLASS_PRICE, buffer, 32);
            flight.firstClassPrice = (float)_wtof(buffer);

            // 验证输入
            if (wcslen(flight.flightNumber) == 0 || wcslen(flight.airline) == 0 ||
                wcslen(flight.departureCity) == 0 || wcslen(flight.arrivalCity) == 0) {
                MessageBox(hDlg, L"请填写完整的航班信息!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }

            if (flight.totalSeats <= 0) {
                MessageBox(hDlg, L"总座位数必须大于0!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }

            if (flight.availableSeats < 0) {
                MessageBox(hDlg, L"余票数不能为负数!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }

            if (AddFlight(&flight)) {
                MessageBox(hDlg, L"添加航班成功!", L"成功", MB_OK | MB_ICONINFORMATION);
                EndDialog(hDlg, IDOK);
            }
            else {
                MessageBox(hDlg, L"添加航班失败!", L"错误", MB_OK | MB_ICONERROR);
            }
            break;
        }

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        break;
    }
    return FALSE;
}

// 处理添加乘客
void HandleAddPassenger(HWND hWnd) {
    printf("=== 执行添加乘客 ===\n");

    // 显示添加乘客对话框
    INT_PTR result = DialogBox(hInst, MAKEINTRESOURCE(IDD_ADD_PASSENGER_DIALOG), hWnd, AddPassengerDialogProc);

    if (result == IDOK) {
        // 刷新乘客列表
        HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
        if (hTab) {
            HWND hAdminPage = GetDlgItem(hTab, IDC_ADMIN_PAGE);
            if (hAdminPage) {
                HWND hPassengerList = GetDlgItem(hAdminPage, IDC_ADMIN_PASSENGER_LIST);
                if (hPassengerList) {
                    RefreshPassengerList(hPassengerList);
                }
            }
        }
        ShowMessage(L"乘客添加成功!", L"成功");
    }
}

// 处理删除乘客
void HandleDeletePassenger(HWND hWnd) {
    printf("=== 执行删除乘客 ===\n");

    HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
    if (!hTab) return;

    HWND hAdminPage = GetDlgItem(hTab, IDC_ADMIN_PAGE);
    if (!hAdminPage) return;

    HWND hPassengerList = GetDlgItem(hAdminPage, IDC_ADMIN_PASSENGER_LIST);
    if (!hPassengerList) {
        ShowMessage(L"乘客列表控件未找到", L"错误");
        return;
    }

    int selectedIndex = ListView_GetNextItem(hPassengerList, -1, LVNI_SELECTED);
    if (selectedIndex == -1) {
        ShowMessage(L"请先选择一个乘客预订", L"提示");
        return;
    }

    // 获取预订ID
    wchar_t bookingIDText[50] = { 0 };
    ListView_GetItemText(hPassengerList, selectedIndex, 0, bookingIDText, 50);
    int bookingID = _wtoi(bookingIDText);

    if (bookingID == 0) {
        ShowMessage(L"获取预订信息失败", L"错误");
        return;
    }

    // 获取乘客姓名用于确认
    wchar_t passengerName[100] = { 0 };
    ListView_GetItemText(hPassengerList, selectedIndex, 2, passengerName, 100);

    // 确认删除
    wchar_t message[256];
    swprintf(message, 256, L"确定要删除乘客 %ls 的预订吗？", passengerName);

    if (MessageBoxW(hWnd, message, L"确认删除", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        if (DeletePassengerBooking(bookingID)) {
            ShowMessage(L"删除乘客预订成功!", L"成功");
            // 刷新乘客列表
            RefreshPassengerList(hPassengerList);
        }
        else {
            ShowMessage(L"删除乘客预订失败", L"错误");
        }
    }
}

// 处理添加候补
void HandleAddWaitlist(HWND hWnd) {
    printf("=== 执行添加候补 ===\n");

    // 显示添加候补对话框
    INT_PTR result = DialogBox(hInst, MAKEINTRESOURCE(IDD_ADD_WAITLIST_DIALOG), hWnd, AddWaitlistDialogProc);

    if (result == IDOK) {
        // 刷新候补列表
        HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
        if (hTab) {
            HWND hAdminPage = GetDlgItem(hTab, IDC_ADMIN_PAGE);
            if (hAdminPage) {
                HWND hWaitlistList = GetDlgItem(hAdminPage, IDC_ADMIN_WAITLIST_LIST);
                if (hWaitlistList) {
                    RefreshWaitlistList(hWaitlistList);
                }
            }
        }
        ShowMessage(L"候补添加成功!", L"成功");
    }
}

// 处理删除候补
void HandleDeleteWaitlist(HWND hWnd) {
    printf("=== 执行删除候补 ===\n");

    HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
    if (!hTab) return;

    HWND hAdminPage = GetDlgItem(hTab, IDC_ADMIN_PAGE);
    if (!hAdminPage) return;

    HWND hWaitlistList = GetDlgItem(hAdminPage, IDC_ADMIN_WAITLIST_LIST);
    if (!hWaitlistList) {
        ShowMessage(L"候补列表控件未找到", L"错误");
        return;
    }

    int selectedIndex = ListView_GetNextItem(hWaitlistList, -1, LVNI_SELECTED);
    if (selectedIndex == -1) {
        ShowMessage(L"请先选择一个候补预订", L"提示");
        return;
    }

    // 获取候补ID
    wchar_t waitlistIDText[50] = { 0 };
    ListView_GetItemText(hWaitlistList, selectedIndex, 0, waitlistIDText, 50);
    int waitlistID = _wtoi(waitlistIDText);

    if (waitlistID == 0) {
        ShowMessage(L"获取候补信息失败", L"错误");
        return;
    }

    // 获取乘客姓名用于确认
    wchar_t passengerName[100] = { 0 };
    ListView_GetItemText(hWaitlistList, selectedIndex, 2, passengerName, 100);

    // 确认删除
    wchar_t message[256];
    swprintf(message, 256, L"确定要删除乘客 %ls 的候补预订吗？", passengerName);

    if (MessageBoxW(hWnd, message, L"确认删除", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        if (DeleteWaitlistBooking(waitlistID)) {
            ShowMessage(L"删除候补预订成功!", L"成功");
            // 刷新候补列表
            RefreshWaitlistList(hWaitlistList);
        }
        else {
            ShowMessage(L"删除候补预订失败", L"错误");
        }
    }
}

// 添加乘客对话框过程函数
INT_PTR CALLBACK AddPassengerDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        // 初始化性别下拉框
    {
        HWND hGender = GetDlgItem(hDlg, IDC_EDIT_PASSENGER_GENDER);
        SendMessageW(hGender, CB_ADDSTRING, 0, (LPARAM)L"男");
        SendMessageW(hGender, CB_ADDSTRING, 0, (LPARAM)L"女");
        SendMessageW(hGender, CB_SETCURSEL, 0, 0);
    }

    // 初始化舱位下拉框
    {
        HWND hClass = GetDlgItem(hDlg, IDC_COMBO_PASSENGER_CLASS);
        SendMessageW(hClass, CB_ADDSTRING, 0, (LPARAM)L"经济舱");
        SendMessageW(hClass, CB_ADDSTRING, 0, (LPARAM)L"商务舱");
        SendMessageW(hClass, CB_ADDSTRING, 0, (LPARAM)L"头等舱");
        SendMessageW(hClass, CB_SETCURSEL, 0, 0);
    }

    // 初始化航班下拉框
    {
        HWND hFlight = GetDlgItem(hDlg, IDC_COMBO_PASSENGER_FLIGHT);
        FlightInfo* flights = NULL;
        int count = 0;
        if (GetAllFlights(&flights, &count)) {
            for (int i = 0; i < count; i++) {
                wchar_t flightInfo[256];
                swprintf(flightInfo, 256, L"%ls - %ls→%ls",
                    flights[i].flightNumber,
                    flights[i].departureCity,
                    flights[i].arrivalCity);
                SendMessageW(hFlight, CB_ADDSTRING, 0, (LPARAM)flightInfo);
            }
            if (flights) free(flights);
        }
        if (count > 0) {
            SendMessageW(hFlight, CB_SETCURSEL, 0, 0);
        }
    }

    // 设置默认票数
    SetDlgItemText(hDlg, IDC_EDIT_PASSENGER_TICKETS, L"1");
    return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BUTTON_SAVE_PASSENGER: {
            // 获取输入数据
            wchar_t passengerName[100] = { 0 };
            wchar_t idCard[20] = { 0 };
            wchar_t gender[10] = { 0 };
            wchar_t phone[20] = { 0 };
            wchar_t email[100] = { 0 };
            int ticketCount, classLevel;

            GetDlgItemText(hDlg, IDC_EDIT_PASSENGER_NAME, passengerName, 100);
            GetDlgItemText(hDlg, IDC_EDIT_PASSENGER_IDCARD, idCard, 20);

            HWND hGender = GetDlgItem(hDlg, IDC_EDIT_PASSENGER_GENDER);
            int genderSel = (int)SendMessageW(hGender, CB_GETCURSEL, 0, 0);
            if (genderSel == 0) wcscpy_s(gender, 10, L"男");
            else wcscpy_s(gender, 10, L"女");

            GetDlgItemText(hDlg, IDC_EDIT_PASSENGER_PHONE, phone, 20);
            GetDlgItemText(hDlg, IDC_EDIT_PASSENGER_EMAIL, email, 100);

            wchar_t ticketText[32];
            GetDlgItemText(hDlg, IDC_EDIT_PASSENGER_TICKETS, ticketText, 32);
            ticketCount = _wtoi(ticketText);
            if (ticketCount <= 0) ticketCount = 1;

            HWND hClass = GetDlgItem(hDlg, IDC_COMBO_PASSENGER_CLASS);
            classLevel = (int)SendMessageW(hClass, CB_GETCURSEL, 0, 0) + 1;

            // 获取选中的航班
            HWND hFlight = GetDlgItem(hDlg, IDC_COMBO_PASSENGER_FLIGHT);
            int flightIndex = (int)SendMessageW(hFlight, CB_GETCURSEL, 0, 0);
            if (flightIndex == -1) {
                MessageBox(hDlg, L"请选择一个航班!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }

            // 获取航班号
            wchar_t flightInfo[256];
            SendMessageW(hFlight, CB_GETLBTEXT, flightIndex, (LPARAM)flightInfo);
            wchar_t* dashPos = wcschr(flightInfo, L'-');
            wchar_t flightNumber[20] = { 0 };
            if (dashPos) {
                wcsncpy_s(flightNumber, 20, flightInfo, dashPos - flightInfo);
                // 去除尾部空格
                wchar_t* end = flightNumber + wcslen(flightNumber) - 1;
                while (end >= flightNumber && *end == L' ') *end-- = L'\0';
            }

            int flightID = GetFlightIDByNumber(flightNumber);
            if (flightID == 0) {
                MessageBox(hDlg, L"获取航班信息失败!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }

            // 验证输入
            if (wcslen(passengerName) == 0) {
                MessageBox(hDlg, L"请输入乘客姓名!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }
            if (wcslen(idCard) == 0) {
                MessageBox(hDlg, L"请输入证件号码!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }

            // 使用管理员权限创建预订
            int originalUserID = currentUserID;
            currentUserID = 1; // 使用管理员用户ID

            if (BookFlightWithDetails(flightID, passengerName, idCard, gender, phone, email, ticketCount, classLevel)) {
                currentUserID = originalUserID; // 恢复原用户ID
                MessageBox(hDlg, L"乘客预订添加成功!", L"成功", MB_OK | MB_ICONINFORMATION);
                EndDialog(hDlg, IDOK);
            }
            else {
                currentUserID = originalUserID; // 恢复原用户ID
                MessageBox(hDlg, L"乘客预订添加失败!", L"错误", MB_OK | MB_ICONERROR);
            }
            break;
        }

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        break;
    }
    return FALSE;
}

// 添加候补对话框过程函数
INT_PTR CALLBACK AddWaitlistDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        // 初始化性别下拉框
    {
        HWND hGender = GetDlgItem(hDlg, IDC_EDIT_WAITLIST_GENDER);
        SendMessageW(hGender, CB_ADDSTRING, 0, (LPARAM)L"男");
        SendMessageW(hGender, CB_ADDSTRING, 0, (LPARAM)L"女");
        SendMessageW(hGender, CB_SETCURSEL, 0, 0);
    }

    // 初始化舱位下拉框
    {
        HWND hClass = GetDlgItem(hDlg, IDC_COMBO_WAITLIST_CLASS);
        SendMessageW(hClass, CB_ADDSTRING, 0, (LPARAM)L"经济舱");
        SendMessageW(hClass, CB_ADDSTRING, 0, (LPARAM)L"商务舱");
        SendMessageW(hClass, CB_ADDSTRING, 0, (LPARAM)L"头等舱");
        SendMessageW(hClass, CB_SETCURSEL, 0, 0);
    }

    // 初始化航班下拉框
    {
        HWND hFlight = GetDlgItem(hDlg, IDC_COMBO_WAITLIST_FLIGHT);
        FlightInfo* flights = NULL;
        int count = 0;
        if (GetAllFlights(&flights, &count)) {
            for (int i = 0; i < count; i++) {
                wchar_t flightInfo[256];
                swprintf(flightInfo, 256, L"%ls - %ls→%ls",
                    flights[i].flightNumber,
                    flights[i].departureCity,
                    flights[i].arrivalCity);
                SendMessageW(hFlight, CB_ADDSTRING, 0, (LPARAM)flightInfo);
            }
            if (flights) free(flights);
        }
        if (count > 0) {
            SendMessageW(hFlight, CB_SETCURSEL, 0, 0);
        }
    }

    // 设置默认票数
    SetDlgItemText(hDlg, IDC_EDIT_WAITLIST_TICKETS, L"1");
    return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BUTTON_SAVE_WAITLIST: {
            // 获取输入数据
            wchar_t passengerName[100] = { 0 };
            wchar_t idCard[20] = { 0 };
            wchar_t gender[10] = { 0 };
            int ticketCount, classLevel;

            GetDlgItemText(hDlg, IDC_EDIT_WAITLIST_NAME, passengerName, 100);
            GetDlgItemText(hDlg, IDC_EDIT_WAITLIST_IDCARD, idCard, 20);

            HWND hGender = GetDlgItem(hDlg, IDC_EDIT_WAITLIST_GENDER);
            int genderSel = (int)SendMessageW(hGender, CB_GETCURSEL, 0, 0);
            if (genderSel == 0) wcscpy_s(gender, 10, L"男");
            else wcscpy_s(gender, 10, L"女");

            wchar_t ticketText[32] = { 0 };
            GetDlgItemText(hDlg, IDC_EDIT_WAITLIST_TICKETS, ticketText, 32);
            ticketCount = _wtoi(ticketText);
            if (ticketCount <= 0) ticketCount = 1;

            HWND hClass = GetDlgItem(hDlg, IDC_COMBO_WAITLIST_CLASS);
            classLevel = (int)SendMessageW(hClass, CB_GETCURSEL, 0, 0) + 1;

            // 获取选中的航班
            HWND hFlight = GetDlgItem(hDlg, IDC_COMBO_WAITLIST_FLIGHT);
            int flightIndex = (int)SendMessageW(hFlight, CB_GETCURSEL, 0, 0);
            if (flightIndex == -1) {
                MessageBox(hDlg, L"请选择一个航班!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }

            // 获取航班号
            wchar_t flightInfo[256] = { 0 };
            SendMessageW(hFlight, CB_GETLBTEXT, flightIndex, (LPARAM)flightInfo);
            wchar_t* dashPos = wcschr(flightInfo, L'-');
            wchar_t flightNumber[20] = { 0 };
            if (dashPos) {
                wcsncpy_s(flightNumber, 20, flightInfo, dashPos - flightInfo);
                // 去除尾部空格
                wchar_t* end = flightNumber + wcslen(flightNumber) - 1;
                while (end >= flightNumber && *end == L' ') *end-- = L'\0';
            }

            int flightID = GetFlightIDByNumber(flightNumber);
            if (flightID == 0) {
                MessageBox(hDlg, L"获取航班信息失败!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }

            // 验证输入
            if (wcslen(passengerName) == 0) {
                MessageBox(hDlg, L"请输入乘客姓名!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }
            if (wcslen(idCard) == 0) {
                MessageBox(hDlg, L"请输入证件号码!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }

            // 使用管理员权限添加候补
            int originalUserID = currentUserID;
            currentUserID = 1; // 使用管理员用户ID

            if (AddToWaitlist(flightID, passengerName, idCard, gender, ticketCount, classLevel)) {
                currentUserID = originalUserID; // 恢复原用户ID
                MessageBox(hDlg, L"候补预订添加成功!", L"成功", MB_OK | MB_ICONINFORMATION);
                EndDialog(hDlg, IDOK);
            }
            else {
                currentUserID = originalUserID; // 恢复原用户ID
                MessageBox(hDlg, L"候补预订添加失败!", L"错误", MB_OK | MB_ICONERROR);
            }
            break;
        }

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        break;
    }
    return FALSE;
}

// 处理修改航班
void HandleUpdateFlight(HWND hWnd) {
    printf("=== 执行修改航班 ===\n");

    HWND hTab = GetDlgItem(hWnd, IDC_MAIN_TAB);
    if (!hTab) return;

    HWND hAdminPage = GetDlgItem(hTab, IDC_ADMIN_PAGE);
    if (!hAdminPage) return;

    HWND hFlightList = GetDlgItem(hAdminPage, IDC_ADMIN_FLIGHT_LIST);
    if (!hFlightList) {
        ShowMessage(L"航班列表控件未找到", L"错误");
        return;
    }

    int selectedIndex = ListView_GetNextItem(hFlightList, -1, LVNI_SELECTED);
    if (selectedIndex == -1) {
        ShowMessage(L"请先选择一个航班", L"提示");
        return;
    }

    // 获取航班号
    wchar_t flightNumber[50] = { 0 };
    ListView_GetItemText(hFlightList, selectedIndex, 0, flightNumber, 50);
    int flightID = GetFlightIDByNumber(flightNumber);

    if (flightID == 0) {
        ShowMessage(L"获取航班信息失败", L"错误");
        return;
    }

    // 显示修改航班对话框
    INT_PTR result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_UPDATE_FLIGHT_DIALOG),
        hWnd, UpdateFlightDialogProc, (LPARAM)flightID);

    if (result == IDOK) {
        // 刷新航班列表
        RefreshAdminFlightList(hFlightList);
        ShowMessage(L"航班修改成功!", L"成功");
    }
}

// 修改航班对话框过程函数
INT_PTR CALLBACK UpdateFlightDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static FlightInfo currentFlight;

    switch (message) {
    case WM_INITDIALOG: {
        int flightID = (int)lParam;

        // 获取航班信息
        if (!GetFlightByID(flightID, &currentFlight)) {
            MessageBox(hDlg, L"获取航班信息失败!", L"错误", MB_OK | MB_ICONERROR);
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }

        // 填充对话框控件
        wchar_t buffer[32];

        // 航班ID（只读）
        swprintf(buffer, 32, L"%d", currentFlight.flightID);
        SetDlgItemText(hDlg, IDC_EDIT_UPDATE_FLIGHT_ID, buffer);

        // 其他字段
        SetDlgItemText(hDlg, IDC_EDIT_UPDATE_FLIGHT_NUMBER, currentFlight.flightNumber);
        SetDlgItemText(hDlg, IDC_EDIT_UPDATE_AIRLINE, currentFlight.airline);
        SetDlgItemText(hDlg, IDC_EDIT_UPDATE_DEPARTURE_CITY, currentFlight.departureCity);
        SetDlgItemText(hDlg, IDC_EDIT_UPDATE_ARRIVAL_CITY, currentFlight.arrivalCity);
        SetDlgItemText(hDlg, IDC_EDIT_UPDATE_DEPARTURE_TIME, currentFlight.departureTime);
        SetDlgItemText(hDlg, IDC_EDIT_UPDATE_ARRIVAL_TIME, currentFlight.arrivalTime);

        swprintf(buffer, 32, L"%d", currentFlight.totalSeats);
        SetDlgItemText(hDlg, IDC_EDIT_UPDATE_TOTAL_SEATS, buffer);

        swprintf(buffer, 32, L"%d", currentFlight.availableSeats);
        SetDlgItemText(hDlg, IDC_EDIT_UPDATE_AVAILABLE_SEATS, buffer);

        swprintf(buffer, 32, L"%.2f", currentFlight.economyPrice);
        SetDlgItemText(hDlg, IDC_EDIT_UPDATE_ECONOMY_PRICE, buffer);

        swprintf(buffer, 32, L"%.2f", currentFlight.businessPrice);
        SetDlgItemText(hDlg, IDC_EDIT_UPDATE_BUSINESS_PRICE, buffer);

        swprintf(buffer, 32, L"%.2f", currentFlight.firstClassPrice);
        SetDlgItemText(hDlg, IDC_EDIT_UPDATE_FIRSTCLASS_PRICE, buffer);

        SetDlgItemText(hDlg, IDC_EDIT_UPDATE_FLIGHT_DAY, currentFlight.flightDay);
        SetDlgItemText(hDlg, IDC_EDIT_UPDATE_PLANE_MODEL, currentFlight.planeModel);

        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BUTTON_UPDATE_FLIGHT: {
            // 获取修改后的数据
            FlightInfo updatedFlight = { 0 };
            updatedFlight.flightID = currentFlight.flightID;

            GetDlgItemText(hDlg, IDC_EDIT_UPDATE_FLIGHT_NUMBER, updatedFlight.flightNumber, 20);
            GetDlgItemText(hDlg, IDC_EDIT_UPDATE_AIRLINE, updatedFlight.airline, 50);
            GetDlgItemText(hDlg, IDC_EDIT_UPDATE_DEPARTURE_CITY, updatedFlight.departureCity, 50);
            GetDlgItemText(hDlg, IDC_EDIT_UPDATE_ARRIVAL_CITY, updatedFlight.arrivalCity, 50);
            GetDlgItemText(hDlg, IDC_EDIT_UPDATE_DEPARTURE_TIME, updatedFlight.departureTime, 50);
            GetDlgItemText(hDlg, IDC_EDIT_UPDATE_ARRIVAL_TIME, updatedFlight.arrivalTime, 50);
            GetDlgItemText(hDlg, IDC_EDIT_UPDATE_FLIGHT_DAY, updatedFlight.flightDay, 20);
            GetDlgItemText(hDlg, IDC_EDIT_UPDATE_PLANE_MODEL, updatedFlight.planeModel, 30);

            wchar_t buffer[32];

            GetDlgItemText(hDlg, IDC_EDIT_UPDATE_TOTAL_SEATS, buffer, 32);
            updatedFlight.totalSeats = _wtoi(buffer);

            GetDlgItemText(hDlg, IDC_EDIT_UPDATE_AVAILABLE_SEATS, buffer, 32);
            updatedFlight.availableSeats = _wtoi(buffer);

            GetDlgItemText(hDlg, IDC_EDIT_UPDATE_ECONOMY_PRICE, buffer, 32);
            updatedFlight.economyPrice = (float)_wtof(buffer);

            GetDlgItemText(hDlg, IDC_EDIT_UPDATE_BUSINESS_PRICE, buffer, 32);
            updatedFlight.businessPrice = (float)_wtof(buffer);

            GetDlgItemText(hDlg, IDC_EDIT_UPDATE_FIRSTCLASS_PRICE, buffer, 32);
            updatedFlight.firstClassPrice = (float)_wtof(buffer);

            // 验证输入
            if (wcslen(updatedFlight.flightNumber) == 0 || wcslen(updatedFlight.airline) == 0 ||
                wcslen(updatedFlight.departureCity) == 0 || wcslen(updatedFlight.arrivalCity) == 0) {
                MessageBox(hDlg, L"请填写完整的航班信息!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }

            if (updatedFlight.totalSeats <= 0) {
                MessageBox(hDlg, L"总座位数必须大于0!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }

            if (updatedFlight.availableSeats < 0 || updatedFlight.availableSeats > updatedFlight.totalSeats) {
                MessageBox(hDlg, L"余票数必须在0到总座位数之间!", L"错误", MB_OK | MB_ICONERROR);
                break;
            }

            // 更新航班信息
            if (UpdateFlight(&updatedFlight)) {
                MessageBox(hDlg, L"航班信息修改成功!", L"成功", MB_OK | MB_ICONINFORMATION);
                EndDialog(hDlg, IDOK);
            }
            else {
                MessageBox(hDlg, L"航班信息修改失败!", L"错误", MB_OK | MB_ICONERROR);
            }
            break;
        }

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        break;
    }
    return FALSE;
}