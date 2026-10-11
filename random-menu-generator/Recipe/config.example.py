class Config:
    DEBUG = True
    HOST = "0.0.0.0"
    PORT = 5000
    MESSAGE_FILE = "messages.txt"
    NO_RECIPE_MSG = "没找到这个食谱~"
    MESSAGE_FILE_PATH = "data/messages.txt"

    DEEPSEEK_API_KEY = "YOUR_DEEPSEEK_API_KEY"
    DEEPSEEK_API_URL = "https://api.deepseek.com/v1/chat/completions"
    MODEL_NAME = "deepseek-chat"