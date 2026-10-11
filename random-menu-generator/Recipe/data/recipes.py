ALL_RECIPES = [
    {
        "id": 1,
        "name": "香煎三文鱼配芦笋",

        "image": "images/recipe1.png",  # 图片路径（相对于static文件夹）
        "video": "videos/video1.mp4",  # 视频路径（可选）
        "tags": ["海鲜", "西餐", "快手菜"],# 新增tags字段，存储该食谱的标签列表
        "ingredients": ["三文鱼", "芦笋", "橄榄油", "柠檬"],  # 新增食材信息
        "equipment": ["炒锅", "平底锅"],  # 新增烹饪器械信息
        "detail": "<h3>食材</h3><ul><li>三文鱼排 2块</li><li>芦笋 10根</li><li>橄榄油 2勺</li><li>盐和黑胡椒 适量</li></ul><h3>步骤</h3><ol><li>三文鱼用厨房纸吸干水分，撒上盐和黑胡椒</li><li>平底锅烧热，加入橄榄油，放入三文鱼皮朝下煎3分钟</li><li>翻面再煎2分钟，同时放入芦笋煎至变软</li><li>装盘即可食用</li></ol>"
    },
    {
        "id": 2,
        "name": "番茄牛腩意面",
        "image": "images/recipe2.png",  # 图片路径（相对于static文件夹）
        "video": "videos/video2.mp4",  # 视频路径（可选）
        "tags": ["家常菜", "意面", "炖菜"],  # 为该食谱添加标签
        "ingredients": ["牛腩", "番茄", "意面", "洋葱"],
        "equipment": ["高压锅", "炒锅"],
        "detail": "<h3>食材</h3><ul><li>意面 200g</li><li>牛腩 300g</li><li>番茄 3个</li><li>洋葱 1个</li></ul><h3>步骤</h3><ol><li>牛腩切块焯水，番茄去皮切丁</li><li>炒香洋葱，加入牛腩翻炒，加番茄丁煮30分钟</li><li>另起锅煮好意面，拌入牛腩酱汁即可</li></ol>"
    },
    {
        "id": 3,
        "name": "泰式青咖喱鸡",
        "image": "images/recipe3.png",  # 图片路径（相对于static文件夹）
        "video": "videos/video3.mp4",  # 视频路径（可选）
        "tags": ["东南亚菜", "咖喱", "鸡肉"],  # 为该食谱添加标签
        "ingredients": ["鸡肉", "青椒", "椰奶"],
        "equipment": ["炒锅"],
        "detail": "<h3>食材</h3><ul><li>鸡胸肉 500g</li><li>青咖喱酱 2勺</li><li>椰奶 400ml</li><li>青椒 1个</li></ul><h3>步骤</h3><ol><li>鸡肉切块，青椒切片</li><li>锅中放少许油，炒香青咖喱酱</li><li>加入鸡肉翻炒至变色，倒入椰奶煮10分钟</li><li>加入青椒片煮2分钟即可</li></ol>"
    },
{
        "id": 4,
        "name": "香辣鳗鱼",
        "image": "images/recipe4.png",  # 图片路径（相对于static文件夹）
        "video": "",  # 视频路径（可选）
        "tags": ["游戏", "辣", "鳗鱼"],  # 为该食谱添加标签
        "ingredients": ["鳗鱼"],
        "equipment": ["炒锅"],
        "detail": "<h3>食材</h3><ul><li>鳗鱼 </li><li>辣椒</li></ul><h3>获取途径</h3><ol><li>击杀骷髅洞穴中的飞蛇有6%概率掉落香辣鳗鱼。</li><li>沙漠商人会以一个红宝石的价格出售香辣鳗鱼</li><li>饲养岩浆鳗鱼的鱼塘也会在数量达到9后以每天3%~4%的概率生产5份香辣鳗鱼。</li></ol>"
    },
    {
        "id": 5,
        "name": "烤箱烤鸡",
        "image": "images/recipe5.png",
        "video": "videos/video4.mp4",
        "tags": ["家常菜", "鸡肉", "烤箱"],
        "ingredients": ["鸡肉", "土豆", "迷迭香", "橄榄油"],
        "equipment": ["烤箱"],
        "detail": "<h3>食材</h3><ul><li>整鸡 1只</li><li>蜂蜜、生抽、老抽、盐、五香粉/十三香、料酒、蚝油、蒜末、姜片、洋葱（塞入鸡肚）</li></ul><h3>步骤</h3><ol><li>鸡去头、爪及多余脂肪，用厨房纸吸干表面水分；在鸡皮上扎小孔以便入味</li><li>将调料混合后涂抹全身，重点按摩鸡胸和鸡腿部位；将部分调料塞入鸡肚，装入保鲜袋冷藏腌制4小时以上（隔夜更佳）</li><li>预热烤箱至200℃‌，将鸡置于烤架上，胸向上先烤30分钟定色；转180℃慢烤40分钟至熟 中途刷2次蜂蜜水增香提色，最后20分钟可调至200℃增强酥脆度 用锡纸包裹翅尖防糊，烤制35-50分钟（根据鸡的大小调整时间）</li></ol>"
    },
    {
        "id": 6,
        "name": "空气炸锅薯条",
        "image": "images/recipe6.png",
        "video": "videos/video5.mp4",
        "tags": ["小吃", "土豆", "快手菜"],
        "ingredients": ["土豆", "盐", "食用油"],
        "equipment": ["空气炸锅"],
        "detail": "<h3>食材</h3><ul><li>土豆 2个</li></ul><h3>步骤</h3><ol><li>空气炸锅预热5分钟</li><li>速冻薯条倒入炸篮</li><li>设定时间20分钟，200度，中途翻滚2次</li></ol>"
    },
    {
      "id": 7,
       "name": "番茄鸡蛋面",
       "image": "images/recipe7.png",
       "video": "videos/video6.mp4",
       "tags": ["主食", "快手菜", "家常"],
       "ingredients": ["面条", "番茄", "鸡蛋", "葱花", "盐", "生抽"],
       "equipment": ["炒锅", "煮锅"],
       "detail": "<h3>食材</h3><ul><li>面条 150g</li><li>番茄 2个</li><li>鸡蛋 2个</li><li>葱花 适量</li></ul><h3>步骤</h3><ol><li>番茄切块，鸡蛋打散备用</li><li>锅中倒油，炒熟鸡蛋盛出</li><li>再倒少许油，炒软番茄至出汁</li><li>加适量水烧开，放入面条煮熟</li><li>加炒好的鸡蛋、盐、生抽调味</li><li>出锅前撒葱花即可</li></ol>"
    },
    {
       "id": 8,
       "name": "凉拌黄瓜",
       "image": "images/recipe8.png",
       "video": "videos/video7.mp4",
       "tags": ["凉菜", "素食", "夏季"],
       "ingredients": ["黄瓜", "蒜末", "小米辣", "生抽", "香醋", "香油", "盐", "糖"],
       "equipment": ["菜刀", "菜板", "碗"],
       "detail": "<h3>食材</h3><ul><li>黄瓜 2根</li><li>蒜末 5瓣</li><li>小米辣 2个（可选）</li></ul><h3>步骤</h3><ol><li>黄瓜拍扁后切段，放入大碗</li><li>加入蒜末、切碎的小米辣</li><li>加2勺生抽、1勺香醋、少许盐和糖</li><li>淋上几滴香油，拌匀静置5分钟</li><li>装盘即可食用</li></ol>"
     }
]

def get_all_recipes():
    return ALL_RECIPES.copy()  # 避免原数据被改


# 新增：添加上传的菜谱到 ALL_RECIPES
def add_recipe(new_recipe):
    # 生成唯一ID：取现有最大ID + 1（避免ID重复）
    if ALL_RECIPES:  # 如果列表不为空
        max_id = max(recipe["id"] for recipe in ALL_RECIPES)
    else:
        max_id = 0  # 极端情况：列表为空时，ID从1开始
    new_recipe["id"] = max_id + 1

    # 补全菜谱的默认字段（按你的原有结构，避免缺失字段报错）
    # 1. 图片默认值（如果没上传图片，用默认图）
    if not new_recipe.get("image"):
        new_recipe["image"] = "images/default.png"  # 需在static/images放一张default.png
    # 2. 视频默认值（可选，没上传则为空字符串）
    new_recipe["video"] = new_recipe.get("video", "")
    # 3. tags、ingredients、equipment默认值（避免空列表报错）
    new_recipe["tags"] = new_recipe.get("tags", []).strip().split(",") if new_recipe.get("tags") else []
    new_recipe["ingredients"] = new_recipe.get("ingredients", []).strip().split(",") if new_recipe.get(
        "ingredients") else []
    new_recipe["equipment"] = new_recipe.get("equipment", []).strip().split(",") if new_recipe.get("equipment") else []
    # 4. detail字段（将食材、步骤转为HTML格式，匹配原有结构）
    new_recipe["detail"] = f"""
        <h3>食材</h3>
        <ul>{"".join([f"<li>{ing}</li>" for ing in new_recipe["ingredients"]])}</ul>
        <h3>烹饪器械</h3>
        <ul>{"".join([f"<li>{eq}</li>" for eq in new_recipe["equipment"]])}</ul>
        <h3>步骤</h3>
        <ol>{"".join([f"<li>{step}</li>" for step in new_recipe.get("steps", [])])}</ol>
    """

    # 将新菜谱添加到 ALL_RECIPES 列表
    ALL_RECIPES.append(new_recipe)
    return new_recipe["id"]  # 返回新菜谱的ID，用于后续提示