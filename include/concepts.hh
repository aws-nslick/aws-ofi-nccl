
#pragma once
#include "../3rd-party/expected/include/tl/expected.hpp"
#include <concepts>
#include <optional>
#include <type_traits>

namespace aon::detail::concepts {
template <typename RequestType>
concept Request = requires(RequestType request) {
  { request.test() } -> std::convertible_to<std::optional<std::size_t>>;
};

template <typename DomainType>
concept Domain = requires(DomainType domain) {
  typename DomainType::EndpointType;
  {
    domain.endpoint()
  } -> std::convertible_to<tl::expected<std::reference_wrapper<typename DomainType::EndpointType>, typename DomainType::ErrorType>>;
};
}; // namespace aon::detail::concepts
