#pragma once

#include "base\base_mutex.h"

namespace mylib
{

    template <typename T>
    class single_ptr
    {
    public:
        single_ptr() = default;
        virtual ~single_ptr() = default;

        single_ptr(const single_ptr& other) = delete;
        single_ptr(single_ptr&& other) = delete;

        static T* get() {
            return _inst;
        }
        static bool is_valid() {
            return (_inst) ? true : false;
        }

        template<class... ARGS>
        static bool make_single(ARGS&&... args) {
            scoped_lt_lock slock(_inst_lock);
            if (is_valid()) {
                return false;
            }
            _inst = new T(std::forward<ARGS>(args)...);
            return true;
        }
        static bool delete_single() {
            scoped_lt_lock slock(_inst_lock);
            if (!is_valid()) {
                return false;
            }
            delete _inst;
            _inst = nullptr;
            return true;
        }

    private:
        inline static lt_lock _inst_lock;
        inline static T* _inst;
    };

}