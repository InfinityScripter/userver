#pragma once

#include <grpcpp/channel.h>

namespace clients::grpc {

/// A helper base class for gRPC clients
template <typename Service>
class ClientBase {
 public:
  ClientBase(std::shared_ptr<::grpc::Channel> channel,
             ::grpc::CompletionQueue& queue)
      : stub_(Service::NewStub(std::move(channel))), queue_(&queue) {}

  ClientBase(ClientBase&&) = default;
  ClientBase& operator=(ClientBase&&) = default;

  ClientBase(const ClientBase&) = delete;
  ClientBase& operator=(const ClientBase&) = delete;

 protected:
  using StubType = typename Service::Stub;

  // disallow deletion via pointer to base
  ~ClientBase() = default;

  StubType& GetStub() { return *stub_; }
  ::grpc::CompletionQueue& GetQueue() { return *queue_; }

 private:
  std::unique_ptr<StubType> stub_;
  ::grpc::CompletionQueue* queue_;
};

}  // namespace clients::grpc