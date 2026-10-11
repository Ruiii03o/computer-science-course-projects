package com.weather.weatherdemo.controller;

import com.weather.weatherdemo.entity.WeatherAnalysisVO;
import com.weather.weatherdemo.entity.WeatherInfo;
import com.weather.weatherdemo.service.WeatherAnalysisService;
import com.weather.weatherdemo.service.WeatherService;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;

import java.util.List;
import java.util.Map;

@Controller
public class WeatherQueryController {

    private final WeatherService weatherService;
    private final WeatherAnalysisService analysisService;

    public WeatherQueryController(WeatherService weatherService, WeatherAnalysisService analysisService) {
        this.weatherService = weatherService;
        this.analysisService = analysisService;
    }

    @GetMapping("/searchWeather")
    public String searchWeather(@RequestParam String city, Model model){
        // 1. 获取基础天气数据
        WeatherInfo weatherInfo = weatherService.getFullWeather(city);
        List<Map<String,String>> hourlyList = weatherService.getHourlyForecast(city);

        // 2. 【关键！】生成分段分析报告
        List<String> reportLines = weatherService.getWeatherReportLines(weatherInfo);

        // 3. 把所有数据都加到Model里，传给页面
        model.addAttribute("city", city);
        model.addAttribute("weather", weatherInfo);
        model.addAttribute("hourlyList", hourlyList);

        model.addAttribute("reportLine1", reportLines.get(0));
        model.addAttribute("reportLine2", reportLines.get(1));
        model.addAttribute("reportLine3", reportLines.get(2));
        model.addAttribute("reportLine4", reportLines.get(3));

        model.addAttribute("warmTip", weatherService.getWarmTip(weatherInfo));
        model.addAttribute("travelAdvice", weatherService.getTravelAdvice(weatherInfo));
        model.addAttribute("clothingAdvice", weatherService.getClothingAdvice(weatherInfo));


        return "weatherQuery";
    }
}