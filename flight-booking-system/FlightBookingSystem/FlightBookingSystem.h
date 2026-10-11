//主程序头文件，包含类声明（如航班、乘客、预订类）、函数原型、宏定义、全局变量声明
//作用：定义系统结构和接口，供其他源文件引用。
#pragma once

#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <commctrl.h>
#include <stdio.h>
#include "Resource.h"

#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "comctl32.lib")

#define MAX_LOADSTRING 100

// 数据库连接结构
typedef struct {
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    BOOL connected;
} DatabaseConnection;

// 扩展的航班信息结构
typedef struct {
    int flightID;
    wchar_t flightNumber[20];
    wchar_t airline[50];
    wchar_t departureCity[50];
    wchar_t arrivalCity[50];
    wchar_t departureTime[50];
    wchar_t arrivalTime[50];
    int totalSeats;
    int availableSeats;
    float economyPrice;
    float businessPrice;
    float firstClassPrice;
    wchar_t flightDay[20];
    wchar_t planeModel[30];
} FlightInfo;

// 预订信息结构
typedef struct {
    int bookingID;
    int userID;
    int flightID;
    wchar_t passengerName[100];
    int ticketCount;
    int classLevel;
    float totalAmount;
    wchar_t bookingDate[50];
    wchar_t flightNumber[20];
    wchar_t departureCity[50];
    wchar_t arrivalCity[50];
    wchar_t status[20];  // 添加状态字段
} BookingInfo;

// 乘客详细信息结构
typedef struct {
    int passengerID;
    int bookingID;
    wchar_t idCardNumber[20];
    wchar_t gender[10];
    wchar_t phone[20];
    wchar_t email[100];
} PassengerDetail;

// 候补预订信息结构
typedef struct {
    int waitlistID;
    int userID;
    int flightID;
    wchar_t passengerName[100];
    wchar_t idCardNumber[20];
    wchar_t gender[10];
    int ticketCount;
    int classLevel;
    wchar_t waitlistDate[50];
    wchar_t status[20];
    wchar_t flightNumber[20];
    wchar_t departureCity[50];
    wchar_t arrivalCity[50];
} WaitlistBooking;

// 全局变量声明
extern HINSTANCE hInst;
extern WCHAR szTitle[MAX_LOADSTRING];
extern WCHAR szWindowClass[MAX_LOADSTRING];
extern DatabaseConnection dbConn;
extern BOOL isAdmin;
extern BOOL isLoggedIn;
extern int currentUserID;
extern wchar_t currentUserName[100];
extern HWND g_hLoginDlg;
extern BOOL g_loginSuccess;

// 窗口和对话框函数
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

//函数声明分类：数据库连接、航班查询、预订管理、候补系统、界面创建
// ============ 数据库操作函数声明 ============
BOOL ConnectToDatabase();
void DisconnectDatabase();
BOOL ExecuteQuery(const wchar_t* query);
BOOL GetFlightsByRoute(const wchar_t* departure, const wchar_t* arrival, FlightInfo** flights, int* count);
BOOL GetAllFlights(FlightInfo** flights, int* count);
BOOL BookFlight(int flightID, const wchar_t* passengerName, int ticketCount, int classLevel);
BOOL CancelBooking(int bookingID);
BOOL AddFlight(const FlightInfo* flight);
BOOL UpdateFlight(const FlightInfo* flight);
BOOL DeleteFlight(int flightID);
BOOL GetUserBookings(int userID, BookingInfo** bookings, int* count);
BOOL RegisterUser(const wchar_t* username, const wchar_t* password, const wchar_t* fullname);
BOOL LoginUser(const wchar_t* username, const wchar_t* password);
int GetFlightIDByNumber(const wchar_t* flightNumber);
BOOL GetAllPassengerBookings(BookingInfo** bookings, int* count);
BOOL GetFlightsAdvanced(const wchar_t* flightNumber, const wchar_t* airline,
    const wchar_t* departure, const wchar_t* arrival,
    const wchar_t* depTime, const wchar_t* arrTime,
    FlightInfo** flights, int* count);
BOOL BookFlightWithDetails(int flightID, const wchar_t* passengerName,
    const wchar_t* idCard, const wchar_t* gender,
    const wchar_t* phone, const wchar_t* email,
    int ticketCount, int classLevel);
BOOL AddToWaitlist(int flightID, const wchar_t* passengerName,
    const wchar_t* idCard, const wchar_t* gender,
    int ticketCount, int classLevel);
BOOL GetWaitlistBookings(WaitlistBooking** waitlists, int* count);
BOOL GetPassengerDetails(int bookingID, PassengerDetail** details, int* count);
BOOL UpdateBooking(int bookingID, const wchar_t* passengerName,
    const wchar_t* idCard, const wchar_t* gender,
    const wchar_t* phone, const wchar_t* email,
    int ticketCount, int classLevel);
BOOL GetUserBookingsWithDetails(int userID, BookingInfo** bookings, PassengerDetail*** details, int* count);
BOOL CheckFlightAvailability(int flightID, int ticketCount);
int GetFlightAvailableSeats(int flightID);
BOOL GetUserBookingsWithStatus(int userID, BookingInfo** bookings, int* count);
BOOL ProcessWaitlistForFlight(int flightID, int availableSeats);


// ============ 界面函数声明 ============
int GetFlightIDByNumber(const wchar_t* flightNumber);
void CreateMainInterface(HWND hWnd);
void CreateLoginDialog(HWND hWnd);
void CreateFlightSearchTab(HWND hParent);
void CreateAdminTab(HWND hParent);
void RefreshFlightList(HWND hList, const wchar_t* departure, const wchar_t* arrival);
void RefreshBookingList(HWND hList, int userID);
void ShowMessage(const wchar_t* message, const wchar_t* title);
void CreateTabPages(HWND hTab);
void CreateEnhancedFlightSearchTab(HWND hParent);
void CreateEnhancedBookingTab(HWND hParent);
void CreateEnhancedMyBookingsTab(HWND hParent);
void CreateEnhancedAdminTab(HWND hParent);
void RefreshEnhancedFlightList(HWND hList);
void RefreshWaitlistList(HWND hList);
void RefreshPassengerList(HWND hList);
void ShowBookingDetailsDialog(int bookingID);
void RefreshAdminData(HWND hParent);
void RefreshAdminFlightList(HWND hList);
void RefreshBookingListWithStatus(HWND hList, int userID);
void DebugCheckWaitlistOrder(int flightID);
void DebugWaitlistAllocation(int flightID, int availableSeats);

// ============ 按钮处理函数声明 ============
LRESULT CALLBACK PageWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void ResetSearchConditions(HWND hWnd);
void HandleFlightSearch(HWND hWnd);
void HandleBookFlight(HWND hWnd);
void HandleWaitlistBooking(HWND hWnd);
void HandleRefreshBookings(HWND hWnd);
void HandleCancelBooking(HWND hWnd);
void HandleRefreshAdminData(HWND hWnd);
void HandleAddFlight(HWND hWnd);
void HandleDeleteFlight(HWND hWnd);
void HandleRefreshFlights(HWND hWnd);

BOOL DeletePassengerBooking(int bookingID);
BOOL DeleteWaitlistBooking(int waitlistID);
INT_PTR CALLBACK AddPassengerDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AddWaitlistDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void HandleAddPassenger(HWND hWnd);
void HandleDeletePassenger(HWND hWnd);
void HandleAddWaitlist(HWND hWnd);
void HandleDeleteWaitlist(HWND hWnd);

// ============ 工具函数声明 ============
void MyGetDlgItemText(HWND hDlg, int nID, wchar_t* buffer, int bufferSize);
int MyGetDlgItemInt(HWND hDlg, int nID);

// ============ 对话框过程函数声明 ============
INT_PTR CALLBACK LoginDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK BookingDetailsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AddFlightDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
 
//修改航班信息
BOOL GetFlightByID(int flightID, FlightInfo* flight);
INT_PTR CALLBACK UpdateFlightDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void HandleUpdateFlight(HWND hWnd);