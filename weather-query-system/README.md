# 天气查询管理系统


## 项目简介

这是一个基于 Spring Boot 开发的天气查询与管理系统。项目调用高德天气 API 获取实时天气数据，并提供城市天气查询、城市对比、天气排行、全国天气浏览，以及用户注册、登录和默认城市管理等功能。

## 主要功能

- 查询指定城市的实时天气；
- 浏览全国城市天气信息；
- 对比多个城市的天气数据；
- 按温度、湿度等维度查看天气排行；
- 用户注册、登录和个人默认城市设置；
- 使用 Thymeleaf 渲染网页界面。

## 技术栈

| 类别 | 使用技术 |
| --- | --- |
| 开发语言 | Java 17 |
| Web 框架 | Spring Boot |
| 模板引擎 | Thymeleaf |
| 数据库 | SQL Server |
| 数据访问 | Spring JDBC / JdbcTemplate |
| 构建工具 | Maven |
| 天气数据 | 高德天气 API |

## 项目结构

```text
weather-query-system/
├─ README.md                 # 项目说明
├─ database-setup.sql        # 数据库建表脚本
└─ weather-demo/
   ├─ pom.xml
   ├─ mvnw / mvnw.cmd
   └─ src/
      └─ main/
         ├─ java/            # 控制器、服务、实体与工具类
         └─ resources/
            ├─ application.properties
            ├─ application-example.properties
            ├─ static/
            └─ templates/
```

## 运行环境

- JDK 17；
- Maven（也可以使用项目自带的 Maven Wrapper）；
- SQL Server 本地实例；
- 高德开放平台 API Key。

## 初始化数据库

在 SQL Server Management Studio（SSMS）中执行仓库根目录的 [database-setup.sql](database-setup.sql)，脚本会创建 `weather_db` 数据库和用户信息表。

## 配置环境变量

请配置以下环境变量：

| 变量名 | 用途 |
| --- | --- |
| `WEATHER_API_KEY` | 高德天气 API Key |
| `DB_USERNAME` | SQL Server 用户名 |
| `DB_PASSWORD` | SQL Server 密码 |



## 运行方式

1. 进入应用目录：

   ```powershell
   cd weather-demo
   ```

2. 配置上述环境变量。
3. 启动项目：

   ```powershell
   .\mvnw.cmd spring-boot:run
   ```

4. 在浏览器打开 `http://localhost:8081`。

