#ifndef RFL_BOX_HPP_
#define RFL_BOX_HPP_

#include <memory>
#include <stdexcept>

namespace rfl {

/// The Box class behaves very similarly to the unique_ptr, but unlike the
/// unique_ptr, it is 100% guaranteed to be filled at all times.
template <class T>
class Box {
   public:
    /// The only way of creating new boxes is
    /// Box<T>::make(...).
    template <class... Args>
    static Box<T> make(Args&&... _args) {
        return Box<T>(std::make_unique<T>(std::forward<Args>(_args)...));
    }

    Box() : ptr_(std::make_unique<T>()) {}

    Box(const Box<T>& _other) = delete;

    Box(Box<T>&& _other) = default;

    template <class U, typename std::enable_if<std::is_convertible_v<U, T>,
                                               bool>::type = true>
    Box(Box<U>&& _other) noexcept
        : ptr_(std::make_unique<T>(std::forward<U>(*_other))) {}

    ~Box() = default;

    /// Returns a pointer to the underlying object
    T* get() const { return ptr_.get(); }

    /// Copy assignment operator
    Box<T>& operator=(const Box<T>& _other) = delete;

    /// Move assignment operator
    Box<T>& operator=(Box<T>&& _other) noexcept = default;

    /// Move assignment operator
    template <class U, typename std::enable_if<std::is_convertible_v<U, T>,
                                               bool>::type = true>
    Box<T>& operator=(Box<U>&& _other) noexcept {
        ptr_ = std::make_unique<T>(std::forward<U>(*_other));
        return *this;
    }

    /// Returns the underlying object.
    T& operator*() { return *ptr_; }

    /// Returns the underlying object.
    T& operator*() const { return *ptr_; }

    /// Returns the underlying object.
    T* operator->() { return ptr_.get(); }

    /// Returns the underlying object.
    T* operator->() const { return ptr_.get(); }

    /// Returns the underlying unique_ptr
    const std::unique_ptr<T>& ptr() const { return ptr_; }

   private:
    /// Only make is allowed to use this constructor.
    explicit Box(std::unique_ptr<T>&& _ptr) : ptr_(std::move(_ptr)) {}

   private:
    /// The underlying unique_ptr_
    std::unique_ptr<T> ptr_;
};

/// Generates a new Ref<T>.
template <class T, class... Args>
Box<T> make_box(Args&&... _args) {
    return Box<T>::make(std::forward<Args>(_args)...);
}

template <class T1, class T2>
inline auto operator<=>(const Box<T1>& _t1, const Box<T2>& _t2) {
    return _t1.ptr() <=> _t2.ptr();
}

template <class CharT, class Traits, class T>
inline std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& _os, const Box<T>& _b) {
    _os << _b.get();
    return _os;
}

}  // namespace rfl

#endif
