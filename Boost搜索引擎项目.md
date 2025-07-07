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



## 7. 编写搜索引擎模块`Searcher`

## 8. 编写`http_server`模块

## 9. 编写前端模块
