package com.weather.weatherdemo;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.Bean;
import org.springframework.web.client.RestTemplate;

@SpringBootApplication
public class WeatherDemoApplication {

    private static final Logger logger = LoggerFactory.getLogger(WeatherDemoApplication.class);

    public static void main(String[] args) {
        SpringApplication.run(WeatherDemoApplication.class, args);
        logger.info("========================================");
        logger.info("    天气查询系统启动成功！作者：崔晓瑞      ");
        logger.info("========================================");
    }

}