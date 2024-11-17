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
        type = int(debugger.GetSelectedTarget().EvaluateExpression("$type").GetValue())
        if type == 0:
            result.AppendMessage("nil")
        elif type == 1:
            debugger.HandleCommand(f"expr bool $val = (bool)(((TValue*){v})->value.b)")
            val = bool(debugger.GetSelectedTarget().EvaluateExpression("$val").GetValue())
            result.AppendMessage("true" if val else "false")
        elif type == 2:
            debugger.HandleCommand(f"expr void* $p = ((TValue*){v})->value.p")
            p = debugger.GetSelectedTarget().EvaluateExpression("$p").GetValueAsUnsigned()
            result.AppendMessage(f"<ludata>{p}")
        elif type == 3:
            debugger.HandleCommand(f"expr double $n = ((TValue*){v})->value.n")
            n = debugger.GetSelectedTarget().EvaluateExpression("$n").GetValue()
            result.AppendMessage(f"{n}")
        elif type == 4:
            debugger.HandleCommand(f"expr TString* $ts = (TString*)(((TValue*){v})->value.gc->ts)")
            ts = debugger.GetSelectedTarget().EvaluateExpression("$ts").GetValue()
            result.AppendMessage(f"'{ts}'")
        elif type == 5:
            debugger.HandleCommand(f"expr Table* $tab = (Table*)(((TValue*){v})->value.gc->h)")
            tab = debugger.GetSelectedTarget().EvaluateExpression("$tab").GetValue()
            result.AppendMessage(f"<tab> {tab}")
            if verbose:
                luaprinttable(debugger, tab, result, internal_dict)
        elif type == 7:
            debugger.HandleCommand(f"expr auto $uv = (((TValue*){v})->value.gc.u->uv)")
            uv = debugger.GetSelectedTarget().EvaluateExpression("$uv").GetValue()
            result.AppendMessage(f"<udata> {uv}")
            if verbose and debugger.GetSelectedTarget().EvaluateExpression("$uv.metatable").IsValid():
                result.AppendMessage("metatable=")
                luaprinttable(debugger, "$uv.metatable", result, internal_dict)
            if verbose and debugger.GetSelectedTarget().EvaluateExpression("$uv.env").IsValid():
                result.AppendMessage("env=")
                luaprinttable(debugger, "$uv.env", result, internal_dict)
        else:
            debugger.HandleCommand(f"expr const char* $typename = lua_typename(L, $type)")
            typename = debugger.GetSelectedTarget().EvaluateExpression("$typename").GetSummary()
            val = debugger.GetSelectedTarget().EvaluateExpression(f"((TValue*){v})->value").GetSummary()
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
        debugger.HandleCommand(f"expr auto $node = ((Table*){t})->node")
        i = 0
        last = 1 << int(debugger.GetSelectedTarget().EvaluateExpression(f"((Table*){t})->lsizenode").GetValue())
        while i < last:
            debugger.HandleCommand(f"expr auto $node_i = $node + {i}")
            debugger.HandleCommand(f"expr auto $key = $node_i->i_key")
            tvk_tt = int(debugger.GetSelectedTarget().EvaluateExpression(f"((TValue*)$key).tt").GetValue())
            if tvk_tt > 0:
                if tvk_tt == 4:
                    debugger.HandleCommand(f"expr TString* $ts = (TString*)(((TValue*)$key).value.gc->ts)")
                    ts = debugger.GetSelectedTarget().EvaluateExpression("$ts").GetValue()
                    result.AppendMessage(f"{ts} = ")
                else:
                    debugger.HandleCommand(f"expr const char* $typename = lua_typename(L, {tvk_tt})")
                    typename = debugger.GetSelectedTarget().EvaluateExpression("$typename").GetSummary()
                    result.AppendMessage(f"<{typename}> = ")
                debugger.HandleCommand(f"expr auto $val = $node_i->i_val")
                luaprint(debugger, f"$val", result, internal_dict)
                result.AppendMessage(",\n")
            i += 1
        i = 0
        sizearray = int(debugger.GetSelectedTarget().EvaluateExpression(f"((Table*){t})->sizearray").GetValue())
        while i < sizearray:
            debugger.HandleCommand(f"expr auto $val = ((Table*){t})->array + {i}")
            luaprint(debugger, f"$val", result, internal_dict)
            result.AppendMessage(", ")
            i += 1
        result.AppendMessage(" } ")

lldb.debugger.HandleCommand('command script add -f __main__.luaprinttable luaprinttable')
lldb.debugger.HandleCommand('''command script add -o -h "luaprinttable <table>\nPretty-prints a Lua Table. Expects a pointer to Table." luaprinttable''')

def luastack(debugger, command, result, internal_dict):
    args = command.split()
    L = args[0] if len(args) > 0 else "L"
    debugger.HandleCommand(f"expr int $stack_size = lua_gettop({L})")
    stack_size = int(debugger.GetSelectedTarget().EvaluateExpression("$stack_size").GetValue())
    result.AppendMessage(f"Lua stack trace: {stack_size} items")
    debugger.HandleCommand(f"expr auto $ptr = {L}->base")
    idx = 1
    while True:
        debugger.HandleCommand(f"expr bool $is_less = $ptr < {L}->top")
        if not bool(debugger.GetSelectedTarget().EvaluateExpression("$is_less").GetValue()):
            break
