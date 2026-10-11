package com.weather.weatherdemo.entity;

import lombok.AllArgsConstructor;
import lombok.Data;

@Data
@AllArgsConstructor
public class WeatherRankDTO {
    private String cityName;
    private int temperature;
    private String weatherDesc;
    private String weatherType; // normal / extreme
}