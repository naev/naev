#!/usr/bin/env python3

# Lua LLDB Helpers
# Original GDB version by Michal Kottman
# License: MIT

import lldb

def luavalue(debugger, command, result, internal_dict):
    args = command.split()
    if len(args) == 0:
        result.AppendMessage("Usage: luavalue <index> [L]")
    else:
        index = int(args[0])
        L = args[1] if len(args) > 1 else "L"
        debugger.HandleCommand(f"expr void* $L = (void*)({L})")
        debugger.HandleCommand(f"expr void* $obj = (void*)index2adr($L, {index})")

lldb.debugger.HandleCommand('command script add -f __main__.luavalue luavalue')
lldb.debugger.HandleCommand('''command script add -o -h "luavalue <index> [L]\nProvides a pointer to a TValue from a stack index. By default, uses the current variable L as a lua_State pointer, but can be specified as the second argument. The return value is passed in variable $obj." luavalue''')

def luaprint(debugger, command, result, internal_dict):
    args = command.split()
    if len(args) == 0:
        result.AppendMessage("Usage: luaprint <value> [verbose]")
    else:
        v = args[0]
        verbose = int(args[1]) if len(args) > 1 else 0
        debugger.HandleCommand(f"expr int $type = ((TValue*){v})->tt")
        if $type == 0:
            result.AppendMessage("nil")
        elif $type == 1:
            val = "(bool)(((TValue*){v})->value.b)"
            debugger.HandleCommand(f"expr bool $val = {val}")
            result.AppendMessage("true" if $val else "false")
        elif $type == 2:
            val = "(void*)(((TValue*){v})->value.p)"
            result.AppendMessage(f"<ludata>{val}")
        elif $type == 3:
            val = "((TValue*){v})->value.n"
            result.AppendMessage(f"{val}")
        elif $type == 4:
            ts = "(TString*)(((TValue*){v})->value.gc->ts)"
            str = "(char*)({ts} + 1)"
            result.AppendMessage(f"'{str}'")
        elif $type == 5:
            tab = "(Table*)(((TValue*){v})->value.gc->h)"
            result.AppendMessage(f"<tab> {tab}")
            if verbose:
                luaprinttable(debugger, tab, result, internal_dict)
        elif $type == 7:
            uv = "(((TValue*){v})->value.gc.u->uv)"
            size = "uv.len"
            result.AppendMessage(f"<udata> {size}")
            if verbose and uv.metatable:
                result.AppendMessage("metatable=")
                luaprinttable(debugger, uv.metatable, result, internal_dict)
            if verbose and uv.env:
                result.AppendMessage("env=")
                luaprinttable(debugger, uv.env, result, internal_dict)
        else:
            typename = "lua_typename(L, $type)"
            val = "((TValue*){v})->value"
            result.AppendMessage(f"<{typename}> {val}")

lldb.debugger.HandleCommand('command script add -f __main__.luaprint luaprint')
lldb.debugger.HandleCommand('''command script add -o -h "luaprint <value> [verbose]\nPretty-prints a TValue passed as argument. Expects a pointer to a TValue. When verbose is 1, expands tables, metatables, and userdata environments." luaprint''')

def luaprinttable(debugger, command, result, internal_dict):
    args = command.split()
    if len(args) != 1:
        result.AppendMessage("Usage: luaprinttable <table>")
    else:
        t = args[0]
        result.AppendMessage(" { ")
        node = f"((Table*){t})->node"
        i = 0
        last = "1 << ((Table*){t})->lsizenode"
        while i < last:
            node_i = f"{node} + {i}"
            key = f"{node_i}->i_key"
            tvk_tt = f"((TValue*){key}).tt"
            if tvk_tt > 0:
                if tvk_tt == 4:
                    ts = f"(TString*)(((TValue*){key}).value.gc->ts)"
                    str = f"(char*)({ts} + 1)"
                    result.AppendMessage(f"{str} = ")
                else:
                    typename = f"lua_typename(L, {tvk_tt})"
                    result.AppendMessage(f"<{typename}> = ")
                val = f"{node_i}->i_val"
                luaprint(debugger, val, result, internal_dict)
                result.AppendMessage(",\n")
            i += 1
        i = 0
        while i < t.sizearray:
            val = f"((Table*){t})->array + {i}"
            luaprint(debugger, val, result, internal_dict)
            result.AppendMessage(", ")
            i += 1
        result.AppendMessage(" } ")

lldb.debugger.HandleCommand('command script add -f __main__.luaprinttable luaprinttable')
lldb.debugger.HandleCommand('''command script add -o -h "luaprinttable <table>\nPretty-prints a Lua Table. Expects a pointer to Table." luaprinttable''')

def luastack(debugger, command, result, internal_dict):
    args = command.split()
    L = args[0] if len(args) > 0 else "L"
    result.AppendMessage(f"Lua stack trace: {lua_gettop(L)} items")
    ptr = f"{L}->base"
    idx = 1
    while ptr < f"{L}->top":
        result.AppendMessage(f"{idx:03d}: ")
        luaprint(debugger, ptr, result, internal_dict)
        result.AppendMessage("\n")
        ptr = f"{ptr} + 1"
        idx += 1

lldb.debugger.HandleCommand('command script add -f __main__.luastack luastack')
lldb.debugger.HandleCommand('''command script add -o -h "luastack [L]\nPrints values on the Lua C stack. Without arguments, uses the current value of 'L' as the lua_State*. You can provide an alternate lua_State as the first argument." luastack''')

def luatrace(debugger, command, result, internal_dict):
    args = command.split()
    L = args[0] if len(args) > 0 else "L"
    if luaL_loadstring(L, "return debug.traceback()") == 0:
        if lua_pcall(L, 0, 1, 0) == 0:
            result.AppendMessage(f"{lua_tolstring(L, -1, 0)}\n")
        else:
            result.AppendMessage(f"ERROR: {lua_tolstring(L, -1, 0)}\n")
        lua_settop(L, -2)

lldb.debugger.HandleCommand('command script add -f __main__.luatrace luatrace')
lldb.debugger.HandleCommand('''command script add -o -h "luatraceback [L]\nDumps Lua execution stack, as debug.traceback() does. Without arguments, uses the current value of 'L' as the lua_State*. You can provide an alternate lua_State as the first argument." luatrace''')
