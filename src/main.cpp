#include "../include/json.hpp"
#include "../include/utils.hpp"
#include "../include/client.hpp"
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

// 所有支持的参数
std::vector<std::string> params{
    "-help", "-h", "-reset", "-r", "-url", "-key", "-model", "-temperature"};

#ifdef _WIN32
// GBK → UTF-8
std::string gbk_to_utf8(const std::string &gbk_str)
{
    int wlen = MultiByteToWideChar(CP_ACP, 0, gbk_str.c_str(), -1, NULL, 0);
    wchar_t *wbuf = new wchar_t[wlen];
    MultiByteToWideChar(CP_ACP, 0, gbk_str.c_str(), -1, wbuf, wlen);

    int ulen = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, NULL, 0, NULL, NULL);
    char *ubuf = new char[ulen];
    WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, ubuf, ulen, NULL, NULL);

    std::string utf8_str(ubuf);
    delete[] wbuf;
    delete[] ubuf;
    return utf8_str;
}
#endif

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::string api_url, api_key, model, content;
    float temperature = 1.0f; // 默认温度

#ifdef _WIN32
    // 将参数转换为字符串
    content = gbk_to_utf8(params_to_string(argc, argv));
#else
    content = params_to_string(argc, argv);
#endif

    // 没有参数则进入对话模式
    bool dialog = (argc == 1 ? true : false);

    if (argc == 2 && find_in_vector(params, argv[1]))
    {
        std::string arg = argv[1];
        if (!arg.empty() && arg[0] == '-')
        {
            arg = arg.substr(1); // 移除第一个 '-'
        }

        if (arg == "help" || arg == "h")
        {
            std::cout << "Agent  - 帮助信息\n";
            std::cout << "用法: ag [选项]\n\n";
            std::cout << "选项:\n";
            std::cout << "  -help, -h           显示帮助信息\n";
            std::cout << "  -reset, -r          重置配置文件\n";
            std::cout << "  -url                设置或修改 API URL\n";
            std::cout << "  -key                设置或修改 API 密钥\n";
            std::cout << "  -model              设置或修改模型名称\n";
            std::cout << "  -temperature        设置或修改温度参数 (0.0 - 1.0)\n\n";
            std::cout << "不带参数启动会进入对话模式\n";
        }
        else if (arg == "reset" || arg == "r")
        {
            set_conf(api_url, api_key, model);
        }
        else if (arg == "url" || arg == "key" || arg == "model" || arg == "temperature")
        {
            modify_conf(arg);
        }

        return 0;
    }

    if (!load_config(api_url, api_key, model, temperature))
    {
        std::cout << "配置文件不存在或损坏，请输入API URL和KEY:\n";
        // 输入Url和key
        set_conf(api_url, api_key, model);
    }

    // 创建API客户端
    ApiClient client(api_url, api_key);

    curl_easy_setopt(client.curl_, CURLOPT_SSL_VERIFYPEER, 0L); // 不验证对端证书
    curl_easy_setopt(client.curl_, CURLOPT_SSL_VERIFYHOST, 0L); // 不验证主机名

    if (!dialog)
    {
        // 请求体
        json request_body = {
            {"model", model},
            {"messages", {{{"role", "user"}, {"content", content}}}},
            {"temperature", temperature},
            {"stream", true}};

        std::cout << "\n🤖 " << model << ": " << std::endl;

        client.send_request(request_body.dump()); // 发送请求
        return 0;
    }
    else
    {
        std::vector<json> message_history;
        std::string input;
        bool first_round = true;

        while (true)
        {
            // 显示提示符
            if (first_round)
            {
                std::cout << "\nYou: " << std::endl;
                first_round = false;
            }
            else
            {
                std::cout << "\nYou (输入 'exit' 退出): ";
            }

            // 获取用户输入
            std::getline(std::cin, input);

#ifdef _WIN32
            input = gbk_to_utf8(input); // Windows下转换编码
#endif

            // 退出条件
            if (input == "exit")
            {
                std::cout << "结束对话.\n";
                break;
            }

            // 添加用户消息到历史
            message_history.push_back({{"role", "user"}, {"content", input}});

            // 构建请求体（包含完整对话历史）
            json request_body = {
                {"model", model},
                {"messages", message_history}, // 包含所有历史消息
                {"temperature", temperature},
                {"stream", true}};

            try
            {
                // 发送请求并获取响应
                std::cout << "\n🤖 " << model << ": " << std::endl;

                client.send_request(request_body.dump());

                // 添加助理回复到历史
                std::string assistant_response = client.get_last_response();
                if (!assistant_response.empty())
                {
                    message_history.push_back({{"role", "assistant"},
                                               {"content", assistant_response}});
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "\n请求失败: " << e.what() << std::endl;
                break;
            }
        }
    }
}