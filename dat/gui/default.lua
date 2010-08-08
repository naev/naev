--[[
	First alternate GUI skin (Mockup)
--]]

function create()

	--Get player
	pp = player.get()
	
	--Get sizes
	screen_w, screen_h = gfx.dim()
	deffont_h = gfx.fontSize()
	smallfont_h = gfx.fontSize(true)
	
	--Colors
	col_txt = colour.new( 192/255, 198/255, 217/255 )
	col_shi = colour.new( 40/255,  51/255,  88/255  )
	col_arm = colour.new( 72/255,  73/255,  60/255  )
	col_ene = colour.new( 41/255,  92/255,  47/255  )
	col_spd = colour.new( 77/255,  80/255,  21/255  )
	
	--Load Images
	local base = "gfx/gui/slim/"
	player_pane = tex.open( base .. "frame_plazer.png" )
	radar_gfx = tex.open( base .. "radar.png" )
	icon_shi = tex.open( base .. "shield.png" )
	icon_arm = tex.open( base .. "armor.png" )
	icon_ene = tex.open( base .. "energy.png" )
	icon_spd = tex.open( base .. "speed.png" )
	bg_shi = tex.open( base .. "bg_shi.png" )
	bg_arm = tex.open( base .. "bg_arm.png" )
	bg_ene = tex.open( base .. "bg_ene.png" )
	bg_spe = tex.open( base .. "bg_spe.png" )
	sheen = tex.open( base .. "sheen.png" )
	gui.targetPlanetGFX( tex.open( base .. "radar_planet.png" )
	gui.targetPilotGFX(  tex.open( base .. "radar_player.png" )
	
	--Get positions
	--Player pane
	pl_pane_w, pl_pane_h = player_pane:dim()
	pl_pane_x = screen_w - pl_pane_w - 16
	pl_pane_y = screen_h - pl_pane_y - 16
	
	--Radar
	radar_w, radar_h = radar_gfx:dim()
	radar_x = pl_pane_x - radar_w + 24
	radar_y = pl_pane_y + 13
	
	
	bar_w = 86
	--Shield Bar
	shield_x = pl_pane_x + 59
	shield_y = pl_pane_y + 102
	
	--Armor Bar
	armor_x = pl_pane_x + 59
	armor_y = shield_y - 28
	
	--Energy Bar
	energy_x = pl_pane_x + 59
	energy_y = armor_y - 28
	
	--Speed Bar
	speed_x = pl_pane_x + 59
	speed_y = energy_y - 28
	
end

function render()
	
	--Radar
	gfx.renderTex( radar, radar_x, radar_y )
	
	--Player pane
	gfx.renderTex( player_pane, pl_pane_x, pl_pane_y )
	
	--Bars
	--Shield
	gfx.renderTex( bg_shi, shield_x + 30, shield_y + 2 )
	gfx.renderRect( 