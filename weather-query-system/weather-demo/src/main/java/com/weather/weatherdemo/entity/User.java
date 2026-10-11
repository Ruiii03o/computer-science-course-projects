package com.weather.weatherdemo.entity;

import lombok.Data;

@Data
public class User {
    private Integer id;
    private String username;
    private String password;
    // 用户设置的常驻地，默认是北京
    private String defaultCity = "北京";
}