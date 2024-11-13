#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <fstream>
#include <spdlog/sinks/basic_file_sink.h>

#include <json/json.h>

#include "Plugin.h"

#define DLLEXPORT __declspec(dllexport)

namespace logger = SKSE::log;
using namespace std::literals;
namespace util
{
    using SKSE::stl::report_and_fail;
}

namespace stl {
    template <class T>
    void write_thunk_call(std::uintptr_t a_src)
    {
        SKSE::AllocTrampoline(14);

        auto& trampoline = SKSE::GetTrampoline();
        T::func = trampoline.write_call<5>(a_src, T::thunk);
    }

    template <typename TDest, typename TSource>
    constexpr auto write_vfunc() noexcept
    {
        REL::Relocation<std::uintptr_t> vtbl{ TDest::VTABLE[0] };
        TSource::func = vtbl.write_vfunc(TSource::idx, TSource::thunk);
    }

    template <typename T>
    constexpr auto write_vfunc(const REL::ID variant_id) noexcept
    {
        REL::Relocation<std::uintptr_t> vtbl{ variant_id };
        T::func = vtbl.write_vfunc(T::idx, T::thunk);
    }

    template <typename R, typename... Args>
    inline std::uintptr_t function_ptr(R(*fn)(Args...))
    {
        return reinterpret_cast<std::uintptr_t>(fn);
    }

    template <typename Class, typename R, typename... Args>
    inline std::uintptr_t function_ptr(R(Class::* fn)(Args...))
    {
        return reinterpret_cast<std::uintptr_t>((void*&)fn);
    }
}