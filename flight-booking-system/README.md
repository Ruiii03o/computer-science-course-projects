# 航空客运订票系统

## 项目简介

本项目是一个基于 C++ 开发的 Windows 桌面端航空客运订票原型系统。系统使用 Win32 API 构建图形界面，通过 ODBC 访问 SQL Server 数据库，实现航班信息维护、用户登录注册、机票预订、订单管理和候补排队等业务。

项目采用“单系统物理架构 + 前后台逻辑分离”的设计：普通用户侧重航班查询和自助订票，管理员拥有航班、订单和候补信息的管理权限。两个逻辑子系统共享同一套用户认证、数据库连接和航班查询基础模块。

## 课程设计目标

- 综合运用 C/C++ 程序设计、数据库和软件工程相关知识完成原型系统开发；
- 熟悉 Windows 桌面程序、ODBC 与 SQL Server 的协同使用；
- 学习从需求分析、模块设计、编码实现到测试验证的基本开发流程；
- 练习数据库表设计、SQL 查询、参数绑定和异常诊断；
- 提升技术文档编写与课程项目总结能力。

## 主要功能

### 公共功能

- 用户注册与登录；
- 基于角色区分普通用户与管理员权限；
- SQL Server 数据持久化；
- ODBC 错误诊断与控制台日志输出。

### 普通用户功能

- 按航班号、航空公司、出发城市、到达城市和时间等条件查询航班；
- 支持多条件组合筛选与全部航班查询；
- 填写乘客信息后进行直接订票；
- 当余票不足或无余票时，可创建候补订单；
- 查看、取消和刷新个人订单。

### 管理员功能

- 添加、修改、删除航班；
- 查看全量航班、已确认订单和候补订单；
- 手动添加或删除乘客及候补记录；
- 在余票释放后按候补顺序分配座位。

## 核心业务逻辑

### 候补队列

当航班余票不足时，系统将未满足的票数记录为候补订单。取消订单、管理员调整余票或增加可用座位后，系统按照候补订单的登记顺序处理队列；可分配的部分将生成正式订单，剩余部分保留为候补状态。

### 数据一致性

订票、取消订单和候补分配等操作会同步更新订单、乘客信息、候补记录与航班余票。查询和写入使用 ODBC 参数绑定，避免直接拼接用户输入作为 SQL 条件。

## 技术栈

| 类别 | 使用技术 |
| --- | --- |
| 开发语言 | C++ |
| 桌面界面 | Win32 API |
| 数据库 | SQL Server |
| 数据访问 | ODBC |
| 开发工具 | Visual Studio 2022（或兼容版本） |

## 数据库设计

系统包含以下五张核心表：

| 数据表 | 用途 |
| --- | --- |
| `Users` | 存储用户名、密码、角色和注册信息 |
| `Flights` | 存储航班、座位和不同舱位票价 |
| `Bookings` | 存储已确认的预订订单 |
| `PassengerDetails` | 存储订单对应的乘客详细信息 |
| `WaitlistBookings` | 存储候补申请、状态和登记时间 |

各表通过用户 ID、航班 ID 和订单 ID 建立关联关系。详细建表语句见 [database-setup.sql](database-setup.sql)。脚本只创建表结构，不包含真实用户、订单或乘客信息。

## 运行环境

- Windows 10/11；
- Visual Studio 2022，安装“使用 C++ 的桌面开发”工作负载；
- SQL Server 本地实例；
- SQL Server ODBC 驱动；
- SQL Server Management Studio（SSMS，推荐，用于执行数据库脚本）。

## 运行步骤

1. 在 SSMS 中打开并执行 [database-setup.sql](database-setup.sql)。
2. 确认当前 Windows 用户有权访问本机 SQL Server 中的 `FlightBookingSystem` 数据库。
3. 使用 Visual Studio 打开 `FlightBookingSystem/FlightBookingSystem.sln`。
4. 选择 `x64` 和 `Debug` 配置，生成并运行项目。
5. 注册测试用户后即可登录并使用普通用户功能。
6. 如需测试管理员功能，可在**本机测试数据库**中把测试用户的 `UserType` 更新为 `admin`。

项目默认使用 Windows 身份验证连接本地数据库：

```text
DRIVER={SQL Server};SERVER=localhost;DATABASE=FlightBookingSystem;Trusted_Connection=yes;
```

该连接字符串位于 `FlightBookingSystem/Database.cpp`。

## 项目结构

```text
flight-booking-system/
├─ README.md                 # 项目说明
├─ database-setup.sql        # 数据库建表脚本
└─ FlightBookingSystem/
   ├─ FlightBookingSystem.sln
   ├─ FlightBookingSystem.cpp # 程序入口与窗口消息处理
   ├─ UI.cpp                  # 界面与交互逻辑
   ├─ Database.cpp            # ODBC 连接与数据库操作
   └─ FlightBookingSystem.h   # 数据结构与函数声明
```

## 已知限制与后续改进

这是课程设计原型，仍有完善空间：

- 当前用户密码处理仅用于课程演示，后续应改为加盐哈希存储；
- 可为订票、退票和候补分配补充更完整的数据库事务与回滚逻辑；
- 可增加操作日志、输入校验、分页查询和数据备份；
- 可扩展支付、通知、统计分析，以及 Web 或移动端界面；
- 可为核心业务补充自动化测试。


