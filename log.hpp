#pragma once

#include<iostream>
#include<string>
#include<ctime>

#define NOAMAL 1
#define WARNING 2
#define DEBUG 3
#define FATAL 4

//#LEVEL：打印宏，__FILE__和__LINE__获取文件和行数
#define LOG(LEVEL, MESSAGE) log(#LEVEL, MESSAGE, __FILE__, __LINE__)

void log (std::string level, std::string message, std::string file, int line)
{
    time_t t = time(nullptr);
    struct tm* tm = localtime(&t);
    std::cout << "[" << level << "]";
    printf("[%d-%d-%d %d:%d:%d]", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,tm->tm_hour, tm->tm_min, tm->tm_sec); 
    std::cout << "[" << message << "]" << "[" << file << " : " << line << "]" << std::endl;
}