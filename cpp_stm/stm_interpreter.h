#ifndef STM_INTERPRETER_H
#define STM_INTERPRETER_H

#include <functional>
#include <any>
#include <variant>
#include <iostream>

#include <unit.h>
#include <identity.h>

#include "tvar.h"
#include "stm_free.h"
#include "context.h"

namespace stm
{

template <typename Ret, template <typename> class Visitor>
RunResult<Ret> runSTML(AtomicRuntime& runtime, const STML<Ret>& stml)
{
    Visitor<Ret> visitor(runtime);
    std::visit(visitor, stml.stml);
    return visitor.result;
}

// forward declaration;
template <typename Ret>
struct StmlVisitor;

template <typename Ret>
struct StmfVisitor
{
    AtomicRuntime& _runtime;

    StmfVisitor(AtomicRuntime& runtime)
        : _runtime(runtime)
    {
    }

    RunResult<Ret> result;

    template <typename A>
    void operator()(const NewTVar<A, STML<Ret>>& f)
    {
        auto tvarId = _runtime.newGUID();

        std::cout << "\nNewTVar. TVar GUID: " << tvarId;

        TVarHandle tvarHandle { _runtime.getUStamp(), f.val };
        _runtime.addTVarHandle(tvarId, tvarHandle);
        TVarAny tvar { tvarId };
        result = runSTML<Ret, StmlVisitor>(_runtime, f.next(tvar));
    }

    template <typename A>
    void operator()(const ReadTVar<A, STML<Ret>>& f)
    {
        std::cout << "\nReadTVar. TVar GUID: " << f.tvar.id;

        TVarHandle tvarHandle = _runtime.getTVarHandle(f.tvar.id);
        result = runSTML<Ret, StmlVisitor>(_runtime, f.next(tvarHandle.data));
    }

    template <typename A>
    void operator()(const WriteTVar<A, STML<Ret>>& f)
    {
        std::cout << "\nWriteTVar. TVar GUID: " << f.tvar.id;

        _runtime.setTVarHandleData(f.tvar.id, f.val);
        result = runSTML<Ret, StmlVisitor>(_runtime, f.next(fp::unit));
    }

    template <typename A>
    void operator()(const Retry<A, STML<Ret>>&)
    {
        std::cout << "\nRetry.";

        result = { true, std::nullopt };
    }

//    void operator()(const RetryA<STML<Ret>>&)
//    {
//        std::cout << "\nRetryA.";

//        result = { true, std::nullopt };
//    }
};

template <typename Ret>
struct StmlVisitor
{
    AtomicRuntime& _runtime;

    StmlVisitor(AtomicRuntime& runtime)
        : _runtime(runtime)
    {
    }

    RunResult<Ret> result;

    void operator()(const PureF<Ret>& p)
    {
        std::cout << "\nPureF";
        result = { false, unPureF(p) };
    }

    void operator()(const FreeF<Ret>& f)
    {
        std::cout << "\nFreeF";
        StmfVisitor<Ret> visitor(_runtime);
        std::visit(visitor, f.stmf.stmf);
        result = visitor.result;
    }
};

} // namespace stm

#endif // STM_INTERPRETER_H