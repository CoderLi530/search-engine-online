#include "cpp-httplib/httplib.h"
#include "searcher.hpp"
#include"log.hpp"

const std::string input = "./data/raw_html/raw.txt";
const std::string root_path = "./wwwroot";

int main()
{
    //错误：在浏览器中输入ip地址和端口号，但是没有跳转 -- 在华为云的服务器上配置对应端口的安全组规则，这样就可以跳转成功了 
    //在网页上建立搜索服务
    ns_searcher::Searcher search;
    search.InitSearcher(input);

    httplib::Server svr;
    //ip:8081直接这样不会跳转，我们得自己设置一个页面
    svr.set_base_dir(root_path.c_str());
    svr.Get("/s", [&search](const httplib::Request &req, httplib::Response &rsp){ //lambda函数要使用外部的对象，要传对象的地址
        //在网页上输入关键字进行搜索
        if (!req.has_param("word")) //没有输入关键字，进行提醒
        {
            rsp.set_content("必须要输入关键字", "text/plain; charset=utf-8"); //text/plain -- 文本类型，charset=utf-8 -- 文本格式
            return ; 
        }
        //输入关键字后，进行搜索
        std::string word = req.get_param_value("word");
        std::string json_string;
        //std::cout << "用户在搜索" << word << std::endl;
        LOG(NORMAL, "用户在搜索");
        search.Search(word, &json_string);
        rsp.set_content(json_string, "application/json"); //application/json -- json类型
        //rsp.set_content("hello world!", "text/plain; charset=utf-8");
    });
    svr.listen("0.0.0.0", 8081);
    return 0;
}