
#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")
#include <graphics.h>
#include <windows.h>  // 用于 MessageBox
#include <conio.h>

int main() {
    // 尝试初始化图形窗口
    HWND hwnd = initgraph(640, 480);
    if (hwnd == NULL) {
        // 用弹窗显示错误信息
        MessageBox(NULL, "initgraph() 失败！", "错误", MB_ICONERROR);
        return -1;
    }

    // 绘制测试图形
    setfillcolor(RED);
    fillcircle(320, 240, 100);
    outtextxy(200, 400, "按任意键退出");

   _getch();  // 等待按键
    closegraph();
    return 0;
}