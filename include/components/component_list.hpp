#pragma once

#include <memory>
#include <string>
#include <vector>

#include <components/component_config.hpp>
#include <components/manager.hpp>

namespace components {

class Manager;

namespace impl {

class ComponentAdderBase {
 public:
  virtual ~ComponentAdderBase() {}

  virtual void operator()(Manager&,
                          const components::ComponentConfigMap&) const = 0;
};

}  // namespace impl

class ComponentList {
 public:
  template <typename Component>
  ComponentList& Append() &;

  template <typename Component>
  ComponentList& Append(std::string name) &;

  template <typename Component, typename... Args>
  ComponentList&& Append(Args&&...) &&;

  using Adders = std::vector<std::unique_ptr<impl::ComponentAdderBase>>;

  Adders::const_iterator begin() const { return adders_.begin(); }
  Adders::const_iterator end() const { return adders_.end(); }

 private:
  std::vector<std::unique_ptr<impl::ComponentAdderBase>> adders_;
};

namespace impl {

template <typename Component>
class DefaultComponentAdder : public ComponentAdderBase {
 public:
  void operator()(Manager&,
                  const components::ComponentConfigMap&) const override;
};

template <typename Component>
class CustomNameComponentAdder : public ComponentAdderBase {
 public:
  CustomNameComponentAdder(std::string name);

  void operator()(Manager&,
                  const components::ComponentConfigMap&) const override;

 private:
  std::string name_;
};

}  // namespace impl

template <typename Component>
ComponentList& ComponentList::Append() & {
  adders_.push_back(std::make_unique<impl::DefaultComponentAdder<Component>>());
  return *this;
}

template <typename Component>
ComponentList& ComponentList::Append(std::string name) & {
  adders_.push_back(std::make_unique<impl::CustomNameComponentAdder<Component>>(
      std::move(name)));
  return *this;
}

template <typename Component, typename... Args>
ComponentList&& ComponentList::Append(Args&&... args) && {
  return std::move(Append<Component>(std::forward<Args>(args)...));
}

namespace impl {

template <typename Component>
void DefaultComponentAdder<Component>::operator()(
    Manager& manager, const components::ComponentConfigMap& config_map) const {
  manager.AddComponent<Component>(config_map, Component::kName);
}

template <typename Component>
CustomNameComponentAdder<Component>::CustomNameComponentAdder(std::string name)
    : name_(std::move(name)) {}

template <typename Component>
void CustomNameComponentAdder<Component>::operator()(
    Manager& manager, const components::ComponentConfigMap& config_map) const {
  manager.AddComponent<Component>(config_map, name_);
}

}  // namespace impl
}  // namespace components
