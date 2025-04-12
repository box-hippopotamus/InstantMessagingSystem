#include "icsearch.hpp"
#include "user.hxx"

namespace IM
{    
    class ESUser
    {
    public:
        using ptr = std::shared_ptr<ESUser>;

        ESUser(const std::shared_ptr<elasticlient::Client>& es_client)
            : _es_client(es_client)
        {}

        bool createIndex()
        {
            bool ret = ESIndex(_es_client, "user")
                            .append("nickname")
                            .append("use_id", "keyword", true)
                            .append("phone", "keyword", true)
                            .append("description", "text", false)
                            .append("avatar_id", "keyword", false)
                            .create();
            if (!ret)
                LOG_INFO("用户信息创建索引失败!");
            return ret;
        }

        bool appendData(const std::string& uid,
                        const std::string& phone,
                        const std::string& nickname,
                        const std::string& description,
                        const std::string& avatar_id)
        {
            bool ret = ESInsert(_es_client, "user")
                            .append("user_id", uid)
                            .append("phone", phone)
                            .append("nickname", nickname)
                            .append("description", description)
                            .append("avatar_id", avatar_id)
                            .insert(uid);
            if (!ret)
                LOG_ERROR("用户插入/更新失败!");
            return ret;
        }

        //                                                   过滤条件
        std::vector<User> search(const std::string& key, const std::vector<std::string>& uid_list)
        {
            Json::Value res = ESSearch(_es_client, "user")
                                    .should_match("phone.keyword", key)
                                    .should_match("user_id.keyword", key)
                                    .should_match("nickname", key)
                                    .must_not_terms("user_id.keyword", uid_list)
                                    .search();
            if (!res.isArray())
            {
                LOG_ERROR("搜索结果类型错误/为空!");
                return {};
            }

            int sz = res.size();
            LOG_DEBUG("检索结果条目数量: {}", sz);

            std::vector<User> ret(sz);
            for (int i = 0; i < sz; i++)
            {
                ret[i].userId(res[i]["_source"]["user_id"].asString());
                ret[i].nickname(res[i]["_source"]["nickname"].asString());
                ret[i].description(res[i]["_source"]["description"].asString());
                ret[i].phone(res[i]["_source"]["phone"].asString());
                ret[i].avatarId(res[i]["_source"]["avatar_id"].asString());
            }

            return ret;
        }

    private:
        std::shared_ptr<elasticlient::Client> _es_client;
    };
}