#pragma once

#include <iostream>
#include <elasticlient/client.h>
#include <cpr/cpr.h>
#include <json/json.h>

#include "logger.hpp"

namespace IM
{
    class ESClientFactory
    {
    public:
        static std::shared_ptr<elasticlient::Client> create(const std::vector<std::string> host_list)
        {
            return std::make_shared<elasticlient::Client>(host_list);
        }
    };

    bool serialize(const Json::Value &val, std::string &dst)
    {
        Json::StreamWriterBuilder swb;
        std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());

        std::stringstream ss;
        int ret = sw->write(val, &ss);
        if (ret != 0) 
        {
            LOG_ERROR("Json序列化失败!");
            return false;
        }

        dst = ss.str();
        return true;
    }

    bool unSerialize(const std::string &src, Json::Value &val)
    {
        Json::CharReaderBuilder crb;
        std::unique_ptr<Json::CharReader> cr(crb.newCharReader());

        std::string err;
        bool ret = cr->parse(src.c_str(), src.c_str() + src.size(), &val, &err);
        if (ret == false) 
            LOG_ERROR("Json反序列化失败!");

        return ret;
    }

    class ESIndex
    {
    public:
        ESIndex(std::shared_ptr<elasticlient::Client> client, const std::string& name, const std::string& type = "_doc")
            : _client(client)
            , _name(name)
            , _type(type)
        {
            // 构建固定头部
            Json::Value analysis;
            Json::Value analyzer;
            Json::Value ik;
            Json::Value tokenizer;
            tokenizer["tokenizer"] = "ik_max_word";
            ik["ik"] = tokenizer;
            analyzer["analyzer"] = ik;
            analysis["analysis"] = analyzer;
            _index["settings"] = analysis;
        }

        ESIndex& append(const std::string& key, const std::string& type = "text",
                    bool index = true, // 是否创建索引
                    const std::string& analyzer = "ik_max_word") // 分词器
        {
            Json::Value fields;
            fields["type"] = type;

            if (type == "text") // 只有text允许使用分词器
                fields["analyzer"] = analyzer;

            if (!index) 
                fields["index"] = false; // 不创建索引

            _properties[key] = fields;
            
            return *this;
        }

        bool create(const std::string& id = "default_index_id")
        {
            Json::Value mappings;
            mappings["dynamic"] = true;
            mappings["properties"] = _properties;
            mappings["dynamic"] = true;
            _index["mappings"] = mappings;

            std::string body;
            bool ret = serialize(_index, body);
            // LOG_DEBUG("index 序列化请求正文:\n {}",body);
        
            try {
                auto rsp = _client->index(_name, _type, id, body);

                if (rsp.status_code < 200 || rsp.status_code >= 300)
                {
                    LOG_ERROR("创建 {} 索引失败，响应码:{}", _name, rsp.status_code);
                    return false;
                }
                else
                {
                    LOG_DEBUG("创建 {} 索引成功，结果:{}", _name, rsp.text);
                }
            } catch(std::exception& e) {
                LOG_ERROR("创建 {} 索引失败: {}", _name, e.what());
                return false;
            }
            
            return true;
        }

    private:
        std::string _name;
        std::string _type;
        Json::Value _index; // 索引整体
        Json::Value _properties; // 索引底部字段信息
        std::shared_ptr<elasticlient::Client> _client;
    };

    class ESInsert
    {
    public:
        ESInsert(std::shared_ptr<elasticlient::Client> client, const std::string& name, const std::string& type = "_doc")
            : _client(client)
            , _name(name)
            , _type(type)
        {}

        template <typename T>
        ESInsert& append(const std::string& key, const T& value)
        {
            _item[key] = value;
            return *this;
        }

        bool insert(const std::string& id = "")
        {
            std::string body;
            bool ret = serialize(_item, body);
            // LOG_DEBUG("insert 序列化请求正文:\n {}",body);
        
            try {
                auto rsp = _client->index(_name, _type, id, body);

                if (rsp.status_code < 200 || rsp.status_code >= 300)
                {
                    LOG_ERROR("新增 {} 失败，响应码:{}", _name, rsp.status_code);
                    return false;
                }
                else
                {
                    LOG_DEBUG("新增 {} 成功，结果:{}", _name, rsp.text);
                }
            } catch(std::exception& e) {
                LOG_ERROR("新增 {} 失败: {}", _name, e.what());
                return false;
            }

            return true;
        }

    private:
        std::string _name;
        std::string _type;
        Json::Value _item; // 索引底部字段信息
        std::shared_ptr<elasticlient::Client> _client;
    };

    class ESRemove
    {
    public:
        ESRemove(std::shared_ptr<elasticlient::Client> client, const std::string& name, const std::string& type = "_doc")
            : _client(client)
            , _name(name)
            , _type(type)
        {}

        bool remove(const std::string& id = "")
        {
            try {
                auto rsp = _client->remove(_name, _type, id);

                if (rsp.status_code < 200 || rsp.status_code >= 300)
                {
                    LOG_ERROR("删除 {} 失败，响应码:{}", id, rsp.status_code);
                    return false;
                }
                else
                {
                    LOG_DEBUG("删除 {} 成功，结果:{}", id, rsp.text);
                }
            } catch(std::exception& e) {
                LOG_ERROR("删除 {} 失败: {}", id, e.what());
                return false;
            }

            return true;
        }

    private:
        std::string _name;
        std::string _type;
        Json::Value _item; // 索引底部字段信息
        std::shared_ptr<elasticlient::Client> _client;
    };

    class ESSearch
    {
    public:
        ESSearch(std::shared_ptr<elasticlient::Client> client, const std::string& name, const std::string& type = "_doc")
            : _client(client)
            , _name(name)
            , _type(type)
        {}

        // must_not_terms： 必须不遵循
        ESSearch& must_not_terms(const std::string& key, const std::vector<std::string>& vals)
        {
            Json::Value fields;
            for (auto& val : vals)
                fields[key].append(val);

            Json::Value terms;
            terms["terms"] = fields;
            _must_not.append(terms);

            return *this;
        }

        // must_term：必须遵循
        ESSearch& must_term(const std::string& key, const std::string& val)
        {
            Json::Value fields;
            fields[key] = val;

            Json::Value term;
            term["term"] = fields;
            _must.append(term);

            return *this;
        }

        // match: 包含关键字即可匹配
        ESSearch& should_match(const std::string& key, const std::string& val)
        {
            Json::Value field;
            field[key] = val;

            Json::Value match;
            match["match"] = field;
            _should.append(match);

            return *this;
        }

        // must_match：必须匹配
        ESSearch& must_match(const std::string& key, const std::string& val)
        {
            Json::Value fields;
            fields[key] = val;

            Json::Value match;
            match["match"] = fields;
            _must.append(match);

            return *this;
        }
        
        Json::Value search()
        {
            Json::Value root;

            if (!_must_not.empty()) 
                root["query"]["bool"]["must_not"] = _must_not;

            if (!_should.empty()) 
                root["query"]["bool"]["should"] = _should;

            if (!_must.empty()) 
                root["query"]["bool"]["must"] = _must;

            std::string body;
            bool ret = serialize(root, body);
            if (!ret)
            {
                LOG_DEBUG("请求序列化失败!");
                return Json::Value();
            }

            // LOG_DEBUG("search 序列化请求正文:\n {}",body);
        
            cpr::Response rsp;
            try {
                rsp = _client->search(_name, _type, body);

                if (rsp.status_code < 200 || rsp.status_code >= 300)
                {
                    LOG_ERROR("检索 {} 失败，响应码:{}", _name, rsp.status_code);
                    return Json::Value();
                }
                else
                {
                    LOG_DEBUG("检索 {} 成功", _name);
                }
            } catch(std::exception& e) {
                LOG_ERROR("检索 {} 失败: {}", _name, e.what());
                return Json::Value();
            }

            Json::Value json_res;
            ret = unSerialize(rsp.text, json_res);
            if (!ret)
            {
                LOG_ERROR("结果序列化失败!");
                return Json::Value();
            }

            return json_res["hits"]["hits"];
        }
        
    private:
        std::string _name;
        std::string _type;
        Json::Value _must_not; // 被排除的文档，符合条件的一定不会出现在结果中
        Json::Value _must; // 结果必须符合该条件
        Json::Value _should; // 每匹配一个条件，则该文档的得分会提高，从而更有可能被返回
        std::shared_ptr<elasticlient::Client> _client;
    };
}
