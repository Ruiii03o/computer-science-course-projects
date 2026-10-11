package com.weather.weatherdemo.controller;

import com.weather.weatherdemo.entity.CityWeatherDTO;
import com.weather.weatherdemo.service.WeatherService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;

import java.util.List;
import java.util.Map;

@Controller
public class ChinaWeatherController {

    @Autowired
    private WeatherService weatherService;

    @GetMapping("/toChinaWeather")
    public String toChinaWeather(Model model) {
        List<CityWeatherDTO> cityList = weatherService.getAllCityWeather();
        List<Map<String, Object>> mapData = weatherService.getMapData();
        List<Integer> pieData = weatherService.getWeatherTypeCount(cityList);

        model.addAttribute("cityList", cityList);
        model.addAttribute("mapData", mapData);
        model.addAttribute("pieData", pieData);

        return "chinaWeather";
    }
}