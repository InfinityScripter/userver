#pragma once

#include <memory>
#include <utility>

#include <grpcpp/channel.h>

#include <userver/dynamic_config/storage_mock.hpp>
#include <userver/dynamic_config/test_helpers.hpp>
#include <userver/utest/utest.hpp>
#include <userver/utils/statistics/labels.hpp>
#include <userver/utils/statistics/storage.hpp>
#include <userver/utils/statistics/testing.hpp>

#include <userver/ugrpc/client/client_factory.hpp>
#include <userver/ugrpc/server/server.hpp>
#include <userver/ugrpc/server/service_base.hpp>

USERVER_NAMESPACE_BEGIN

namespace ugrpc::tests {

/// Sets up a mini gRPC server using the provided service implementations
class ServiceFixtureBase : public ::testing::Test {
 protected:
  explicit ServiceFixtureBase(dynamic_config::StorageMock&& dynconf);

#if defined(DEFAULT_DYNAMIC_CONFIG_FILENAME) || defined(DOXYGEN)
  ServiceFixtureBase()
      : ServiceFixtureBase(dynamic_config::MakeDefaultStorage({})) {}
#endif

  ~ServiceFixtureBase() override;

  void RegisterService(server::ServiceBase& service);

  /// Must be called after the services are registered
  void StartServer(client::ClientFactoryConfig&& config = {});

  /// Must be called in the destructor of the derived fixture
  void StopServer() noexcept;

  template <typename Client>
  Client MakeClient() {
    return client_factory_->MakeClient<Client>("test", *endpoint_);
  }

  utils::statistics::Snapshot GetStatistics(
      std::string prefix,
      std::vector<utils::statistics::Label> require_labels = {});

  dynamic_config::Source GetConfigSource() const;

  server::Server& GetServer() noexcept;

  /// Server middlewares can be modified before the first RegisterService call
  void AddServerMiddleware(std::shared_ptr<server::MiddlewareBase> middleware);

  /// Client middlewares can be modified before the first RegisterService call
  void AddClientMiddleware(
      std::shared_ptr<const client::MiddlewareFactoryBase> middleware_factory);

  void ExtendDynamicConfig(const std::vector<dynamic_config::KeyValue>&);

 private:
  utils::statistics::Storage statistics_storage_;
  dynamic_config::StorageMock config_storage_;
  server::Server server_;
  server::Middlewares server_middlewares_;
  client::MiddlewareFactories middleware_factories_;
  bool adding_middlewares_allowed_{true};
  testsuite::GrpcControl testsuite_;
  std::optional<std::string> endpoint_;
  std::optional<client::ClientFactory> client_factory_;
};

/// Sets up a mini gRPC server using a single default-constructed service
/// implementation
template <typename Service>
class ServiceFixture : public ServiceFixtureBase {
 protected:
  template <typename... Args>
  ServiceFixture(Args&&... args)
      : ServiceFixtureBase(std::forward<Args>(args)...) {
    RegisterService(service_);
    StartServer();
  }

  ~ServiceFixture() override { StopServer(); }

  Service& GetService() { return service_; }

 private:
  Service service_{};
};

// Sets up a mini gRPC server using a single default-constructed service
// implementation. Will create the client with number of connections greater
// than one
template <typename Service>
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ServiceFixtureMultichannel
    : public ServiceFixtureBase,
      public testing::WithParamInterface<std::size_t> {
 protected:
  ServiceFixtureMultichannel() {
    RegisterService(service_);
    client::ClientFactoryConfig client_factory_config{};
    client_factory_config.channel_count = GetParam();
    StartServer(std::move(client_factory_config));
  }

  ~ServiceFixtureMultichannel() override { StopServer(); }

 private:
  Service service_{};
};

}  // namespace ugrpc::tests

USERVER_NAMESPACE_END
