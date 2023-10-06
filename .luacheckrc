-- vim: set expandtab shiftwidth=3 syntax=lua:

-- Load automatically generated API
local stds_gen = require "utils.luacheckrc_gen"
for k,v in pairs(stds_gen) do
   stds[k] = v
end
-- We do some modifications locally here
stds.ai.read_globals.__ai = {}

-- Note that naev actually contains all the API, so we assume the developers knows what they are using
std = "lua51+love+Basic+naev"

-- It would be nice to enforce a ~120 char limit for regular lines of code, but missions etc. have inline text of considerable size.
-- Note: there's a `max_string_line_length` option, but that only applies when the line ending is inside a string. Useless.
max_line_length = 4096

-- Ignore unused variables whose names start with "_".
-- The default convention allows "_" itself as an unused variable, but we reserve that for gettext.
-- Furthermore, ignore all unused loop variables.
-- Finally, don't complain an _-prefixed variable is "never accessed" (e.g., if you set the tuple _foo, bar twice).
-- Iteration idioms in Lua all but require ipairs(), and whether we happen to use the key or value isn't fundamental.
ignore = {"21./_.*", "213", "231/_.*"}

-- The following corresponds to internal Lua functions added by naev
stds.Basic={
   globals={
      "__resize", -- Can be defined to handle when a resize event happens
      "_LOADED", -- NLUA_LOAD_TABLE
   },
   read_globals={
      table={
         fields={"unpack", "pack"}
      },
      "inlist",
      "tcopy",
      "tmerge",
      "tmerge_r",
      "tmergei",
      "trepeat",
      "treverse",
      "N_",
      "_",
      "__debugging",
      "gettext",
      "n_",
      "p_",
      "warn",
   },
}

-- The following standards correspond to useful combinations of libraries
PILOT = "+pilot+ship+asteroid"
STANDARD = "+naev+var+spob+system+jump+time+player" .. PILOT .. "+rnd+diff+faction+vec2+outfit+commodity+news+shiplog+file+data+linopt+safelanes+spfx+audio"
GFX = "+gfx+colour+tex+font+transform+shader+canvas"
TK = "+tk+colour" .. GFX

-- In addition, the Naev code base looks for APIs *exported* by certain types of scripts:
-- This is done using nlua_refenv-family calls for exported functions, and nlua_getenv/nlua_setenv for magic variables.
stds.API_ai = {globals={
   "attacked",
   "attacked_manual",
   "control",
   "control_manual",
   "control_rate",
   "create",
   "discovered",
   "distress",
   "hail",
   "mem",
   "refuel",
   -- Also, numerous tasks are brought into the API by nlua_pilot.c functions:
   "attack_forced",             -- pilotL_attack
   "brake",                     -- pilotL_brake
   "face",                      -- pilotL_face
   "face_towards",              -- pilotL_face
   "follow",                    -- pilotL_follow
   "follow_accurate",           -- pilotL_follow
   "gather",                    -- pilotL_gather
   "hyperspace",                -- pilotL_hyperspace
   "hyperspace_shoot",          -- pilotL_hyperspace
   "land",                      -- pilotL_land
   "land_shoot",                -- pilotL_land
   "moveto",                    -- pilotL_moveto
   "moveto_nobrake",            -- pilotL_moveto
   "moveto_nobrake_raw",        -- pilotL_moveto
   "lunge",                     -- the_bite
   "runaway",                   -- pilotL_runaway
   "runaway_jump",              -- pilotL_runaway
   "runaway_land",              -- pilotL_runaway
   "runaway_nojump",            -- pilotL_runaway
   "stealth",                   -- pilotL_stealth
   -- FIXME: Internal APIs formed by dat/ai code that are very difficult to untangle...
   "control_funcs",             -- a shared dispatch table...
   "create_pre",                -- the standardized initial step of every create()
   "create_post",               -- the standardized final step of every create()
   "idle",                      -- a task, commonly called and overridden
   "should_attack",             -- discretion may or may not be the better part of valor. Everyone gets an opinion.
   "should_investigate",
   "taunt",                     -- everyone has their own!
   "transportParam",            -- initialization helper for the AIs
}}
stds.API_board = {globals={"board"}}    -- C function: player_board()
stds.API_comm = {globals={"comm"}}      -- C function: comm_openPilot()
stds.API_datapath = {globals={"datapath"}} -- C functon: conf_loadConfigPath
stds.API_loadscreen = {globals={
   "update",                           -- C function: loadscreen_update()
   "render",                           -- C function: naev_renderLoadscreen()
}}
stds.API_autoequip = {globals={"autoequip"}}            -- C function: equipment_autoequipShip()
stds.API_equip = {globals={"equip", "equip_generic"}}   -- C function: ai_create
stds.API_faction = {globals={
   "hit",            -- C functions: faction_modPlayerLua
   "text_rank",      -- C functions: faction_getStandingText
   "text_broad",     -- C functions: faction_getStandingBroad
   "reputation_max", -- C functions: faction_reputationMax
   "friendly_at",
}}
stds.API_land = {globals={"land"}}      -- C function: spob_updateLand()
stds.API_rescue = {globals={"rescue"}}  -- C function: land_stranded
stds.API_save_updater = {globals={
   "license",                           -- C function: player_tryAddLicense
   "outfit",                            -- C function: player_tryGetOutfit
   "ship",                              -- C function: player_tryGetShip
}}
stds.API_shipai = {globals={"create"}}  -- C function: info_shipAI
stds.API_spawn = {globals={
   "create",                            -- C function: system_scheduler()
   "decrease",                          -- C function: system_rmCurrentPresence()
   "spawn",                             -- C function: system_scheduler()
}}
stds.API_autonav = {globals={
   "autonav_system",
   "autonav_spob",
   "autonav_pilot",
   "autonav_board",
   "autonav_pos",
   "autonav_end",
   "autonav_reset",
   "autonav_abort",
   "autonav_think",
   "autonav_update",
   "autonav_enter",
}}
stds.API_spob = {globals={
   "mem",
   "init",     -- C function: spob_luaInit
   "load",     -- C function: spob_gfxLoad
   "unload",   -- C function: spob_gfxUnload
   "can_land", -- C function: spob_updateLand
   "land",     -- C function: player_land
   "render",   -- C function: space_renderSpob
   "update",   -- C function: space_updateSpob
   "comm",     -- C function: comm_openSpob
}}
stds.API_effects = {globals={
   "add",      -- C function: effect_add
   "extend",   -- C function: effect_add
   "remove",   -- C function: effect_update
}}
stds.API_background = {globals={
   "background", "renderbg", "rendermg", "renderfg", "renderov"
}}
stds.API_evt = {globals={
   "create", "mem"
}}
stds.API_gui = {globals={
   "create",
   "cooldown_end",
   "mouse_click",
   "mouse_move",
   "render",
   "render_cooldown",
   "update_cargo",
   "update_faction",
   "update_nav",
   "update_ship",
   "update_system",
   "update_effects",
   "update_target",
}}
stds.API_misn = {globals={
   "abort", "accept", "create", "mem"
}}
stds.API_music = {globals={
   "choose",
   "update",
   "play",
   "stop",
   "pause",
   "resume",
   "info",
   "volume",
}}
stds.API_pilotoutfit = {globals={
   "notactive",
   "price",
   "buy",
   "sell",
   "cleanup",
   "cooldown",
   "init",
   "land",
   "descextra",
   "onadd",
   "onhit",
   "onimpact",
   "onmiss",
   "onload",
   "onremove",
   "onscan",
   "onscanned",
   "onshoot",
   "onstealth",
   "ontoggle",
   "outofenergy",
   "takeoff",
   "jumpin",
   "board",
   "keydoubletap",
   "keyrelease",
   "update",
   "mem", -- Automatically created using nlua_setenv().
}}
stds.API_pilotship = {globals={
   "update_dt",
   "init",
   "cleanup",
   "update",
   "explode_init",
   "explode_update",
   "mem", -- Automatically created using nlua_setenv().
}}
stds.API_mem = {globals={
   "mem",
}}

files["dat/ai/**/*.lua"].std = STANDARD .. "+ai+API_ai"
files["dat/autonav.lua"].std = STANDARD .. "+ai+API_autonav"
files["dat/autoequip.lua"].std = STANDARD .. TK .. "+API_autoequip"
files["dat/bkg/**/*.lua"].std = STANDARD .. GFX .. "+bkg+camera+API_background"
files["dat/board.lua"].std = STANDARD .. "+API_board"
files["dat/comm.lua"].std = STANDARD .. "+API_comm"
files["dat/common.lua"].std = STANDARD
files["dat/loadscreen.lua"].std = STANDARD .."+API_loadscreen"
files["dat/events/**/*.lua"].std = STANDARD .. "+API_evt+evt+hook+camera+tex+bkg+music" .. TK
files["dat/factions/equip/*.lua"].std = STANDARD .. "+API_equip"
files["dat/factions/spawn/**/*.lua"].std = STANDARD .. "+API_spawn"
files["dat/factions/standing/**/*.lua"].std = STANDARD .. "+API_faction"
files["dat/gui/*.lua"].std = STANDARD .. GFX .. "+gui+API_gui" .. TK
files["dat/landing.lua"].std = STANDARD .. "+API_land"
files["dat/lua-repl/**/*.lua"].only = {}  -- not our code, so shut up, please
files["dat/missions/**/*.lua"].std = STANDARD .. "+API_misn+misn+hook+camera+tex+bkg+music" .. TK
files["dat/outfits/**/*.lua"].std = STANDARD .. GFX .. "+API_pilotoutfit+pilotoutfit+camera+munition" -- TODO doesn't really support the full API
files["dat/ships/**/*.lua"].std = STANDARD .. GFX .. "+API_pilotship+camera" -- TODO doesn't really support the full API
files["dat/rescue.lua"].std = STANDARD .. TK .. "+API_rescue"
files["dat/rep.lua"].std = STANDARD .. TK .. "+tex+colour+bkg+cli+camera+music+linopt"
files["dat/save_updater.lua"].std = "API_save_updater"
files["dat/shipai.lua"].std = STANDARD .. TK .. "+API_shipai"
files["dat/snd/music.lua"].std = STANDARD .. TK .. "+API_music+music"
files["dat/spob/**/*.lua"].std = STANDARD .. GFX .."+camera+API_spob"
files["dat/effects/**/*.lua"].std = STANDARD .. "+API_effects"

-- No way to be sure what type of environment will load these.
files["dat/scripts/**/*.lua"].std = STANDARD .. TK .. "+API_mem+misn+hook+camera+tex+bkg+music" .. TK

files["docs/ai/**/*.lua"].std = files["dat/ai/**/*.lua"].std
-- TODO: Enable when no one is likely to invoke a pre-1.0 Luacheck manually.
-- files["docs/lua/config.ld"].std = 'ldoc'
files["docs/missions/**/*.lua"].std = files["dat/missions/**/*.lua"].std

files["extras/autotests.lua"].std = STANDARD
files["utils/**/*.lua"].std = STANDARD

files["**/datapath.lua"].std = "API_datapath"

-- This file should have no dependencies and sets globals, so we'll just remove it from luachecking
exclude_files = {"dat/common.lua"}
