package com.weather.weatherdemo.util;

public class WindLevelUtil {

    public static String getWindAnalysis(String windPower, String windDir) {
        if (windPower == null || windDir == null) {
            return "暂无风向风力数据，出行无明显风力影响";
        }
        String res = "当前主导风向为" + windDir + "风，风力等级" + windPower + "级";
        if (windPower.contains("0") || windPower.contains("1")) {
            res += "，几乎无风，出行、散步、户外活动不受任何影响";
        } else if (windPower.contains("2") || windPower.contains("3")) {
            res += "，微风拂面，体感舒适，十分适合外出游玩";
        } else if (windPower.contains("4") || windPower.contains("5")) {
            res += "，风力偏大，外出注意防风，轻薄衣物易被吹乱";
        } else {
            res += "，风力较大，尽量减少长时间户外停留，注意出行安全";
        }
        return res;
    }
}