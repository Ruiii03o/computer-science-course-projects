import os
import requests
from urllib.parse import urlparse

def download_video(video_url, save_dir="downloads"):
    """
    下载视频到指定文件夹

    参数:
        video_url (str): 视频的URL地址
        save_dir (str): 保存视频的文件夹路径，默认为"downloads"
    """
    try:
        # 创建保存目录（如果不存在）
        if not os.path.exists(save_dir):
            os.makedirs(save_dir)

        # 发送HTTP请求获取视频数据
        print(f"开始下载: {video_url}")
        response = requests.get(video_url, stream=True, timeout=10)
        response.raise_for_status()  # 检查请求是否成功

        # 从URL中提取文件名
        parsed_url = urlparse(video_url)
        file_name = os.path.basename(parsed_url.path)

        # 如果URL中没有文件名，使用默认名称
        if not file_name:
            file_name = "video.mp4"

        # 完整的保存路径
        save_path = os.path.join(save_dir, file_name)

        # 写入视频文件
        with open(save_path, 'wb') as f:
            for chunk in response.iter_content(chunk_size=1024 * 1024):  # 1MB chunks
                if chunk:  # 过滤掉保持连接的空块
                    f.write(chunk)
                    f.flush()

        print(f"下载完成，保存路径: {save_path}")
        return save_path

    except requests.exceptions.RequestException as e:
        print(f"下载失败: {e}")
    except Exception as e:
        print(f"发生错误: {e}")


if __name__ == "__main__":
    # 示例：下载视频到指定文件夹
    video_url = ""  # 替换为实际的视频URL
    save_directory = "C:/Users/q3250/Desktop/Recipe/static/videos"  # 指定保存文件夹

    download_video(video_url, save_directory)