#pragma once
#include <iostream>
#include <fstream>
#include <chrono>
#include <cctype>
#include <map>

const std::string blue_text = "\033[38;2;0;175;255m";
const std::string head = "\033[1m\033[38;2;255;255;135m\033[48;2;95;95;255m";
const std::string orange = "\033[38;2;215;135;95m";
const std::string pink = "\033[38;2;215;135;135m"; // 255, 135, 135
const std::string green = "\033[38;2;106;153;82m";
const std::string purple = "\033[38;2;197;134;192m"; // 197, 134, 192
const std::string blue_key = "\033[38;2;86;156;214m";
const std::string blue = "\033[38;2;156;220;254m";
const std::string bg_dark = "\033[48;5;236m"; // 深灰背景
const std::string reset = "\033[0m";          // 初始颜色
const std::string white = "\033[37m";
const std::string cyan = "\033[36m";
const std::string yellow = "\033[38;2;215;215;175m";

std::map<std::string, std::string> keyword_colors{
    // C++关键字
    {"auto", blue_key},
    {"break", purple},
    {"case", purple},
    {"char", blue_key},
    {"const", blue_key},
    {"continue", purple},
    {"default", purple},
    {"do", purple},
    {"double", blue_key},
    {"else", purple},
    {"enum", blue_key},
    {"extern", purple},
    {"float", blue_key},
    {"for", purple},
    {"goto", purple},
    {"if", purple},
    {"int", blue_key},
    {"long", blue_key},
    {"register", blue_key},
    {"return", purple},
    {"short", blue_key},
    {"signed", blue_key},
    {"sizeof", blue_key},
    {"static", blue_key},
    {"struct", blue_key},
    {"switch", purple},
    {"typedef", blue_key},
    {"union", blue_key},
    {"unsigned", blue_key},
    {"void", blue_key},
    {"volatile", blue_key},
    {"while", purple},
    {"class", purple},
    {"namespace", purple},
    {"template", blue_key},
    {"typename", blue_key},

    // 类型和字面量
    {"true", cyan},
    {"false", cyan},
    {"nullptr", cyan},
    {"NULL", cyan},
    {"include", purple}, // 没有#防止python代码乱码
    {"define", purple},

    // // 字符串和注释
    // {"//", green},
    // {"/*", green},
    // {"*/", green},
    // {"\"", green},
    // {"'", green}
};

// 用于判断
int process_line(std::string &line, bool &inblock)
{
    size_t start = 0;
    // 跳过前导空格
    while (start < line.size() && std::isspace(line[start]))
    {
        ++start;
    }

    // 代码块
    if ((line.size() - start >= 3) &&
        line[start] == '`' &&
        line[start + 1] == '`' &&
        line[start + 2] == '`')
    {
        inblock = !inblock;
        return 4;
    }

    // 处于代码块中
    if (inblock)
    {
        std::string linecp = line;
        std::string token;
        size_t offset = 0; // 偏移量

        bool instring = false;

        for (int i = start; i != line.length(); ++i)
        {
            char c = line[i];
            // 处理注释
            if (c == '/' && !instring)
            {
                linecp.insert(i + offset, green);
                offset += green.length();
                break;
            }
            // 处理字符串
            if (c == '"' || c == '\'')
            {
                instring = !instring;
                if (instring)
                {
                    linecp.insert(i + offset, orange);
                    offset += orange.length();
                }
            }
            else if ((std::isalnum(c) || c == '_') && !instring)
            {
                token += c;
            }
            else if (!instring)
            {
                if (!token.empty()) // 检查字符串是否为空
                {
                    auto it = keyword_colors.find(token);
                    if (it != keyword_colors.end())
                    {
                        linecp.insert(i + offset - token.length(), it->second);
                        offset += it->second.length();
                    }
                    else
                    {
                        linecp.insert(i + offset - token.length(), blue);
                        offset += blue.length();
                    }
                    token.clear();
                }

                if (
                    c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == ',')
                {
                    linecp.insert(i + offset, yellow);
                    offset += yellow.length();
                }
                else
                {
                    linecp.insert(i + offset, pink);
                    offset += pink.length();
                }
            }
        }
        line = linecp;
        return 5;
    }
    // 检查是否以 "# "、"## " 或 "### " 开头
    if (start + 2 <= line.size() && (line.compare(start, 2, "# ") == 0))
    {
        line.insert(start, head);
        return 1; // "# "
    }
    if (start + 2 <= line.size() && (line.compare(start, 2, "- ") == 0))
    {
        line.replace(start, 1, "•"); // 替换为 UTF-8 字符串
        return 1;
    }
    if (start + 3 <= line.size() && (line.compare(start, 3, "## ") == 0))
    {
        line.insert(start, blue_text);
        return 2; // "## "
    }
    if (start + 4 <= line.size() && (line.compare(start, 4, "### ") == 0))
    {
        line.insert(start, blue_text);
        return 3; // "### "
    }
    if (start + 5 <= line.size() && (line.compare(start, 4, "#### ") == 0))
    {
        line.insert(start, blue_text);
        return 4; // "#### "
    }

    return 0;
}