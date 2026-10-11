from flask import Flask, render_template, request
import datetime
import random
from config import Config  # 导入配置
from utils.recipe_utils import get_recipe_by_id, get_random_recipes, filter_recipes  # 导入工具函数
from data.recipes import get_all_recipes,add_recipe  # 新增：导入获取所有食谱的函数
from utils.recipe_generator import generate_recipe
import os
from werkzeug.utils import secure_filename

app = Flask(__name__)
app.config.from_object(Config)  # 加载配置
if not hasattr(app.config, 'UPLOAD_IMAGE_EXTENSIONS'):
    app.config['UPLOAD_IMAGE_EXTENSIONS'] = {'png', 'jpg', 'jpeg', 'gif'}
if not hasattr(app.config, 'UPLOAD_VIDEO_EXTENSIONS'):
    app.config['UPLOAD_VIDEO_EXTENSIONS'] = {'mp4', 'mov', 'avi'}

@app.route('/')
def index():
    # 调用工具函数，获取3个随机食谱
    all_recipes = get_all_recipes()  # 调用函数获取所有食谱
    random.shuffle(all_recipes)
    return render_template('index.html', recipes=all_recipes)


@app.route('/recipe/<int:recipe_id>')
def recipe_detail(recipe_id):
    recipe = get_recipe_by_id(recipe_id)
    if not recipe:
        return app.config["NO_RECIPE_MSG"], 404
    return render_template('recipe_detail.html', recipe=recipe)


@app.route('/about')
def about():
    return render_template('about.html')


@app.route('/contact', methods=['GET', 'POST'])
def contact():
    if request.method == 'POST':
        name = request.form.get('name', '匿名用户')
        email = request.form.get('email', '未提供')
        message = request.form.get('message', '')
        time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        # 验证留言内容
        if not message.strip():
            return render_template('contact.html', error="留言内容不能为空！")

        # 按配置路径保存留言
        with open(app.config["MESSAGE_FILE_PATH"], 'a', encoding='utf-8') as f:
            f.write(f"时间：{time}\n姓名：{name}\n邮箱：{email}\n留言：{message}\n{'-' * 50}\n")

        return render_template('contact.html', success=True)

    return render_template('contact.html')


# 随机抽取页面路由（已修复all_recipes导入）
@app.route('/random')
def random_recipe():
    all_recipes = get_all_recipes()  # 调用函数获取所有食谱
    return render_template('random.html', all_recipes=all_recipes)


@app.route('/filter')
def filter_recipes():
    all_recipes = get_all_recipes()  # 调用函数获取所有食谱
    return render_template('filter.html', all_recipes=all_recipes)

# 生成菜谱页面
@app.route('/generate', methods=['GET', 'POST'])
def generate():
    recipe = None
    keyword = ""
    if request.method == 'POST':
        keyword = request.form.get('keyword', '').strip()
        if keyword:
            recipe = generate_recipe(keyword)  # 调用API生成菜谱
    return render_template('generate.html', keyword=keyword, recipe=recipe)

def init_upload_folders():
    # 创建图片和视频上传目录
    image_folder = os.path.join(app.root_path, 'static', 'images')
    video_folder = os.path.join(app.root_path, 'static', 'videos')
    os.makedirs(image_folder, exist_ok=True)
    os.makedirs(video_folder, exist_ok=True)

# 新增：定义 allowed_file 辅助函数（关键！解决报错）
def allowed_file(filename, allowed_extensions):
    """检查文件扩展名是否在允许的列表中"""
    # 确保文件名有扩展名，且扩展名在允许列表内（不区分大小写）
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in allowed_extensions
# app.py 中已有的 /upload 路由（保持不变）
@app.route('/upload', methods=['GET', 'POST'])
def upload_recipe():
    if request.method == 'POST':
        # 1. 获取表单数据（处理步骤空值）
        recipe_name = request.form.get('name', '').strip()
        tags = request.form.get('tags', '').strip()
        ingredients = request.form.get('ingredients', '').strip()
        equipment = request.form.get('equipment', '').strip()
        steps = [s.strip() for s in request.form.get('steps', '').split('\n') if s.strip()]

        # 2. 验证必填项
        missing_fields = []
        if not recipe_name:
            missing_fields.append("菜谱名称")
        if not ingredients:
            missing_fields.append("食材")
        if not steps:
            missing_fields.append("步骤")
        if missing_fields:
            error_msg = f"以下字段不能为空：{', '.join(missing_fields)}"
            return render_template('upload.html', error=error_msg)  # 错误时返回上传页并提示

        # 3. 处理图片上传（含默认图）
        image_path = "images/default.png"
        if 'image' in request.files:
            image_file = request.files['image']
            if image_file.filename != '':
                if allowed_file(image_file.filename, app.config['UPLOAD_IMAGE_EXTENSIONS']):
                    safe_filename = secure_filename(image_file.filename)
                    save_path = os.path.join(app.root_path, 'static', 'images', safe_filename)
                    image_file.save(save_path)
                    image_path = f"images/{safe_filename}"
                else:
                    return render_template('upload.html', error="图片格式不支持！仅允许png、jpg、jpeg、gif")

        # 4. 处理视频上传
        video_path = ""
        if 'video' in request.files:
            video_file = request.files['video']
            if video_file.filename != '':
                if allowed_file(video_file.filename, app.config['UPLOAD_VIDEO_EXTENSIONS']):
                    safe_filename = secure_filename(video_file.filename)
                    save_path = os.path.join(app.root_path, 'static', 'videos', safe_filename)
                    video_file.save(save_path)
                    video_path = f"videos/{safe_filename}"
                else:
                    return render_template('upload.html', error="视频格式不支持！仅允许mp4、mov、avi")

        # 5. 组装并保存菜谱
        new_recipe = {
            "name": recipe_name,
            "image": image_path,
            "video": video_path,
            "tags": tags,
            "ingredients": ingredients,
            "equipment": equipment,
            "steps": steps
        }
        new_recipe_id = add_recipe(new_recipe)

        # 6. 上传成功后跳转到成功提示页（下一步创建）
        return render_template('upload_success.html',
                               recipe_id=new_recipe_id, recipe_name=recipe_name)

    # GET 请求：显示上传页面（即刚创建的 upload.html）
    return render_template('upload.html')
if __name__ == '__main__':
    app.run(host=app.config["HOST"], port=app.config["PORT"], debug=app.config["DEBUG"])
