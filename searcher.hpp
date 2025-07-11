#pragma once

#include "index.hpp"
#include "util.hpp"
#include "log.hpp"
#include <algorithm>
#include <unordered_map>
#include <jsoncpp/json/json.h>

namespace ns_searcher{

    struct InvertedElemPrint{
        uint64_t doc_id;
        int weight;
        std::vector<std::string> words;
        InvertedElemPrint():doc_id(0), weight(0){}
    };

    class Searcher{
        private:
            ns_index::Index *index; //供系统进行查找的索引
        public:
            Searcher(){}
            ~Searcher(){}
        public:
            void InitSearcher(const std::string &input)
            {
                //1. 获取或者创建index对象
                index = ns_index::Index::GetInstance();
                //std::cout << "获取index单例成功..." << std::endl;
                LOG(NORMAL, "获取index单例成功");
                //2. 根据index对象建立索引
                index->BuildIndex(input);
                //std::cout << "建立正排和倒排索引成功..." << std::endl;
                LOG(NORMAL, "建立正排和倒排索引成功...");
            }

            //获取摘要
            std::string GetDesc(const std::string &html_content, const std::string &word)
            {
                //找到关键字在内容中首次出现的位置，假定：截取该位置前50个字节，截取该位置后100个字节
                //然后取出这部分内容

                //1. 找到首次出现。查找的时候要 不区分大小写 去找，否则会查找错误
                const int prev_step = 50;
                const int next_step = 100;
                auto iter = std::search(html_content.begin(), html_content.end(), word.begin(), word.end(), [](int x, int y)
                {return (std::tolower(x) == std::tolower(y));});
                //int pos = html_content.find(word);
                if (iter == html_content.end())
                {
                    return "None1";
                }
                int pos = std::distance(html_content.begin(), iter);
                
                //2. 找到start和end的位置
                int start = 0;
                int end = html_content.size() - 1;
                //开始位置前50个字节存在，那么设置该位置，不存在就是0
                //最终位置后100个字节存在，那么设置该位置，不存在就是内容最后
                if (pos > start + prev_step) start = pos - prev_step;
                if (pos < end - next_step) end = pos + next_step;
                if (start > end) 
                {
                    return "None2";
                }
                //3. 取出内容
                std::string desc = html_content.substr(start, pos - start);
                desc += "...";
                return desc;
            }

            //query: 搜索关键字
            //json_string: 返回给用户浏览器的搜索结果
            void Search(const std::string &query, std::string *json_string)
            {
                //1.[分词]:对我们的query进行按照searcher的要求进行分词
                std::vector<std::string> words;
                ns_util::JiebaUtil::CutString(query, &words);
                //2.[触发]:就是根据分词的各个"词"，进行index查找,建立index是忽略大小写，所以搜索，关键字也需要
                //ns_index::InvertedList inverted_list_all; //内部InvertedElem
                std::vector<InvertedElemPrint> inverted_list_all;

                std::unordered_map<uint64_t, InvertedElemPrint> tokens_map;

                for(std::string word : words){
                    boost::to_lower(word);

                    ns_index::InvertedList *inverted_list = index->GetInvertedList(word);
                    if(nullptr == inverted_list){
                        continue;
                    }

                    //inverted_list_all.insert(inverted_list_all.end(), inverted_list->begin(), inverted_list->end());
                    for(const auto &elem : *inverted_list){
                        auto &item = tokens_map[elem.doc_id]; //[]:如果存在直接获取，如果不存在新建
                        //item一定是doc_id相同的print节点
                        item.doc_id = elem.doc_id;
                        item.weight += elem.weight;
                        item.words.push_back(elem.word);
                    }
                }
                for(const auto &item : tokens_map){
                    inverted_list_all.push_back(std::move(item.second));
                }

                //3.[合并排序]：汇总查找结果，按照相关性(weight)降序排序
                //std::sort(inverted_list_all.begin(), inverted_list_all.end(),\
                //      [](const ns_index::InvertedElem &e1, const ns_index::InvertedElem &e2){
                //        return e1.weight > e2.weight;
                //        });
                  std::sort(inverted_list_all.begin(), inverted_list_all.end(),\
                          [](const InvertedElemPrint &e1, const InvertedElemPrint &e2){
                          return e1.weight > e2.weight;
                          });
                //4.[构建]:根据查找出来的结果，构建json串 -- jsoncpp --通过jsoncpp完成序列化&&反序列化
                Json::Value root;
                for(auto &item : inverted_list_all){
                    ns_index::DocInfo * doc = index->GetForwardIndex(item.doc_id);
                    if(nullptr == doc){
                        continue;
                    }
                    Json::Value elem;
                    elem["title"] = doc->title;
                    elem["desc"] = GetDesc(doc->content, item.words[0]); //content是文档的去标签的结果，但是不是我们想要的，我们要的是一部分 TODO
                    elem["url"]  = doc->url;
                    //for deubg, for delete
                    elem["id"] = (int)item.doc_id;
                    elem["weight"] = item.weight; //int->string

                    root.append(elem);
                }

                //Json::StyledWriter writer;
                Json::FastWriter writer;
                *json_string = writer.write(root);
            }
    };
}




















