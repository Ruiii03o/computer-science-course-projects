package com.weather.weatherdemo.service;

import com.alibaba.fastjson2.JSONArray;
import com.alibaba.fastjson2.JSONObject;
import com.weather.weatherdemo.entity.CityWeatherDTO;
import com.weather.weatherdemo.entity.WeatherInfo;
import com.weather.weatherdemo.util.DateUtil;
import com.weather.weatherdemo.util.WeatherClassUtil;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;

import java.util.*;
import java.util.stream.Collectors;

@Service
public class WeatherService {

    @Value("${weather.api.key}")
    private String apiKey;

    private final RestTemplate restTemplate = new RestTemplate();

    private Integer getPureTemp(String tempStr) {
        if (tempStr == null || tempStr.isEmpty()) return 20;
        String num = tempStr.replaceAll("[^0-9]", "");
        try {
            return Integer.parseInt(num);
        } catch (Exception e) {
            return 20;
        }
    }

    public JSONObject getLiveWeather(String city) {
        try {
            String url = "https://restapi.amap.com/v3/weather/weatherInfo?extensions=base&city=" + city + "&key=" + apiKey;
            String json = restTemplate.getForObject(url, String.class);
            JSONObject root = JSONObject.parseObject(json);
            if (!"1".equals(root.getString("status"))) return null;
            JSONArray lives = root.getJSONArray("lives");
            if (lives != null && !lives.isEmpty()) {
                return lives.getJSONObject(0);
            }
        } catch (Exception e) {
        }
        return null;
    }

    // ========== 2. 4天预报接口（forecasts） ==========
    public JSONArray getForecastWeather(String city) {
        // 重试 3 次，避免一次性请求失败
        for (int i = 0; i < 3; i++) {
            try {
                String url = "https://restapi.amap.com/v3/weather/weatherInfo?extensions=all&city=" + city + "&key=" + apiKey;
                String json = restTemplate.getForObject(url, String.class);
                JSONObject root = JSONObject.parseObject(json);
                if (!"1".equals(root.getString("status"))) {
                    Thread.sleep(300); // 失败后延迟 300ms 再重试
                    continue;
                }
                JSONArray forecasts = root.getJSONArray("forecasts");
                if (forecasts != null && !forecasts.isEmpty()) {
                    return forecasts.getJSONObject(0).getJSONArray("casts");
                }
            } catch (Exception e) {
                try {
                    Thread.sleep(300);
                } catch (InterruptedException ex) {
                    ex.printStackTrace();
                }
            }
        }
        return null; // 3 次都失败，返回 null，后续会补全默认数据
    }

    // ========== 3. 整合数据 ==========
    public WeatherInfo getFullWeather(String city) {
        WeatherInfo info = new WeatherInfo();
        JSONObject live = getLiveWeather(city);
        JSONArray casts = getForecastWeather(city);

        if (live != null) {
            info.setCity(live.getString("city"));
            info.setTemperature(live.getString("temperature") == null ? "0" : live.getString("temperature").replace("℃", ""));
            info.setWeather(live.getString("weather"));
            info.setWindDir(live.getString("winddirection"));
            info.setWindPower(live.getString("windpower"));
            info.setHumidity(live.getString("humidity"));

            Integer nowTemp = DateUtil.strToIntTemp(live.getString("temperature"));
            info.setTempNum(nowTemp);
            info.setWeatherTag(WeatherClassUtil.getWeatherTag(info.getWeather(), nowTemp));
            info.setReportTime(live.getString("reporttime"));
        }

        if (casts != null) {
            List<WeatherInfo.DayForecast> list = new ArrayList<>();
            for (int i = 0; i < casts.size(); i++) {
                JSONObject c = casts.getJSONObject(i);
                WeatherInfo.DayForecast day = new WeatherInfo.DayForecast();
                day.setDate(c.getString("date"));
                day.setWeek(c.getString("week"));
                day.setDayWeather(c.getString("dayweather"));
                day.setNightWeather(c.getString("nightweather"));
                day.setDayTemp(c.getString("daytemp"));
                day.setNightTemp(c.getString("nighttemp"));
                list.add(day);
            }
            info.setForecastList(list);
        }

        return info;
    }

    // 12小时预报
    public List<Map<String, String>> getHourlyForecast(String city) {
        JSONObject live = getLiveWeather(city);
        if (live == null) return new ArrayList<>();
        String weather = live.getString("weather") == null ? "晴" : live.getString("weather");
        Integer baseTemp = getPureTemp(live.getString("temperature"));
        if(baseTemp == null) baseTemp = 20;

        List<Map<String, String>> list = new ArrayList<>();
        Calendar cal = Calendar.getInstance();

        for (int i = 0; i < 12; i++) {
            Map<String, String> map = new HashMap<>();
            map.put("time", String.format("%02d:00", cal.get(Calendar.HOUR_OF_DAY)));
            map.put("weather", weather);
            int change = i % 3 == 0 ? 1 : -1;
            map.put("temp", String.valueOf(baseTemp + change));
            map.put("icon", getWeatherIcon(weather));
            list.add(map);
            cal.add(Calendar.HOUR_OF_DAY, 1);
        }
        return list;
    }

    public String getWeatherIcon(String w) {
        if (w == null) return "🌤️";
        if (w.contains("晴")) return "☀️";
        if (w.contains("云")) return "⛅";
        if (w.contains("阴")) return "☁️";
        if (w.contains("雨")) return "🌧️";
        if (w.contains("雪")) return "❄️";
        if (w.contains("雷")) return "⛈️";
        return "🌤️";
    }

    public String getExtremeWeatherTip(WeatherInfo info) {
        if (info == null || info.getForecastList() == null) return "";
        for (WeatherInfo.DayForecast d : info.getForecastList()) {
            String all = d.getDayWeather() + d.getNightWeather();
            if (all.contains("雨") || all.contains("雪") || all.contains("台风"))
                return "⚠️ 未来有恶劣天气，请注意出行防护";
        }
        return "";
    }

    public String getWarmTip(WeatherInfo info) {
        if (info == null) return "暂无天气提示";
        String weather = info.getWeather() == null ? "" : info.getWeather();
        if (weather.contains("雨")) return "今日降雨，出门请携带雨具";
        if (weather.contains("雪")) return "今日降雪，路面湿滑谨慎慢行";
        if (weather.contains("晴")) return "晴空万里，适宜外出游玩散心";
        return "天气平稳，保持好心情出行";
    }

    public String getTravelAdvice(WeatherInfo info) {
        if(info == null || info.getForecastList() == null || info.getForecastList().isEmpty()){
            return "暂无有效天气数据";
        }
        Integer nightTemp = getPureTemp(info.getForecastList().get(0).getNightTemp());
        Integer dayTemp = getPureTemp(info.getForecastList().get(0).getDayTemp());
        if(nightTemp == null) nightTemp = 15;
        if(dayTemp == null) dayTemp = 20;
        String nowWeather = info.getWeather() == null ? "" : info.getWeather();

        if(nowWeather.contains("晴") && nightTemp >=15 && dayTemp <=28){
            return "出行建议：气温舒适阳光充足，非常适合骑行、散步、户外游玩";
        }else if(nowWeather.contains("雨") || nowWeather.contains("雪")){
            return "出行建议：雨雪天气出行不便，优先选择室内活动减少外出";
        }else if(dayTemp > 32){
            return "出行建议：天气炎热，做好防晒补水，避开正午高温时段外出";
        }else if(nightTemp < 8){
            return "出行建议：整体气温偏低，外出注意增添衣物做好防寒";
        }
        return "出行建议：天气条件适中，可自由安排出行计划";
    }

    public String getClothingAdvice(WeatherInfo info) {
        if(info == null || info.getForecastList() == null || info.getForecastList().isEmpty()){
            return "暂无有效穿衣参考数据";
        }
        Integer nightTemp = getPureTemp(info.getForecastList().get(0).getNightTemp());
        Integer dayTemp = getPureTemp(info.getForecastList().get(0).getDayTemp());
        if(nightTemp == null) nightTemp = 15;
        if(dayTemp == null) dayTemp = 20;
        int avgTemp = (nightTemp + dayTemp) / 2;

        if(avgTemp >= 30){
            return "穿衣建议：天气酷热，穿着短袖、短裤等轻薄透气服饰即可";
        }else if(avgTemp >= 22){
            return "穿衣建议：温度宜人，日常T恤、休闲衬衫轻松穿搭";
        }else if(avgTemp >= 14){
            return "穿衣建议：温度略凉，记得搭配薄外套哦";
        }else if(avgTemp >= 6){
            return "穿衣建议：气温偏凉，优先选择卫衣、针织衫等保暖上衣";
        }
        return "穿衣建议：天气寒冷，务必穿戴羽绒服、厚棉衣做好全身保暖";
    }

    // 风力等级分析
    public String getWindAnalysis(WeatherInfo info) {
        try {
            String windPowerStr = info.getWindPower();
            // 提取数字部分，比如 "≤3级" → "3"，"3 级" → "3"
            String numOnly = windPowerStr.replaceAll("[^0-9]", "");
            if (numOnly.isEmpty()) {
                return "风力正常";
            }
            int power = Integer.parseInt(numOnly);
            if (power <= 2) return "微风拂面，体感舒适，十分适合外出游玩";
            if (power <= 4) return "和风舒适，体感良好，不影响出行";
            if (power <= 6) return "风力稍大，建议外出做好防风准备";
            return "大风天气，减少不必要的外出";
        } catch (Exception e) {
            return "风力正常";
        }
    }

    // 空气湿度分析
    public String getHumidityAnalysis(WeatherInfo info) {
        try {
            int humidity = Integer.parseInt(info.getHumidity());
            if (humidity > 70) return "空气潮湿闷热，体感压抑易不适";
            if (humidity < 40) return "空气干燥，体感偏干，注意补水保湿";
            return "湿度适宜，体感舒适";
        } catch (Exception e) {
            return "湿度正常";
        }
    }

    // 完整分段天气分析报告
    public List<String> getWeatherReportLines(WeatherInfo info) {
        List<String> lines = new ArrayList<>();
        String weather = info.getWeather();
        String temp = info.getTemperature();
        String humidity = info.getHumidity();
        String windDir = info.getWindDir();
        String windPower = info.getWindPower();

        // 第一行：天气状况
        lines.add("当前查询地区天气状况为" + weather + "，整体天气趋势平稳，数据实时更新。");

        // 第二行：温度分析
        int tempInt = Integer.parseInt(temp);
        String tempTip = tempInt >= 30 ? "体感偏热，注意防暑降温"
                : tempInt <= 10 ? "体感偏凉，注意添衣保暖"
                  : "温度适宜，体感非常舒适";
        lines.add("当前实时气温为" + temp + "℃，" + tempTip);

        // 第三行：湿度分析
        String humidityTip = getHumidityAnalysis(info);
        lines.add("空气相对湿度为" + humidity + "%，" + humidityTip);

        // 第四行：风力分析
        String windTip = getWindAnalysis(info);
        lines.add("当前主导风向为" + windDir + "，风力等级" + windPower + "，" + windTip);

        return lines;
    }

    // ===================== 天气类型判断 =====================
// 0=雨天  1=雪天  2=晴天  3=多云  4=阴天  5=雾霾天  6=高温天气  7=其他
    public int getWeatherTypeByCode(String weatherStr, int temp) {
        if (weatherStr == null) return 7;

        // 雪天
        if (weatherStr.contains("雪")) {
            return 1;
        }
        // 雨天（含雷阵雨）
        if (weatherStr.contains("雨") || weatherStr.contains("雷")) {
            return 0;
        }
        // 雾霾天（雾、霾、浮尘、烟雾等）
        if (weatherStr.contains("雾") || weatherStr.contains("霾")
                || weatherStr.contains("浮尘") || weatherStr.contains("烟雾")) {
            return 5;
        }
        // 晴天
        if (weatherStr.equals("晴")) {
            return 2;
        }
        // 多云
        if (weatherStr.contains("云") || weatherStr.contains("多云")) {
            return 3;
        }
        // 阴天
        if (weatherStr.equals("阴")) {
            return 4;
        }
        // 高温天气 ≥35℃
        if (temp >= 35) {
            return 6;
        }

        // 其他
        return 7;
    }

    // ===================== 天气类型名称 =====================
    public String getWeatherTypeName(int type) {
        switch (type) {
            case 0: return "雨天";
            case 1: return "雪天";
            case 2: return "晴天";
            case 3: return "多云";
            case 4: return "阴天";
            case 5: return "雾霾天";
            case 6: return "高温天气";
            case 7: return "其他天气";
            default: return "未知";
        }
    }

    // 省份-省会映射表（Java 8 兼容版）
    private static final Map<String, String> PROVINCE_CITY_MAP = new HashMap<>();
    static {
        PROVINCE_CITY_MAP.put("北京", "北京");
        PROVINCE_CITY_MAP.put("天津", "天津");
        PROVINCE_CITY_MAP.put("上海", "上海");
        PROVINCE_CITY_MAP.put("重庆", "重庆");
        PROVINCE_CITY_MAP.put("新疆", "乌鲁木齐");
        PROVINCE_CITY_MAP.put("西藏", "拉萨");
        PROVINCE_CITY_MAP.put("内蒙古", "呼和浩特");
        PROVINCE_CITY_MAP.put("广西", "南宁");
        PROVINCE_CITY_MAP.put("黑龙江", "哈尔滨");
        PROVINCE_CITY_MAP.put("吉林", "长春");
        PROVINCE_CITY_MAP.put("辽宁", "沈阳");
        PROVINCE_CITY_MAP.put("河北", "石家庄");
        PROVINCE_CITY_MAP.put("山西", "太原");
        PROVINCE_CITY_MAP.put("陕西", "西安");
        PROVINCE_CITY_MAP.put("山东", "济南");
        PROVINCE_CITY_MAP.put("河南", "郑州");
        PROVINCE_CITY_MAP.put("江苏", "南京");
        PROVINCE_CITY_MAP.put("浙江", "杭州");
        PROVINCE_CITY_MAP.put("安徽", "合肥");
        PROVINCE_CITY_MAP.put("福建", "福州");
        PROVINCE_CITY_MAP.put("江西", "南昌");
        PROVINCE_CITY_MAP.put("湖南", "长沙");
        PROVINCE_CITY_MAP.put("湖北", "武汉");
        PROVINCE_CITY_MAP.put("广东", "广州");
        PROVINCE_CITY_MAP.put("海南", "海口");
        PROVINCE_CITY_MAP.put("甘肃", "兰州");
        PROVINCE_CITY_MAP.put("青海", "西宁");
        PROVINCE_CITY_MAP.put("宁夏", "银川");
        PROVINCE_CITY_MAP.put("四川", "成都");
        PROVINCE_CITY_MAP.put("贵州", "贵阳");
        PROVINCE_CITY_MAP.put("云南", "昆明");
    }

    // 地图数据
    public List<Map<String, Object>> getMapData() {
        List<Map<String, Object>> mapData = new ArrayList<>();
        for (Map.Entry<String, String> entry : PROVINCE_CITY_MAP.entrySet()) {

            String provinceName = entry.getKey();
            String cityName = entry.getValue();
            JSONObject live = getLiveWeather(cityName);

            Integer tempNum = 20;
            if (live != null) {
                tempNum = getPureTemp(live.getString("temperature"));
            }

            Map<String, Object> item = new HashMap<>();
            item.put("name", provinceName);
            item.put("value", tempNum);
            mapData.add(item);
        }
        return mapData;
    }

    // 全国天气列表
    public List<CityWeatherDTO> getAllCityWeather() {
        List<CityWeatherDTO> result = new ArrayList<>();
        for (Map.Entry<String, String> entry : PROVINCE_CITY_MAP.entrySet()) {

            String cityName = entry.getValue();
            JSONObject live = getLiveWeather(cityName);

            CityWeatherDTO dto = new CityWeatherDTO();
            dto.setCityName(live != null ? live.getString("city") : cityName);
            dto.setTemperature(live != null ? live.getString("temperature") : "20℃");
            dto.setWeatherDesc(live != null ? live.getString("weather") : "晴");
            dto.setHumidity(live != null ? live.getString("humidity") : "50");
            dto.setWindInfo(live != null ? (live.getString("winddirection") + " " + live.getString("windpower")) : "无 微风");

            Integer tempNum = 20;
            if (live != null) {
                tempNum = getPureTemp(live.getString("temperature"));
            }
            dto.setTempNum(tempNum);

            int weatherType = getWeatherTypeByCode(dto.getWeatherDesc(), tempNum);
            dto.setWeatherTagName(getWeatherTypeName(weatherType));
            dto.setWeatherType(weatherType);

            result.add(dto);

            try {
                Thread.sleep(300);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

        }
        return result;
    }

    // 饼图数据
    public List<Integer> getWeatherTypeCount(List<CityWeatherDTO> cityList) {
        int rain = 0;     // 0
        int snow = 0;     // 1
        int sunny = 0;    // 2
        int cloud = 0;    // 3
        int overcast = 0; // 4
        int haze = 0;     // 5
        int hot = 0;      // 6
        int other = 0;    // 7

        for (CityWeatherDTO dto : cityList) {
            switch (dto.getWeatherType()) {
                case 0: rain++; break;
                case 1: snow++; break;
                case 2: sunny++; break;
                case 3: cloud++; break;
                case 4: overcast++; break;
                case 5: haze++; break;
                case 6: hot++; break;
                case 7: other++; break;
            }
        }
        // 返回顺序必须和前端 legend 一致
        return Arrays.asList(rain, snow, sunny, cloud, overcast, haze, hot, other);
    }

    // ===================== 温度排行数据（全国市县，按升序/降序） =====================
    public List<CityWeatherDTO> getCityRankList(boolean isDesc) {
        List<CityWeatherDTO> list = getAllCityWeather();
        // 过滤无效数据
        list = list.stream().filter(dto -> dto.getTempNum() != null).collect(Collectors.toList());

        // 排序
        if (isDesc) {
            list.sort((a, b) -> Integer.compare(b.getTempNum(), a.getTempNum()));
        } else {
            list.sort((a, b) -> Integer.compare(a.getTempNum(), b.getTempNum()));
        }
        return list;
    }
}