#ifndef STM_H
#define STM_H

#include "context.h"
#include "stm_runtime.h"
#include "stm_free.h"
#include "stm_bind.h"
#include "tvar.h"

namespace stm
{

template <typename A>
using STML = free::STML<A>;

template <typename A>
A atomically(Context& context, const STML<A>& stml)
{
    return runSTM(context, stml);
}

const auto newTVar = [](const auto& val)
{
    return free::newTVar(val);
};

const auto readTVar = [](const auto& tvar)
{
    return free::readTVar(tvar);
};

const auto writeTVar = [](const auto& tvar)
{
    return [&](const auto& val)
    {
        return free::writeTVar(tvar, val);
    };
};

const auto writeTVarV = [](const auto& val)
{
    return [&](const auto& tvar)
    {
        return free::writeTVar(tvar, val);
    };
};

const auto modifyTVarV = [](const auto& tvar, const auto& f)
{
    auto m = free::readTVar(tvar);
    return free::bind(m, [=](const auto& val)
    {
        return free::writeTVar(tvar, f(val));
    } );
};

const auto retry = [](const auto&)
{
    return free::retry<fp::Unit>();
};

const auto pure = [](const auto& a)
{
    return free::pureF(a);
};

template <typename A, typename B>
STML<B> bind(const STML<A> ma, const free::ArrowFunc<A, B>& f)
{
    return free::bind(ma, f);
}

template <typename A, typename B, typename Ret>
STML<Ret> both(const STML<A>& ma,
               const STML<B>& mb,
               const std::function<Ret(A, B)>& f)
{
    return bind(ma, [=](const A& a){
        return bind(mb, [=](const B& b){
            return pure(f(a, b));
        });
    });
}

template <typename A, typename B>
STML<fp::Unit>
bothVoided(const STML<A>& ma,
           const STML<B>& mb)
{
    return bind(ma, [&](const A&){
        return bind(mb, [](const B&){
            return pure(fp::unit);
        });
    });
}

const auto voided = [](const auto& ma)
{
    return bind(ma, [](const auto&) {
        return pure(fp::unit);
    });
};

} // namespace stm

#endif // STM_H
