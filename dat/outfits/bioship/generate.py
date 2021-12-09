#!/usr/bin/python

TEMPLATEPATH = "./templates/"

def lerpr( a, b ):
    return lambda x: int(round(a + x * (b-a)))

def lerp( a, b ):
    return lambda x: a + x * (b-a)

class BioOutfit:
    def __init__( self, template, params ):
        self.template = template
        self.params = params
        with open(TEMPLATEPATH+template,"r") as f:
            self.txt = f.read()

    def run(self, names):
        for k,n in enumerate(names):
            x = 1 if len(names)==1 else k/(len(names)-1)
            p = {k: v(x) if callable(v) else v for k, v in self.params.items()}
            p["name"]  = n
            with open( f"{n.lower().replace(' ','_')}.xml", "w" ) as f:
                f.write( self.txt.format(**p) )

"""
pincer
stinger
barb
fang
talon
claw
"""

"""
Perlevis (ul)
Laevis (l)
Mediocris (m)
Largus (hm)
Ponderosus (h)
Immanis (uh)

Cerebrum -> nominative neutral
    perleve
    laevum
    mediocre
    largum
    ponderosum
    immane

Cortex -> nominative masculine
    perlevis
    laevis
    mediocris
    largus
    ponderosus
    immanis
"""

desc = {}
desc["brain"] = "The brain is a Soromid bioship's equivalent to core systems in synthetic ships. Possibly the most important organ, the brain provides processing power and allocates energy to the rest of the organism. All brains start off undeveloped, but over time, just like the ships themselves, they grow and improve."

BioOutfit( "pincer.xml.template", {
    "price" :   lerpr(   0, 20e3 ),
    "damage":   lerp(    8,  17 ),
    "energy":   4,
    "range" :   lerp(  750, 900 ),
    "falloff":  lerp(  450, 750 ),
    "speed" :   lerp(  550, 700 ),
    "heatup":   lerp(   25,  40 ),
} ).run( ["Pincer Organ Stage I", "Pincer Organ Stage II", "Pincer Organ Stage III"] )

BioOutfit( "perleve_cerebrum.xml.template", {
    "price":        lerpr( 0, 120e3 ),
    "mass":         14,
    "desc":         desc["brain"],
    "cpu":          lerpr(   3,   8 ),
    "shield" :      lerp(  160, 250 ),
    "shield_regen": lerp(    5,   9 ),
    "energy":       lerp(  130, 250 ),
    "energy_regen": lerp(   12,  19 ),
} ).run( ["Perleve Cerebrum Stage I", "Perleve Cerebrum Stage 2"] )
