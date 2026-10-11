package com.weather.weatherdemo.repository;

import com.weather.weatherdemo.entity.User;
import org.springframework.jdbc.core.BeanPropertyRowMapper;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public class UserRepository {

    private final JdbcTemplate jdbcTemplate;

    public UserRepository(JdbcTemplate jdbcTemplate) {
        this.jdbcTemplate = jdbcTemplate;
    }

    // 根据用户名查询用户
    public User findByUsername(String username) {
        String sql = "SELECT * FROM [user] WHERE username = ?";
        List<User> list = jdbcTemplate.query(sql, new BeanPropertyRowMapper<>(User.class), username);
        return list.isEmpty() ? null : list.get(0);
    }

    // 新增用户
    public int addUser(User user) {
        String sql = "INSERT INTO [user] (username, password, default_city) VALUES (?, ?, ?)";
        return jdbcTemplate.update(sql, user.getUsername(), user.getPassword(), user.getDefaultCity());
    }

    // 更新用户默认城市
    public int updateCity(int userId, String city) {
        String sql = "UPDATE [user] SET default_city = ? WHERE id = ?";
        return jdbcTemplate.update(sql, city, userId);
    }
}