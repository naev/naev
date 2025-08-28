#!/usr/bin/env python3

# Lua LLDB Helpers
# LLDB port based on the original GDB version by Michal Kottman
# License: MIT

import lldb

# Utility functions
def get_uint(expr):
   return int(expr.GetValueAsUnsigned())

def get_str(expr):
   return expr.GetSummary().strip('"')

# Command functions
def luavalue(debugger, command, result, internal_dict):
   args = command.split()
   if len(args) == 0:
      result.AppendMessage("Usage: luavalue <index> [L]")
   else:
      index = int(args[0])
      L = args[1] if len(args) > 1 else "L"
      debugger.HandleCommand(f"expr void* $L = (void*)({L})")
      debugger.HandleCommand(f"expr void* $obj = (void*)index2adr($L, {index})")

def luaprint(debugger, command, result, internal_dict):
   args = command.split()
   if len(args) == 0:
      result.AppendMessage("Usage: luaprint <value> [verbose]")
   else:
      v = args[0]
      verbose = int(args[1]) if len(args) > 1 else 0
      debugger.HandleCommand(f"expr int $type = ((TValue*){v})->tt")
      type_val = get_uint(debugger.GetSelectedTarget().EvaluateExpression("$type"))
      if type_val == 0:
         result.AppendMessage("nil")
      elif type_val == 1:
         debugger.HandleCommand(f"expr bool $val = ((TValue*){v})->value.b")
         val = bool(get_uint(debugger.GetSelectedTarget().EvaluateExpression("$val")))
         result.AppendMessage("true" if val else "false")
      elif type_val == 2:
         debugger.HandleCommand(f"expr void* $p = ((TValue*){v})->value.p")
         p = get_uint(debugger.GetSelectedTarget().EvaluateExpression("$p"))
         result.AppendMessage(f"<ludata>{p}")
      elif type_val == 3:
         debugger.HandleCommand(f"expr double $n = ((TValue*){v})->value.n")
         n = get_str(debugger.GetSelectedTarget().EvaluateExpression("$n"))
         result.AppendMessage(f"{n}")
      elif type_val == 4:
         debugger.HandleCommand(f"expr TString* $ts = (TString*)(((TValue*){v})->value.gc->ts)")
         ts = get_str(debugger.GetSelectedTarget().EvaluateExpression("$ts"))
         result.AppendMessage(f"'{ts}'")
      elif type_val == 5:
         debugger.HandleCommand(f"expr Table* $tab = (Table*)(((TValue*){v})->value.gc->h)")
         tab = get_str(debugger.GetSelectedTarget().EvaluateExpression("$tab"))
         result.AppendMessage(f"<tab> {tab}")
         if verbose:
            luaprinttable(debugger, "$tab", result, internal_dict)
      elif type_val == 7:
         debugger.HandleCommand(f"expr auto $uv = (((TValue*){v})->value.gc.u->uv)")
         uv = get_str(debugger.GetSelectedTarget().EvaluateExpression("$uv"))
         result.AppendMessage(f"<udata> {uv}")
         uv_metatable = debugger.GetSelectedTarget().EvaluateExpression("$uv.metatable")
         if verbose and uv_metatable.IsValid():
            result.AppendMessage(" metatable=")
            luaprinttable(debugger, "$uv.metatable", result, internal_dict)
         uv_env = debugger.GetSelectedTarget().EvaluateExpression("$uv.env")
         if verbose and uv_env.IsValid():
            result.AppendMessage(" env=")
            luaprinttable(debugger, "$uv.env", result, internal_dict)
      else:
         debugger.HandleCommand("expr const char* $typename = lua_typename(L, $type)")
         typename = get_str(debugger.GetSelectedTarget().EvaluateExpression("$typename"))
         val = get_str(debugger.GetSelectedTarget().EvaluateExpression(f"((TValue*){v})->value"))
         result.AppendMessage(f"<{typename}> {val}")

def luaprinttable(debugger, command, result, internal_dict):
   args = command.split()
   if len(args) != 1:
      result.AppendMessage("Usage: luaprinttable <table>")
   else:
      t = args[0]
      result.AppendMessage(" { ")
      debugger.HandleCommand(f"expr auto $node = ((Table*){t})->node")
      lsizenode_expr = debugger.GetSelectedTarget().EvaluateExpression(f"((Table*){t})->lsizenode")
      lsizenode = get_uint(lsizenode_expr)
      last = 1 << lsizenode
      i = 0
      while i < last:
         debugger.HandleCommand(f"expr auto $node_i = $node + {i}")
         debugger.HandleCommand("expr auto $key = $node_i->i_key")
         tvk_expr = debugger.GetSelectedTarget().EvaluateExpression("((TValue*)$key)->tt")
         tvk_tt = get_uint(tvk_expr)
         if tvk_tt > 0:
            if tvk_tt == 4:
               debugger.HandleCommand("expr TString* $ts = (TString*)(((TValue*)$key)->value.gc->ts)")
               ts = get_str(debugger.GetSelectedTarget().EvaluateExpression("$ts"))
               result.AppendMessage(f"{ts} = ")
            else:
               debugger.HandleCommand(f"expr const char* $typename = lua_typename(L, {tvk_tt})")
               typename = get_str(debugger.GetSelectedTarget().EvaluateExpression("$typename"))
               result.AppendMessage(f"<{typename}> = ")
            debugger.HandleCommand("expr auto $val = $node_i->i_val")
            luaprint(debugger, "$val", result, internal_dict)
            result.AppendMessage(",\n")
         i += 1
      i = 0
      sizearray_expr = debugger.GetSelectedTarget().EvaluateExpression(f"((Table*){t})->sizearray")
      sizearray = get_uint(sizearray_expr)
      while i < sizearray:
         debugger.HandleCommand(f"expr auto $val = ((Table*){t})->array + {i}")
         luaprint(debugger, "$val", result, internal_dict)
         result.AppendMessage(", ")
         i += 1
      result.AppendMessage(" } ")

def luastack(debugger, command, result, internal_dict):
   args = command.split()
   L = args[0] if len(args) > 0 else "L"
   debugger.HandleCommand(f"expr int $stack_size = lua_gettop({L})")
   stack_size = get_uint(debugger.GetSelectedTarget().EvaluateExpression("$stack_size"))
   result.AppendMessage(f"Lua stack trace: {stack_size} items")
   debugger.HandleCommand(f"expr auto $ptr = {L}->base")
   idx = 1
   while True:
      debugger.HandleCommand(f"expr bool $is_less = $ptr < {L}->top")
      is_less_expr = debugger.GetSelectedTarget().EvaluateExpression("$is_less")
      if is_less_expr.GetValue().strip() != "true":
         break
      result.AppendMessage(f"{idx:03d}: ")
      luaprint(debugger, "$ptr", result, internal_dict)
      result.AppendMessage("\n")
      debugger.HandleCommand("expr $ptr = $ptr + 1")
      idx += 1

# Registration function to add all commands
def register_commands():
   lldb.debugger.HandleCommand('command script add -f __main__.luavalue luavalue')
   lldb.debugger.HandleCommand('''command script add -o -h "luavalue <index> [L]\nProvides a pointer to a TValue from a stack index. By default, uses the current variable L as a lua_State pointer, but can be specified as the second argument. The return value is passed in variable $obj." luavalue''')
   lldb.debugger.HandleCommand('command script add -f __main__.luaprint luaprint')
   lldb.debugger.HandleCommand('''command script add -o -h "luaprint <value> [verbose]\nPretty-prints a TValue passed as argument. Expects a pointer to a TValue. When verbose is 1, expands tables, metatables, and userdata environments." luaprint''')
   lldb.debugger.HandleCommand('command script add -f __main__.luaprinttable luaprinttable')
   lldb.debugger.HandleCommand('''command script add -o -h "luaprinttable <table>\nPretty-prints a Lua Table. Expects a pointer to Table." luaprinttable''')
   lldb.debugger.HandleCommand('command script add -f __main__.luastack luastack')

# Register commands immediately
register_commands()
