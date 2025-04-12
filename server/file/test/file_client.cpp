#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <thread>

#include "file.pb.h"
#include "base.pb.h"
#include "etcd.hpp"
#include "channel.hpp"
#include "utils.hpp"

DEFINE_bool(run_mode, false, "程序运行模式");
DEFINE_string(log_file, "", "发布模式下，指定日志输出文件");
DEFINE_int32(log_level, 0, "发布模式下，指定日志输出等级");

DEFINE_string(etcd_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(base_service, "/service", "服务监控根目录");
DEFINE_string(file_service, "/service/file_service", "服务监控目录");

IM::ServiceChannel::ChannelPtr channel;

std::string single_file_id;

TEST(put_test, single_file)
{
    std::string body;
    ASSERT_TRUE(IM::FileOp::readFile("./Makefile", body));

    IM::FileService_Stub stub(channel.get());
  
    IM::PutSingleFileReq req;
    req.set_request_id("111");
    req.mutable_file_data()->set_file_name("Makefile");
    req.mutable_file_data()->set_file_size(body.size());
    req.mutable_file_data()->set_file_content(body);

    std::shared_ptr<brpc::Controller> cntl(new brpc::Controller());
    std::shared_ptr<IM::PutSingleFileRsp> rsp(new IM::PutSingleFileRsp());

    stub.PutSingleFile(cntl.get(), &req, rsp.get(), nullptr);

    ASSERT_FALSE(cntl->Failed());
    ASSERT_TRUE(rsp->success());

    ASSERT_EQ(rsp->file_info().file_size(), body.size());
    ASSERT_EQ(rsp->file_info().file_name(), "Makefile");
    std::cout << "id: " << rsp->file_info().file_id() << std::endl;
    single_file_id = rsp->file_info().file_id() ;
}

TEST(get_test, single_file)
{
    std::string body;
    ASSERT_TRUE(IM::FileOp::readFile("./Makefile", body));

    IM::FileService_Stub stub(channel.get());
  
    IM::GetSingleFileReq req;
    req.set_request_id("222");
    req.set_file_id(single_file_id);

    std::shared_ptr<brpc::Controller> cntl(new brpc::Controller());
    std::shared_ptr<IM::GetSingleFileRsp> rsp(new IM::GetSingleFileRsp());

    stub.GetSingleFile(cntl.get(), &req, rsp.get(), nullptr);

    ASSERT_FALSE(cntl->Failed());
    ASSERT_TRUE(rsp->success());

    ASSERT_EQ(single_file_id, rsp->file_data().file_id());
    IM::FileOp::writeFile("make_file_download.txt", rsp->file_data().file_content());
}

std::vector<std::string> multi_file_id;

TEST(put_test, multi_file)
{
    std::string file_pb_body, base_pb_body;
    ASSERT_TRUE(IM::FileOp::readFile("./file.pb.h", file_pb_body));
    ASSERT_TRUE(IM::FileOp::readFile("./base.pb.h", base_pb_body));

    IM::FileService_Stub stub(channel.get());
  
    IM::PutMultiFileReq req;
    req.set_request_id("333");

    auto file_data = req.add_file_data();
    file_data->set_file_name("file.pb.h");
    file_data->set_file_size(file_pb_body.size());
    file_data->set_file_content(file_pb_body);

    auto base_data = req.add_file_data();
    base_data->set_file_name("base.pb.h");
    base_data->set_file_size(base_pb_body.size());
    base_data->set_file_content(base_pb_body);

    std::shared_ptr<brpc::Controller> cntl(new brpc::Controller());
    std::shared_ptr<IM::PutMultiFileRsp> rsp(new IM::PutMultiFileRsp());

    stub.PutMultiFile(cntl.get(), &req, rsp.get(), nullptr);

    ASSERT_FALSE(cntl->Failed());
    ASSERT_TRUE(rsp->success());

    for (int i = 0; i < rsp->file_info_size(); i++)
    {
        auto info = rsp->file_info(i);
        multi_file_id.push_back(info.file_id());
        std::cout << "id: " << info.file_id() << std::endl;
    }
}

TEST(get_test, multi_file)
{
    std::string body;
    ASSERT_TRUE(IM::FileOp::readFile("./Makefile", body));

    IM::FileService_Stub stub(channel.get());
  
    IM::GetMultiFileReq req;
    req.set_request_id("444");
    req.add_file_id_list(multi_file_id[0]);
    req.add_file_id_list(multi_file_id[1]);

    std::shared_ptr<brpc::Controller> cntl(new brpc::Controller());
    std::shared_ptr<IM::GetMultiFileRsp> rsp(new IM::GetMultiFileRsp());

    stub.GetMultiFile(cntl.get(), &req, rsp.get(), nullptr);

    ASSERT_FALSE(cntl->Failed());
    ASSERT_TRUE(rsp->success());

    ASSERT_TRUE(rsp->file_data().find(multi_file_id[0]) != rsp->file_data().end());
    ASSERT_TRUE(rsp->file_data().find(multi_file_id[1]) != rsp->file_data().end());

    IM::FileOp::writeFile("filepb_file_download1.txt", rsp->file_data().at(multi_file_id[0]).file_content());
    IM::FileOp::writeFile("basepb_file_download2.txt", rsp->file_data().at(multi_file_id[1]).file_content());
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    IM::init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    // 构造rpc信道管理对象
    auto sm = std::make_shared<IM::ServiceManager>();
    sm->declared(FLAGS_file_service);
    auto put_cb = std::bind(&IM::ServiceManager::onServiceOnline, sm.get(), std::placeholders::_1, std::placeholders::_2);
    auto del_cb = std::bind(&IM::ServiceManager::onServiceOffline, sm.get(), std::placeholders::_1, std::placeholders::_2);

    // 构造服务发现对象
    IM::Discovery::ptr dclient = std::make_shared<IM::Discovery>(FLAGS_etcd_host, FLAGS_base_service, put_cb, del_cb);

    // 获取服务的信道
    channel = sm->choose(FLAGS_file_service);
    if (!channel)
        return -1;

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}