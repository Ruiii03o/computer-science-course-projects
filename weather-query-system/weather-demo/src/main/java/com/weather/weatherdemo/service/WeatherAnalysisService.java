package com.weather.weatherdemo.service;

import com.weather.weatherdemo.entity.WeatherAnalysisVO;
import com.weather.weatherdemo.entity.WeatherInfo;
import com.weather.weatherdemo.util.HumidityUtil;
import com.weather.weatherdemo.util.TempLevelUtil;
import com.weather.weatherdemo.util.WeatherClassUtil;
import com.weather.weatherdemo.util.WindLevelUtil;
import org.springframework.stereotype.Service;

@Service
public class WeatherAnalysisService {

    private final WeatherService weatherService;

    public WeatherAnalysisService(WeatherService weatherService) {
        this.weatherService = weatherService;
    }

    public WeatherAnalysisVO buildAnalysis(WeatherInfo info) {
        WeatherAnalysisVO vo = new WeatherAnalysisVO();
        String weather = info.getWeather() == null ? "未知天气" : info.getWeather();
        Integer temp = info.getTempNum();
        String humidity = info.getHumidity();
        String windDir = info.getWindDir();
        String windPower = info.getWindPower();

        vo.setBaseSummary("当前查询地区天气状况为" + weather + "，整体天气趋势平稳，数据实时更新。");
        vo.setTempAnalysis("当前实时气温为" + temp + "℃，" + TempLevelUtil.getTempFeel(temp));
        vo.setHumidityAnalysis(HumidityUtil.getHumidityDesc(humidity));
        vo.setWindAnalysis(WindLevelUtil.getWindAnalysis(windPower, windDir));
        vo.setDressAdvice(weatherService.getClothingAdvice(info));
        vo.setTravelAdvice(weatherService.getTravelAdvice(info));

        if(WeatherClassUtil.isRainSnow(weather)){
            vo.setHealthTip("雨雪天气路面湿滑，注意防滑，及时增添衣物预防感冒。");
        }else if(TempLevelUtil.isHighTemp(temp)){
            vo.setHealthTip("高温天气多补充温水，减少暴晒，谨防中暑。");
        }else{
            vo.setHealthTip("天气舒适，适合日常锻炼与外出活动，保持规律作息即可。");
        }
        return vo;
    }
}