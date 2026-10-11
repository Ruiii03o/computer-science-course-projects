package com.weather.weatherdemo.entity;

import lombok.Data;

@Data
public class WeatherAnalysisVO {
    private String baseSummary;
    private String tempAnalysis;
    private String humidityAnalysis;
    private String windAnalysis;
    private String dressAdvice;
    private String travelAdvice;
    private String healthTip;
}