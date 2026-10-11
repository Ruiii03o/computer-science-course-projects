/*
  Flight Booking System database setup
  Creates the schema required by the C++ application.
  This script deliberately contains no real users, passwords, orders, or passenger data.
*/

IF DB_ID(N'FlightBookingSystem') IS NULL
BEGIN
    CREATE DATABASE FlightBookingSystem;
END;
GO

USE FlightBookingSystem;
GO

IF OBJECT_ID(N'dbo.Users', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.Users (
        UserID       INT IDENTITY(1,1) NOT NULL PRIMARY KEY,
        Username     NVARCHAR(50) NOT NULL UNIQUE,
        [Password]   NVARCHAR(100) NOT NULL,
        UserType     NVARCHAR(20) NOT NULL CONSTRAINT DF_Users_UserType DEFAULT N'customer',
        FullName     NVARCHAR(100) NOT NULL,
        RegisterDate DATETIME NOT NULL CONSTRAINT DF_Users_RegisterDate DEFAULT GETDATE()
    );
END;
GO

IF OBJECT_ID(N'dbo.Flights', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.Flights (
        FlightID       INT IDENTITY(1,1) NOT NULL PRIMARY KEY,
        FlightNumber   NVARCHAR(20) NOT NULL UNIQUE,
        Airline        NVARCHAR(100) NOT NULL,
        DepartureCity  NVARCHAR(100) NOT NULL,
        ArrivalCity    NVARCHAR(100) NOT NULL,
        DepartureTime  DATETIME NOT NULL,
        ArrivalTime    DATETIME NOT NULL,
        TotalSeats     INT NOT NULL,
        AvailableSeats INT NOT NULL,
        EconomyPrice   DECIMAL(10, 2) NOT NULL,
        BusinessPrice  DECIMAL(10, 2) NOT NULL,
        FirstClassPrice DECIMAL(10, 2) NOT NULL,
        FlightDay      NVARCHAR(50) NULL,
        PlaneModel     NVARCHAR(100) NULL,
        CONSTRAINT CK_Flights_SeatCounts CHECK (TotalSeats >= 0 AND AvailableSeats >= 0 AND AvailableSeats <= TotalSeats)
    );
END;
GO

IF OBJECT_ID(N'dbo.Bookings', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.Bookings (
        BookingID      INT IDENTITY(1,1) NOT NULL PRIMARY KEY,
        UserID         INT NOT NULL,
        FlightID       INT NOT NULL,
        PassengerName  NVARCHAR(100) NOT NULL,
        PassengerCount INT NOT NULL,
        ClassLevel     INT NOT NULL,
        TotalAmount    DECIMAL(10, 2) NOT NULL,
        BookingDate    DATETIME NOT NULL CONSTRAINT DF_Bookings_BookingDate DEFAULT GETDATE(),
        Status         NVARCHAR(30) NOT NULL CONSTRAINT DF_Bookings_Status DEFAULT N'confirmed',
        CONSTRAINT CK_Bookings_PassengerCount CHECK (PassengerCount > 0),
        CONSTRAINT FK_Bookings_Users FOREIGN KEY (UserID) REFERENCES dbo.Users(UserID),
        CONSTRAINT FK_Bookings_Flights FOREIGN KEY (FlightID) REFERENCES dbo.Flights(FlightID)
    );
END;
GO

IF OBJECT_ID(N'dbo.PassengerDetails', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.PassengerDetails (
        PassengerDetailID INT IDENTITY(1,1) NOT NULL PRIMARY KEY,
        BookingID         INT NOT NULL,
        IDCardNumber      NVARCHAR(30) NOT NULL,
        Gender            NVARCHAR(10) NULL,
        Phone             NVARCHAR(30) NULL,
        Email             NVARCHAR(100) NULL,
        CONSTRAINT FK_PassengerDetails_Bookings FOREIGN KEY (BookingID) REFERENCES dbo.Bookings(BookingID)
    );
END;
GO

IF OBJECT_ID(N'dbo.WaitlistBookings', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.WaitlistBookings (
        WaitlistID    INT IDENTITY(1,1) NOT NULL PRIMARY KEY,
        UserID        INT NOT NULL,
        FlightID      INT NOT NULL,
        PassengerName NVARCHAR(100) NOT NULL,
        IDCardNumber  NVARCHAR(30) NOT NULL,
        Gender        NVARCHAR(10) NULL,
        TicketCount   INT NOT NULL,
        ClassLevel    INT NOT NULL,
        WaitlistDate  DATETIME NOT NULL CONSTRAINT DF_WaitlistBookings_WaitlistDate DEFAULT GETDATE(),
        Status        NVARCHAR(30) NOT NULL CONSTRAINT DF_WaitlistBookings_Status DEFAULT N'waiting',
        CONSTRAINT CK_WaitlistBookings_TicketCount CHECK (TicketCount > 0),
        CONSTRAINT FK_WaitlistBookings_Users FOREIGN KEY (UserID) REFERENCES dbo.Users(UserID),
        CONSTRAINT FK_WaitlistBookings_Flights FOREIGN KEY (FlightID) REFERENCES dbo.Flights(FlightID)
    );
END;
GO

/*
  To test administrator-only functions locally:
  1. Start the application and register a test user.
  2. Run the following statement after replacing the placeholder:

     UPDATE dbo.Users SET UserType = N'admin' WHERE Username = N'YOUR_TEST_USERNAME';
*/
