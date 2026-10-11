package com.weather.weatherdemo.util;

public class HumidityUtil {

    public static String getHumidityDesc(String humidityStr) {
        Integer humidity = DateUtil.strToIntTemp(humidityStr);
        if (humidity == null) return "湿度未知";
        if (humidity <= 30) return "空气十分干燥，容易口干皮肤干涩";
        else if (humidity <= 50) return "空气偏干，日常舒适";
        else if (humidity <= 70) return "湿度适中，人体体感最佳";
        else if (humidity <= 85) return "空气偏潮湿，略显沉闷";
        else return "空气潮湿闷热，体感压抑易不适";
    }
}