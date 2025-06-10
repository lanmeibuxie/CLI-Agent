# 简介
#### Cli Agent 是一个基于命令行的AI 聊天客户端，支持与 OpenAI/DeepSeek 等兼容 API 的模型进行对话，支持流式输出、代码高亮，markdown渲染
#### 本项目基于 C++ 实现，适用于 Windows/Linux 平台。相比于python实现,它更轻量,程序本体仅200-300kb.

## 项目结构

```
Lite Cli Agent/
├── include/
│   ├── client.hpp
│   ├── json.hpp
│   ├── render.hpp
│   └── utils.hpp
├── src/
│   ├── main.cpp
│   └── .config.json         # 运行后自动生成
├── .gitignore
├── Readme.md
```

## 编译与运行
### 依赖
- nlohmann/json（已集成）
- libcurl

### 编译

```bash
g++ main.cpp -o ag -O2 -lcurl
```

## 运行
### 命令行参数
| 参数         | 说明                     |
| ------------ | ------------------------ |
| -help, -h    | 显示帮助信息             |
| -reset, -r   | 重置配置文件             |
| -url         | 设置或修改 API URL       |
| -key         | 设置或修改 API 密钥      |
| -model       | 设置或修改模型名称       |
| -temperature | 设置或修改温度参数 (0-1) |

- 不带参数启动将进入对话模式。

## 配置文件
配置文件自动保存在程序所在目录, .config.json。
配置内容示例：
```json
{
    "key": "sk-xxxxxx", 
    "url": "https://api.deepseek.com/v1/chat/completions",
    "model": "deepseek-chat",
    "temperature": 1.0
}
```
## 对话模式
- 直接运行 ag.exe，即可进入多轮对话：

- 输入你的问题，回车发送。
- 输入 exit 退出对话。
## 其他说明
- 支持中文输入输出（需命令行窗口支持 UTF-8）。
- 支持流式输出和代码高亮。
- 支持通过参数快速修改配置，无需手动编辑配置文件。
## 常见问题
- 乱码/输入输出异常：请确保命令行窗口已切换到 UTF-8 编码（chcp 65001），并使用 Windows Terminal 或 VS Code 终端。
- 依赖缺失：请确保已正确安装 libcurl，并在编译时链接。