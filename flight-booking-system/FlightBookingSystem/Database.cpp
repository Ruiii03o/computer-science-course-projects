
/*数据库操作源文件，实现：1.连接数据库（SQL Serve） 2.执行 SQL 查询（插入、更新、删除航班 / 预订）
3.数据验证与错误处理 。功能：持久化存储航班和预订信息。*/

#include "framework.h"
#include "FlightBookingSystem.h"
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>

// 数据库连接
BOOL ConnectToDatabase() {
    SQLRETURN ret;

    // 分配环境句柄，创建ODBC环境
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &dbConn.henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("!!! 无法分配环境句柄 !!!\n");
        return FALSE;
    }

    // 设置ODBC版本
    ret = SQLSetEnvAttr(dbConn.henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("!!! 无法设置ODBC版本 !!!\n");
        SQLFreeHandle(SQL_HANDLE_ENV, dbConn.henv);
        return FALSE;
    }

    // 分配连接句柄，创建数据库连接
    ret = SQLAllocHandle(SQL_HANDLE_DBC, dbConn.henv, &dbConn.hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("!!! 无法分配连接句柄 !!!\n");
        SQLFreeHandle(SQL_HANDLE_ENV, dbConn.henv);
        return FALSE;
    }

    printf("尝试数据库连接...\n");

    // 连接字符串，连接到SQL Server数据库
    SQLWCHAR connStr[] = L"DRIVER={SQL Server};SERVER=localhost;DATABASE=FlightBookingSystem;Trusted_Connection=yes;";
    printf("使用连接字符串: %ls\n", connStr);
    // 建立连接
    ret = SQLDriverConnectW(dbConn.hdbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        printf("数据库连接成功\n");
        dbConn.connected = TRUE;

        // 测试查询验证连接
        if (ExecuteQuery(L"SELECT COUNT(*) FROM Flights")) {
            SQLINTEGER rowCount = 0;
            if (SQLFetch(dbConn.hstmt) == SQL_SUCCESS) {
                SQLGetData(dbConn.hstmt, 1, SQL_C_LONG, &rowCount, 0, NULL);
                printf("数据库连接验证成功，Flights表中有 %d 条记录\n", rowCount);
            }
            SQLFreeHandle(SQL_HANDLE_STMT, dbConn.hstmt);
        }
        return TRUE;
    }

    // 连接失败处理
    printf("!!! 数据库连接失败 !!!\n");
    SQLWCHAR sqlState[6] = { 0 };
    SQLWCHAR errorMsg[1024] = { 0 };
    SQLINTEGER nativeError = 0;
    SQLSMALLINT msgLength = 0;

    SQLGetDiagRec(SQL_HANDLE_DBC, dbConn.hdbc, 1, sqlState, &nativeError, errorMsg, sizeof(errorMsg) / sizeof(SQLWCHAR), &msgLength);
    printf("错误代码: %d\n", nativeError);
    printf("错误状态: %ls\n", sqlState);
    printf("错误信息: %ls\n", errorMsg);

    // 清理资源
    SQLFreeHandle(SQL_HANDLE_DBC, dbConn.hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, dbConn.henv);

    return FALSE;
}

void DisconnectDatabase() {
    if (dbConn.hstmt) {
        SQLFreeHandle(SQL_HANDLE_STMT, dbConn.hstmt);
        dbConn.hstmt = NULL;
    }
    if (dbConn.connected && dbConn.hdbc) {
        SQLDisconnect(dbConn.hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, dbConn.hdbc);
        dbConn.hdbc = NULL;
    }
    if (dbConn.henv) {
        SQLFreeHandle(SQL_HANDLE_ENV, dbConn.henv);
        dbConn.henv = NULL;
    }
    dbConn.connected = FALSE;
}

BOOL ExecuteQuery(const wchar_t* query) {
    SQLRETURN ret;

    if (dbConn.hstmt) {
        SQLFreeHandle(SQL_HANDLE_STMT, dbConn.hstmt);
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &dbConn.hstmt);
    if (ret != SQL_SUCCESS) {
        return FALSE;
    }

    ret = SQLExecDirectW(dbConn.hstmt, (SQLWCHAR*)query, SQL_NTS);
    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

// 航班查询 
BOOL GetFlightsByRoute(const wchar_t* departure, const wchar_t* arrival, FlightInfo** flights, int* count){
    if (!dbConn.connected) {
        printf("数据库未连接，无法查询航班\n");
        return FALSE;
    }

    wchar_t query[1024];
    SQLHSTMT hstmt;

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) {
        printf("无法分配语句句柄\n");
        return FALSE;
    }

    // 构建查询语句,动态构建SQL查询
    if (wcscmp(departure, L"") == 0 && wcscmp(arrival, L"") == 0) {
        swprintf(query, 1024,
            L"SELECT FlightID, FlightNumber, Airline, DepartureCity, ArrivalCity, "
            L"CONVERT(varchar(16), DepartureTime, 120), CONVERT(varchar(16), ArrivalTime, 120), "
            L"TotalSeats, AvailableSeats, EconomyPrice, BusinessPrice, FirstClassPrice, "
            L"ISNULL(FlightDay, ''), ISNULL(PlaneModel, '') "
            L"FROM Flights ORDER BY DepartureTime");
    }
    else {
        swprintf(query, 1024,
            L"SELECT FlightID, FlightNumber, Airline, DepartureCity, ArrivalCity, "
            L"CONVERT(varchar(16), DepartureTime, 120), CONVERT(varchar(16), ArrivalTime, 120), "
            L"TotalSeats, AvailableSeats, EconomyPrice, BusinessPrice, FirstClassPrice, "
            L"ISNULL(FlightDay, ''), ISNULL(PlaneModel, '') "
            L"FROM Flights WHERE DepartureCity LIKE N'%%%s%%' AND ArrivalCity LIKE N'%%%s%%' " 
            L"ORDER BY DepartureTime", departure, arrival);
    }

    printf("执行航班查询: %ls\n", query);

    ret = SQLExecDirectW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("查询执行失败\n");
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    // 计算记录数
    *count = 0;
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        (*count)++;
    }

    printf("找到 %d 条航班记录\n", *count);

    if (*count == 0) {
        *flights = NULL;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return TRUE;
    }

    // 分配内存
    *flights = (FlightInfo*)malloc(*count * sizeof(FlightInfo));
    if (*flights == NULL) {
        printf("内存分配失败\n");
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    // 重新执行查询获取数据
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    SQLExecDirectW(hstmt, (SQLWCHAR*)query, SQL_NTS);

    int index = 0;
    SQLLEN indicator;

    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        memset(&(*flights)[index], 0, sizeof(FlightInfo));

        // 获取数据
        SQLGetData(hstmt, 1, SQL_C_LONG, &(*flights)[index].flightID, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_WCHAR, (*flights)[index].flightNumber, sizeof((*flights)[index].flightNumber), NULL);
        SQLGetData(hstmt, 3, SQL_C_WCHAR, (*flights)[index].airline, sizeof((*flights)[index].airline), NULL);
        SQLGetData(hstmt, 4, SQL_C_WCHAR, (*flights)[index].departureCity, sizeof((*flights)[index].departureCity), NULL);
        SQLGetData(hstmt, 5, SQL_C_WCHAR, (*flights)[index].arrivalCity, sizeof((*flights)[index].arrivalCity), NULL);

        wchar_t depTime[50] = { 0 }, arrTime[50] = { 0 };
        SQLGetData(hstmt, 6, SQL_C_WCHAR, depTime, sizeof(depTime), &indicator);
        SQLGetData(hstmt, 7, SQL_C_WCHAR, arrTime, sizeof(arrTime), &indicator);

        if (indicator != SQL_NULL_DATA) {
            wcsncpy_s((*flights)[index].departureTime, sizeof((*flights)[index].departureTime) / sizeof(wchar_t), depTime, _TRUNCATE);
            wcsncpy_s((*flights)[index].arrivalTime, sizeof((*flights)[index].arrivalTime) / sizeof(wchar_t), arrTime, _TRUNCATE);
        }

        SQLGetData(hstmt, 8, SQL_C_LONG, &(*flights)[index].totalSeats, 0, NULL);
        SQLGetData(hstmt, 9, SQL_C_LONG, &(*flights)[index].availableSeats, 0, NULL);
        SQLGetData(hstmt, 10, SQL_C_FLOAT, &(*flights)[index].economyPrice, 0, NULL);
        SQLGetData(hstmt, 11, SQL_C_FLOAT, &(*flights)[index].businessPrice, 0, NULL);
        SQLGetData(hstmt, 12, SQL_C_FLOAT, &(*flights)[index].firstClassPrice, 0, NULL);

        wchar_t flightDay[20] = { 0 }, planeModel[30] = { 0 };
        SQLGetData(hstmt, 13, SQL_C_WCHAR, flightDay, sizeof(flightDay), &indicator);
        SQLGetData(hstmt, 14, SQL_C_WCHAR, planeModel, sizeof(planeModel), &indicator);

        if (indicator != SQL_NULL_DATA) {
            wcsncpy_s((*flights)[index].flightDay, sizeof((*flights)[index].flightDay) / sizeof(wchar_t), flightDay, _TRUNCATE);
            wcsncpy_s((*flights)[index].planeModel, sizeof((*flights)[index].planeModel) / sizeof(wchar_t), planeModel, _TRUNCATE);
        }

        printf("加载航班: %ls %ls->%ls 余票:%d\n",
            (*flights)[index].flightNumber,
            (*flights)[index].departureCity,
            (*flights)[index].arrivalCity,
            (*flights)[index].availableSeats);

        index++;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return TRUE;
}

//多条件查询,支持航班号、航空公司、城市、时间等多个条件的任意组合
BOOL GetFlightsAdvanced(const wchar_t* flightNumber, const wchar_t* airline,
    const wchar_t* departure, const wchar_t* arrival,
    const wchar_t* depTime, const wchar_t* arrTime,
    FlightInfo** flights, int* count) {

    if (!dbConn.connected) {
        printf("数据库未连接，无法查询航班\n");
        return FALSE;
    }

    wchar_t query[2048];
    SQLHSTMT hstmt;

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) {
        return FALSE;
    }

    // 构建基础查询 
    swprintf(query, 2048,
        L"SELECT FlightID, FlightNumber, Airline, DepartureCity, ArrivalCity, "
        L"CONVERT(varchar(16), DepartureTime, 120), CONVERT(varchar(16), ArrivalTime, 120), "
        L"TotalSeats, AvailableSeats, EconomyPrice, BusinessPrice, FirstClassPrice, "
        L"ISNULL(FlightDay, ''), ISNULL(PlaneModel, '') "
        L"FROM Flights WHERE 1=1 ");  // 显示所有航班

    // 动态添加查询条件,使用LIKE操作符支持部分匹配
    if (wcscmp(flightNumber, L"") != 0 && wcslen(flightNumber) > 0) {
        wchar_t temp[256];
        swprintf(temp, 256, L" AND FlightNumber LIKE N'%%%ls%%'", flightNumber);
        wcscat_s(query, 2048, temp);
    }
    if (wcscmp(airline, L"") != 0 && wcslen(airline) > 0) {
        wchar_t temp[256];
        swprintf(temp, 256, L" AND Airline LIKE N'%%%ls%%'", airline);
        wcscat_s(query, 2048, temp);
    }
    if (wcscmp(departure, L"") != 0 && wcslen(departure) > 0) {
        wchar_t temp[256];
        swprintf(temp, 256, L" AND DepartureCity LIKE N'%%%ls%%'", departure);
        wcscat_s(query, 2048, temp);
    }
    if (wcscmp(arrival, L"") != 0 && wcslen(arrival) > 0) {
        wchar_t temp[256];
        swprintf(temp, 256, L" AND ArrivalCity LIKE N'%%%ls%%'", arrival);
        wcscat_s(query, 2048, temp);
    }
    // 添加时间查询条件
    if (wcscmp(depTime, L"") != 0 && wcslen(depTime) > 0) {
        wchar_t temp[256];
        swprintf(temp, 256, L" AND CONVERT(varchar(10), DepartureTime, 120) LIKE '%%%ls%%'", depTime);
        wcscat_s(query, 2048, temp);
    }
    if (wcscmp(arrTime, L"") != 0 && wcslen(arrTime) > 0) {
        wchar_t temp[256];
        swprintf(temp, 256, L" AND CONVERT(varchar(10), ArrivalTime, 120) LIKE '%%%ls%%'", arrTime);
        wcscat_s(query, 2048, temp);
    }

    // 排序
    wcscat_s(query, 2048, L" ORDER BY DepartureTime");

    printf("执行高级查询: %ls\n", query);

    ret = SQLExecDirectW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("高级查询执行失败\n");
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    // 计算记录数
    *count = 0;
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        (*count)++;
    }

    printf("找到 %d 条航班记录\n", *count);

    if (*count == 0) {
        *flights = NULL;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return TRUE;
    }

    // 分配内存
    *flights = (FlightInfo*)malloc(*count * sizeof(FlightInfo));
    if (*flights == NULL) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    // 重新执行查询获取数据
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    SQLExecDirectW(hstmt, (SQLWCHAR*)query, SQL_NTS);

    int index = 0;
    SQLLEN indicator;

    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        memset(&(*flights)[index], 0, sizeof(FlightInfo));

        // 获取数据
        SQLGetData(hstmt, 1, SQL_C_LONG, &(*flights)[index].flightID, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_WCHAR, (*flights)[index].flightNumber, sizeof((*flights)[index].flightNumber), NULL);
        SQLGetData(hstmt, 3, SQL_C_WCHAR, (*flights)[index].airline, sizeof((*flights)[index].airline), NULL);
        SQLGetData(hstmt, 4, SQL_C_WCHAR, (*flights)[index].departureCity, sizeof((*flights)[index].departureCity), NULL);
        SQLGetData(hstmt, 5, SQL_C_WCHAR, (*flights)[index].arrivalCity, sizeof((*flights)[index].arrivalCity), NULL);

        wchar_t depTime[50] = { 0 }, arrTime[50] = { 0 };
        SQLGetData(hstmt, 6, SQL_C_WCHAR, depTime, sizeof(depTime), &indicator);
        SQLGetData(hstmt, 7, SQL_C_WCHAR, arrTime, sizeof(arrTime), &indicator);

        if (indicator != SQL_NULL_DATA) {
            wcsncpy_s((*flights)[index].departureTime, sizeof((*flights)[index].departureTime) / sizeof(wchar_t), depTime, _TRUNCATE);
            wcsncpy_s((*flights)[index].arrivalTime, sizeof((*flights)[index].arrivalTime) / sizeof(wchar_t), arrTime, _TRUNCATE);
        }

        SQLGetData(hstmt, 8, SQL_C_LONG, &(*flights)[index].totalSeats, 0, NULL);
        SQLGetData(hstmt, 9, SQL_C_LONG, &(*flights)[index].availableSeats, 0, NULL);
        SQLGetData(hstmt, 10, SQL_C_FLOAT, &(*flights)[index].economyPrice, 0, NULL);
        SQLGetData(hstmt, 11, SQL_C_FLOAT, &(*flights)[index].businessPrice, 0, NULL);
        SQLGetData(hstmt, 12, SQL_C_FLOAT, &(*flights)[index].firstClassPrice, 0, NULL);

        wchar_t flightDay[20] = { 0 }, planeModel[30] = { 0 };
        SQLGetData(hstmt, 13, SQL_C_WCHAR, flightDay, sizeof(flightDay), &indicator);
        SQLGetData(hstmt, 14, SQL_C_WCHAR, planeModel, sizeof(planeModel), &indicator);

        if (indicator != SQL_NULL_DATA) {
            wcsncpy_s((*flights)[index].flightDay, sizeof((*flights)[index].flightDay) / sizeof(wchar_t), flightDay, _TRUNCATE);
            wcsncpy_s((*flights)[index].planeModel, sizeof((*flights)[index].planeModel) / sizeof(wchar_t), planeModel, _TRUNCATE);
        }

        printf("加载航班: %ls %ls->%ls 余票:%d\n",
            (*flights)[index].flightNumber,
            (*flights)[index].departureCity,
            (*flights)[index].arrivalCity,
            (*flights)[index].availableSeats);

        index++;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return TRUE;
}

// 根据航班号获取航班ID
int GetFlightIDByNumber(const wchar_t* flightNumber) {
    if (!dbConn.connected) return 0;

    SQLHSTMT hstmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) return 0;

    wchar_t query[256];
    swprintf(query, 256, L"SELECT FlightID FROM Flights WHERE FlightNumber = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, (SQLPOINTER)flightNumber, 0, NULL);
    SQLExecute(hstmt);

    int flightID = 0;
    if (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_LONG, &flightID, 0, NULL);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return flightID;
}

// 获取所有航班
BOOL GetAllFlights(FlightInfo** flights, int* count) {
    return GetFlightsByRoute(L"", L"", flights, count);
}

// 用户登录
BOOL LoginUser(const wchar_t* username, const wchar_t* password) {
    if (!dbConn.connected) {
        printf("数据库未连接，无法登录\n");
        return FALSE;
    }

    wchar_t query[512];
    SQLHSTMT hstmt;

    // 分配语句句柄
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) {
        return FALSE;
    }

    // 使用参数化查询防止SQL注入
    swprintf(query, 512,
        L"SELECT UserID, UserType, FullName FROM Users WHERE Username = ? AND Password = ?");

    ret = SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    if (ret != SQL_SUCCESS) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    // 绑定参数
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)username, 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)password, 0, NULL);

    ret = SQLExecute(hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    ret = SQLFetch(hstmt);
    if (ret == SQL_SUCCESS) {
        SQLLEN indicator;

        SQLGetData(hstmt, 1, SQL_C_LONG, &currentUserID, 0, NULL);

        wchar_t userType[20] = { 0 };
        SQLGetData(hstmt, 2, SQL_C_WCHAR, userType, sizeof(userType) / sizeof(wchar_t), &indicator);
        SQLGetData(hstmt, 3, SQL_C_WCHAR, currentUserName, sizeof(currentUserName) / sizeof(wchar_t), &indicator);

        isAdmin = (wcscmp(userType, L"admin") == 0);
        isLoggedIn = TRUE;

        printf("用户登录成功: ID=%d, 姓名=%ls, 类型=%ls\n", currentUserID, currentUserName, userType);

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return TRUE;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return FALSE;
}

//用户注册
BOOL RegisterUser(const wchar_t* username, const wchar_t* password, const wchar_t* fullname) {
    if (!dbConn.connected) {
        printf("数据库未连接，无法注册\n");
        return FALSE;
    }

    // 首先检查用户名是否已存在
    wchar_t checkQuery[256];
    SQLHSTMT hstmt;

    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    swprintf(checkQuery, 256, L"SELECT COUNT(*) FROM Users WHERE Username = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)checkQuery, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)username, 0, NULL);
    SQLExecute(hstmt);

    int count = 0;
    if (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_LONG, &count, 0, NULL);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (count > 0) {
        printf("用户名已存在: %ls\n", username);
        return FALSE;
    }

    // 插入新用户
    wchar_t insertQuery[512];
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);

    swprintf(insertQuery, 512,
        L"INSERT INTO Users (Username, Password, UserType, FullName, RegisterDate) "
        L"VALUES (?, ?, 'customer', ?, GETDATE())");

    SQLPrepareW(hstmt, (SQLWCHAR*)insertQuery, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)username, 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)password, 0, NULL);
    SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)fullname, 0, NULL);

    SQLRETURN ret = SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        printf("用户注册成功: %ls\n", username);
        return TRUE;
    }

    printf("用户注册失败: %ls\n", username);
    return FALSE;
}


// 机票预订
BOOL BookFlight(int flightID, const wchar_t* passengerName, int ticketCount, int classLevel) {
    if (!dbConn.connected) {
        printf("数据库未连接，无法预订\n");
        return FALSE;
    }

    if (!isLoggedIn) {
        printf("用户未登录，无法预订\n");
        return FALSE;
    }

    SQLHSTMT hstmt;
    SQLRETURN ret;

    // 开始事务
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) return FALSE;

    // 首先检查余票
    wchar_t query[512];
    swprintf(query, 512, L"SELECT AvailableSeats, Price FROM Flights WHERE FlightID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
    SQLExecute(hstmt);

    int availableSeats = 0;
    float price = 0.0f;
    if (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_LONG, &availableSeats, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_FLOAT, &price, 0, NULL);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (availableSeats < ticketCount) {
        printf("余票不足: 需要%d张，只有%d张\n", ticketCount, availableSeats);
        return FALSE;
    }

    // 计算总金额
    float totalAmount = price * ticketCount;
    if (classLevel == 2) totalAmount *= 1.5f;      // 商务舱
    else if (classLevel == 3) totalAmount *= 2.0f; // 头等舱

    // 创建预订
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    swprintf(query, 512,
        L"INSERT INTO Bookings (UserID, FlightID, PassengerName, PassengerCount, ClassLevel, TotalAmount, BookingDate, Status) "
        L"VALUES (?, ?, ?, ?, ?, ?, GETDATE(), 'confirmed')");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &currentUserID, 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
    SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)passengerName, 0, NULL);
    SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &ticketCount, 0, NULL);
    SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &classLevel, 0, NULL);
    SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &totalAmount, 0, NULL);

    ret = SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("创建预订失败\n");
        return FALSE;
    }

    // 更新航班余票
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    swprintf(query, 512, L"UPDATE Flights SET AvailableSeats = AvailableSeats - ? WHERE FlightID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &ticketCount, 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);

    ret = SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        printf("机票预订成功: 航班ID=%d, 乘客=%ls, 票数=%d, 金额=%.2f\n",
            flightID, passengerName, ticketCount, totalAmount);
        return TRUE;
    }

    printf("更新余票失败\n");
    return FALSE;
}

// 带详细信息的机票预订
BOOL BookFlightWithDetails(int flightID, const wchar_t* passengerName,
    const wchar_t* idCard, const wchar_t* gender,
    const wchar_t* phone, const wchar_t* email,
    int ticketCount, int classLevel) {

    if (!dbConn.connected) {
        printf("数据库未连接，无法预订\n");
        return FALSE;
    }

    if (!isLoggedIn) {
        printf("用户未登录，无法预订\n");
        return FALSE;
    }

    SQLHSTMT hstmt;
    SQLRETURN ret;

    // 开始事务
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) return FALSE;

    // 首先检查余票
    wchar_t query[512];
    swprintf(query, 512, L"SELECT AvailableSeats FROM Flights WHERE FlightID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
    SQLExecute(hstmt);

    int availableSeats = 0;
    if (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_LONG, &availableSeats, 0, NULL);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    printf("航班ID=%d, 余票=%d, 需要票数=%d\n", flightID, availableSeats, ticketCount);

    // 计算可预订数量和需要候补的数量
    int bookableCount = (availableSeats >= ticketCount) ? ticketCount : availableSeats;
    int waitlistCount = ticketCount - bookableCount;

    // 获取价格信息
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    swprintf(query, 512,
        L"SELECT EconomyPrice, BusinessPrice, FirstClassPrice FROM Flights WHERE FlightID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
    SQLExecute(hstmt);

    float economyPrice = 0, businessPrice = 0, firstClassPrice = 0;
    if (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_FLOAT, &economyPrice, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_FLOAT, &businessPrice, 0, NULL);
        SQLGetData(hstmt, 3, SQL_C_FLOAT, &firstClassPrice, 0, NULL);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    // 计算总金额（只计算实际预订的部分）
    float pricePerTicket = 0;
    switch (classLevel) {
    case 1: pricePerTicket = economyPrice; break;
    case 2: pricePerTicket = businessPrice; break;
    case 3: pricePerTicket = firstClassPrice; break;
    default: pricePerTicket = economyPrice; break;
    }
    float totalAmount = pricePerTicket * bookableCount;

    BOOL success = FALSE;

    // 如果有可预订的票，创建预订
    if (bookableCount > 0) {
        SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
        swprintf(query, 512,
            L"INSERT INTO Bookings (UserID, FlightID, PassengerName, PassengerCount, ClassLevel, TotalAmount, BookingDate, Status) "
            L"VALUES (?, ?, ?, ?, ?, ?, GETDATE(), 'confirmed')");

        SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &currentUserID, 0, NULL);
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
        SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)passengerName, 0, NULL);
        SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &bookableCount, 0, NULL);
        SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &classLevel, 0, NULL);
        SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &totalAmount, 0, NULL);

        ret = SQLExecute(hstmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            printf("创建预订失败\n");
            return FALSE;
        }

        // 获取新创建的预订ID
        int newBookingID = 0;
        SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
        swprintf(query, 512, L"SELECT MAX(BookingID) FROM Bookings WHERE UserID = ?");
        SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &currentUserID, 0, NULL);
        SQLExecute(hstmt);
        if (SQLFetch(hstmt) == SQL_SUCCESS) {
            SQLGetData(hstmt, 1, SQL_C_LONG, &newBookingID, 0, NULL);
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

        printf("新创建的预订ID: %d, 票数: %d\n", newBookingID, bookableCount);

        // 添加乘客详细信息
        SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
        swprintf(query, 512,
            L"INSERT INTO PassengerDetails (BookingID, IDCardNumber, Gender, Phone, Email) "
            L"VALUES (?, ?, ?, ?, ?)");

        SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &newBookingID, 0, NULL);
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, (SQLPOINTER)idCard, 0, NULL);
        SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, (SQLPOINTER)gender, 0, NULL);
        SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, (SQLPOINTER)phone, 0, NULL);
        SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)email, 0, NULL);

        ret = SQLExecute(hstmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            printf("添加乘客详情失败\n");
            // 回滚预订
            SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
            swprintf(query, 512, L"DELETE FROM Bookings WHERE BookingID = ?");
            SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
            SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &newBookingID, 0, NULL);
            SQLExecute(hstmt);
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            return FALSE;
        }

        // 更新航班余票
        SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
        swprintf(query, 512, L"UPDATE Flights SET AvailableSeats = AvailableSeats - ? WHERE FlightID = ?");

        SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &bookableCount, 0, NULL);
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);

        ret = SQLExecute(hstmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            printf("更新余票失败\n");
            return FALSE;
        }

        success = TRUE;
        printf("成功预订 %d 张票\n", bookableCount);
    }

    // 如果有需要候补的票，创建候补
    if (waitlistCount > 0) {
        SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
        swprintf(query, 512,
            L"INSERT INTO WaitlistBookings (UserID, FlightID, PassengerName, IDCardNumber, Gender, TicketCount, ClassLevel, Status) "
            L"VALUES (?, ?, ?, ?, ?, ?, ?, 'waiting')");

        SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &currentUserID, 0, NULL);
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
        SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)passengerName, 0, NULL);
        SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, (SQLPOINTER)idCard, 0, NULL);
        SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, (SQLPOINTER)gender, 0, NULL);
        SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &waitlistCount, 0, NULL);
        SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &classLevel, 0, NULL);

        ret = SQLExecute(hstmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
            printf("候补预订添加成功: %d 张票\n", waitlistCount);
            success = TRUE;
        }
        else {
            printf("候补预订添加失败\n");
        }
    }

    if (success) {
        wchar_t message[256];
        if (bookableCount > 0 && waitlistCount > 0) {
            swprintf(message, 256, L"预订成功 %d 张票，%d 张票进入候补", bookableCount, waitlistCount);
        }
        else if (bookableCount > 0) {
            swprintf(message, 256, L"预订成功 %d 张票", bookableCount);
        }
        else {
            swprintf(message, 256, L"%d 张票进入候补", waitlistCount);
        }
        printf("机票处理完成: %ls\n", message);
        return TRUE;
    }

    printf("预订处理失败\n");
    return FALSE;
}

// 获取用户订单
BOOL GetUserBookings(int userID, BookingInfo** bookings, int* count) {
    if (!dbConn.connected) {
        printf("数据库未连接，无法获取订单\n");
        return FALSE;
    }

    wchar_t query[1024];
    SQLHSTMT hstmt;

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) {
        return FALSE;
    }

    swprintf(query, 1024,
        L"SELECT b.BookingID, b.UserID, b.FlightID, b.PassengerName, b.PassengerCount, "
        L"b.ClassLevel, b.TotalAmount, CONVERT(varchar(19), b.BookingDate, 120), "
        L"f.FlightNumber, f.DepartureCity, f.ArrivalCity "
        L"FROM Bookings b "
        L"INNER JOIN Flights f ON b.FlightID = f.FlightID "
        L"WHERE b.UserID = ? "
        L"ORDER BY b.BookingDate DESC");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userID, 0, NULL);

    ret = SQLExecDirectW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    // 计算记录数
    *count = 0;
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        (*count)++;
    }

    printf("找到 %d 条订单记录\n", *count);

    if (*count == 0) {
        *bookings = NULL;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return TRUE;
    }

    // 分配内存
    *bookings = (BookingInfo*)malloc(*count * sizeof(BookingInfo));
    if (*bookings == NULL) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    // 重新执行查询获取数据
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userID, 0, NULL);
    SQLExecute(hstmt);

    int index = 0;
    SQLLEN indicator;

    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        // 清空结构体
        memset(&(*bookings)[index], 0, sizeof(BookingInfo));

        // 获取数据
        SQLGetData(hstmt, 1, SQL_C_LONG, &(*bookings)[index].bookingID, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_LONG, &(*bookings)[index].userID, 0, NULL);
        SQLGetData(hstmt, 3, SQL_C_LONG, &(*bookings)[index].flightID, 0, NULL);
        SQLGetData(hstmt, 4, SQL_C_WCHAR, (*bookings)[index].passengerName, 100, NULL);
        SQLGetData(hstmt, 5, SQL_C_LONG, &(*bookings)[index].ticketCount, 0, NULL);
        SQLGetData(hstmt, 6, SQL_C_LONG, &(*bookings)[index].classLevel, 0, NULL);
        SQLGetData(hstmt, 7, SQL_C_FLOAT, &(*bookings)[index].totalAmount, 0, NULL);

        wchar_t bookDate[50] = { 0 };
        SQLGetData(hstmt, 8, SQL_C_WCHAR, bookDate, 50, &indicator);
        if (indicator != SQL_NULL_DATA) {
            wcsncpy_s((*bookings)[index].bookingDate, 50, bookDate, _TRUNCATE);
        }

        SQLGetData(hstmt, 9, SQL_C_WCHAR, (*bookings)[index].flightNumber, 20, NULL);
        SQLGetData(hstmt, 10, SQL_C_WCHAR, (*bookings)[index].departureCity, 50, NULL);
        SQLGetData(hstmt, 11, SQL_C_WCHAR, (*bookings)[index].arrivalCity, 50, NULL);

        printf("加载订单: ID=%d, 乘客=%ls, 航班=%ls\n",
            (*bookings)[index].bookingID,
            (*bookings)[index].passengerName,
            (*bookings)[index].flightNumber);

        index++;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return TRUE;
}

// 取消预订机票
BOOL CancelBooking(int bookingID) {
    if (!dbConn.connected) {
        printf("数据库未连接，无法取消预订\n");
        return FALSE;
    }

    if (!isLoggedIn) {
        printf("用户未登录，无法取消预订\n");
        return FALSE;
    }

    SQLHSTMT hstmt;
    SQLRETURN ret;

    printf("=== 取消预订详细调试 ===\n");
    printf("预订ID: %d, 当前用户ID: %d\n", bookingID, currentUserID);

    // 首先检查预订是否存在且属于当前用户
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    wchar_t query[512];
    swprintf(query, 512,
        L"SELECT FlightID, PassengerCount, UserID FROM Bookings WHERE BookingID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &bookingID, 0, NULL);
    SQLExecute(hstmt);

    int flightID = 0;
    int passengerCount = 0;
    int bookingUserID = 0;

    if (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_LONG, &flightID, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_LONG, &passengerCount, 0, NULL);
        SQLGetData(hstmt, 3, SQL_C_LONG, &bookingUserID, 0, NULL);

        printf("找到预订记录: 航班ID=%d, 票数=%d, 预订用户ID=%d\n", flightID, passengerCount, bookingUserID);

        // 检查权限
        if (bookingUserID != currentUserID && !isAdmin) {
            printf("权限拒绝: 当前用户ID(%d)不是预订用户(%d)，且不是管理员\n", currentUserID, bookingUserID);
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            return FALSE;
        }
    }
    else {
        printf("未找到预订记录，预订ID: %d\n", bookingID);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    printf("取消预订信息: 航班ID=%d, 票数=%d\n", flightID, passengerCount);

    // 开始事务：恢复余票并删除预订
    // 恢复余票
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    swprintf(query, 512, L"UPDATE Flights SET AvailableSeats = AvailableSeats + ? WHERE FlightID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &passengerCount, 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);

    ret = SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("恢复余票失败\n");
        return FALSE;
    }
    printf("余票恢复成功\n");

    // 删除乘客详细信息
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    swprintf(query, 512, L"DELETE FROM PassengerDetails WHERE BookingID = ?");
    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &bookingID, 0, NULL);
    SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    printf("乘客详情删除成功\n");

    // 删除预订 - 管理员可以删除任何预订，普通用户只能删除自己的
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (isAdmin) {
        swprintf(query, 512, L"DELETE FROM Bookings WHERE BookingID = ?");
        SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &bookingID, 0, NULL);
    }
    else {
        swprintf(query, 512, L"DELETE FROM Bookings WHERE BookingID = ? AND UserID = ?");
        SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &bookingID, 0, NULL);
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &currentUserID, 0, NULL);
    }

    ret = SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        printf("取消预订成功: 预订ID=%d\n", bookingID);

        // 触发候补分配
        printf("触发候补分配处理，航班ID=%d，释放座位=%d\n", flightID, passengerCount);
        ProcessWaitlistForFlight(flightID, passengerCount);

        return TRUE;
    }

    printf("取消预订失败\n");
    return FALSE;
}

//添加航班
BOOL AddFlight(const FlightInfo* flight) {
    if (!dbConn.connected) {
        printf("数据库未连接，无法添加航班\n");
        return FALSE;
    }

    if (!isAdmin) {
        printf("无管理员权限\n");
        return FALSE;
    }

    SQLHSTMT hstmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) return FALSE;

    wchar_t query[1024];
    swprintf(query, 1024,
        L"INSERT INTO Flights (FlightNumber, Airline, DepartureCity, ArrivalCity, "
        L"DepartureTime, ArrivalTime, TotalSeats, AvailableSeats, EconomyPrice, BusinessPrice, FirstClassPrice, FlightDay, PlaneModel) "
        L"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);

    // 绑定参数
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, (SQLPOINTER)flight->flightNumber, 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)flight->airline, 0, NULL);
    SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)flight->departureCity, 0, NULL);
    SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)flight->arrivalCity, 0, NULL);

    // 转换时间字符串为数据库格式
    wchar_t depTime[50], arrTime[50];
    swprintf(depTime, 50, L"%s:00", flight->departureTime);
    swprintf(arrTime, 50, L"%s:00", flight->arrivalTime);

    SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)depTime, 0, NULL);
    SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)arrTime, 0, NULL);
    SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&flight->totalSeats, 0, NULL);

    // 使用输入的余票量，而不是总座位数
    SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&flight->availableSeats, 0, NULL);

    SQLBindParameter(hstmt, 9, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, (SQLPOINTER)&flight->economyPrice, 0, NULL);
    SQLBindParameter(hstmt, 10, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, (SQLPOINTER)&flight->businessPrice, 0, NULL);
    SQLBindParameter(hstmt, 11, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, (SQLPOINTER)&flight->firstClassPrice, 0, NULL);
    SQLBindParameter(hstmt, 12, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, (SQLPOINTER)flight->flightDay, 0, NULL);
    SQLBindParameter(hstmt, 13, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 30, 0, (SQLPOINTER)flight->planeModel, 0, NULL);

    ret = SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        printf("添加航班成功: %ls, 总座位=%d, 余票=%d\n",
            flight->flightNumber, flight->totalSeats, flight->availableSeats);
        return TRUE;
    }

    printf("添加航班失败: %ls\n", flight->flightNumber);
    return FALSE;
}

// 更新航班 
BOOL UpdateFlight(const FlightInfo* flight) {
    if (!dbConn.connected || !isAdmin) {
        return FALSE;
    }

    SQLHSTMT hstmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) return FALSE;

    wchar_t query[1024];
    // 在UPDATE语句中添加AvailableSeats字段
    swprintf(query, 1024,
        L"UPDATE Flights SET FlightNumber=?, Airline=?, DepartureCity=?, ArrivalCity=?, "
        L"DepartureTime=?, ArrivalTime=?, TotalSeats=?, AvailableSeats=?, EconomyPrice=?, BusinessPrice=?, FirstClassPrice=?, FlightDay=?, PlaneModel=? "
        L"WHERE FlightID=?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);

    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, (SQLPOINTER)flight->flightNumber, 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)flight->airline, 0, NULL);
    SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)flight->departureCity, 0, NULL);
    SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)flight->arrivalCity, 0, NULL);

    wchar_t depTime[50], arrTime[50];
    swprintf(depTime, 50, L"%s:00", flight->departureTime);
    swprintf(arrTime, 50, L"%s:00", flight->arrivalTime);

    SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)depTime, 0, NULL);
    SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)arrTime, 0, NULL);
    SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&flight->totalSeats, 0, NULL);

    // 添加AvailableSeats参数绑定
    SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&flight->availableSeats, 0, NULL);

    SQLBindParameter(hstmt, 9, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, (SQLPOINTER)&flight->economyPrice, 0, NULL);
    SQLBindParameter(hstmt, 10, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, (SQLPOINTER)&flight->businessPrice, 0, NULL);
    SQLBindParameter(hstmt, 11, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, (SQLPOINTER)&flight->firstClassPrice, 0, NULL);
    SQLBindParameter(hstmt, 12, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, (SQLPOINTER)flight->flightDay, 0, NULL);
    SQLBindParameter(hstmt, 13, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 30, 0, (SQLPOINTER)flight->planeModel, 0, NULL);
    SQLBindParameter(hstmt, 14, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&flight->flightID, 0, NULL);

    ret = SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        printf("更新航班成功: ID=%d, 余票=%d\n", flight->flightID, flight->availableSeats);

        // 如果余票从0变为大于0，触发候补分配逻辑
        if (flight->availableSeats > 0) {
            printf("航班 %d 余票从0变为%d，触发候补分配\n", flight->flightID, flight->availableSeats);
            ProcessWaitlistForFlight(flight->flightID, flight->availableSeats);
        }

        return TRUE;
    }

    printf("更新航班失败: ID=%d\n", flight->flightID);
    return FALSE;
}

// 删除航班
BOOL DeleteFlight(int flightID) {
    if (!dbConn.connected || !isAdmin) {
        return FALSE;
    }

    // 检查是否有相关预订
    SQLHSTMT hstmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) return FALSE;

    wchar_t query[256];
    swprintf(query, 256, L"SELECT COUNT(*) FROM Bookings WHERE FlightID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
    SQLExecute(hstmt);

    int bookingCount = 0;
    if (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_LONG, &bookingCount, 0, NULL);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (bookingCount > 0) {
        printf("无法删除航班，存在相关预订: %d\n", bookingCount);
        return FALSE;
    }

    // 删除航班
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    swprintf(query, 256, L"DELETE FROM Flights WHERE FlightID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);

    ret = SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        printf("删除航班成功: ID=%d\n", flightID);
        return TRUE;
    }

    printf("删除航班失败: ID=%d\n", flightID);
    return FALSE;
}


// 添加候补预订
BOOL AddToWaitlist(int flightID, const wchar_t* passengerName,
    const wchar_t* idCard, const wchar_t* gender,
    int ticketCount, int classLevel) {
    if (!dbConn.connected) {
        printf("数据库未连接，无法添加候补\n");
        return FALSE;
    }

    if (!isLoggedIn) {
        printf("用户未登录，无法添加候补\n");
        return FALSE;
    }

    SQLHSTMT hstmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) return FALSE;

    wchar_t query[512];
    swprintf(query, 512,
        L"INSERT INTO WaitlistBookings (UserID, FlightID, PassengerName, IDCardNumber, Gender, TicketCount, ClassLevel, Status) "
        L"VALUES (?, ?, ?, ?, ?, ?, ?, 'waiting')");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &currentUserID, 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
    SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)passengerName, 0, NULL);
    SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, (SQLPOINTER)idCard, 0, NULL);
    SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, (SQLPOINTER)gender, 0, NULL);
    SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &ticketCount, 0, NULL);
    SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &classLevel, 0, NULL);

    ret = SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        printf("候补预订添加成功: 航班ID=%d, 乘客=%ls, 票数=%d\n", flightID, passengerName, ticketCount);
        return TRUE;
    }

    printf("候补预订添加失败\n");
    return FALSE;
}

// 获取候补预订列表
BOOL GetWaitlistBookings(WaitlistBooking** waitlists, int* count) {
    if (!dbConn.connected) {
        printf("数据库未连接，无法获取候补列表\n");
        return FALSE;
    }

    wchar_t query[1024];
    SQLHSTMT hstmt;

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) {
        return FALSE;
    }

    if (isAdmin) {
        // 管理员：查看所有候补
        swprintf(query, 1024,
            L"SELECT w.WaitlistID, w.UserID, w.FlightID, w.PassengerName, w.IDCardNumber, "
            L"w.Gender, w.TicketCount, w.ClassLevel, CONVERT(varchar(19), w.WaitlistDate, 120), w.Status, "
            L"f.FlightNumber, f.DepartureCity, f.ArrivalCity "
            L"FROM WaitlistBookings w "
            L"INNER JOIN Flights f ON w.FlightID = f.FlightID "
            L"ORDER BY w.WaitlistDate DESC");
    }
    else {
        // 普通用户：只查看自己的候补
        swprintf(query, 1024,
            L"SELECT w.WaitlistID, w.UserID, w.FlightID, w.PassengerName, w.IDCardNumber, "
            L"w.Gender, w.TicketCount, w.ClassLevel, CONVERT(varchar(19), w.WaitlistDate, 120), w.Status, "
            L"f.FlightNumber, f.DepartureCity, f.ArrivalCity "
            L"FROM WaitlistBookings w "
            L"INNER JOIN Flights f ON w.FlightID = f.FlightID "
            L"WHERE w.UserID = %d "
            L"ORDER BY w.WaitlistDate DESC", currentUserID);
    }

    ret = SQLExecDirectW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    // 计算记录数
    *count = 0;
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        (*count)++;
    }

    printf("找到 %d 条候补记录\n", *count);

    if (*count == 0) {
        *waitlists = NULL;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return TRUE;
    }

    // 分配内存
    *waitlists = (WaitlistBooking*)malloc(*count * sizeof(WaitlistBooking));
    if (*waitlists == NULL) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    // 重新执行查询获取数据
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    SQLExecDirectW(hstmt, (SQLWCHAR*)query, SQL_NTS);

    int index = 0;
    SQLLEN indicator;

    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        memset(&(*waitlists)[index], 0, sizeof(WaitlistBooking));

        SQLGetData(hstmt, 1, SQL_C_LONG, &(*waitlists)[index].waitlistID, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_LONG, &(*waitlists)[index].userID, 0, NULL);
        SQLGetData(hstmt, 3, SQL_C_LONG, &(*waitlists)[index].flightID, 0, NULL);
        SQLGetData(hstmt, 4, SQL_C_WCHAR, (*waitlists)[index].passengerName, 100, NULL);
        SQLGetData(hstmt, 5, SQL_C_WCHAR, (*waitlists)[index].idCardNumber, 20, NULL);
        SQLGetData(hstmt, 6, SQL_C_WCHAR, (*waitlists)[index].gender, 10, NULL);
        SQLGetData(hstmt, 7, SQL_C_LONG, &(*waitlists)[index].ticketCount, 0, NULL);
        SQLGetData(hstmt, 8, SQL_C_LONG, &(*waitlists)[index].classLevel, 0, NULL);

        wchar_t waitlistDate[50] = { 0 };
        SQLGetData(hstmt, 9, SQL_C_WCHAR, waitlistDate, 50, &indicator);
        if (indicator != SQL_NULL_DATA) {
            wcsncpy_s((*waitlists)[index].waitlistDate, 50, waitlistDate, _TRUNCATE);
        }

        SQLGetData(hstmt, 10, SQL_C_WCHAR, (*waitlists)[index].status, 20, NULL);
        SQLGetData(hstmt, 11, SQL_C_WCHAR, (*waitlists)[index].flightNumber, 20, NULL);
        SQLGetData(hstmt, 12, SQL_C_WCHAR, (*waitlists)[index].departureCity, 50, NULL);
        SQLGetData(hstmt, 13, SQL_C_WCHAR, (*waitlists)[index].arrivalCity, 50, NULL);

        index++;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return TRUE;
}

// 获取乘客详细信息
BOOL GetPassengerDetails(int bookingID, PassengerDetail** details, int* count) {
    if (!dbConn.connected) {
        printf("数据库未连接，无法获取乘客详情\n");
        return FALSE;
    }

    wchar_t query[512];
    SQLHSTMT hstmt;

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) {
        return FALSE;
    }

    swprintf(query, 512,
        L"SELECT PassengerID, BookingID, IDCardNumber, Gender, Phone, Email "
        L"FROM PassengerDetails "
        L"WHERE BookingID = %d", bookingID);

    ret = SQLExecDirectW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    // 计算记录数
    *count = 0;
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        (*count)++;
    }

    if (*count == 0) {
        *details = NULL;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return TRUE;
    }

    // 分配内存
    *details = (PassengerDetail*)malloc(*count * sizeof(PassengerDetail));
    if (*details == NULL) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    // 重新执行查询获取数据
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    SQLExecDirectW(hstmt, (SQLWCHAR*)query, SQL_NTS);

    int index = 0;
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        memset(&(*details)[index], 0, sizeof(PassengerDetail));

        SQLGetData(hstmt, 1, SQL_C_LONG, &(*details)[index].passengerID, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_LONG, &(*details)[index].bookingID, 0, NULL);
        SQLGetData(hstmt, 3, SQL_C_WCHAR, (*details)[index].idCardNumber, 20, NULL);
        SQLGetData(hstmt, 4, SQL_C_WCHAR, (*details)[index].gender, 10, NULL);
        SQLGetData(hstmt, 5, SQL_C_WCHAR, (*details)[index].phone, 20, NULL);
        SQLGetData(hstmt, 6, SQL_C_WCHAR, (*details)[index].email, 100, NULL);

        index++;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return TRUE;
}

// 更新预订信息
BOOL UpdateBooking(int bookingID, const wchar_t* passengerName,
    const wchar_t* idCard, const wchar_t* gender,
    const wchar_t* phone, const wchar_t* email,
    int ticketCount, int classLevel) {
    if (!dbConn.connected) {
        printf("数据库未连接，无法更新预订\n");
        return FALSE;
    }

    SQLHSTMT hstmt;
    SQLRETURN ret;

    // 首先获取原预订信息来计算价格差异
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    wchar_t query[512];
    swprintf(query, 512,
        L"SELECT b.FlightID, b.PassengerCount, f.EconomyPrice, f.BusinessPrice, f.FirstClassPrice "
        L"FROM Bookings b "
        L"INNER JOIN Flights f ON b.FlightID = f.FlightID "
        L"WHERE b.BookingID = ? AND b.UserID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &bookingID, 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &currentUserID, 0, NULL);
    SQLExecute(hstmt);

    int flightID = 0, oldTicketCount = 0;
    float economyPrice = 0, businessPrice = 0, firstClassPrice = 0;
    if (SQLFetch(hstmt) != SQL_SUCCESS) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        printf("未找到预订记录或无权操作\n");
        return FALSE;
    }

    SQLGetData(hstmt, 1, SQL_C_LONG, &flightID, 0, NULL);
    SQLGetData(hstmt, 2, SQL_C_LONG, &oldTicketCount, 0, NULL);
    SQLGetData(hstmt, 3, SQL_C_FLOAT, &economyPrice, 0, NULL);
    SQLGetData(hstmt, 4, SQL_C_FLOAT, &businessPrice, 0, NULL);
    SQLGetData(hstmt, 5, SQL_C_FLOAT, &firstClassPrice, 0, NULL);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    // 计算新总金额
    float pricePerTicket = 0;
    switch (classLevel) {
    case 1: pricePerTicket = economyPrice; break;
    case 2: pricePerTicket = businessPrice; break;
    case 3: pricePerTicket = firstClassPrice; break;
    default: pricePerTicket = economyPrice; break;
    }
    float totalAmount = pricePerTicket * ticketCount;

    // 更新预订表
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    swprintf(query, 512,
        L"UPDATE Bookings SET PassengerName = ?, PassengerCount = ?, ClassLevel = ?, TotalAmount = ? "
        L"WHERE BookingID = ? AND UserID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)passengerName, 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &ticketCount, 0, NULL);
    SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &classLevel, 0, NULL);
    SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &totalAmount, 0, NULL);
    SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &bookingID, 0, NULL);
    SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &currentUserID, 0, NULL);

    ret = SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("更新预订失败\n");
        return FALSE;
    }

    // 更新乘客详情
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    swprintf(query, 512,
        L"UPDATE PassengerDetails SET IDCardNumber = ?, Gender = ?, Phone = ?, Email = ? "
        L"WHERE BookingID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, (SQLPOINTER)idCard, 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, (SQLPOINTER)gender, 0, NULL);
    SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, (SQLPOINTER)phone, 0, NULL);
    SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)email, 0, NULL);
    SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &bookingID, 0, NULL);

    ret = SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("更新乘客详情失败\n");
        return FALSE;
    }

    // 更新航班余票（如果票数有变化）
    if (ticketCount != oldTicketCount) {
        int seatDifference = oldTicketCount - ticketCount; // 正数表示释放座位，负数表示占用更多座位

        SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
        swprintf(query, 512, L"UPDATE Flights SET AvailableSeats = AvailableSeats + ? WHERE FlightID = ?");

        SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &seatDifference, 0, NULL);
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);

        ret = SQLExecute(hstmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            printf("更新余票失败\n");
            return FALSE;
        }
    }

    printf("预订更新成功: 预订ID=%d\n", bookingID);
    return TRUE;
}

// 获取用户订单及详细信息
BOOL GetUserBookingsWithDetails(int userID, BookingInfo** bookings, PassengerDetail*** details, int* count) {
    if (!dbConn.connected) {
        printf("数据库未连接，无法获取订单详情\n");
        return FALSE;
    }

    // 首先获取订单基本信息
    if (!GetUserBookings(userID, bookings, count)) {
        return FALSE;
    }

    if (*count == 0) {
        *details = NULL;
        return TRUE;
    }

    // 为详细信息分配内存
    *details = (PassengerDetail**)malloc(*count * sizeof(PassengerDetail*));
    if (*details == NULL) {
        return FALSE;
    }

    // 为每个订单获取乘客详情
    for (int i = 0; i < *count; i++) {
        int detailCount = 0;
        if (!GetPassengerDetails((*bookings)[i].bookingID, &((*details)[i]), &detailCount)) {
            // 如果获取详情失败，设置为空
            (*details)[i] = NULL;
        }
    }

    return TRUE;
}


// 获取所有乘客预订信息（管理员用）
BOOL GetAllPassengerBookings(BookingInfo** bookings, int* count) {
    if (!dbConn.connected) {
        printf("数据库未连接，无法获取乘客信息\n");
        return FALSE;
    }

    wchar_t query[1024];
    SQLHSTMT hstmt;

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) {
        return FALSE;
    }

    swprintf(query, 1024,
        L"SELECT b.BookingID, b.UserID, b.FlightID, b.PassengerName, b.PassengerCount, "
        L"b.ClassLevel, b.TotalAmount, CONVERT(varchar(19), b.BookingDate, 120), "
        L"f.FlightNumber, f.DepartureCity, f.ArrivalCity "
        L"FROM Bookings b "
        L"INNER JOIN Flights f ON b.FlightID = f.FlightID "
        L"ORDER BY b.BookingDate DESC");

    ret = SQLExecDirectW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    // 计算记录数
    *count = 0;
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        (*count)++;
    }

    printf("找到 %d 条乘客预订记录\n", *count);

    if (*count == 0) {
        *bookings = NULL;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return TRUE;
    }

    // 分配内存
    *bookings = (BookingInfo*)malloc(*count * sizeof(BookingInfo));
    if (*bookings == NULL) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    // 重新执行查询获取数据
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    SQLExecDirectW(hstmt, (SQLWCHAR*)query, SQL_NTS);

    int index = 0;
    SQLLEN indicator;

    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        memset(&(*bookings)[index], 0, sizeof(BookingInfo));

        SQLGetData(hstmt, 1, SQL_C_LONG, &(*bookings)[index].bookingID, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_LONG, &(*bookings)[index].userID, 0, NULL);
        SQLGetData(hstmt, 3, SQL_C_LONG, &(*bookings)[index].flightID, 0, NULL);
        SQLGetData(hstmt, 4, SQL_C_WCHAR, (*bookings)[index].passengerName, 100, NULL);
        SQLGetData(hstmt, 5, SQL_C_LONG, &(*bookings)[index].ticketCount, 0, NULL);
        SQLGetData(hstmt, 6, SQL_C_LONG, &(*bookings)[index].classLevel, 0, NULL);
        SQLGetData(hstmt, 7, SQL_C_FLOAT, &(*bookings)[index].totalAmount, 0, NULL);

        wchar_t bookDate[50] = { 0 };
        SQLGetData(hstmt, 8, SQL_C_WCHAR, bookDate, 50, &indicator);
        if (indicator != SQL_NULL_DATA) {
            wcsncpy_s((*bookings)[index].bookingDate, 50, bookDate, _TRUNCATE);
        }

        SQLGetData(hstmt, 9, SQL_C_WCHAR, (*bookings)[index].flightNumber, 20, NULL);
        SQLGetData(hstmt, 10, SQL_C_WCHAR, (*bookings)[index].departureCity, 50, NULL);
        SQLGetData(hstmt, 11, SQL_C_WCHAR, (*bookings)[index].arrivalCity, 50, NULL);

        index++;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return TRUE;
}

// 检查航班余票
BOOL CheckFlightAvailability(int flightID, int ticketCount) {
    if (!dbConn.connected) return FALSE;

    SQLHSTMT hstmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) return FALSE;

    wchar_t query[256];
    swprintf(query, 256, L"SELECT AvailableSeats FROM Flights WHERE FlightID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
    SQLExecute(hstmt);

    int availableSeats = 0;
    if (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_LONG, &availableSeats, 0, NULL);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return (availableSeats >= ticketCount);
}

// 获取航班余票数量
int GetFlightAvailableSeats(int flightID) {
    if (!dbConn.connected) return 0;

    SQLHSTMT hstmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) return 0;

    wchar_t query[256];
    swprintf(query, 256, L"SELECT AvailableSeats FROM Flights WHERE FlightID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
    SQLExecute(hstmt);

    int availableSeats = 0;
    if (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_LONG, &availableSeats, 0, NULL);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return availableSeats;
}

// 获取带状态的用户订单
BOOL GetUserBookingsWithStatus(int userID, BookingInfo** bookings, int* count) {
    if (!dbConn.connected) {
        printf("数据库未连接，无法获取订单\n");
        return FALSE;
    }

    wchar_t query[2048];
    SQLHSTMT hstmt;

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) {
        return FALSE;
    }

    // 合并查询：获取普通预订和候补预订
    swprintf(query, 2048,
        L"SELECT "
        L"b.BookingID as ID, b.UserID, b.FlightID, b.PassengerName, b.PassengerCount as TicketCount, "
        L"b.ClassLevel, b.TotalAmount, CONVERT(varchar(19), b.BookingDate, 120) as OrderDate, "
        L"f.FlightNumber, f.DepartureCity, f.ArrivalCity, 'confirmed' as Status, 'booking' as Type "
        L"FROM Bookings b "
        L"INNER JOIN Flights f ON b.FlightID = f.FlightID "
        L"WHERE b.UserID = %d "
        L"UNION ALL "
        L"SELECT "
        L"w.WaitlistID as ID, w.UserID, w.FlightID, w.PassengerName, w.TicketCount, "
        L"w.ClassLevel, 0 as TotalAmount, CONVERT(varchar(19), w.WaitlistDate, 120) as OrderDate, "
        L"f.FlightNumber, f.DepartureCity, f.ArrivalCity, w.Status, 'waitlist' as Type "
        L"FROM WaitlistBookings w "
        L"INNER JOIN Flights f ON w.FlightID = f.FlightID "
        L"WHERE w.UserID = %d "
        L"ORDER BY OrderDate DESC", userID, userID);

    printf("执行订单查询: %ls\n", query);

    ret = SQLExecDirectW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    // 计算记录数
    *count = 0;
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        (*count)++;
    }

    printf("找到 %d 条订单记录\n", *count);

    if (*count == 0) {
        *bookings = NULL;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return TRUE;
    }

    // 分配内存
    *bookings = (BookingInfo*)malloc(*count * sizeof(BookingInfo));
    if (*bookings == NULL) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    // 重新执行查询获取数据
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    SQLExecDirectW(hstmt, (SQLWCHAR*)query, SQL_NTS);

    int index = 0;
    SQLLEN indicator;

    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        memset(&(*bookings)[index], 0, sizeof(BookingInfo));

        // 获取数据
        SQLGetData(hstmt, 1, SQL_C_LONG, &(*bookings)[index].bookingID, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_LONG, &(*bookings)[index].userID, 0, NULL);
        SQLGetData(hstmt, 3, SQL_C_LONG, &(*bookings)[index].flightID, 0, NULL);
        SQLGetData(hstmt, 4, SQL_C_WCHAR, (*bookings)[index].passengerName, 100, NULL);
        SQLGetData(hstmt, 5, SQL_C_LONG, &(*bookings)[index].ticketCount, 0, NULL);
        SQLGetData(hstmt, 6, SQL_C_LONG, &(*bookings)[index].classLevel, 0, NULL);
        SQLGetData(hstmt, 7, SQL_C_FLOAT, &(*bookings)[index].totalAmount, 0, NULL);

        // 获取预订时间
        wchar_t orderDate[50] = { 0 };
        SQLGetData(hstmt, 8, SQL_C_WCHAR, orderDate, 50, &indicator);
        if (indicator != SQL_NULL_DATA) {
            wcsncpy_s((*bookings)[index].bookingDate, 50, orderDate, _TRUNCATE);
        }

        SQLGetData(hstmt, 9, SQL_C_WCHAR, (*bookings)[index].flightNumber, 20, NULL);
        SQLGetData(hstmt, 10, SQL_C_WCHAR, (*bookings)[index].departureCity, 50, NULL);
        SQLGetData(hstmt, 11, SQL_C_WCHAR, (*bookings)[index].arrivalCity, 50, NULL);

        // 获取状态字段
        wchar_t status[20] = { 0 };
        SQLGetData(hstmt, 12, SQL_C_WCHAR, status, 20, &indicator);

        // 获取类型（预订或候补）
        wchar_t type[20] = { 0 };
        SQLGetData(hstmt, 13, SQL_C_WCHAR, type, 20, &indicator);

        printf("原始数据 - 类型: %ls, 状态: %ls, 金额: %.2f\n", type, status, (*bookings)[index].totalAmount);

        // 根据类型和状态设置显示状态
        if (wcscmp(type, L"waitlist") == 0) {
            // 候补订单
            if (wcscmp(status, L"waiting") == 0) {
                wcscpy_s((*bookings)[index].status, 20, L"候补中");
            }
            else if (wcscmp(status, L"partially_success") == 0) {
                wcscpy_s((*bookings)[index].status, 20, L"部分成功");
            }
            else {
                wcscpy_s((*bookings)[index].status, 20, L"候补中");
            }
            printf("设置为候补状态: %ls\n", (*bookings)[index].status);
        }
        else {
            // 普通预订订单 - 候补成功的预订也在这里
            wcscpy_s((*bookings)[index].status, 20, L"已预订");
            printf("设置为预订状态\n");
        }

        printf("加载订单: ID=%d, 乘客=%ls, 类型=%ls, 显示状态=%ls, 金额=%.2f, 时间=%ls\n",
            (*bookings)[index].bookingID,
            (*bookings)[index].passengerName,
            type,
            (*bookings)[index].status,
            (*bookings)[index].totalAmount,
            (*bookings)[index].bookingDate);

        index++;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return TRUE;
}

// 删除乘客预订
BOOL DeletePassengerBooking(int bookingID) {
    if (!dbConn.connected) {
        printf("数据库未连接，无法删除乘客预订\n");
        return FALSE;
    }

    SQLHSTMT hstmt;
    SQLRETURN ret;

    printf("=== 删除乘客预订详细调试 ===\n");
    printf("预订ID: %d\n", bookingID);

    // 获取预订信息以恢复余票
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    wchar_t query[512];
    swprintf(query, 512,
        L"SELECT FlightID, PassengerCount FROM Bookings WHERE BookingID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &bookingID, 0, NULL);
    SQLExecute(hstmt);

    int flightID = 0;
    int passengerCount = 0;
    if (SQLFetch(hstmt) != SQL_SUCCESS) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        printf("未找到预订记录\n");
        return FALSE;
    }

    SQLGetData(hstmt, 1, SQL_C_LONG, &flightID, 0, NULL);
    SQLGetData(hstmt, 2, SQL_C_LONG, &passengerCount, 0, NULL);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    printf("删除预订信息: 航班ID=%d, 票数=%d\n", flightID, passengerCount);

    // 开始事务：恢复余票并删除预订
    // 恢复余票
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    swprintf(query, 512, L"UPDATE Flights SET AvailableSeats = AvailableSeats + ? WHERE FlightID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &passengerCount, 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);

    ret = SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("恢复余票失败\n");
        return FALSE;
    }
    printf("余票恢复成功\n");

    // 删除乘客详细信息
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    swprintf(query, 512, L"DELETE FROM PassengerDetails WHERE BookingID = ?");
    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &bookingID, 0, NULL);
    SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    printf("乘客详情删除成功\n");

    // 删除预订
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    swprintf(query, 512, L"DELETE FROM Bookings WHERE BookingID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &bookingID, 0, NULL);

    ret = SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        printf("删除乘客预订成功: 预订ID=%d\n", bookingID);

        // 触发候补分配
        printf("触发候补分配处理，航班ID=%d，释放座位=%d\n", flightID, passengerCount);
        ProcessWaitlistForFlight(flightID, passengerCount);

        return TRUE;
    }

    printf("删除乘客预订失败\n");
    return FALSE;
}

// 删除候补预订
BOOL DeleteWaitlistBooking(int waitlistID) {
    if (!dbConn.connected) {
        printf("数据库未连接，无法删除候补预订\n");
        return FALSE;
    }

    SQLHSTMT hstmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) return FALSE;

    printf("删除候补预订，候补ID: %d\n", waitlistID);

    wchar_t query[512];
    swprintf(query, 512, L"DELETE FROM WaitlistBookings WHERE WaitlistID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &waitlistID, 0, NULL);

    ret = SQLExecute(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        printf("删除候补预订成功: 候补ID=%d\n", waitlistID);
        return TRUE;
    }

    printf("删除候补预订失败，候补ID: %d\n", waitlistID);

    // 添加详细错误信息
    SQLWCHAR sqlState[6] = { 0 };
    SQLWCHAR errorMsg[1024] = { 0 };
    SQLINTEGER nativeError = 0;
    SQLSMALLINT msgLength = 0;

    SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlState, &nativeError, errorMsg, sizeof(errorMsg) / sizeof(SQLWCHAR), &msgLength);
    printf("错误代码: %d\n", nativeError);
    printf("错误状态: %ls\n", sqlState);
    printf("错误信息: %ls\n", errorMsg);

    return FALSE;
}

// 根据航班ID获取航班信息
BOOL GetFlightByID(int flightID, FlightInfo* flight) {
    if (!dbConn.connected || !flight) {
        return FALSE;
    }

    SQLHSTMT hstmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) return FALSE;

    wchar_t query[1024];
    swprintf(query, 1024,
        L"SELECT FlightID, FlightNumber, Airline, DepartureCity, ArrivalCity, "
        L"CONVERT(varchar(16), DepartureTime, 120), CONVERT(varchar(16), ArrivalTime, 120), "
        L"TotalSeats, AvailableSeats, EconomyPrice, BusinessPrice, FirstClassPrice, "
        L"ISNULL(FlightDay, ''), ISNULL(PlaneModel, '') "
        L"FROM Flights WHERE FlightID = ?");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);

    ret = SQLExecute(hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    SQLLEN indicator;
    if (SQLFetch(hstmt) == SQL_SUCCESS) {
        memset(flight, 0, sizeof(FlightInfo));

        SQLGetData(hstmt, 1, SQL_C_LONG, &flight->flightID, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_WCHAR, flight->flightNumber, sizeof(flight->flightNumber), NULL);
        SQLGetData(hstmt, 3, SQL_C_WCHAR, flight->airline, sizeof(flight->airline), NULL);
        SQLGetData(hstmt, 4, SQL_C_WCHAR, flight->departureCity, sizeof(flight->departureCity), NULL);
        SQLGetData(hstmt, 5, SQL_C_WCHAR, flight->arrivalCity, sizeof(flight->arrivalCity), NULL);

        wchar_t depTime[50] = { 0 }, arrTime[50] = { 0 };
        SQLGetData(hstmt, 6, SQL_C_WCHAR, depTime, sizeof(depTime), &indicator);
        SQLGetData(hstmt, 7, SQL_C_WCHAR, arrTime, sizeof(arrTime), &indicator);

        if (indicator != SQL_NULL_DATA) {
            wcsncpy_s(flight->departureTime, sizeof(flight->departureTime) / sizeof(wchar_t), depTime, _TRUNCATE);
            wcsncpy_s(flight->arrivalTime, sizeof(flight->arrivalTime) / sizeof(wchar_t), arrTime, _TRUNCATE);
        }

        SQLGetData(hstmt, 8, SQL_C_LONG, &flight->totalSeats, 0, NULL);
        SQLGetData(hstmt, 9, SQL_C_LONG, &flight->availableSeats, 0, NULL);
        SQLGetData(hstmt, 10, SQL_C_FLOAT, &flight->economyPrice, 0, NULL);
        SQLGetData(hstmt, 11, SQL_C_FLOAT, &flight->businessPrice, 0, NULL);
        SQLGetData(hstmt, 12, SQL_C_FLOAT, &flight->firstClassPrice, 0, NULL);

        wchar_t flightDay[20] = { 0 }, planeModel[30] = { 0 };
        SQLGetData(hstmt, 13, SQL_C_WCHAR, flightDay, sizeof(flightDay), &indicator);
        SQLGetData(hstmt, 14, SQL_C_WCHAR, planeModel, sizeof(planeModel), &indicator);

        if (indicator != SQL_NULL_DATA) {
            wcsncpy_s(flight->flightDay, sizeof(flight->flightDay) / sizeof(wchar_t), flightDay, _TRUNCATE);
            wcsncpy_s(flight->planeModel, sizeof(flight->planeModel) / sizeof(wchar_t), planeModel, _TRUNCATE);
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return TRUE;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return FALSE;
}

// 自动处理候补分配
BOOL ProcessWaitlistForFlight(int flightID, int availableSeats) {
    if (!dbConn.connected || availableSeats <= 0) {
        printf("数据库未连接或无可分配座位\n");
        return FALSE;
    }

    printf("=== 开始处理候补分配 ===\n");
    printf("航班ID: %d, 可用座位: %d\n", flightID, availableSeats);

    SQLHSTMT hstmt;
    SQLRETURN ret;

    // 使用游标方式确保顺序 
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) {
        printf("!!! 无法分配语句句柄 !!!\n");
        return FALSE;
    }

    // 设置游标属性以确保顺序和可滚动
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, SQL_IS_UINTEGER);
    SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_READ_ONLY, SQL_IS_UINTEGER);
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER)SQL_SCROLLABLE, SQL_IS_UINTEGER);

    printf("设置游标属性完成\n");

    wchar_t query[1024];
    swprintf(query, 1024,
        L"SELECT WaitlistID, UserID, PassengerName, IDCardNumber, Gender, TicketCount, ClassLevel, WaitlistDate "
        L"FROM WaitlistBookings "
        L"WHERE FlightID = ? AND Status = 'waiting' "
        L"ORDER BY WaitlistID ASC");

    printf("执行游标查询:\n%ls\n", query);

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);

    ret = SQLExecute(hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("!!! 执行候补查询失败 !!!\n");

        // 获取详细错误信息
        SQLWCHAR sqlState[6] = { 0 };
        SQLWCHAR errorMsg[1024] = { 0 };
        SQLINTEGER nativeError = 0;
        SQLSMALLINT msgLength = 0;
        SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlState, &nativeError, errorMsg, sizeof(errorMsg) / sizeof(SQLWCHAR), &msgLength);
        printf("错误代码: %d\n", nativeError);
        printf("错误状态: %ls\n", sqlState);
        printf("错误信息: %ls\n", errorMsg);

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }

    printf("=== 游标查询执行成功 ===\n");

    // 验证游标是否按正确顺序返回数据
    printf("=== 验证游标返回顺序 ===\n");
    int verifyCount = 0;
    SQLRETURN fetchRet;

    // 首先移动到第一行
    fetchRet = SQLFetchScroll(hstmt, SQL_FETCH_FIRST, 0);
    while (fetchRet == SQL_SUCCESS) {
        verifyCount++;
        int waitlistID = 0;
        wchar_t passengerName[100] = { 0 };

        SQLGetData(hstmt, 1, SQL_C_LONG, &waitlistID, 0, NULL);
        SQLGetData(hstmt, 3, SQL_C_WCHAR, passengerName, 100, NULL);

        printf("  游标顺序 %d: ID=%d, 乘客=%ls\n", verifyCount, waitlistID, passengerName);

        // 移动到下一行
        fetchRet = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
    }

    printf("游标返回了 %d 条记录\n", verifyCount);

    // 重新回到第一行开始处理
    SQLFetchScroll(hstmt, SQL_FETCH_FIRST, 0);

    // 获取航班价格信息
    float economyPrice = 0, businessPrice = 0, firstClassPrice = 0;
    SQLHSTMT hstmtPrice;
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmtPrice);
    swprintf(query, 1024, L"SELECT EconomyPrice, BusinessPrice, FirstClassPrice FROM Flights WHERE FlightID = ?");
    SQLPrepareW(hstmtPrice, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmtPrice, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
    SQLExecute(hstmtPrice);

    if (SQLFetch(hstmtPrice) == SQL_SUCCESS) {
        SQLGetData(hstmtPrice, 1, SQL_C_FLOAT, &economyPrice, 0, NULL);
        SQLGetData(hstmtPrice, 2, SQL_C_FLOAT, &businessPrice, 0, NULL);
        SQLGetData(hstmtPrice, 3, SQL_C_FLOAT, &firstClassPrice, 0, NULL);
        printf("获取到航班价格: 经济舱=%.2f, 商务舱=%.2f, 头等舱=%.2f\n", economyPrice, businessPrice, firstClassPrice);
    }
    else {
        printf("!!! 获取航班价格失败 !!!\n");
        SQLFreeHandle(SQL_HANDLE_STMT, hstmtPrice);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return FALSE;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmtPrice);

    int remainingSeats = availableSeats;
    BOOL anySuccess = FALSE;
    int processedCount = 0;

    printf("开始分配候补座位...\n");

    // 使用游标遍历候补列表进行分配
    SQLRETURN fetchResult = SQLFetchScroll(hstmt, SQL_FETCH_FIRST, 0);
    while (remainingSeats > 0 && fetchResult == SQL_SUCCESS) {
        processedCount++;
        int waitlistID = 0, userID = 0, ticketCount = 0, classLevel = 0;
        wchar_t passengerName[100] = { 0 }, idCard[20] = { 0 }, gender[10] = { 0 }, waitlistDate[50] = { 0 };

        SQLGetData(hstmt, 1, SQL_C_LONG, &waitlistID, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_LONG, &userID, 0, NULL);
        SQLGetData(hstmt, 3, SQL_C_WCHAR, passengerName, 100, NULL);
        SQLGetData(hstmt, 4, SQL_C_WCHAR, idCard, 20, NULL);
        SQLGetData(hstmt, 5, SQL_C_WCHAR, gender, 10, NULL);
        SQLGetData(hstmt, 6, SQL_C_LONG, &ticketCount, 0, NULL);
        SQLGetData(hstmt, 7, SQL_C_LONG, &classLevel, 0, NULL);
        SQLGetData(hstmt, 8, SQL_C_WCHAR, waitlistDate, 50, NULL);

        printf("【游标顺序第%d位】处理候补: ID=%d, 乘客=%ls, 时间=%ls, 需要票数=%d, 舱位=%d, 剩余座位=%d\n",
            processedCount, waitlistID, passengerName, waitlistDate, ticketCount, classLevel, remainingSeats);

        // 计算可分配的票数
        int assignableTickets = (ticketCount <= remainingSeats) ? ticketCount : remainingSeats;
        int remainingTickets = ticketCount - assignableTickets;

        // 计算金额
        float pricePerTicket = 0;
        switch (classLevel) {
        case 1: pricePerTicket = economyPrice; break;
        case 2: pricePerTicket = businessPrice; break;
        case 3: pricePerTicket = firstClassPrice; break;
        default: pricePerTicket = economyPrice; break;
        }

        float totalAmount = pricePerTicket * assignableTickets;
        printf("金额计算: 单价=%.2f × 票数=%d = 总金额=%.2f\n", pricePerTicket, assignableTickets, totalAmount);

        // 创建实际预订
        SQLHSTMT hstmtBooking;
        SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmtBooking);
        swprintf(query, 1024,
            L"INSERT INTO Bookings (UserID, FlightID, PassengerName, PassengerCount, ClassLevel, TotalAmount, BookingDate, Status) "
            L"VALUES (?, ?, ?, ?, ?, ?, GETDATE(), 'confirmed')");

        SQLPrepareW(hstmtBooking, (SQLWCHAR*)query, SQL_NTS);
        SQLBindParameter(hstmtBooking, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userID, 0, NULL);
        SQLBindParameter(hstmtBooking, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
        SQLBindParameter(hstmtBooking, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)passengerName, 0, NULL);
        SQLBindParameter(hstmtBooking, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &assignableTickets, 0, NULL);
        SQLBindParameter(hstmtBooking, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &classLevel, 0, NULL);
        SQLBindParameter(hstmtBooking, 6, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &totalAmount, 0, NULL);

        ret = SQLExecute(hstmtBooking);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            printf("!!! 创建预订失败，候补ID: %d !!!\n", waitlistID);
            SQLFreeHandle(SQL_HANDLE_STMT, hstmtBooking);

            // 继续处理下一行
            fetchResult = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
            continue;
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hstmtBooking);

        // 获取新创建的预订ID
        int newBookingID = 0;
        SQLHSTMT hstmtGetID;
        SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmtGetID);
        swprintf(query, 1024,
            L"SELECT BookingID FROM Bookings WHERE UserID = ? AND FlightID = ? AND PassengerName = ? AND TotalAmount = ? AND PassengerCount = ?");
        SQLPrepareW(hstmtGetID, (SQLWCHAR*)query, SQL_NTS);
        SQLBindParameter(hstmtGetID, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userID, 0, NULL);
        SQLBindParameter(hstmtGetID, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
        SQLBindParameter(hstmtGetID, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)passengerName, 0, NULL);
        SQLBindParameter(hstmtGetID, 4, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &totalAmount, 0, NULL);
        SQLBindParameter(hstmtGetID, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &assignableTickets, 0, NULL);
        SQLExecute(hstmtGetID);

        if (SQLFetch(hstmtGetID) == SQL_SUCCESS) {
            SQLGetData(hstmtGetID, 1, SQL_C_LONG, &newBookingID, 0, NULL);
            printf("成功创建预订: ID=%d, 金额=%.2f\n", newBookingID, totalAmount);
        }
        else {
            printf("!!! 未能获取新预订ID !!!\n");
            SQLFreeHandle(SQL_HANDLE_STMT, hstmtGetID);

            // 继续处理下一行
            fetchResult = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
            continue;
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hstmtGetID);

        // 添加乘客详细信息
        SQLHSTMT hstmtDetails;
        SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmtDetails);
        swprintf(query, 1024,
            L"INSERT INTO PassengerDetails (BookingID, IDCardNumber, Gender, Phone, Email) "
            L"VALUES (?, ?, ?, '', '')");

        SQLPrepareW(hstmtDetails, (SQLWCHAR*)query, SQL_NTS);
        SQLBindParameter(hstmtDetails, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &newBookingID, 0, NULL);
        SQLBindParameter(hstmtDetails, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, (SQLPOINTER)idCard, 0, NULL);
        SQLBindParameter(hstmtDetails, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, (SQLPOINTER)gender, 0, NULL);
        SQLExecute(hstmtDetails);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmtDetails);

        // 更新候补状态
        SQLHSTMT hstmtUpdateWaitlist;
        SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmtUpdateWaitlist);

        if (remainingTickets > 0) {
            // 部分成功，更新票数
            swprintf(query, 1024,
                L"UPDATE WaitlistBookings SET TicketCount = ?, Status = 'partially_success' "
                L"WHERE WaitlistID = ?");
            SQLPrepareW(hstmtUpdateWaitlist, (SQLWCHAR*)query, SQL_NTS);
            SQLBindParameter(hstmtUpdateWaitlist, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &remainingTickets, 0, NULL);
            SQLBindParameter(hstmtUpdateWaitlist, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &waitlistID, 0, NULL);
        }
        else {
            // 完全成功，删除候补记录
            swprintf(query, 1024, L"DELETE FROM WaitlistBookings WHERE WaitlistID = ?");
            SQLPrepareW(hstmtUpdateWaitlist, (SQLWCHAR*)query, SQL_NTS);
            SQLBindParameter(hstmtUpdateWaitlist, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &waitlistID, 0, NULL);
        }

        SQLExecute(hstmtUpdateWaitlist);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmtUpdateWaitlist);

        // 更新已使用的座位数
        remainingSeats -= assignableTickets;

        printf("候补分配成功: 候补ID=%d → 预订ID=%d, 分配票数=%d, 总金额=%.2f\n",
            waitlistID, newBookingID, assignableTickets, totalAmount);
        anySuccess = TRUE;

        if (remainingSeats <= 0) {
            printf("所有可用座位已分配完毕\n");
            break;
        }

        // 移动到下一行
        fetchResult = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    // 如果有成功分配，更新航班余票
    if (anySuccess) {
        int usedSeats = availableSeats - remainingSeats;
        SQLHSTMT hstmtUpdateFlight;
        SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmtUpdateFlight);
        swprintf(query, 1024, L"UPDATE Flights SET AvailableSeats = AvailableSeats - ? WHERE FlightID = ?");
        SQLPrepareW(hstmtUpdateFlight, (SQLWCHAR*)query, SQL_NTS);
        SQLBindParameter(hstmtUpdateFlight, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &usedSeats, 0, NULL);
        SQLBindParameter(hstmtUpdateFlight, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
        SQLExecute(hstmtUpdateFlight);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmtUpdateFlight);

        printf("航班 %d 候补分配完成，使用了 %d 个座位\n", flightID, usedSeats);
    }
    else {
        printf("没有成功分配任何候补座位\n");
    }

    printf("=== 候补分配处理结束 ===\n");
    return anySuccess;
}

// 调试函数：检查候补分配顺序
void DebugCheckWaitlistOrder(int flightID) {
    if (!dbConn.connected) return;

    SQLHSTMT hstmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) return;

    wchar_t query[512];
    swprintf(query, 512,
        L"SELECT WaitlistID, PassengerName, WaitlistDate, Status "
        L"FROM WaitlistBookings "
        L"WHERE FlightID = ? "
        L"ORDER BY WaitlistDate ASC");

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
    SQLExecute(hstmt);

    printf("=== 候补分配顺序调试 (航班ID: %d) ===\n", flightID);
    int count = 0;
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        int waitlistID = 0;
        wchar_t passengerName[100] = { 0 };
        wchar_t waitlistDate[50] = { 0 };
        wchar_t status[20] = { 0 };

        SQLGetData(hstmt, 1, SQL_C_LONG, &waitlistID, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_WCHAR, passengerName, 100, NULL);
        SQLGetData(hstmt, 3, SQL_C_WCHAR, waitlistDate, 50, NULL);
        SQLGetData(hstmt, 4, SQL_C_WCHAR, status, 20, NULL);

        printf("候补 %d: ID=%d, 乘客=%ls, 时间=%ls, 状态=%ls\n",
            ++count, waitlistID, passengerName, waitlistDate, status);
    }
    printf("=== 候补分配顺序调试结束 ===\n");

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
}

// 详细调试候补分配过程
void DebugWaitlistAllocation(int flightID, int availableSeats) {
    if (!dbConn.connected) return;

    printf("=== 详细候补分配调试开始 ===\n");
    printf("航班ID: %d, 可用座位: %d\n", flightID, availableSeats);

    SQLHSTMT hstmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    if (ret != SQL_SUCCESS) return;

    wchar_t query[1024];
    // 检查候补记录排序（按WaitlistID）
    swprintf(query, 1024,
        L"SELECT WaitlistID, PassengerName, WaitlistDate, TicketCount, ClassLevel, Status "
        L"FROM WaitlistBookings "
        L"WHERE FlightID = ? AND Status = 'waiting' "
        L"ORDER BY WaitlistID ASC");  // 改为按 WaitlistID ASC

    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
    SQLExecute(hstmt);

    printf("候补记录排序结果 (按WaitlistID ASC):\n");
    int count = 0;
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        int waitlistID = 0, ticketCount = 0, classLevel = 0;
        wchar_t passengerName[100] = { 0 };
        wchar_t waitlistDate[50] = { 0 };
        wchar_t status[20] = { 0 };

        SQLGetData(hstmt, 1, SQL_C_LONG, &waitlistID, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_WCHAR, passengerName, 100, NULL);
        SQLGetData(hstmt, 3, SQL_C_WCHAR, waitlistDate, 50, NULL);
        SQLGetData(hstmt, 4, SQL_C_LONG, &ticketCount, 0, NULL);
        SQLGetData(hstmt, 5, SQL_C_LONG, &classLevel, 0, NULL);
        SQLGetData(hstmt, 6, SQL_C_WCHAR, status, 20, NULL);

        printf("  %d. ID=%d, 乘客=%ls, 时间=%ls, 票数=%d, 舱位=%d, 状态=%ls\n",
            ++count, waitlistID, passengerName, waitlistDate, ticketCount, classLevel, status);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    // 检查航班价格
    SQLAllocHandle(SQL_HANDLE_STMT, dbConn.hdbc, &hstmt);
    swprintf(query, 1024, L"SELECT EconomyPrice, BusinessPrice, FirstClassPrice FROM Flights WHERE FlightID = ?");
    SQLPrepareW(hstmt, (SQLWCHAR*)query, SQL_NTS);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &flightID, 0, NULL);
    SQLExecute(hstmt);

    float economyPrice = 0, businessPrice = 0, firstClassPrice = 0;
    if (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_FLOAT, &economyPrice, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_FLOAT, &businessPrice, 0, NULL);
        SQLGetData(hstmt, 3, SQL_C_FLOAT, &firstClassPrice, 0, NULL);
        printf("航班价格: 经济舱=%.2f, 商务舱=%.2f, 头等舱=%.2f\n", economyPrice, businessPrice, firstClassPrice);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    printf("=== 详细候补分配调试结束 ===\n");
}