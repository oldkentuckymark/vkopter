#pragma once

#include <array>
#include <vector>
#include <cstdint>

template<class T, uint64_t WIDTH, uint64_t HEIGHT>
class Array2d
{
public:
    auto operator() (std::size_t const x, std::size_t const y) -> T&
    {
        return data_[y*WIDTH+x];
    }

    auto at(std::size_t const x, std::size_t const y) -> T&
    {
        return data_[y*WIDTH+x];
    }

    auto data() -> T*
    {
        return data_.data();
    }

private:
    std::array<T, WIDTH*HEIGHT> data_;

};

template<class T>
class Vector2d
{
public:
    explicit Vector2d(std::size_t const width = 0, std::size_t const height = 0) :
        width_(width), height_(height)
    {
        std::size_t const size = width * height;
        data_.resize(size);
    }

    auto resize(std::size_t const width, std::size_t const height) -> void
    {
        data_.clear();
        width_ = width;
        height_ = height;
        data_.resize(width_*height_);
    }

    auto clear() -> void
    {
        data_.clear();
    }

    auto fill(T value) -> void
    {
        for(auto& i : data_)
        {
            i = value;
        }
    }

    auto operator() (std::size_t const x, std::size_t const y) -> T&
    {
        return data_[y*width_+x];
    }

    auto data() -> T*
    {
        return data_.data();
    }

    auto at(size_t const x, std::size_t const y) -> T&
    {
        return data_[y*width_+x];
    }

    [[nodiscard]] auto getSize() const -> size_t
    {
        return width_ * height_;
    }

    [[nodiscard]] auto getWidth() const -> size_t
    {
        return width_;
    }

    [[nodiscard]] auto getHeight() const -> size_t
    {
        return height_;
    }


private:
    std::vector<T> data_;
    std::size_t width_ = 0;
    std::size_t height_ = 0;
};
