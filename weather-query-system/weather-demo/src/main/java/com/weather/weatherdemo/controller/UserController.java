package com.weather.weatherdemo.controller;

import com.weather.weatherdemo.entity.User;
import com.weather.weatherdemo.service.UserService;
import jakarta.servlet.http.HttpSession;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;

@Controller
public class UserController {

    private final UserService userService;

    public UserController(UserService userService) {
        this.userService = userService;
    }

    // 去登录页
    @GetMapping("/login")
    public String toLogin() {
        return "login";
    }

    // 登录提交
    @PostMapping("/login")
    public String login(@RequestParam String username,
                        @RequestParam String password,
                        HttpSession session,
                        Model model) {
        User user = userService.login(username, password);
        if (user != null) {
            session.setAttribute("loginUser", user);
            return "redirect:/";
        }
        model.addAttribute("error", "用户名或密码错误");
        return "login";
    }

    // 去注册页
    @GetMapping("/register")
    public String toRegister() {
        return "register";
    }

    // 注册提交
    @PostMapping("/register")
    public String register(@RequestParam String username,
                           @RequestParam String password,
                           Model model) {
        if (userService.register(username, password)) {
            return "redirect:/login";
        }
        model.addAttribute("error", "用户名已存在");
        return "register";
    }

    // 个人中心
    @GetMapping("/profile")
    public String toProfile(HttpSession session, Model model) {
        User loginUser = (User) session.getAttribute("loginUser");
        if (loginUser == null) {
            return "redirect:/login";
        }
        model.addAttribute("user", loginUser);
        return "profile";
    }

    // 修改城市
    @PostMapping("/updateCity")
    public String updateCity(@RequestParam String defaultCity,
                             HttpSession session) {
        User loginUser = (User) session.getAttribute("loginUser");
        if (loginUser == null) {
            return "redirect:/login";
        }
        userService.updateDefaultCity(loginUser.getId(), defaultCity);
        loginUser.setDefaultCity(defaultCity);
        session.setAttribute("loginUser", loginUser);
        return "redirect:/profile";
    }

    // 退出登录
    @GetMapping("/logout")
    public String logout(HttpSession session) {
        session.invalidate();
        return "redirect:/";
    }
}