package com.weather.weatherdemo.service;

import com.weather.weatherdemo.entity.User;
import com.weather.weatherdemo.repository.UserRepository;
import org.springframework.stereotype.Service;

@Service
public class UserService {

    private final UserRepository userRepository;

    public UserService(UserRepository userRepository) {
        this.userRepository = userRepository;
    }

    // 登录验证
    public User login(String username, String password) {
        User user = userRepository.findByUsername(username);
        if (user != null && user.getPassword().equals(password)) {
            return user;
        }
        return null;
    }

    // 注册
    public boolean register(String username, String password) {
        if (userRepository.findByUsername(username) != null) {
            return false;
        }
        User user = new User();
        user.setUsername(username);
        user.setPassword(password);
        return userRepository.addUser(user) > 0;
    }

    // 修改默认城市
    public boolean updateDefaultCity(Integer userId, String city) {
        return userRepository.updateCity(userId, city) > 0;
    }
}