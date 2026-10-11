package com.weather.weatherdemo.entity;

import lombok.Data;
import java.util.ArrayList;
import java.util.List;

@Data
public class WeatherInfo {
    // 实时天气
    private String city;
    private String temperature;
    private String weather;
    private String windDir;
    private String windPower;
    private String humidity;

    // 空气质量
    private String aqi;
    private String aqiDesc;
   // 实时温度纯数字
    private Integer tempNum;
    // 天气分类标签
    private String weatherTag;
    // 数据发布时间
    private String reportTime;


    // 未来预报集合
    private List<DayForecast> forecastList = new ArrayList<>();

    // 单日预报内部类
    @Data
    public static class DayForecast{
        private String date;
        private String week;
        private String dayWeather;
        private String nightWeather;
        private String dayTemp;
        private String nightTemp;
    }
}