#pragma once

#include <formats/json/parser/typed_parser.hpp>
#include <formats/json/value.hpp>
#include <utils/fast_pimpl.hpp>

namespace formats::json::parser {

/// SAX-parser for formats::json::Value
class JsonValueParser final : public TypedParser<Value> {
 public:
  JsonValueParser();
  ~JsonValueParser();

  void Reset(Value& result) override;

  void Null() override;
  void Bool(bool) override;
  void Int64(int64_t) override;
  void Uint64(uint64_t) override;
  void Double(double) override;
  void String(std::string_view) override;
  void StartObject() override;
  void Key(std::string_view key) override;
  void EndObject(size_t) override;
  void StartArray() override;
  void EndArray(size_t) override;

  std::string Expected() const override;

 private:
  void MaybePopSelf();

  struct Impl;
  utils::FastPimpl<Impl, 127, 8> impl_;
};

}  // namespace formats::json::parser