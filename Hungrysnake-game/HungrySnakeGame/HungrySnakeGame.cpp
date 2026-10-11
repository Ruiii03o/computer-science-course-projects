#include<stdio.h>
#include<stdlib.h>   /*用于控制台输入输出操作*/
#include<graphics.h>  /*用于图形编程*/
#include<windows.h>   /*添加windows.h用于sleep函数*/
#include<time.h>  /*用于取随机数*/
#include<math.h>
#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup") 
/*隐藏控制台窗口的编译指令，/entry:mainCRTStartup 用于指定程序的入口点为 mainCRTStartup 函数，
而 mainCRTStartup 函数会先执行一些 C 程序初始化等相关处理，之后再调用 main 函数，这样就能保证以 main 函数作为程序逻辑起点的代码，在窗口程序模式下也能正常运行 。*/
#include <mmsystem.h>  // Windows多媒体API
#pragma comment(lib, "winmm.lib")  // 链接多媒体库

#define MAX 100   //宏定义最大节数为100，方便修改蛇身长度
#define MAX_OBSTACLES 20   //定义最大障碍物数量

/* 难度系统实现 */
enum GameLevel { EASY, NORMAL, HARD, EXPERT };       // 难度枚举 ： 定义四种游戏难度级别
int speedLevels[] = { 200, 150, 100, 70 };        // 不同难度对应的蛇移动速度(毫秒)，数值越小速度越快
int currentLevel = EASY;       // 当前选择的难度级别，初始为EASY

/* 游戏状态 */
enum GameState { LEVEL_SELECT, PLAYING, GAME_OVER };
enum GameState currentState = LEVEL_SELECT;     //初始显示难度选择
/*
把程序所要用的数据抽象出来
*/
typedef struct piontxy
{
	int x;
	int y;
}MYPOINT;    //将坐标相关的数据封装在该结构体中，当使用坐标时无需重复定义包含x,y成员的结构体
HWND hwnd = NULL;  //创建句柄用来标识窗口，句柄是 Windows 操作系统用来标识和访问资源的一个不透明值，初始化为 NULL表示尚未关联任何窗口

//蛇
struct Snake
{
	int num;  //蛇身节数
	MYPOINT xy[MAX];  //每一节蛇身的坐标
	char position;   //用标记表示方向，即蛇往哪个方向走
}snake;

//食物
struct Food
{
	MYPOINT foodxy = { 0, 0 }; //食物坐标
	int flag=0;    //食物存在的标记，开始时不存在
	int eatGrade = 0;   //进食得分
	int type = 0;   // 0-普通食物，1-加速食物，2-减速食物，3-加分食物
}food;
// 食物颜色数组
const COLORREF FOOD_COLORS[] = { RED, BLUE, YELLOW, MAGENTA };

//把方向枚举出来
enum movPosition { right, left, down, up };

//障碍物
struct Obstacle 
{
	int x, y;      // 圆心坐标
	int radius;    // 半径
};
Obstacle obstacles[MAX_OBSTACLES];
int obstacleCount = 0;     //当前无障碍物,初始化为0

/* 函数声明 */
void initSnake();
void drawSnake();
void moveSnake();
void keyDown();
void initFood();
void drawFood();
void initObstacles();
void addObstacle();
void drawObstacles();
void eatFood();
int snakeDie();
void restartGame();
void showLevelMenu();
void showGameOver();
void handleLevelInput();
void handleRestart();

//整个项目的划分
//1.初始化蛇
void initSnake()
{
	snake.xy[2].x = 0;  //初始蛇身为3节，每节10像素
	snake.xy[2].y = 0;  //初始最后一节蛇坐标

	snake.xy[1].x = 10; 
	snake.xy[1].y = 0;  //初始倒数第二节蛇坐标


	snake.xy[0].x = 20;
	snake.xy[0].y = 0;   //蛇头坐标
	printf("initsnake\n");

	snake.num = 3;  //初始为3节
	snake.position = right;  //初始方向为右
	food.flag = 0;  //初始食物标记为0

}

//2.绘制蛇
void drawSnake()
{

	printf("drawsnake\n");
	for (int i = 0;i < snake.num;i++)
	{
		setlinecolor(BLACK);    //设置线条颜色
		setfillcolor(GREEN);  //设置填充颜色
		fillrectangle(snake.xy[i].x, snake.xy[i].y, snake.xy[i].x + 10, snake.xy[i].y + 10);  //依次为每节蛇填充颜色
	}
}

//3.移动蛇
void moveSnake()
{
	for (int i = snake.num - 1;i > 0;i--)
	{
		snake.xy[i].x = snake.xy[i - 1].x;
		snake.xy[i].y = snake.xy[i - 1].y;  //除了第一节之外，后面的坐标都是前一节坐标
	}
	switch (snake.position)      //蛇头根据方向标志移动
	{
	case right:
		snake.xy[0].x += 10;
		break;
	case left:
		snake.xy[0].x -= 10;
		break;
	case down:
		snake.xy[0].y += 10;  //往下是坐标轴正方向为加
		break;
	case up:
		snake.xy[0].y -= 10;  //往上是坐标轴负方向为减
		break;

	}
}

//4.按键交互
void keyDown() {
	// 使用Windows API检测按键状态（无需处理方向键这样的双键码）
	//GetAsyncKeyState()返回一个16位的SHORT值，当GetAsyncKeyState(key) & 0x8000结果为非零时，表示该按键当前正处于按下状态
	if (GetAsyncKeyState('W') & 0x8000 && snake.position != down)  // 检测'W'键是否被按下，且当前方向不是向下
	{
		snake.position = up;   //向上
	}
	if (GetAsyncKeyState('S') & 0x8000 && snake.position != up)   // 检测'S'键是否被按下，且当前方向不是向上
	{
		snake.position = down;  //向下
	}
	if (GetAsyncKeyState('A') & 0x8000 && snake.position != right) // 检测'A'键是否被按下，且当前方向不是向右
	{
		snake.position = left;  //向左
	} 
	if (GetAsyncKeyState('D') & 0x8000 && snake.position != left)  // 检测'D'键是否被按下，且当前方向不是向左
	{
		snake.position = right;  //向右
	}

	// 检测方向键（虚拟键码）
	if (GetAsyncKeyState(VK_UP) & 0x8000 && snake.position != down)  // 检测上箭头键是否被按下，且当前方向不是向下
	{
		snake.position = up;    //向上
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000 && snake.position != up)  // 检测下箭头键是否被按下，且当前方向不是向上
	{
		snake.position = down;  //向下
	}
	if (GetAsyncKeyState(VK_LEFT) & 0x8000 && snake.position != right)  // 检测左箭头键是否被按下，且当前方向不是向右
	{
		snake.position = left;  //向左
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && snake.position != left)  // 检测右箭头键是否被按下，且当前方向不是向左
	{
		snake.position = right;  //向右
	}
}

//5.初始化食物
void initFood()
{
	bool overlap;
	int retryCount = 0;
	do {
		overlap = false;
	    food.foodxy.x = rand() % 64 * 10;  //用随机函数初始化食物，再%64*10使食物横坐标都为10的整数倍，刚好与蛇头对齐（蛇是由边长为10的小矩形构成）
	    food.foodxy.y = rand() % 48 * 10;   //使食物纵坐标都为10的整数倍，刚好与蛇头对齐
		food.type = rand() % 4; // 随机生成0-3的食物类型
		
        // 检查是否与障碍物重叠
        for (int i = 0; i < obstacleCount; i++) {
            int distX = food.foodxy.x + 5 - obstacles[i].x;
            int distY = food.foodxy.y + 5 - obstacles[i].y;
            if (sqrt(distX * distX + distY * distY) < obstacles[i].radius + 5)
			{
                overlap = true;
                break;
            }
        }
		// 检查与蛇身重叠
		for (int i = 0; i < snake.num; i++) {
			if (food.foodxy.x == snake.xy[i].x && food.foodxy.y == snake.xy[i].y) 
			{
				overlap = true;
				break;
			}
		}
		retryCount++;
		if (retryCount > 100) {
			// 防止无限循环，强制生成一个位置
			food.foodxy.x = (rand() % 30 + 10) * 10;
			food.foodxy.y = (rand() % 20 + 10) * 10;
			break;
		}
    } while (overlap);
	food.flag = 1;  //产生食物，标记为1
}

//6.绘制食物
void drawFood()
{
	setfillcolor(FOOD_COLORS[food.type]); // 根据食物类型设置颜色
	fillrectangle(food.foodxy.x, food.foodxy.y, food.foodxy.x + 10, food.foodxy.y + 10);
}

//7.障碍物 
void initObstacles() {     //初始化障碍物，只在游戏开始时调用一次，初始没有障碍物

	obstacleCount = 0;
}
// 添加新障碍物（每次吃食物时调用）
void addObstacle() 
{
	if (obstacleCount >= MAX_OBSTACLES) { 
		return; }    // 检查是否已达到最大障碍物数量，若是则直接返回
    obstacleCount++;     // 增加障碍物计数
	for (int i = obstacleCount - 1; i < obstacleCount; i++) {       // 生成新障碍物
		bool overlap;  // 标记是否与现有元素重叠，循环生成障碍物位置，直到找到不重叠的位置
		int retryCount = 0;
		do {
			overlap = false;  // 初始化重叠标记为false
			obstacles[i].x = 30 + rand() % (640 - 60);      // 随机生成障碍物位置，避开边界，预留30像素边距
			obstacles[i].y = 30 + rand() % (480 - 60);
			obstacles[i].radius = 15;  // 设置障碍物半径为15像素

			// 检查与蛇身的重叠（计算障碍物与每个蛇身段的距离）
			for (int j = 0; j < snake.num; j++) {
				int distX = obstacles[i].x - snake.xy[j].x;  // X方向距离
				int distY = obstacles[i].y - snake.xy[j].y;  // Y方向距离
				if (sqrt(distX * distX + distY * distY) < obstacles[i].radius + 5) {     // 如果距离小于安全阈值即小于障碍物半径+蛇身宽度的一半
					overlap = true;  // 标记为重叠
					break;  // 跳出循环，重新生成位置
				}
			}

			// 检查与现有障碍物的重叠（计算与已存在障碍物的距离）
			for (int j = 0; j < i; j++) {
				int distX = obstacles[i].x - obstacles[j].x;
				int distY = obstacles[i].y - obstacles[j].y;
				if (sqrt(distX * distX + distY * distY) < obstacles[i].radius * 2 + 5) {    // 如果距离小于两障碍物半径之和+安全余量（蛇身通不过）
					overlap = true;  // 标记为重叠
					break;  // 跳出循环，重新生成位置
				}
			}

			// 检查与食物的重叠（仅当食物存在时）
			if (food.flag) {
				int distX = obstacles[i].x - food.foodxy.x;
				int distY = obstacles[i].y - food.foodxy.y;
				if (sqrt(distX * distX + distY * distY) < obstacles[i].radius + 5) {    // 如果距离小于安全阈值即小于障碍物半径+蛇身宽度的一半
					overlap = true;  // 标记为重叠
				}
			}
			retryCount++;
			if (retryCount > 100) {
				// 防止无限循环，强制生成一个位置
				obstacles[i].x = (rand() % 30 + 10) * 10;
				obstacles[i].y = (rand() % 20 + 10) * 10;
				break;
			}
		} while (overlap);  // 若重叠则继续循环，直到找到合适位置
	}
}
// 绘制障碍物函数
void drawObstacles()
{
	for (int i = 0; i < obstacleCount; i++) {
		setfillcolor(BLACK);
		solidcircle(obstacles[i].x, obstacles[i].y, obstacles[i].radius);
	}
}

//8.蛇吃食物
void eatFood()
{
	if (snake.xy[0].x == food.foodxy.x && snake.xy[0].y == food.foodxy.y)
	{
		snake.num++;
		// 根据食物类型应用不同效果
		switch (food.type) {
		case 0:   // 普通食物
			food.eatGrade += 10;
			break;
		case 1:    // 加速食物
			if (speedLevels[currentLevel] > 50) {      //最快：50ms（加速食物多次生效后）
				speedLevels[currentLevel] -= 10;   // 加快游戏速度
			}
			food.eatGrade += 10;
			break;
		case 2:    // 减速食物
			if (speedLevels[currentLevel] < 300) {      //最慢：300ms（减速食物多次生效后）
				speedLevels[currentLevel] += 10; // 减慢游戏速度
			}
			food.eatGrade += 10;
			break;
		case 3:    // 加分食物
			food.eatGrade += 30; // 获得更多分数
			break;
		}
		food.flag = 0;
		PlaySound(TEXT("eat_food.wav"), NULL, SND_ASYNC | SND_FILENAME);     //添加蛇吃食物音效
		addObstacle();    // 每吃一个食物添加一个障碍物
	}
}

//9.死亡判断
//返回值1表示蛇已死亡，0表示存活
int snakeDie()
{  //撞墙死亡
	if (snake.xy[0].x > 640 || snake.xy[0].x < 0 || snake.xy[0].y>480 || snake.xy[0].y < 0)  
	{
	    PlaySound(TEXT("crash.wav"), NULL, SND_ASYNC | SND_FILENAME);  //添加碰撞音效
		
		MessageBox(hwnd, "游戏结束！", "撞到墙了！", MB_OK | MB_ICONWARNING);  //MessageBox 是 Windows API 提供的弹窗函数，在程序中显示一个简单的消息对话框，此处用于在游戏结束时向玩家显示提示信息
		//消息框成为 hwnd 对应窗口的模态对话，使消息框居中显示在游戏窗口上，这时游戏窗口会被禁用直到用户关闭消息框
		//"游戏结束！"是对话框显示的主要内容；"撞墙死亡！"是对话框标题栏文字；MB_OK | MB_ICONWARNING显示 "确定"按钮 + 警告图标
		return 1;
	}
	//撞自己死亡自己
	for (int i = 1;i < snake.num;i++)
	{
		if (snake.xy[0].x == snake.xy[i].x && snake.xy[0].y == snake.xy[i].y)
		{
			PlaySound(TEXT("crash.wav"), NULL, SND_ASYNC | SND_FILENAME);  //添加碰撞音效

			MessageBox(hwnd, "游戏结束！", "撞到自己了！", MB_OK | MB_ICONWARNING);
			return 1;
		}
	}
	//撞障碍物死亡
		for (int i = 0; i < obstacleCount; i++) {
			int distX = snake.xy[0].x + 5 - obstacles[i].x;
			int distY = snake.xy[0].y + 5 - obstacles[i].y;
			if (sqrt(distX * distX + distY * distY) < obstacles[i].radius + 5) {
				PlaySound(TEXT("crash.wav"), NULL, SND_ASYNC | SND_FILENAME);   //添加碰撞音效
				MessageBox(hwnd, "游戏结束！", "撞到障碍物了！", MB_OK | MB_ICONWARNING);
				return 1;
			}
		}
	return 0;
}

//10.重新开始游戏
void restartGame() 
{
	// 重置障碍物
	initObstacles();
	// 重置蛇的状态
	initSnake();
   // 重置食物状态
	food.flag = 0;
	food.eatGrade = 0;
   // 清屏
	cleardevice();
}

//选择菜单
void showLevelMenu() {
	cleardevice();
	settextstyle(30, 0, "微软雅黑");
	outtextxy(200, 100, "选择难度");
	settextstyle(20, 0, "微软雅黑");
	// 高亮显示当前选择的难度
	const char* levelNames[] = { "简单", "普通", "困难", "专家" };
	for (int i = 0; i < 4; i++) {
		if (i == currentLevel) {
			settextcolor(YELLOW);
		}
		else {
			settextcolor(BLACK);
		}
		outtextxy(250, 200 + i * 50, levelNames[i]);    //s水平位置不变，垂直位置根据循环变量i动态计算
	}

	settextcolor(BLACK);
	outtextxy(200, 400, "按数字键1-4选择难度，按空格开始游戏");
	FlushBatchDraw();
}
// 游戏结束界面
void showGameOver()
{
	settextstyle(30, 0, "微软雅黑");
	settextcolor(RED);
	outtextxy(200, 200, "游戏结束! 按R键重新开始");
	FlushBatchDraw();
}

//处理难度选择
void handleLevelInput() {
	if (currentState == LEVEL_SELECT) {
		if (GetAsyncKeyState('1') & 0x8000) { currentLevel = EASY; showLevelMenu(); }
		if (GetAsyncKeyState('2') & 0x8000) { currentLevel = NORMAL; showLevelMenu(); }
		if (GetAsyncKeyState('3') & 0x8000) { currentLevel = HARD; showLevelMenu(); }
		if (GetAsyncKeyState('4') & 0x8000) { currentLevel = EXPERT; showLevelMenu(); }

		if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
			currentState = PLAYING;
			restartGame();
		}
	}
}

void handleRestart()
{
	if (currentState == GAME_OVER) {
		if (GetAsyncKeyState('R') & 0x8000) {
			currentState = LEVEL_SELECT;
			showLevelMenu();
		}
	}
}
 

int main()
{
	
	srand((unsigned int)time(NULL));   //随机函数的种子
	hwnd = initgraph(640, 480);  //创建图形窗口，返回创建的窗口的句柄
	// 添加错误检查代码
	if (hwnd == NULL) {
		// 处理图形库初始化失败的情况，如输出错误信息并退出程序
		printf("图形库初始化失败!原因为:\n");
		printf("1. 未安装EasyX图形库\n");
		printf("2. 项目设置不正确\n");
		system("pause");
		return -1;
	}
	SetWindowText(hwnd, "贪吃蛇游戏");     // 设置窗口标题
	setbkcolor(WHITE);
	BeginBatchDraw();  //开启双缓冲，解决画面闪烁问题并提升绘制效率,调用BeginBatchDraw()后所有绘图操作先在内存缓冲区中完成
	cleardevice();

	// 显示难度选择菜单
	showLevelMenu();

	// 游戏主循环
	while (1) {
		handleLevelInput();  // 处理难度选择
		handleRestart();     // 处理游戏重启

		if (currentState == PLAYING) {
			keyDown();
			cleardevice();

			//生成食物
			if (food.flag == 0) {
				initFood();
			}
			//绘制游戏元素
			drawSnake(); //绘制蛇
			drawFood();  // 绘制食物
			drawObstacles();  // 绘制障碍物

			// 显示分数
			settextstyle(16, 0, "微软雅黑");  // 设置字体
			settextcolor(BLACK);  // 设置接下来要输出的文字颜色为黑色
			setbkmode(TRANSPARENT);  //设置透明背景
			char scoreText[20];    //定义一个长度为20的字符数组，用于存储分数文本
			snprintf(scoreText, sizeof(scoreText), "Score: %d", food.eatGrade);    //格式化字符串
			outtextxy(10, 10, scoreText);  //在左上角的坐标(10,10处)输出得分

			// 显示当前难度
			const char* levelNames[] = { "简单", "普通", "困难", "专家" };
			char levelText[20];
			snprintf(levelText, sizeof(levelText), "难度: %s", levelNames[currentLevel]);
			outtextxy(10, 30, levelText);

			//更新游戏状态
			if (snakeDie())        //蛇死亡游戏结束
			{
				currentState = GAME_OVER;
				showGameOver();
			}
			else {
				moveSnake();
				eatFood();
			}
			
		}

			FlushBatchDraw();  //执行FlushBatchDraw()一次性将整个缓冲区内容输出屏幕
			Sleep(speedLevels[currentLevel]);   //控制速度使蛇移动速度适宜
	}

		
	EndBatchDraw();
	closegraph();   //关闭窗口，释放创建窗口所占用的资源
	return 0;
}


