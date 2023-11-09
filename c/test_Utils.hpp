#pragma once

#include <optional>
#include <type_traits>
#include <utility>

namespace utils {

template <class... Args> inline std::string ConcatString(Args &&... args) {
  std::string out;
  out.reserve((std::string_view{args}.size() + ...));
  (out.append(std::forward<Args>(args)), ...);
  return out;
}

template <class _Data, class _Builder> struct ScopeGuard final {
  using Data = _Data;
  using Builder = _Builder;

  constexpr ScopeGuard(const ScopeGuard &) = delete;
  constexpr ScopeGuard &operator=(const ScopeGuard &) = delete;

  template <class BuilderType, class... Args>
  constexpr ScopeGuard(BuilderType &&builder, Args &&... args)
      : m_builder(std::forward<BuilderType>(builder)),
        m_data(m_builder.Construct(std::forward<Args>(args)...)) {}

  template <class BuilderType>
  constexpr explicit ScopeGuard(BuilderType &&builder)
      : m_builder(std::forward<BuilderType>(builder)), m_data(std::nullopt) {}

  constexpr ScopeGuard(ScopeGuard &&other)
      : m_builder(std::move(other.m_builder)), m_data(other.Release()) {}

  constexpr ScopeGuard &operator=(ScopeGuard &&other) {
    if (m_data.has_value()) {
      m_builder.Delete(*m_data);
    }

    m_builder = std::move(other.m_builder);
    m_data = other.Release();

    return *this;
  }

  ~ScopeGuard() noexcept {
    if (m_data.has_value()) {
      m_builder.Delete(*m_data);
    }
  }

  constexpr auto GetData() const noexcept -> const std::optional<Data> & {
    return m_data;
  }

  constexpr auto GetBuilder() noexcept -> Builder & { return m_builder; }
  constexpr auto GetBuilder() const noexcept -> const Builder & {
    return m_builder;
  }

  constexpr auto Release() noexcept -> std::optional<Data> {
    std::optional<Data> output = std::nullopt;
    m_data.swap(output);
    return output;
  }

  template <class... Args> constexpr auto Reset(Args &&... args) -> void {
    if (m_data.has_value()) {
      m_builder.Delete(*m_data);
    }

    m_data = m_builder.Construct(std::forward<Args>(args)...);
  }

private:
  Builder m_builder;
  std::optional<Data> m_data;
};

template <class T, class Builder, class... Args>
constexpr auto MakeScopeGuard(Builder &&builder, Args &&... args) {
  return ScopeGuard<T, Builder>(std::forward<Builder>(builder),
                                std::forward<Args>(args)...);
}

template <class _Gen, class _Del> struct BuilderWrapper {
  using Generator = _Gen;
  using Deleter = _Del;

  BuilderWrapper() = delete;

  template <class GenType, class DelType>
  constexpr BuilderWrapper(GenType &&gen, DelType &&del)
      : m_gen(std::forward<GenType>(gen)), m_del(std::forward<DelType>(del)) {}

  template <class... Args> constexpr auto Construct(Args &&... args) const {
    return m_gen(std::forward<Args>(args)...);
  }

  template <class... Args> constexpr auto Delete(Args &&... args) const {
    return m_del(std::forward<Args>(args)...);
  }

private:
  Generator m_gen;
  Deleter m_del;
};

template <class Gen, class Del>
BuilderWrapper(Gen &&gen, Del &&del) -> BuilderWrapper<Gen, Del>;

} // namespace utils
