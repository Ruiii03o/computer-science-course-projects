package com.weather.weatherdemo.service;

import com.alibaba.fastjson2.JSONArray;
import com.alibaba.fastjson2.JSONObject;
import com.weather.weatherdemo.entity.WeatherRankDTO;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;
import org.springframework.web.client.ResourceAccessException;
import org.springframework.web.client.RestTemplate;

import java.util.*;
import java.util.concurrent.*;
import java.util.stream.Collectors;

@Service
@Slf4j
public class WeatherRankService {

    private final RestTemplate restTemplate;

    private static final String AMAP_KEY = "43210d62123af7865bdacb289230f412";
    private static final String WEATHER_API =
            "https://restapi.amap.com/v3/weather/weatherInfo?city={city}&key=" + AMAP_KEY + "&extensions=base";

    private final Map<String, CacheItem> cache = new ConcurrentHashMap<>();
    private static final long CACHE_TTL = 30 * 60 * 1000;

    public WeatherRankService(RestTemplate restTemplate) {
        this.restTemplate = restTemplate;
    }

    public Map<String, List<WeatherRankDTO>> getCityRankData() {
        List<String> cityCodeList = Arrays.asList(
                "110000","120000","310000","500100",
                "130100","130200","130300","130400","130500","130600","130800","130900","131000","131100",
                "140100","140200","140300","140400","140500","140600","140700","140800","140900","141000","141100",
                "150100","150200","150300","150400","150500","150600","150700","150800","150900","151000","151100","151200",
                "210100","210200","210300","210400","210500","210600","210700","210800","210900","211000","211100","211200","211300","211400",
                "220100","220200","220300","220400","220500","220600","220700","220800","220900",
                "230100","230200","230300","230400","230500","230600","230700","230800","230900","231000","231100","231200","232700",
                "320100","320200","320300","320400","320500","320600","320700","320800","320900","321000","321100","321200","321300",
                "330100","330200","330300","330400","330500","330600","330700","330800","330900","331000","331100",
                "340100","340200","340300","340400","340500","340600","340700","340800","340900","341000","341100","341200","341300","341500","341600","341700","341800",
                "350100","350200","350300","350400","350500","350600","350700","350800","350900",
                "360100","360200","360300","360400","360500","360600","360700","360800","360900","361000","361100",
                "370100","370200","370300","370400","370500","370600","370700","370800","370900","371000","371100","371300","371400","371500","371600","371700",
                "410100","410200","410300","410400","410500","410600","410700","410800","410900","411000","411100","411200","411300","411400","411500","411600","411700",
                "420100","420200","420300","420400","420500","420600","420700","420800","420900","421000","421100","421200","421300","422800",
                "430100","430200","430300","430400","430500","430600","430700","430800","430900","431000","431100","431200","431300","433100",
                "440100","440200","440300","440400","440500","440600","440700","440800","440900","441000","441100","441200","441300","441400","441500","441600","441700","441800","441900","442000","445100","445200","445300",
                "450100","450200","450300","450400","450500","450600","450700","450800","450900","451000","451100","451200","451300","451400",
                "460100","460200","460300","460400","460500",
                "510100","510300","510400","510500","510600","510700","510800","510900","511000","511100","511300","511400","511500","511600","511700","511800","511900","512000","513200","513300","513400",
                "520100","520200","520300","520400","520500","520600","522300","522600","522700",
                "530100","530200","530300","530400","530500","530600","530700","530800","530900","532300","532500","532600","532800","532900","533100","533300","533400",
                "540100","540200","540300","540400","540500","540600","540700","542500","542600",
                "610100","610200","610300","610400","610500","610600","610700","610800","610900","611000",
                "620100","620200","620300","620400","620500","620600","620700","620800","620900","621000","621100","621200","622900","623000",
                "630100","630200","630300","630400","630500","630600","630700","632200","632300","632500","632600","632700","632800",
                "640100","640200","640300","640400","640500",
                "650100","650200","650300","650400","650500","650600","650700","650800","650900","651000","652200","652300","652700","652800","652900","653000","653100","653200","654000"
        );

        ExecutorService pool = Executors.newFixedThreadPool(3);
        List<Future<WeatherRankDTO>> futures = new ArrayList<>();

        for (String cityCode : cityCodeList) {
            futures.add(pool.submit(() -> fetchOneCity(cityCode)));
        }

        List<WeatherRankDTO> cityWeatherList = new ArrayList<>();
        for (Future<WeatherRankDTO> f : futures) {
            try {
                WeatherRankDTO dto = f.get(8, TimeUnit.SECONDS);
                if (dto != null) {
                    cityWeatherList.add(dto);
                }
            } catch (Exception ignored) {
                // 所有错误都静默处理，不打印日志
            }
        }
        pool.shutdown();

        log.info("成功获取天气城市数：{}", cityWeatherList.size());

        List<WeatherRankDTO> listForHigh = new ArrayList<>(cityWeatherList);
        List<WeatherRankDTO> listForLow = new ArrayList<>(cityWeatherList);

        List<WeatherRankDTO> highTop30 = listForHigh.stream()
                .sorted((a, b) -> Integer.compare(b.getTemperature(), a.getTemperature()))
                .limit(30)
                .collect(Collectors.toList());

        List<WeatherRankDTO> lowBottom30 = listForLow.stream()
                .sorted(Comparator.comparingInt(WeatherRankDTO::getTemperature))
                .limit(30)
                .collect(Collectors.toList());

        List<WeatherRankDTO> extremeList = cityWeatherList.stream()
                .filter(dto -> "extreme".equals(dto.getWeatherType()))
                .collect(Collectors.toList());

        Map<String, List<WeatherRankDTO>> result = new HashMap<>();
        result.put("highList", highTop30);
        result.put("lowList", lowBottom30);
        result.put("extremeList", extremeList);
        return result;
    }

    private WeatherRankDTO fetchOneCity(String cityCode) {
        try {
            CacheItem item = cache.get(cityCode);
            if (item != null && item.dto != null && System.currentTimeMillis() - item.time < CACHE_TTL) {
                return item.dto;
            }

            Thread.sleep(300);

            JSONObject response = restTemplate.getForObject(WEATHER_API, JSONObject.class, cityCode);

            if (response == null || !"1".equals(response.getString("status"))) {
                return null;
            }

            JSONArray lives = response.getJSONArray("lives");
            if (lives == null || lives.isEmpty()) {
                return null;
            }

            JSONObject live = lives.getJSONObject(0);
            if (live == null) {
                return null;
            }

            String cityName = live.getString("city");
            int temp = 20;
            try {
                temp = Integer.parseInt(live.getString("temperature"));
            } catch (Exception ignored) {}

            String weather = live.getString("weather");
            String type = judgeWeatherType(weather);

            WeatherRankDTO dto = new WeatherRankDTO(cityName, temp, weather, type);
            cache.put(cityCode, new CacheItem(dto, System.currentTimeMillis()));
            return dto;

        } catch (Exception ignored) {
            // 所有错误都不打印日志
            return null;
        }
    }

    private String judgeWeatherType(String weather) {
        if (weather == null) return "normal";
        if (weather.contains("暴雨") || weather.contains("大雨") ||
                weather.contains("暴雪") || weather.contains("冰雹") ||
                weather.contains("台风") || weather.contains("雷暴")) {
            return "extreme";
        }
        return "normal";
    }

    private static class CacheItem {
        WeatherRankDTO dto;
        long time;
        CacheItem(WeatherRankDTO dto, long time) {
            this.dto = dto;
            this.time = time;
        }
    }

    public Map<String, List<WeatherRankDTO>> getRankData() {
        return getCityRankData();
    }
}