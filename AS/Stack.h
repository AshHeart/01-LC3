/**
 * Copyright (c) 2022, the AshHeart/01-LC3 developers.
 */

#pragma once

#include <vector>

namespace AS {

    template<typename T>
    class Stack {
    public:
        Stack() = default;
        ~Stack() = default;

        bool push(const T& item)
        {
            if (m_stack.size() >= m_stack_size)
                return false;

            m_stack.push_back(item);
            return true;
        }

        bool is_empty() const
        {
            return m_stack.empty();
        }

        std::size_t size() const
        {
            return m_stack.size();
        }

        bool pop()
        {
            if (is_empty())
                return false;

            m_stack.pop_back();
            return true;
        }

        T& pop_element()
        {
            return m_stack.back();
        }

    private:
        std::vector<T> m_stack;
        std::size_t m_stack_size;
    };
}

using AS::Stack;
