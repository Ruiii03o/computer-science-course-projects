package com.weather.weatherdemo.util;

public class WeatherClassUtil {

    public static String getWeatherTag(String weather, Integer temp) {
        if (weather == null) return "常规天气";
        if (weather.contains("雨") || weather.contains("雪") || weather.contains("雷")) {
            return "雨雪天气";
        }
        if (TempLevelUtil.isHighTemp(temp)) {
            return "高温天气";
        }
        return "舒适晴好天气";
    }

    public static boolean isRainSnow(String weather) {
        if (weather == null) return false;
        return weather.contains("雨") || weather.contains("雪") || weather.contains("雷");
    }
}