package com.weather.weatherdemo.util;

public class TempLevelUtil {

    public static String getTempFeel(Integer temp) {
        if (temp == null) return "温度未知";
        if (temp <= 0) return "严寒刺骨，极度寒冷";
        else if (temp <= 8) return "天气寒冷，凉意十足";
        else if (temp <= 15) return "气温偏凉，体感清爽";
        else if (temp <= 25) return "温度适宜，体感非常舒适";
        else if (temp <= 30) return "气温偏热，微微燥热";
        else if (temp <= 35) return "天气炎热，体感闷热";
        else return "天气酷热，极易中暑";
    }

    public static boolean isHighTemp(Integer temp) {
        return temp != null && temp >= 30;
    }
}