package com.weather.weatherdemo.util;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

public class DateUtil {

    private static final SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd");

    public static List<String> getFourDayDate() {
        List<String> dateList = new ArrayList<>();
        Calendar calendar = Calendar.getInstance();
        for (int i = 0; i < 4; i++) {
            dateList.add(sdf.format(calendar.getTime()));
            calendar.add(Calendar.DAY_OF_MONTH, 1);
        }
        return dateList;
    }

    public static Integer strToIntTemp(String temp) {
        if (temp == null || temp.isEmpty()) return null;
        try {
            return Integer.parseInt(temp.trim());
        } catch (Exception e) {
            return null;
        }
    }
}