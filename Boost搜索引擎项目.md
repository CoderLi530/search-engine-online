# Boost搜索引擎项目

## 1. 项目的相关背景

> - 公司：百度、搜狗、360搜索、头条新闻客户端 – 我们自己实现是不可能的！成本非常高
>
>
> 我们要实现的是：
>
> - 站内搜索：搜索的数据更垂直（和数据直接相关），数据量更小
>
> - boost

![image-20250515202913325](C:\Users\Haope\AppData\Roaming\Typora\typora-user-images\image-20250515202913325.png)

## 2. 搜索引擎的相关宏观原理

![image-20250703190726778](C:\Users\Haope\AppData\Roaming\Typora\typora-user-images\image-20250703190726778.png)

## 3. 搜索引擎技术栈和项目环境

> - 技术栈：C/C++、C++11，STL，准标准库Boost，JSoncpp，cppjieba，cpp-httplib

> - 项目环境：Centos 7云服务器，vim/gcc（g++）/Makefile，vs2022 and vscode

## 4. 正排索引 vs 倒排索引 - 搜索引擎具体原理

> - 文档1：我下载了游乐园游戏
>
> - 文档2：我去了游乐园

**正排索引：从文档ID找到文档内容（文档的关键字）**

| 文档ID | 文档内容           |
| ------ | ------------------ |
| 1      | 我下载了游乐园游戏 |
| 2      | 我去了游乐园       |

**对目标文档进行分词（目的：方便建立倒排索引和查找）：**

> - 文档1（我下载了游乐园游戏）：我/下载/游乐园/游戏/游乐园游戏
>
> - 文档2（我去了游乐园）：我/去/游乐园

停止词：了，的，吗，a，the…，一般我们在分词的时候可以不考虑

**倒排索引：根据文档内容，进行分词，整理不重复的各个关键字，对应联系到文档ID的方案**

| 关键字（具有唯一性） | 文档ID，weight（权重） |
| -------------------- | ---------------------- |
| 我                   | 文档1，文档2           |
| 下载                 | 文档1                  |
| 游乐园               | 文档1，文档2           |
| 游戏                 | 文档1                  |
| 游乐园游戏           | 文档1                  |
| 去                   | 文档2                  |

**模拟一次查找的过程：**

用户输入：游乐园 ——> 倒排索引查找 ——> 提取出文档（1，2）——> 根据正排索引 ——> 找到文档的内容 ——> `title + content（desc）+ url`文档结果进行摘要 ——> 构建响应结果

## 5. 编写数据去标签与数据清洗的模块`Parser`

> [boost官网](https://www.boost.org/)
>
> 在官网下载tar.gz版本后，在Linux上进行解压，保存boost_1_88_0/doc/html（当前版本为1_88_0）目录下的html文件，使用它来建立索引

**去标签**

```cpp
[lzh@hcss-ecs-1552 boost_searcher]$ ll
total 4
drwxrwxr-x 3 lzh lzh 4096 Jul  4 09:30 data
-rw-rw-r-- 1 lzh lzh    0 Jul  4 09:34 parser.cpp
//我们要把原始数据 --> 去标签后的数据
  
//一个html文档部分内容：    
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>Chapter 30. Boost.Proto</title>
<link rel="stylesheet" href="../../doc/src/boostbook.css" type="text/css">
<meta name="generator" content="DocBook XSL Stylesheets V1.79.1">
<link rel="home" href="index.html" title="The Boost C++ Libraries BoostBook Documentation Subset">
<link rel="up" href="libraries.html" title="Part I. The Boost C++ Libraries (BoostBook Subset)">
<link rel="prev" href="doxygen/namespaceboost_1_1property__tree_1_1xml__parser_1a97d3d5235ef9b235d70110bac7e4f497.html" title="Function template write_xml">
<link rel="next" href="proto/users_guide.html" title="Users' Guide">
<meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body bgcolor="white" text="black" link="#0000FF" vlink="#840084" alink="#0000FF">
<table cellpadding="2" width="100%"><tr>
<td valign="top"><img alt="Boost C++ Libraries" width="277" height="86" src="../../boost.png"></td>
<td align="center"><a href="../../index.html">Home</a></td>
<td align="center"><a href="../../libs/libraries.htm">Libraries</a></td>
<td align="center"><a href="http://www.boost.org/users/people.html">People</a></td>
<td align="center"><a href="http://www.boost.org/users/faq.html">FAQ</a></td>
<td align="center"><a href="../../more/index.htm">More</a></td>

// < >：html的标签，这个标签对我们进行搜索是没有价值的，需要去掉这些标签，一般标签是成对出现的
    
drwxrwxr-x 3 lzh lzh 4096 Jul  4 17:28 input
[lzh@hcss-ecs-1552 data]$ mkdir raw_html
[lzh@hcss-ecs-1552 data]$ ll
total 8
drwxrwxr-x 3 lzh lzh 4096 Jul  4 17:28 input    //原始html文档
drwxrwxr-x 2 lzh lzh 4096 Jul  4 17:42 raw_html //去标签后的html文档
[lzh@hcss-ecs-1552 data]$ ls -Rl | grep -E '*.html' | wc -l
9822 //html的数量

目标：把每个文档都去标签，然后写入到同一个文件中。每个文档内容不需要任何\n，文档和文档之间使用 \3 区分（也可以使用ASCII表中其它的控制字符）
```

**编写parser**

```cpp
#include<iostream>
#include<string>
#include<vector>

const std::string src_path = "data/input/html/"; //存放所有的html网页 
const std::string output = "data/raw_html/raw.txt"; //去标签并且整理好内容放进raw.txt中

typedef struct DocInfo{
    std::string title; //文档标题
    std::string content; //文档内容
    std::string url; //文档在官网中的url
}DocInfo_t;  

//const &：输入
//*：输出
//&：输入输出
bool EnumFile(const std::string &src_path, std::vector<std::string> *files_list)
{
    return true;
}
bool ParseHtml(const std::vector<std::string> &files_list, std::vector<DocInfo_t> *results)
{
    return true;
}
bool SaveHtml(const std::vector<DocInfo_t> &results, const std::string &output)
{
    return true;
}

int main()
{
    //1. 递归式的把每个html文件名带路径，保存到files_list中，方便后期进行一个一个的文件读取
    std::vector<std::string> files_list;
    if (!EnumFile(src_path, &files_list))
    {
        std::cerr << "enum file name error!" << std::endl; //打印错误信息
        return 1; //失败就不需要往下走了
    }
    //2. 按照files_list读取每个文件的内容，并进行解析。解析的内容：标题（title），内容（content），链接（url）
    std::vector<DocInfo_t> results;
    if (!ParseHtml(files_list, &results))
    {
        std::cerr << "parse html error" << std::endl;
        return 2;
    }
    //3. 把解析完毕的各个文件写入到output中，将\3作为每个文档的分隔符
    if (!SaveHtml(results, output))
    {
        std::cerr << "save html error" << std::endl;
        return 3;
    }
    return 0;
}
```

```cpp
[lzh@hcss-ecs-1552 ~]$ sudo yum install -y boost-devel //boost开发库的安装
```

**提取title**

![image-20250706204620512](C:\Users\Haope\AppData\Roaming\Typora\typora-user-images\image-20250706204620512.png)

> 找到`<title>`和`</title>`的位置，两个位置之间的内容是title，注意[begin, end)

**提取内容 —— 去标签**

![image-20250706204924506](C:\Users\Haope\AppData\Roaming\Typora\typora-user-images\image-20250706204924506.png)

> 进行遍历时，遇到`>`说明遍历标签结束，开始遍历内容；遇到`<`说明遍历内容结束，开始遍历标签

**构建URL**

```cpp
boost库的官方文档，和我们下载的文档，是有路径的对应关系
    
官方url：https://www.boost.org/doc/libs/1_78_0/doc/html/accumulators.html

下载后url样例：1_78_0/doc/html/accumulators.html

拷贝到项目的样例：data/input/accumulators.html
    
url_head = https://www.boost.org/doc/libs/1_78_0/doc/html
url_tail = [data/input](删除) /accumulators.html --> url_tail = /accumulators.html
 
url = url_head + url_tail; //形成了一个官网链接
```

将解析文件内容写入文件中

```cpp
类似：title\3content\3url\n title\3content\3url\n ……
方便我们 getline(ifstream, line)，直接获取文档的全部内容：title\3content\3url
```

## 6. 编写建立索引的模块`Index`

`index.hpp`**基本结构**

```cpp
#pragma once

#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>

namespace ns_index
{
    struct DocInfo
    {
        std::string title;   //文档的标题
        std::string content; //文档的内容
        std::string url;     //官网url
        uint64_t doc_id;          //文档的ID。uint64_t --> unsigned long int
    };
    struct InvertedElem
    {
        uint64_t doc_id;       //文档ID
        std::string word; //关键字
        int weight;       //权重
    };

    //倒排拉链
    typedef std::vector<InvertedElem> InvertedList;
    class index
    {
        private:
            std::vector<DocInfo> forward_index; //正排索引的数据结构用数组，数组下标和文档ID一样，查找方便
            std::unordered_map<std::string, InvertedList> inverted_index; //倒排索引是一个关键字和一组（个）InvertedElem对应[关键字和倒排拉链的映射关系]
        public:
            index()
            {

            }
            ~index()
            {

            }
        public:
            //根据doc_id找到文档内容
            DocInfo *GetForwardIndex(uint64_t doc_id)
            {
                return nullptr;
            }
            
            //根据关键字string, 找到倒排拉链
            InvertedElem *GetInvertedIndex(const std::string &word)
            {
                return nullptr;
            }

            //根据去标签，格式化之后的文档，构建正排和倒排索引
            bool BuildIndex(const std::string &input) //把parse处理完毕的数据给我
            {
                return true;
            }
    };
}
```

**建立正排的基本代码**

```cpp
DocInfo *BuildForwardIndex(const std::string &line)
{
    //已经获得一行内容，要进行切割字符串
    //line -> title, content, url 这三个部分是由\3分隔开的
    std::vector<std::string> results;
    const std::string sep = "\3";
    ns_util::StringUtil::CutString(line, &results, sep);
    //切割之后的vector不是三份，说明切割失败
    if (results.size() != 3)
    {
        return nullptr;
    }

    //将results内容放入doc中
    DocInfo doc;
    doc.title = results[0];
    doc.content = results[1];
    doc.url = results[2];
    doc.doc_id = results.size(); //添加第一个文档，ID为0，田间第二个文档，ID为1......

    //将doc放进forward_index中
    forward_index.push_back(doc);
    return &doc;
}

```

**建立倒排**

```cpp
struct DocInfo
{
    std::string title;   //文档的标题
    std::string content; //文档的内容
    std::string url;     //官网url
    uint64_t doc_id;          //文档的ID。uint64_t --> unsigned long int
};   
struct InvertedElem
{
    uint64_t doc_id;       //文档ID
    std::string word; //关键字
    int weight;       //权重
};

//倒排拉链
typedef std::vector<InvertedElem> InvertedList;
std::unordered_map<std::string, InvertedList> inverted_index; //倒排索引是一个关键字和一组（个）InvertedElem对应[关键字和倒排拉链的映射关系]


//文档：
title：吃葡萄
content：吃葡萄不吐葡萄皮
url：http://xxxxx
doc_id：123
    
根据文档内容，形成一个或多个InvertedElem(倒排拉链)
因为我们是对一个一个文档进行处理的，一个文档可能会有多个词，都对应到当前的文档ID（doc_id）
    
1. 需要对title && content进行分词 -- 使用jieba分词
title：吃/葡萄/吃葡萄（title_word）
content：吃/葡萄/不吐/葡萄皮（content_word）
    
词和文档的相关性（词频：在标题中出现的词，可以认为相关性更高一点，在内容中出现的词相关性就低一些）
2. 词频统计
struct word_cnt
{
	title_cnt;
    content_cnt;
};
unordered_map<std::string, word_cnt> word_cnt; //保存关键字和在标题与内容中出现的次数

for (auto& word : title_word)
{	
    word_cnt[word].title_cnt++; //吃（1）/葡萄（1）/吃葡萄（1）
}

for (auto& word : content_word)
{
    word_cnt[word].content_cnt++; //吃（1）/葡萄（1）/不吐（1）/葡萄皮（1）
}

知道了在文档中，标题和内容的每个词出现的次数
3. 自定义相关性
for (auto& word : word_cnt)
{
    //具体一个词和123文档的对应关系，当有不同的词，指向同一个文档的时候，首先显示谁？由相关性决定
    struct InvertedElem elem;
 	elem.doc_id = 123;
    elem.word = word.first;
    elem.weight = 10 * word.title_cnt + word.content_cnt; //我们就简单写相关性了
    
    //一个倒排元素完成，放到倒排拉链中
    inverted_index[word.first].push_back(elem);
}


//jieba的使用--cppjieba
获取链接：git clone https://gitcode.net/mirrors/yanyiwu/cppjieba.git
我们要自己执行：cp -rf deps/limonp include/cppjieba/
    
//如下是样例代码：
#include "inc/cppjieba/Jieba.hpp"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

const char* const DICT_PATH = "./dict/jieba.dict.utf8";
const char* const HMM_PATH = "./dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "./dict/user.dict.utf8";
const char* const IDF_PATH = "./dict/idf.utf8";
const char* const STOP_WORD_PATH = "./dict/stop_words.utf8";
int main(int argc, char** argv) {
    cppjieba::Jieba jieba(DICT_PATH,
                          HMM_PATH,
                          USER_DICT_PATH,
                          IDF_PATH,
                          STOP_WORD_PATH);
    vector<string> words;
    string s;
    s = "⼩明硕⼠毕业于中国科学院计算所，后在⽇本京都⼤学深造";
    cout << s << endl;
    cout << "[demo] CutForSearch" << endl;
    jieba.CutForSearch(s, words);
    cout << limonp::Join(words.begin(), words.end(), "/") << endl;
    return EXIT_SUCCESS;
}

//编写倒排索引的代码
//注意：建立倒排索引的时候，要忽略大小写
```

## 7. 编写搜索引擎模块`Searcher`

**基本代码结构**

```cpp
#pragma once

#include"index.hpp"

namespace ns_searcher
{
    class Searcher
    {
        private:
            ns_index::Index *index; //供系统进行查找的索引
        private:
            Searcher();
            ~Searcher();
        public:
            void InitSearcher(const std::string &input)
            {
                //1. 获取或者创建index对象
                //2. 根据index对象创建索引

            }

            //query：搜索关键字
            //json_string：返回给用户浏览器的搜索结果
            void Search(const std::string &query, std::string *json_string)
            {
                //1. [分词]：对我们query进行分词
                //2. [触发]：根据分词后的各个“词”，进行index查找
                //3. [合并排序]：汇总查找结果，按照相关性（weight）降序排序
                //4. [构建]：根据查找结果，构建json串 -- jsoncpp
            }
    };
}
```

**安装**`jsoncpp`

```cpp
[lzh@hcss-ecs-1552 ~]$ sudo yum install -y jsoncpp-devel
[sudo] password for lzh: 
Loaded plugins: fastestmirror
Loading mirror speeds from cached hostfile
base                                                                
centos-sclo-rh                                                  
epel
extras                                                                    
updates                                                                   
Package jsoncpp-devel-0.10.5-2.el7.x86_64 already installed and latest version
Nothing to do //已经安装好了

```

**获取摘要**

> 我们这里就取关键字位置前50个字节，关键字后100个字节

```cpp
//部分代码

const int prev_step = 50;
const int next_step = 100;

//找到start和end的位置
int start = 0;
int end = html_content.size() - 1;
//开始位置前50个字节存在，那么设置该位置，不存在就是0
//最终位置后100个字节存在，那么设置该位置，不存在就是内容最后
if (pos > start + prev_step) start = pos - prev_step;
if (pos < end - next_step) end = pos + next_step;
```

## 8. 编写`http_server`模块

编写`http_server`

> cpp-httplib库：https://gitee.com/welldonexing/cpp-httplib/tree/v0.7.15
>
> **注意**：
>
> 1. 使用cpp-httplib库时需要使用新版本的gcc，centos7默认gcc 4.8.5
>
> 2. 如果使用最新的cpp-httplib，gcc不是最新的话，运行时可能会报错，所以这个cpp-httplib链接是0.7.15版本的，可以使用
> 3. 下载zip文件，在Linux上unzip，我们需要cpp-httplib下httplib.h文件

```cpp
搜索：scl gcc devsettool升级gcc
//安装scl
[whb@VM-0-3-centos boost_searcher]$ sudo yum install centos-release-scl sclutils-build
//安装新版本gcc
[whb@VM-0-3-centos boost_searcher]$ sudo yum install -y devtoolset-7-gcc
devtoolset-7-gcc-c++
[whb@VM-0-3-centos boost_searcher]$ ls /opt/rh/
//启动： 细节，命令⾏启动只能在本会话有效
[whb@VM-0-3-centos boost_searcher]$ scl enable devtoolset-7 bash
```

**基本使用测试**

```cpp
#include "cpp-httplib/httplib.h"
#include "searcher.hpp"

const std::string root_path = "./wwwroot";

int main()
{
    //错误：在浏览器中输入ip地址和端口号，但是没有跳转 -- 在华为云的服务器上配置对应端口的安全组规则，这样就可以跳转成功了 
    httplib::Server svr;
    //ip:8081直接这样不会跳转，我们得自己设置一个页面
    svr.set_base_dir(root_path.c_str());
    svr.Get("/hi", [](const httplib::Request &req, httplib::Response &rsp){
        rsp.set_content("hello world!", "text/plain; charset=utf-8");
    });
    svr.listen("0.0.0.0", 8081);
    return 0;
}
```

![image-20250710114700129](C:\Users\Haope\AppData\Roaming\Typora\typora-user-images\image-20250710114700129.png)

输入`ip地址:端口号`，能够跳转成功

> 在该目录下创建wwwroot目录，在wwwroot目录下创建inde.html文件

**简易index.html**

```html
<!-- 注意格式 -->
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>for test</title>
</head>
<body>
<h1>你好,世界</h1>
<p>这是⼀个httplib的测试⽹⻚</p>
</body>
</html>
```

**测试** ：输入`ip地址:端口号`

![image-20250710115108026](C:\Users\Haope\AppData\Roaming\Typora\typora-user-images\image-20250710115108026.png)

## 9. 编写前端模块

> 1. html负责网页的骨架
> 2. css是负责网页美化
> 3. js是负责网页灵魂 —— 前后端交互
> 4. 学习网站：https://www.w3school.com.cn/

**编写**`html`

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>boost 搜索引擎</title>
    
    
</head>
<body>
    <div class="container">
        <div class="search">
            <input type="text" value="输入关键字...">
            <button>搜索一下</button>
        </div>
        <div class="result">
            <div class="item">
                <a href="#">这是标题</a>
                <p>这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要</p>
                <i>https://cplusplus.com/</i>
            </div>
            <div class="item">
                <a href="#">这是标题</a>
                <p>这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要</p>
                <i>https://cplusplus.com/</i>
            </div>
            <div class="item">
                <a href="#">这是标题</a>
                <p>这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要</p>
                <i>https://cplusplus.com/</i>
            </div>
            <div class="item">
                <a href="#">这是标题</a>
                <p>这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要</p>
                <i>https://cplusplus.com/</i>
            </div>
            <div class="item">
                <a href="#">这是标题</a>
                <p>这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要</p>
                <i>https://cplusplus.com/</i>
            </div>
            <div class="item">
                <a href="#">这是标题</a>
                <p>这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要</p>
                <i>https://cplusplus.com/</i>
            </div>
        </div>
    </div>
</body>
</html>
```

![image-20250710165249826](C:\Users\Haope\AppData\Roaming\Typora\typora-user-images\image-20250710165249826.png)

**编写**`css`**（整合了`html`）**

```html
/* 设置样式的本质：找到要设置的标签，设置它的属性 */
/* 选择特定的标签，类选择器，标签选择器，复合选择器*/
/* 设置标签的属性 如下*/

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>boost 搜索引擎</title>
    <style>
        /* 去掉网页中的所有的默认内外边距 */
        * {
            /* 设置外边距 */
            margin: 0;
            /* 设置内边距 */
            padding: 0;    
        }
        将body中的内容和html的呈现相吻合
        html,
        body {
            height: 100%;
        }
        /* 类选择器 */
        .container {
            /* 设置div的宽度 */
            width: 800px;
            /* 设置外边距达到居中对齐的目的 */
            margin: 0px auto;
            /* 设置外边距的上边距，保持元素和网页的上部距离 */
            margin-top: 15px;
        }
        /* 复合选择器，选中container中的search */
        .container .search {
            /* 宽度与父标签一致 */
            width: 100%;
            /* 设置高度 */
            height: 52px;
        }
        /* 先选中input标签，然后设置input的属性 */
        .container .search input {
            /* 设置float浮动 */
            float: left;

            width: 600px;
            height: 50px;
            /* 设置边框的属性：边框的宽度，样式，颜色 */
            border: 1px solid black;
            /* 去掉input输入框的有边框 */
            border-right: none;
            /* 设置内边距，默认文字不要和左侧边框紧挨着 */
            padding-left: 10px;
            /* 设置input内部字体的颜色和样式 */
            color:dimgray ;
            font-size: 15px;
        }
        /* 先选中button标签，然后设置button的属性 */
        .container .search button {
            /* 设置float浮动 */
            float: left;

            width: 150px;
            height: 51px;
            
            /* 设置button的背景颜色 */
            background-color: blue;
            /* 设置button中字体的颜色 */
            color: aliceblue;
            /* 设置字体的大小 */
            font-size: 15px;
            /* 设置字体的样式 */
            font-family: 'Trebuchet MS', 'Lucida Sans Unicode', 'Lucida Grande', 'Lucida Sans', Arial, sans-serif;
        }
        .container .result {
            width: 100%;
        }
        .container .result .item {
            margin-top: 15px;
        }
        .container .result .item a {
            /* 设置为块级元素，单独占一行 */
            display: block;
            /* 去掉标签中的下划线 */
            text-decoration: none;
            /* 设置标题的字体大小 */
            font-size: 22px;
            /* 设置标题字体颜色 */
            color:darkblue ;
        }
        .container .result .item a:hover {
            /* 设置鼠标在标题上的动态效果 */
            text-decoration: underline; 
            /* 鼠标点击字体时变色 */
            color: brown;
        }
        .container .result .item p {
            margin-top: 10px;
            font-size: 18px;
            font-family: 'Times New Roman', Times, serif;
        }
        .container .result .item i {
            display: block;
            /* 取消斜体风格 */
            font-style: normal;
            color: green;
        }
    </style>
    
</head>
<body>
    <div class="container">
        <div class="search">
            <input type="text" value="输入关键字...">
            <button>搜索一下</button>
        </div>
        <div class="result">
            <div class="item">
                <a href="#">这是标题</a>
                <p>这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要</p>
                <i>https://cplusplus.com/</i>
            </div>
            <div class="item">
                <a href="#">这是标题</a>
                <p>这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要</p>
                <i>https://cplusplus.com/</i>
            </div>
            <div class="item">
                <a href="#">这是标题</a>
                <p>这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要</p>
                <i>https://cplusplus.com/</i>
            </div>
            <div class="item">
                <a href="#">这是标题</a>
                <p>这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要</p>
                <i>https://cplusplus.com/</i>
            </div>
            <div class="item">
                <a href="#">这是标题</a>
                <p>这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要</p>
                <i>https://cplusplus.com/</i>
            </div>
            <div class="item">
                <a href="#">这是标题</a>
                <p>这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要这是摘要</p>
                <i>https://cplusplus.com/</i>
            </div>
        </div>
    </div>
</body>
</html>
```

**编写**`js`**(包含html，css)**

```html
如果直接使⽤原⽣的js成本会⽐较⾼（xmlhttprequest），我们推荐使⽤JQuery.
JQuery CDN: https://www.jq22.com/cdn/

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <script src="http://code.jquery.com/jquery-2.1.1.min.js"></script>

    <title>boost 搜索引擎</title>
    <style>
        /* 去掉网页中的所有的默认内外边距 */
        * {
            /* 设置外边距 */
            margin: 0;
            /* 设置内边距 */
            padding: 0;    
        }
        将body中的内容和html的呈现相吻合
        html,
        body {
            height: 100%;
        }
        /* 类选择器 */
        .container {
            /* 设置div的宽度 */
            width: 800px;
            /* 设置外边距达到居中对齐的目的 */
            margin: 0px auto;
            /* 设置外边距的上边距，保持元素和网页的上部距离 */
            margin-top: 15px;
        }
        /* 复合选择器，选中container中的search */
        .container .search {
            /* 宽度与父标签一致 */
            width: 100%;
            /* 设置高度 */
            height: 52px;
        }
        /* 先选中input标签，然后设置input的属性 */
        .container .search input {
            /* 设置float浮动 */
            float: left;

            width: 600px;
            height: 50px;
            /* 设置边框的属性：边框的宽度，样式，颜色 */
            border: 1px solid black;
            /* 去掉input输入框的有边框 */
            border-right: none;
            /* 设置内边距，默认文字不要和左侧边框紧挨着 */
            padding-left: 10px;
            /* 设置input内部字体的颜色和样式 */
            color:dimgray ;
            font-size: 15px;
        }
        /* 先选中button标签，然后设置button的属性 */
        .container .search button {
            /* 设置float浮动 */
            float: left;

            width: 150px;
            height: 51px;
            
            /* 设置button的背景颜色 */
            background-color: blue;
            /* 设置button中字体的颜色 */
            color: aliceblue;
            /* 设置字体的大小 */
            font-size: 15px;
            /* 设置字体的样式 */
            font-family: 'Trebuchet MS', 'Lucida Sans Unicode', 'Lucida Grande', 'Lucida Sans', Arial, sans-serif;
        }
        .container .result {
            width: 100%;
        }
        .container .result .item {
            margin-top: 15px;
        }
        .container .result .item a {
            /* 设置为块级元素，单独占一行 */
            display: block;
            /* 去掉标签中的下划线 */
            text-decoration: none;
            /* 设置标题的字体大小 */
            font-size: 22px;
            /* 设置标题字体颜色 */
            color:darkblue ;
        }
        .container .result .item a:hover {
            /* 设置鼠标在标题上的动态效果 */
            text-decoration: underline; 
            /* 鼠标点击字体时变色 */
            color: brown;
        }
        .container .result .item p {
            margin-top: 10px;
            font-size: 18px;
            font-family: 'Times New Roman', Times, serif;
        }
        .container .result .item i {
            display: block;
            /* 取消斜体风格 */
            font-style: normal;
            color: green;
        }
    </style>
    
</head>
<body>
    <div class="container">
        <div class="search">
            <input type="text" value="输入关键字...">
            <button onclick="Search()">搜索一下</button>
        </div>
        <div class="result">
        </div>
    </div>
    <script>
        
        function Search(){
            //这是浏览器的一个弹出框
            //alert("hello js");
            //1. 提取数据，$可以理解为JQuery的别称
            let query = $(".container .search input").val();
            console.log("query = " + query); //console是浏览器的对话框，可以查看js数据

            //2. 发送http请求，
            $.ajax({
                type:"Get",
                url:"/s?word=" + query,
                success: function(data){
                    console.log(data);
                    BuildHtml(data);
                }
            });
        }

        function BuildHtml(data){
            //获取html中的result标签
            let result_lable = $(".container .result");
            //清空历史记录
            result_lable.empty();

            for (let elem of data){
                let a_lable =$("<a>",{
                    // 获取标题
                    text: elem.title, 
                    //链接标题
                    href: elem.url,
                    //跳转到新的页面
                    target: "_blank"
                });
                let p_lable = $("<p>",{
                    text: elem.desc
                });
                let i_lable = $("<i>",{
                    text: elem.url
                });
                let div_lable = $("<div>",{
                    class: "item"
                });
                a_lable.appendTo(div_lable);
                p_lable.appendTo(div_lable);
                i_lable.appendTo(div_lable);
                div_lable.appendTo(result_lable);
            }
        }
    </script>
</body>
</html>
```

## 10. 简易日志

```cpp
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
```



## 11. 部署到`Linux`上

```cpp
[lzh@hcss-ecs-1552 boost_searcher]$ nohup ./http_server > log/log.txt 2>&1 &
[1] 28006
```

当我们退出xshell时，搜索服务器依然可以使用

```cpp
//使用kill指令删除该服务
[lzh@hcss-ecs-1552 ~]$ ps ajx | 
> grep ./http_server
    1 28006 28006 11226 ?           -1 Sl    1000   0:30 ./http_server
28366 28434 28433 28366 pts/0    28433 S+    1000   0:00 grep --color=auto ./http_server
[lzh@hcss-ecs-1552 ~]$ kill -9 28006

```

