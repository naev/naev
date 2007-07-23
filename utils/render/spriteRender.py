#!BPY

"""
Name: 'NAEV Sprite Renderer'
Blender: '242'
Group: 'Export'
Tip: 'Renders images and places them in a directory'
"""
#
# NAEV Sprite Renderer 
#
# Written by Joel Hans for the Not Another Escape Velocity project:
# https://trac.bobbens.dyndns.org/naev/
# 2006, Joel Hans (wolorf@gmail.com)
# 
# Distributed under GNU/GPL license: http://www.gnu.org/copyleft/gpl.html
# Feel free to use it for your own projects!
#
# NOTE: You must set the context.renderPath variable on line 49 to your chosen directory.
#

###########################
# importing modules
###########################
import Blender
import math
from math import *
from Blender.BGL import *
from Blender.Draw import *
from Blender import Scene
from Blender import Object
from Blender import Lamp
from Blender import Camera
from Blender import NMesh
from Blender import Types
from Blender.Scene import Render

###########################
# random initialization
###########################
objList = Blender.Object.Get()
print objList
fileName = Blender.Get('filename')
scn = Blender.Scene.GetCurrent()
context = scn.getRenderingContext()
Render.EnableDispWin()
context.extensions = True
context.imageType = Render.PNG
context.enableOversampling(0)
osaafter = 1
ypos=5

sliderLight = Create(0)

###########################
# GUI
###########################
def draw() :
	global Button3, Button2, Button1 
	global sliderWidth, sliderLight, sliderSpriteSize, sliderObjectSize
	global ypos

	glClearColor(0.753, 0.753, 0.753, 0.0)
	Blender.BGL.glClear(Blender.BGL.GL_COLOR_BUFFER_BIT)

	glColor3f(0.000, 0.000, 0.000)
	glRasterPos2i(10, ypos)

	Button('Exit', 1, 10, ypos, 50, 15, 'Exit script')
	ypos = ypos + 20
	Button('Render', 2, 10, ypos, 50, 15, 'Render the ship with current settings')
	ypos = ypos + 20
	Button('Test', 3, 10, ypos, 50, 15, 'Test your settings')
	ypos = ypos + 20
	Button('Initialize', 4, 10, ypos, 50, 15, 'Initialize the camera and lights')
	ypos = ypos + 40

	sliderWidth = 300

	sliderLight = Slider('Light Intensity   ', 5, 10, ypos, sliderWidth, 25, sliderLight.val, 0, 10, .1, 'Light Itensity')
	ypos = ypos + 40

	glColor3f(0.000, 0.000, 0.000)
	glRasterPos2i(10, ypos)
	Text('3: Finally push the "Render" button to render your frames.')
	ypos = ypos + 20
	glRasterPos2i(10, ypos)
	Text('2: Hit the "Test" button to see if your lighting is good.')
	ypos = ypos + 20
	glRasterPos2i(10, ypos)
	Text('1: Put in your settings and hit the "initialization" button.')
	ypos = ypos + 20
	glRasterPos2i(10, ypos)
	Text('Usage:')
	ypos = ypos + 20
	glRasterPos2i(10, ypos)
	Text('Created for NAEV: https://trac.bobbens.dyndns.org/naev/')
	ypos = ypos + 20
	glRasterPos2i(10, ypos)
	Text('NAEV Sprite Renderer')

def event(evt, val) :
	if (evt == QKEY and not val) :
		Exit()

def bevent(evt) :
	if evt == 1 :
		Exit()
	elif evt == 2 :
		render()
	elif evt == 3 :
		test()
	elif evt == 4 :
		initialize()

Blender.Draw.Register(draw,event,bevent)

###########################
# variables that should be put into gui but will remain in the script for now
###########################
spriteSize = 500
context.renderPath = "/home/joel/src/naev/utils/mksprite/"

def initialize() :
	
	####################
	# orienting the object(s) in the correct position
	####################
	for obj in objList :
		if obj.getType() == 'Mesh' :
			obj.RotZ = 0
			obj.RotY = 0
	#		obj.RotX = 45

	##########################
	# initializing the camera
	##########################
	cam = Blender.Object.Get("Camera")
	cam.LocZ = 9.0
	cam.LocX = -5.0
	cam.LocY = 0
	cam.RotY = 0
	cam.RotX = 0
	cam.RotZ = 0
	cam.RotZ = -(pi / 2)
	cam.RotX = (pi / 7)

	###########################
	# setting up the lighting, bitches!
	###########################
	for obj in objList :
		if obj.getType() == 'Lamp' :
			scn.unlink(obj)
	lamp = Blender.Lamp.New()
	Lamp1 = Blender.Object.New('Lamp', 'Lamp1')
	Lamp2 = Blender.Object.New('Lamp', 'Lamp2')
	Lamp1.LocZ = 4.0
	Lamp1.LocX = 4.0
	Lamp1.LocY = 0
	Lamp2.LocZ = 2.0
	Lamp2.LocX = -4.0
	Lamp2.LocY = 0
	Lamp1.link(lamp)
	Lamp2.link(lamp)
	lamp.setEnergy(sliderLight.val)
	scn.link (Lamp1)
	scn.link (Lamp2)

def test() :
	context.imageSizeX(spriteSize)
	context.imageSizeY(spriteSize)
	context.render()

def render() :
	###########################
	# rendering the first frame
	###########################
	origSX = context.imageSizeX()
	origSY = context.imageSizeY()
	context.imageSizeX(spriteSize)
	context.imageSizeY(spriteSize)
	spriteName = '001'
	context.render()
	context.saveRenderedImage(spriteName)
	Render.CloseRenderWindow()
	
	###########################
	# rendering the rest of the frames
	###########################
	for i in range(1,35+1) :
		n = str(i + 1)
		for obj in objList :
			if obj.getType() == 'Mesh' :
				obj.RotZ = ((pi * 2) / 36) * i 
		spriteName = n.zfill(3)

		context.render()
		context.saveRenderedImage(spriteName)
		Render.CloseRenderWindow()

	###########################
	# returning settings to normal
	###########################
	context.imageSizeX(origSX)
	context.imageSizeY(origSY)
	context.enableOversampling(osaafter)
