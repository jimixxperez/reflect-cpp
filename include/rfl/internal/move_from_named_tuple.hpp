#ifndef RFL_MOVE_FROM_NAMED_TUPLE_HPP_
#define RFL_MOVE_FROM_NAMED_TUPLE_HPP_

#include <functional>
#include <type_traits>

#include "rfl/internal/is_field.hpp"
#include "rfl/internal/is_named_tuple.hpp"
#include "rfl/internal/nt_to_ptr_named_tuple.hpp"
#include "rfl/internal/ptr_field_tuple_t.hpp"
#include "rfl/named_tuple_t.hpp"

namespace rfl {
namespace internal {

template <class PtrFieldTupleType, class PtrNamedTupleType, class... Args>
auto make_pointers(PtrNamedTupleType& _n, Args... _args) {
    constexpr auto i = sizeof...(Args);

    constexpr auto size = std::tuple_size_v<std::decay_t<PtrFieldTupleType>>;

    if constexpr (i == size) {
        return std::make_tuple(_args...);
    } else {
        using Field = std::decay_t<std::tuple_element_t<i, PtrFieldTupleType>>;
        using T = std::decay_t<std::remove_pointer_t<typename Field::Type>>;

        if constexpr (is_named_tuple_v<T>) {
            using SubPtrNamedTupleType =
                typename std::invoke_result<decltype(nt_to_ptr_named_tuple<T>),
                                            T>::type;

            return make_pointers<PtrFieldTupleType>(
                _n, _args..., SubPtrNamedTupleType(_n).fields());

        } else if constexpr (is_flatten_field<Field>::value) {
            using SubPtrFieldTupleType = std::decay_t<ptr_field_tuple_t<T>>;

            return make_pointers<PtrFieldTupleType>(
                _n, _args..., make_pointers<SubPtrFieldTupleType>(_n));

        } else {
            return make_pointers<PtrFieldTupleType>(
                _n, _args..., _n.template get_field<Field::name_>());
        }
    }
}

template <class T, class Pointers, class... Args>
auto move_from_pointers(Pointers& _ptrs, Args&&... _args) {
    constexpr auto i = sizeof...(Args);
    if constexpr (i == std::tuple_size_v<std::decay_t<Pointers>>) {
        return T{std::move(_args)...};
    } else {
        using FieldType = std::tuple_element_t<i, std::decay_t<Pointers>>;

        if constexpr (is_field_v<FieldType>) {
            return move_from_pointers<T>(
                _ptrs, std::move(_args)...,
                rfl::make_field<FieldType::name_>(
                    std::move(*std::get<i>(_ptrs).value())));

        } else {
            using PtrFieldTupleType = std::decay_t<ptr_field_tuple_t<T>>;

            using U = std::decay_t<std::remove_pointer_t<
                typename std::tuple_element_t<i, PtrFieldTupleType>::Type>>;

            return move_from_pointers<T>(
                _ptrs, std::move(_args)...,
                move_from_pointers<U>(std::get<i>(_ptrs)));
        }
    }
}

/// Creates a struct of type T from a named tuple by moving the underlying
/// fields.
template <class T, class NamedTupleType>
T move_from_named_tuple(NamedTupleType&& _n) {
    using RequiredType = std::decay_t<named_tuple_t<T>>;

    if constexpr (is_named_tuple_v<std::decay_t<T>>) {
        return std::move(_n);

    } else if constexpr (std::is_same<std::decay_t<NamedTupleType>,
                                      RequiredType>()) {
        auto ptr_named_tuple = nt_to_ptr_named_tuple(_n);

        using PtrFieldTupleType = std::decay_t<ptr_field_tuple_t<T>>;

        auto pointers = make_pointers<PtrFieldTupleType>(ptr_named_tuple);

        return move_from_pointers<T>(pointers);

    } else {
        return move_from_named_tuple<T, RequiredType>(
            RequiredType(std::move(_n)));
    }
}

}  // namespace internal
}  // namespace rfl

#endif
