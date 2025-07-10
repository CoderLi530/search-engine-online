#include "searcher.hpp"
#include<string>
#include<iostream>

//解析后的文件
const std::string input = "data/raw_html/raw.txt";

int main()
{
    ns_searcher::Searcher *search = new ns_searcher::Searcher();
    search->InitSearcher(input);

    //输入关键字进行搜索
    std::string query;
    std::string json_string;
    char buffer[1024];
    while (true)
    {
        //cin是以空格作为分隔符的，输入aaa bbb ccc会被分成三部分，但我们想要搜索这个整体，所以不用cin
        std::cout << "Please Enter Your Query: ";
        //std::cin >> query;
        fgets(buffer, sizeof(buffer), stdin);
        //fgets会把 换行 也添加进去，进行覆盖
        buffer[strlen(buffer) - 1] = 0;
        query = buffer;
        //开始查找内容
        std::cout << query << std::endl;
        search->Search(query, &json_string);
        std::cout << json_string << std::endl;
    }
    return 0;
}