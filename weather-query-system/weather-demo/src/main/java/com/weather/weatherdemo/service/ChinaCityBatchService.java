package com.weather.weatherdemo.service;

import com.alibaba.fastjson2.JSONObject;
import com.weather.weatherdemo.entity.CityWeatherDTO;
import org.springframework.stereotype.Service;
import java.util.*;
import java.util.concurrent.*;

@Service
public class ChinaCityBatchService {

    private final WeatherService weatherService;

    public ChinaCityBatchService(WeatherService weatherService) {
        this.weatherService = weatherService;
    }

    // 缓存：30分钟内不再请求接口，秒开
    private final Map<String, CacheItem> cache = new ConcurrentHashMap<>();
    private static final long CACHE_TTL = 30 * 60 * 1000;

    private static final List<String> PROVINCE_CITY = List.of(
            "北京","天津","上海","重庆",
            "石家庄","太原","沈阳","长春","哈尔滨",
            "南京","杭州","合肥","福州","南昌","济南",
            "郑州","武汉","长沙","广州","南宁","海口",
            "成都","贵阳","昆明","拉萨","西安","兰州",
            "西宁","银川","乌鲁木齐","呼和浩特"
    );

    // 工具方法：字符串温度转数字
    private Integer strToIntTemp(String tempStr) {
        if (tempStr == null || tempStr.isEmpty()) return null;
        String num = tempStr.replaceAll("[^0-9]", "");
        try {
            return Integer.parseInt(num);
        } catch (Exception e) {
            return null;
        }
    }

    public List<CityWeatherDTO> getAllProvinceWeather() {
        List<CityWeatherDTO> result = new ArrayList<>();

        // 多线程
        ExecutorService executor = Executors.newFixedThreadPool(10);
        List<Future<CityWeatherDTO>> futures = new ArrayList<>();

        for (String city : PROVINCE_CITY) {
            futures.add(executor.submit(() -> getOneCityWeather(city)));
        }

        // 收集结果
        for (Future<CityWeatherDTO> future : futures) {
            try {
                CityWeatherDTO dto = future.get(2, TimeUnit.SECONDS);
                if (dto != null) result.add(dto);
            } catch (Exception ignored) {}
        }

        executor.shutdown();
        return result;
    }

    // 单个城市查询（带缓存）
    private CityWeatherDTO getOneCityWeather(String city) {
        // 先读缓存
        CacheItem item = cache.get(city);
        if (item != null && System.currentTimeMillis() - item.time < CACHE_TTL) {
            return item.dto;
        }

        try {
            JSONObject live = weatherService.getLiveWeather(city);
            if (live == null) return null;

            CityWeatherDTO dto = new CityWeatherDTO();
            dto.setCityName(city);
            dto.setTemperature(live.getString("temperature"));
            dto.setWeatherDesc(live.getString("weather"));
            dto.setHumidity(live.getString("humidity"));
            dto.setWindInfo(live.getString("winddirection") + " " + live.getString("windpower"));

            Integer tempNum = strToIntTemp(live.getString("temperature"));
            dto.setTempNum(tempNum == null ? 20 : tempNum);

            int weatherType = weatherService.getWeatherTypeByCode(live.getString("weather"), dto.getTempNum());
            dto.setWeatherType(weatherType);
            dto.setWeatherTagName(weatherService.getWeatherTypeName(weatherType));

            // 放入缓存
            cache.put(city, new CacheItem(dto, System.currentTimeMillis()));
            return dto;

        } catch (Exception e) {
            return null;
        }
    }

    // 统计饼图数据（不用改）
    public List<Integer> getWeatherTypeCount(List<CityWeatherDTO> list) {
        int rain = 0, hot = 0, comfort = 0;
        for (CityWeatherDTO dto : list) {
            if (dto.getWeatherType() == 0) rain++;
            else if (dto.getWeatherType() == 1) hot++;
            else comfort++;
        }
        return List.of(rain, hot, comfort);
    }

    // 缓存实体
    private static class CacheItem {
        CityWeatherDTO dto;
        long time;
        CacheItem(CityWeatherDTO dto, long time) {
            this.dto = dto;
            this.time = time;
        }
    }
}