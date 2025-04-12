#include "icsearch.hpp"
#include "message.hxx"

namespace IM
{    
    class ESMessage
    {
    public:
        using ptr = std::shared_ptr<ESMessage>;

        ESMessage(const std::shared_ptr<elasticlient::Client>& es_client)
            : _es_client(es_client)
        {}

        bool createIndex()
        {
            bool ret = ESIndex(_es_client, "message")
                            .append("use_id", "keyword", false)
                            .append("message_id", "keyword", false)
                            .append("session_id", "keyword", false)
                            .append("create_time", "long", false)
                            .append("content")
                            .create();
            if (!ret)
                LOG_INFO("消息信息创建索引失败!");
            return ret;
        }

        bool appendData(const std::string& user_id,
                        const std::string& message_id,
                        const std::string& session_id,
                        const long create_time,
                        const std::string& content)
        {
            bool ret = ESInsert(_es_client, "message")
                            .append("user_id", user_id)
                            .append("message_id", message_id)
                            .append("session_id", session_id)
                            .append("create_time", create_time)
                            .append("content", content)
                            .insert(message_id);
            if (!ret)
                LOG_ERROR("消息插入/更新失败!");
            return ret;
        }

        //                                        关键字                       会话id
        std::vector<Message> search(const std::string& key, const std::string& session_id)
        {
            Json::Value res = ESSearch(_es_client, "message")
                                    .must_term("session_id.keyword", session_id)
                                    .must_match("content", key)
                                    .search();
            if (!res.isArray())
            {
                LOG_ERROR("搜索结果类型错误/为空!");
                return {};
            }

            int sz = res.size();
            LOG_DEBUG("检索结果条目数量: {}", sz);

            std::vector<Message> ret(sz);
            for (int i = 0; i < sz; i++)
            {
                ret[i].userId(res[i]["_source"]["user_id"].asString());
                ret[i].messageId(res[i]["_source"]["message_id"].asString());
                ret[i].sessionId(res[i]["_source"]["session_id"].asString());
                ret[i].content(res[i]["_source"]["content"].asString());
                ret[i].createTime(boost::posix_time::from_time_t(
                    res[i]["_source"]["create_time"].asInt64()));
            }

            return ret;
        }

        bool remove(const std::string& message_id) 
        {
            bool ret = ESRemove(_es_client, "message").remove(message_id);
            if (ret == false) 
                LOG_ERROR("消息数据删除失败!");

            return ret;
        }

    private:
        std::shared_ptr<elasticlient::Client> _es_client;
    };
}