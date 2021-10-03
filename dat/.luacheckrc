-- vim: set expandtab shiftwidth=3 syntax=lua:

std = "lua51c+love+Basic"

stds.ai = {globals={"mem"}, read_globals={"ai"}}

-- The following standards correspond to nlua_load* functions.
-- Most of them just load one metatable, but some have dependencies, like nlua_loadGFX.
-- This is represented as: stds._GFX for the stuff loaded directly by nlua_loadGFX, and GFX representing all corresponding stds.

stds.Basic={
   globals={
      table={
         fields={"unpack"}
      },
      "warn", "_", "N_", "n_", "gettext",
   }
}
stds.Audio = {read_globals={"audio"}}
stds.Background = {read_globals={"bkg"}}
stds.Camera = {read_globals={"camera"}}
stds.Canvas = {read_globals={"canvas"}}
stds.Col = {read_globals={"colour"}}
stds.Commodity = {read_globals={"commodity"}}
stds.Data = {read_globals={"data"}}
stds.Debug = {read_globals={"debug"}}
stds.Diff = {read_globals={"diff"}}
stds.Evt = {read_globals={"evt"}}
stds.Faction = {read_globals={"faction"}}
stds.File = {read_globals={"file"}}
stds.Font = {read_globals={"font"}}
stds._GFX = {read_globals={"gfx"}}
stds.GUI = {read_globals={"gui"}}
stds.Hook = {read_globals={"hook"}}
stds.Jump = {read_globals={"jump"}}
stds.LinOpt = {read_globals={"linopt"}}
stds.Misn = {read_globals={"misn"}}
stds.Music = {read_globals={"music"}}
stds.Naev = {read_globals={"naev"}}
stds.News = {read_globals={"news"}}
stds.Outfit = {read_globals={"outfit"}}
stds._Pilot = {read_globals={"pilot"}}
stds.PilotOutfit = {read_globals={"pilotoutfit"}}
stds.Planet = {read_globals={"planet"}}
stds.Player = {read_globals={"player"}}
stds.Rnd = {read_globals={"rnd"}}
stds.Safelanes = {read_globals={"safelanes"}}
stds.Shader = {read_globals={"shader"}}
stds.Ship = {read_globals={"ship"}}
stds.Shiplog = {read_globals={"shiplog"}}
stds.System = {read_globals={"system"}}
stds.Tex = {read_globals={"tex"}}
stds.Time = {read_globals={"time"}}
stds._Tk = {read_globals={"tk"}}
stds.Transform = {read_globals={"transform"}}
stds.Var = {read_globals={"var"}}
stds.Vector = {read_globals={"vec2"}}

PILOT = "+_Pilot+Ship"
STANDARD = "+Naev+Var+Planet+System+Jump+Time+Player" .. PILOT .. "+Rnd+Diff+Faction+Vector+Outfit+Commodity+News+Shiplog+File+Data+Debug+LinOpt+Safelanes"
GFX = "+_GFX+Col+Tex+Font+Transform+Shader+Canvas"
TK = "+_Tk+Col" .. GFX

files["ai/**/*.lua"].std = STANDARD .. "+ai"
files["autoequip.lua"].std = STANDARD .. TK
files["bkg/*.lua"].std = STANDARD .. "+Tex+Col+Background+Camera"
files["comm.lua"].std = STANDARD
files["events/**/*.lua"].std = STANDARD .. "+Evt+Hook+Camera+Tex+Background+Music+Audio" .. TK
files["factions/equip/*.lua"].std = STANDARD
files["factions/spawn/*.lua"].std = STANDARD
files["factions/standing/*.lua"].std = STANDARD
files["gui/*.lua"].std = STANDARD .. GFX .. "+GUI" .. TK
files["landing.lua"].std = STANDARD
files["missions/**/*.lua"].std = STANDARD .. "+Misn+Hook+Camera+Tex+Background+Music+Audio" .. TK
files["outfits/**/*.lua"].std = STANDARD .. GFX .. "+PilotOutfit"
files["rescue.lua"].std = STANDARD .. TK
files["snd/music.lua"].std = STANDARD .. "+Music"
