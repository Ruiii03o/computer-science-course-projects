package com.weather.weatherdemo.controller;

import com.weather.weatherdemo.entity.CompareChartDTO;
import com.weather.weatherdemo.service.CityCompareService;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

@Controller
public class CityCompareController {

    private final CityCompareService compareService;

    public CityCompareController(CityCompareService compareService) {
        this.compareService = compareService;
    }

    @GetMapping("/compareCity")
    public String compareCity(@RequestParam String cityNames, Model model){
        // 分割多个城市名
        List<String> cityList = new ArrayList<>(Arrays.asList(cityNames.split(",")));
        CompareChartDTO chartData = compareService.buildCompareData(cityList);
        model.addAttribute("chartData",chartData);
        model.addAttribute("inputCities",cityNames);
        return "cityCompare";
    }
}