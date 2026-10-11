/*
  Weather Query System database setup
  Creates the SQL Server database and table used by UserRepository.
  This script contains no real user accounts or passwords.
*/

IF DB_ID(N'weather_db') IS NULL
BEGIN
    CREATE DATABASE weather_db;
END;
GO

USE weather_db;
GO

IF OBJECT_ID(N'dbo.[user]', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.[user] (
        id           INT IDENTITY(1,1) NOT NULL PRIMARY KEY,
        username     NVARCHAR(50) NOT NULL,
        [password]   NVARCHAR(255) NOT NULL,
        default_city NVARCHAR(100) NOT NULL CONSTRAINT DF_User_DefaultCity DEFAULT N'北京',
        CONSTRAINT UQ_User_Username UNIQUE (username)
    );
END;
GO
