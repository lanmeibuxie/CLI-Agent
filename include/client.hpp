#pragma once

#include <curl/curl.h>
#include "json.hpp"
#include "render.hpp"

using json = nlohmann::json;

class ApiClient
{
public:
    // 获取最后一次响应内容
    std::string get_last_response() const
    {
        return last_response_;
    }

    // 非静态成员函数隐式包含this指针作为第一个参数,用于访问类的成员变量和方法
    // 导致与libcurl回调函数签名不匹配
    static size_t WriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
    {
        ApiClient *self = static_cast<ApiClient *>(userdata); // 获取对象指针
        return self->HandleData(ptr, size, nmemb);            // 调用成员函数处理实际逻辑
    }

    size_t HandleData(char *ptr, size_t size, size_t nmemb)
    {

        size_t totalSize = size * nmemb; // 计算当前数据块的大小

        // 将新数据追加到缓冲区
        buffer.append(ptr, totalSize);

        // 按行分割处理（保持兼容 \r\n 和 \n 换行）
        size_t pos = 0;
        while ((pos = buffer.find('\n')) != std::string::npos)
        {
            std::string line = buffer.substr(0, pos); // 提取一行数据
            buffer.erase(0, pos + 1);                 // 从缓冲区中移除已处理的数据

            // 处理空行（可能会出现在chunk边界）
            if (line.empty())
                continue;

            // 移除可能的回车符（兼容 Windows 的 \r\n 换行符）
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            // 处理数据行
            if (line.substr(0, 6) == "data: ") // 检查是否是以 "data: " 开头的行
            {
                std::string json_str = line.substr(6); // 提取 JSON 数据部分

                // 处理结束标记
                if (json_str == "[DONE]") // 如果接收到 "[DONE]"，表示流式响应结束
                {
                    std::cout << std::endl;

                    return totalSize;
                }

                // 解析JSON
                try
                {
                    json chunk = json::parse(json_str); // 解析 JSON 数据
                    if (chunk.contains("choices"))      // 检查是否包含 "choices" 字段
                    {
                        auto &content = chunk["choices"][0]["delta"]["content"];
                        if (!content.is_null() && content.get<std::string>() != "")
                        {
                            std::string text = content.get<std::string>();

                            size_t pos = text.find('\n');
                            if (pos != std::string::npos)
                            {
                                std::string it = text.substr(0, pos + 1);
                                current_line += it; // 取到\n
                                std::cout << it << std::flush;
                                last_response_ += current_line;
                                if (process_line(current_line, inblock) != 0)
                                {
                                    // 删除上一行输出
                                    std::cout << "\033[A" // 光标上移一行
                                              << "\033[K" // 清除当前行
                                              << std::flush;
                                    // 在换行前重置颜色
                                    current_line.insert(current_line.size() - 1, "\033[0m");
                                    std::cout << current_line;
                                }
                                current_line.clear();
                                it = text.substr(pos + 1);
                                current_line += it;
                                std::cout << it << std::flush;
                            }
                            else
                            {
                                current_line += text;
                                // 输出解析后的内容
                                std::cout << text << std::flush;
                            }
                        }
                    }
                }
                catch (const json::exception &e)
                {
                    // 如果 JSON 解析失败，打印错误信息和原始数据
                    std::cerr << "\nJSON解析错误: " << e.what()
                              << "\n原始数据: " << json_str << std::endl;
                }
            }
        }

        return totalSize; // 返回已处理的数据大小
    }

    // 构造函数
    ApiClient(const std::string &base_url, const std::string &api_key)
        : base_url_(base_url), api_key_(api_key), curl_(nullptr)
    {
        // 全局初始化 (应只调用一次)
        static bool global_initialized = []()
        {
            return curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK;
        }();

        if (!global_initialized)
        {
            throw std::runtime_error("Failed to initialize libcurl!");
        }

        // 创建实例级句柄
        curl_ = curl_easy_init();
        if (!curl_)
        {
            throw std::runtime_error("Failed to create CURL handle!");
        }
    }
    // 析构函数
    ~ApiClient()
    {
        if (curl_)
        {
            curl_easy_cleanup(curl_);
        }
    }

    // 发送请求
    void send_request(std::string request_body_str)
    {
        if (!curl_)
        {
            throw std::runtime_error("CURL handle is not initialized!");
        }

        // 设置请求选项
        curl_easy_setopt(curl_, CURLOPT_URL, base_url_.c_str()); // 设置请求的URL

        // 设置请求头
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key_).c_str());
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl_, CURLOPT_POST, 1L);                             // 启用POST方法
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, request_body_str.c_str()); // 设置POST数据

        // 设置流式回调
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, &ApiClient::WriteCallback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, this); // 传递this指针

        // 执行请求
        CURLcode res = curl_easy_perform(curl_);
        if (res != CURLE_OK)
        {
            throw std::runtime_error("Request failed: " + std::string(curl_easy_strerror(res)));
        }

        // 清理请求头
        curl_slist_free_all(headers);
    }

    std::string base_url_;
    std::string api_key_;
    std::string buffer;       // 缓冲区
    std::string current_line; // 当前行
    std::string last_response_;
    bool inblock = false; // 是否处于代码块中
    CURL *curl_;          // 保存句柄
};