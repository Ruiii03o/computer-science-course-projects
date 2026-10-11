package com.weather.weatherdemo.entity;

import lombok.Data;
import java.util.List;

@Data
public class CompareChartDTO {
    private List<String> xDateList;
    private List<String> validCityNames;
    private List<List<Integer>> allTempData;

    // 今日气温卡片
    private List<String> todayTemp;
    private List<String> todayWeather;
    private List<String> todayWind;
}