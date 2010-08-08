--[[
	First alternate GUI skin (Mockup)
--]]

function create()

	--Get player
	pp = player.pilot()
	
	--Get sizes
	screen_w, screen_h = gfx.dim()
	deffont_h = gfx.fontSize()
	smallfont_h = gfx.fontSize(true)
	
	--Colors
	col_txt = colour.new( 192/255, 198/255, 217/255 )
	col_shi = colour.new( 40/255,  51/255,  88/255  )
	col_arm = colour.new( 72/255,  73/255,  60/255  )
	col_ene = colour.new( 41/255,  92/255,  47/255  )
	col_spe = colour.new( 77/255,  80/255,  21/255  )
	
	--Load Images
	local base = "gfx/gui/slim/"
	player_pane = tex.open( base .. "frame_player.png" )
	radar_gfx = tex.open( base .. "radar.png" )
	icon_shi = tex.open( base .. "shield.png" )
	icon_arm = tex.open( base .. "armor.png" )
	icon_ene = tex.open( base .. "energy.png" )
	icon_spe = tex.open( base .. "speed.png" )
	bg_bar = tex.open( base .. "bg_bar.png" )
	bg_shi = tex.open( base .. "bg_shi.png" )
	bg_arm = tex.open( base .. "bg_arm.png" )
	bg_ene = tex.open( base .. "bg_ene.png" )
	bg_spe = tex.open( base .. "bg_spe.png" )
	sheen = tex.open( base .. "sheen.png" )
	warnlight2 = tex.open( base .. "warnlight2.png" )
	gui.targetPlanetGFX( tex.open( base .. "radar_planet.png" ) )
	gui.targetPilotGFX(  tex.open( base .. "radar_ship.png" ) )
	
	--Get positions
	--Player pane
	pl_pane_w, pl_pane_h = player_pane:dim()
	pl_pane_x = screen_w - pl_pane_w - 16
	pl_pane_y = screen_h - pl_pane_h - 16
	
	--Radar
	radar_w, radar_h = radar_gfx:dim()
	radar_x = pl_pane_x - radar_w + 24
	radar_y = pl_pane_y + 13
	
	
	bar_w = 86
	bar_h = 22
	--Shield Bar
	shield_x = pl_pane_x + 46
	shield_y = pl_pane_y + 102
	
	--Armor Bar
	armor_x = pl_pane_x + 46
	armor_y = shield_y - 28
	
	--Energy Bar
	energy_x = pl_pane_x + 46
	energy_y = armor_y - 28
	
	--Speed Bar
	speed_x = pl_pane_x + 46
	speed_y = energy_y - 28
	
end

function update_target()
end

function update_nav()
end

function render()
	
	--Radar
	gfx.renderTex( radar_gfx, radar_x, radar_y )
	
	--Player pane
	gfx.renderTex( player_pane, pl_pane_x, pl_pane_y )
	
	--Bars
	armor, shield = pp:health()
	energy = pp:energy()
	speed = pp:vel()
	--Shield
	gfx.renderTex( bg_shi, shield_x + 30, shield_y + 2 )
	gfx.renderRect( shield_x + 30, shield_y + 2, shield/100. * bar_w, bar_h, col_shi )
	gfx.renderTex( bg_bar, shield_x, shield_y )
	gfx.renderTex( icon_shi, shield_x + 7, shield_y + 4 )
	gfx.renderTex( sheen, shield_x + 31, shield_y +15 )
	gfx.print( false, tostring( math.floor(shield)) .. "%", shield_x + 30, shield_y + 6, col_txt, 86, true)
	--Armor
	gfx.renderTex( bg_arm, armor_x + 30, armor_y + 2 )
	gfx.renderRect( armor_x + 30, armor_y + 2, armor/100. * bar_w, bar_h, col_arm )
	gfx.renderTex( bg_bar, armor_x, armor_y )
	gfx.renderTex( icon_arm, armor_x + 7, armor_y + 4 )
	gfx.renderTex( sheen, armor_x + 31, armor_y +15 )
	gfx.print( false, tostring( math.floor(armor)) .. "%", armor_x + 30, armor_y + 6, col_txt, 86, true)
	--Energy
	gfx.renderTex( bg_ene, energy_x + 30, energy_y + 2 )
	gfx.renderRect( energy_x + 30, energy_y + 2, energy/100. * bar_w, bar_h, col_ene )
	gfx.renderTex( bg_bar, energy_x, energy_y )
	gfx.renderTex( icon_ene, energy_x + 7, energy_y + 4 )
	gfx.renderTex( sheen, energy_x + 31, energy_y +15 )
	gfx.print( false, tostring( math.floor(energy)) .. "%", energy_x + 30, energy_y + 6, col_txt, 86, true)
	--Speed
	gfx.renderTex( bg_spe, speed_x + 30, speed_y + 2 )
	gfx.renderRect( speed_x + 30, speed_y + 2, bar_w, bar_h, col_spe )
	gfx.renderTex( bg_bar, speed_x, speed_y )
	gfx.renderTex( icon_spe, speed_x + 7, speed_y + 4 )
	gfx.renderTex( sheen, speed_x + 31, speed_y +15 )
	gfx.print( false, tostring( math.floor(speed:mod()) ), speed_x + 30, speed_y + 6, col_txt, 86, true)
	
	--Warning Light
	if armor <= 20 then
	   gfx.renderTex( warnlight2, pl_pane_x + 29, pl_pane_y - 2 )
	end
end

function destroy()
end