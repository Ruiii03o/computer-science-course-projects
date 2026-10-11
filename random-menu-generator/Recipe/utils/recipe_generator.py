import requests
from config import Config
def generate_recipe(keyword):
    """调用DeepSeek生成菜谱"""
    headers = {
        "Authorization": f"Bearer {Config.DEEPSEEK_API_KEY}",
        "Content-Type": "application/json"
    }
    # 提示词：明确生成格式
    prompt = f"""根据关键词“{keyword}”生成一个详细菜谱，包含：
    1. 菜谱名称（醒目）
    2. 食材清单（带用量）
    3. 烹饪步骤（分点说明）
    用简洁中文输出，不要多余内容。"""

    data = {
        "model": Config.MODEL_NAME,
        "messages": [{"role": "user", "content": prompt}]
    }

    try:
        response = requests.post(
            Config.DEEPSEEK_API_URL,
            headers=headers,
            json=data,
            timeout=50
        )
        result = response.json()
        return result["choices"][0]["message"]["content"]
    except Exception as e:
        return f"生成失败：{str(e)}"