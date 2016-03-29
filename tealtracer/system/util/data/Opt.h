///
/// Opt.h
/// -----
/// Nikolai Shkurkin
/// Utility Library
///

#ifndef ____util_Opt__
#define ____util_Opt__

#include <src/util/util_base.h>

namespace util {

    /// `Opt` is a sort of C++ way of handling optionally set types (e.g. like
    /// with the new Swift Language). Supports Eigen types. You might use this
    /// sort of like a pointer, where `NULL` means you point to nothing yet.
    /// So here, isSet() indicates whether you have set your "pointer" to
    /// something.
    template<typename T>
    struct Opt {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        
        /// The underlying value that is stored.
        T value;
        /// Whether or not `value` has been set.
        bool _isSet;
        
        /// Creates an Optional type using `T`'s default constructor.
        Opt() : _isSet(false) {};
        /// Creates an Optional type using `defVaue` as a base value.
        Opt(T defValue): _isSet(false) {value = defValue;}
        
        /// Sets `value` to `newValue`, and causes `isSet` to return true.
        void set(T newValue) {
            _isSet = true;
            value = newValue;
        }
        
        /// Whether or not `value` has been set.
        bool isSet() {return _isSet;}
        void clearSet() {_isSet = false;}
        /// Gets the underlying `value` of this Opt type.
        T& get() {return value;}
    };
}

#endif // ____util_Opt__
