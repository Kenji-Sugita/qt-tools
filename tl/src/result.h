// 成功値とエラーを一つの戻り値で扱うための軽量 Result 型。
#pragma once

#include "tlerror.h"

#include <optional>

template <typename T>
class Result
{
public:
    static Result success(T value)
    {
        Result result;
        result.m_value = std::move(value);
        return result;
    }

    static Result failure(TlError error)
    {
        Result result;
        result.m_error = std::move(error);
        return result;
    }

    bool ok() const
    {
        return m_value.has_value();
    }

    const T &value() const
    {
        return *m_value;
    }

    T &&takeValue()
    {
        return std::move(*m_value);
    }

    const TlError &error() const
    {
        return *m_error;
    }

private:
    std::optional<T> m_value;
    std::optional<TlError> m_error;
};
