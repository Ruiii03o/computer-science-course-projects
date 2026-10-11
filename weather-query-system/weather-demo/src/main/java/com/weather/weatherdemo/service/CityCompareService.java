package com.weather.weatherdemo.service;

import com.alibaba.fastjson2.JSONArray;
import com.alibaba.fastjson2.JSONObject;
import com.weather.weatherdemo.entity.CompareChartDTO;
import org.springframework.stereotype.Service;
import java.time.LocalDate;
import java.util.*;

@Service
public class CityCompareService {

    private final WeatherService weatherService;

    public CityCompareService(WeatherService weatherService) {
        this.weatherService = weatherService;
    }

    public CompareChartDTO buildCompareData(List<String> cityNames) {
        CompareChartDTO dto = new CompareChartDTO();

        List<String> validCityNames = new ArrayList<>();
        List<List<Integer>> allTempData = new ArrayList<>();
        List<String> dateList = new ArrayList<>();

        List<String> todayTempList = new ArrayList<>();
        List<String> todayWeatherList = new ArrayList<>();
        List<String> todayWindList = new ArrayList<>();

        for (int i = 0; i < 4; i++) {
            dateList.add(LocalDate.now().plusDays(i).toString());
        }

        // 按输入顺序遍历，不排序
        for (String city : cityNames) {
            JSONArray casts = null;

            for (int retry = 0; retry < 2; retry++) {
                try {
                    casts = weatherService.getForecastWeather(city);
                    if (casts != null && casts.size() >= 1) break;
                    Thread.sleep(200);
                } catch (Exception e) {}
            }

            if (casts == null || casts.size() == 0) continue;

            List<Integer> tempList = new ArrayList<>();
            boolean tempOk = true;
            for (int i = 0; i < Math.min(4, casts.size()); i++) {
                try {
                    JSONObject obj = casts.getJSONObject(i);
                    tempList.add(Integer.parseInt(obj.getString("daytemp")));
                } catch (Exception e) {
                    tempOk = false;
                    break;
                }
            }
            if (!tempOk) continue;

            try {
                JSONObject today = casts.getJSONObject(0);

                String dayTemp = today.getString("daytemp");
                String nightTemp = today.getString("nighttemp");
                String weather = today.getString("dayweather");
                String windDir = today.getString("daywind");
                String windPower = today.getString("daypower");

                // 温度格式：低温 - 高温 ℃  例如 18-26℃
                int t1 = Integer.parseInt(nightTemp);
                int t2 = Integer.parseInt(dayTemp);
                String tempStr = Math.min(t1, t2) + "-" + Math.max(t1, t2) + "℃";

                String windStr = "";
                if (windDir != null && windPower != null) {
                    windStr = windDir + " " + windPower + "级";
                }

                todayTempList.add(tempStr);
                todayWeatherList.add(weather);
                todayWindList.add(windStr);

            } catch (Exception e) {
                todayTempList.add("--℃");
                todayWeatherList.add("");
                todayWindList.add("");
            }

            validCityNames.add(city);
            allTempData.add(tempList);
        }

        dto.setXDateList(dateList);
        dto.setValidCityNames(validCityNames);
        dto.setAllTempData(allTempData);
        dto.setTodayTemp(todayTempList);
        dto.setTodayWeather(todayWeatherList);
        dto.setTodayWind(todayWindList);

        return dto;
    }
}