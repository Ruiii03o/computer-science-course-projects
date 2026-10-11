package com.weather.weatherdemo.entity;

public class CityWeatherDTO {
    private String cityName;
    private String temperature;
    private String weatherDesc;
    private String humidity;
    private String windInfo;
    private Integer tempNum;
    private String weatherTagName;
    private int weatherType; // 👈 这个必须有！

    public String getCityName() {
        return cityName;
    }

    public void setCityName(String cityName) {
        this.cityName = cityName;
    }

    public String getTemperature() {
        return temperature;
    }

    public void setTemperature(String temperature) {
        this.temperature = temperature;
    }

    public String getWeatherDesc() {
        return weatherDesc;
    }

    public void setWeatherDesc(String weatherDesc) {
        this.weatherDesc = weatherDesc;
    }

    public String getHumidity() {
        return humidity;
    }

    public void setHumidity(String humidity) {
        this.humidity = humidity;
    }

    public String getWindInfo() {
        return windInfo;
    }

    public void setWindInfo(String windInfo) {
        this.windInfo = windInfo;
    }

    public Integer getTempNum() {
        return tempNum;
    }

    public void setTempNum(Integer tempNum) {
        this.tempNum = tempNum;
    }

    public String getWeatherTagName() {
        return weatherTagName;
    }

    public void setWeatherTagName(String weatherTagName) {
        this.weatherTagName = weatherTagName;
    }

    public int getWeatherType() {
        return weatherType;
    }

    public void setWeatherType(int weatherType) {
        this.weatherType = weatherType;
    }
}