package com.weather.weatherdemo.controller;

import com.weather.weatherdemo.entity.WeatherRankDTO;
import com.weather.weatherdemo.service.WeatherRankService;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;

import java.util.List;
import java.util.Map;

@Controller
@RequiredArgsConstructor
public class WeatherRankController {

    // 规范注入（比@Autowired更标准）
    private final WeatherRankService weatherRankService;

    // 数据加载 + 页面渲染
    @GetMapping("/toRank")
    public String loadWeatherRankData(Model model) {
        System.out.println("=== 正在加载全国市县天气排行数据 ===");

        Map<String, List<WeatherRankDTO>> rankData = weatherRankService.getCityRankData();

        model.addAttribute("highList", rankData.get("highList"));
        model.addAttribute("lowList", rankData.get("lowList"));
        model.addAttribute("extremeList", rankData.get("extremeList"));

        return "weatherRank";
    }
}