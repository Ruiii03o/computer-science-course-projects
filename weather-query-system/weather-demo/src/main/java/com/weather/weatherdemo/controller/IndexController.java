package com.weather.weatherdemo.controller;

import com.alibaba.fastjson2.JSONObject;
import com.weather.weatherdemo.entity.User;
import com.weather.weatherdemo.service.WeatherRankService;
import com.weather.weatherdemo.service.WeatherService;
import jakarta.servlet.http.HttpSession;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;

@Controller
public class IndexController {

    @Autowired
    private WeatherRankService weatherRankService;

    // 天气查询服务
    @Autowired
    private WeatherService weatherService;

    // 首页：添加默认天气 + 用户登录判断
    @GetMapping("/")
    public String goIndex(HttpSession session, Model model){
        // 1. 获取登录用户
        User loginUser = (User) session.getAttribute("loginUser");
        String defaultCity = "北京";

        // 2. 如果登录，用用户设置的城市
        if (loginUser != null) {
            defaultCity = loginUser.getDefaultCity();
        }

        // 3. 查询默认城市天气
        JSONObject defaultWeather = weatherService.getLiveWeather(defaultCity);

        // 4. 传给页面
        model.addAttribute("loginUser", loginUser);
        model.addAttribute("defaultWeather", defaultWeather);

        return "index";
    }

    @GetMapping("/toQuery")
    public String toWeatherQuery(){
        return "weatherQuery";
    }

    @GetMapping("/toCompare")
    public String toCityCompare(){
        return "cityCompare";
    }

}