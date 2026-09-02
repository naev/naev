# linopt-bench

Compares the solvers that could replace glpk in `nlua_linopt.c`, using real
problems the outfit optimiser produced.

    cargo run --release

With no arguments it uses the problems in `problems/`. Pass file paths to use
your own.

Every solver is driven through `good_lp`, since that is the API a replacement
would use.

The `HiGHS+` column is HiGHS with its search heuristics switched off. It is
built for problems thousands of times larger than ours, so it clears them
during its preparation step and then spends the rest of the time looking for an
answer it already has. A `*` marks a solver that picked a different loadout
than glpk.

## Collecting your own problems

The game can write a problem out, but only does it when equipping a pilot
fails. To catch every one, add this to `dat/scripts/equipopt/optimize.lua`
after `lp:load_matrix( ia, ja, ar )`:

```lua
if __debugging then
   local outfile = fmt.f( "logs/linopt-{date}-{ship}{shipid}.mps",
      {date=naev.date("%Y-%m-%d_%H-%M-%S"), ship=p:name(), shipid=p:id()})
   lp:write_problem( outfile )
end
```

Play a debug build for a minute and the files appear under `logs/` in naev's
save directory.

One catch: the file format cannot record whether the goal is to maximise or
minimise, and the optimiser maximises. Anything reading these files has to be
told, or it quietly solves the opposite problem and answers zero. This
benchmark sets it for every solver.
