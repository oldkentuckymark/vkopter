/*
Copyright 2022 Mark Richardson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <algorithm>
#include <deque>
#include <bit>
#include <iostream>
#include <limits>

template<class T, size_t MAX_SIZE, bool AUTO_CONSOLIDATE = true, class index_t = size_t>
class FixedVector
{
public:
    FixedVector() : pdata_(std::bit_cast<T*>(new char [MAX_SIZE * sizeof (T)]))
    {

        free_ranges_.emplace_front(0,MAX_SIZE - 1);
        \
    }

    ~FixedVector()
    {
        std::deque<size_t> skiplist;
        for(auto const & fr : free_ranges_)
        {
            for(size_t i = fr.first; i <= fr.second; ++i)
            {
                skiplist.push_back(i);
            }
        }

        for(size_t i = 0; i < current_size_; ++i)
        {
            T* p = pdata_ + i;
            if(std::find(skiplist.begin(),skiplist.end(),i) == skiplist.end() )
            {
                p->~T();
            }
        }

        delete [] std::bit_cast<char*>(pdata_);
    }

    FixedVector(FixedVector const & that) = delete;

    auto operator = (FixedVector const & that) -> FixedVector& = delete;

    auto operator [] (index_t const i) -> T&
    {
        return pdata_[i];
    }

    auto operator [] (index_t const i) const -> T
    {
        return pdata_[i];
    }

    template <typename... Args>
    auto emplace(Args&&... args) -> index_t
    {
        auto& fr = free_ranges_.front();
        auto const r = fr.first;
        new (pdata_ + r) T(std::forward<Args>(args)...);

        ++fr.first;
        if(fr.first > fr.second)
        {
            free_ranges_.erase(free_ranges_.begin());
        }
        if(r == current_size_)
        {
            ++current_size_;
        }

        return r;
    }

    auto insert(T const & t) -> index_t
    {
        auto& fr = free_ranges_.front();
        auto const r = fr.first;
        pdata_[r] = t;
        ++fr.first;
        if(fr.first > fr.second)
        {
            free_ranges_.erase(free_ranges_.begin());
        }
        if(r == current_size_)
        {
            ++current_size_;
        }
        return r;
    }

    auto erase(size_t const i) -> void
    {
        T* const op = pdata_ + i;
        op->~T();

        for(auto fri = free_ranges_.begin(); fri != free_ranges_.end(); ++fri)
        {
            if(fri->first > i)
            {
                free_ranges_.insert(fri,{i,i});
                if constexpr (AUTO_CONSOLIDATE)
                {
                    consolidateRanges();
                }
                return;
            }
        }
        free_ranges_.emplace_back(i,i);
        if constexpr (AUTO_CONSOLIDATE)
        {
            consolidateRanges();
        }
    }

    [[nodiscard]] auto getSize() const -> index_t
    {
        return current_size_;
    }

    [[nodiscard]] auto getCurrentSizeInBytes() const -> index_t
    {
        return current_size_ * sizeof(T);
    }

    [[nodiscard]] static auto getMaxSizeInBytes() -> index_t
    {
        return MAX_SIZE * sizeof(T);
    }

    [[nodiscard]] auto isEmpty() const -> bool
    {
        return current_size_ == 0;
    }

    [[nodiscard]] auto isFull() const -> bool
    {
        return current_size_ == MAX_SIZE;
    }

    auto data() -> T*
    {
        return pdata_;
    }

    auto clear() -> void
    {
        std::deque<size_t> skiplist;
        for(auto const& fr : free_ranges_)
        {
            for(size_t i = fr.first; i <= fr.second; ++i)
            {
                skiplist.push_back(i);
            }
        }

        for(size_t i = 0; i < current_size_; ++i)
        {
            T* p = pdata_ + i;
            if(std::find(skiplist.begin(),skiplist.end(),i) == skiplist.end() )
            {
                p->~T();
            }
        }

        current_size_ = 0;
        free_ranges_.clear();
        free_ranges_.emplace_front(0,MAX_SIZE-1);
    }

    auto consolidateRanges() -> void
    {

        bool pair_found = true;
        while(pair_found)
        {
            pair_found = join_first_pair();
        }
    }


    friend struct Iterator;
    struct Iterator
    {

        explicit Iterator(FixedVector<T, MAX_SIZE, AUTO_CONSOLIDATE,index_t> & o, index_t const i) :
            obj(o),
            idx(i)
        {

        }

        auto operator*() const -> T& { return obj[idx]; }

        auto operator++() -> Iterator&
        {
            idx = obj.get_next(idx);
            return *this;
        }
        auto operator++(int) -> Iterator { Iterator tmp = *this; ++(*this); return tmp; }
        friend auto operator== (const Iterator& a, const Iterator& b) -> bool { return &a.obj == &b.obj && a.idx == b.idx; };
        friend auto operator!= (const Iterator& a, const Iterator& b) -> bool { return !(&a.obj == &b.obj && a.idx == b.idx); };

    private:
        FixedVector<T, MAX_SIZE, AUTO_CONSOLIDATE,index_t> & obj;
        index_t idx;
    };

    auto begin() -> Iterator { return Iterator(*this,0); }
    auto end() -> Iterator   { return Iterator(*this, current_size_); }


private:
    T* pdata_;
    std::deque<std::pair<size_t, size_t>> free_ranges_;
    size_t current_size_ = 0;


    static auto valid_range(std::pair<size_t, size_t> r) -> bool
    {
        return r.first <= r.second;
    }

    auto join_first_pair() -> bool
    {
        if(free_ranges_.size() < 2) { return false; }

        auto fri1 = free_ranges_.begin();
        decltype(fri1) fri2;// = free_ranges_.begin();

        for(fri1 = free_ranges_.begin(); fri1 != free_ranges_.end() - 1; ++fri1 )
        {
            fri2 = fri1 + 1;
            if(fri1->second + 1 == fri2->first)
            {
                fri2->first = fri1->first;
                free_ranges_.erase(fri1);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] auto get_next(size_t const c) const -> size_t
    {
        auto c2 = c+1;
        for(auto const & fr : free_ranges_)
        {
            if(fr.first > c2)
            {
                return c2;
            }
            else if(fr.second < c2)
            {

            }
            else
            {
                c2 = fr.second+1;
            }
        }

        if(c2 >= current_size_)
        {
            c2 = current_size_;
        }

        return c2;
    }



};

