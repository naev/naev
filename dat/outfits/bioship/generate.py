#!/usr/bin/python3

N_ = lambda text: text
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
desc["brain"] = N_("The brain is a Soromid bioship's equivalent to core systems in synthetic ships. Possibly the most important organ, the brain provides processing power and allocates energy to the rest of the organism. All brains start off undeveloped, but over time, just like the ships themselves, they grow and improve.")
desc["engine"] = N_("The gene drive is a Soromid bioship's equivalent to engines in synthetic ships. It is charged with moving the organism through space and is even capable of hyperspace travel. All gene drives start off undeveloped, but over time, just like the ships themselves, they grow and improve.")
desc["hull"] = N_("The shell is a Soromid bioship's natural protection, equivalent to hulls of synthetic ships. The shell is responsible both for protecting the organism's internal organs and for creating camouflage to reduce the risk of being detected by hostiles. All shells start off undeveloped, but over time, just like the ships themselves, they grow and improve.")

typename = {}
typename["brain"] = N_("Bioship Brain")
typename["engine"] = N_("Bioship Gene Drive")
typename["hull"] = N_("Bioship Shell")

BioOutfit( "pincer.xml.template", {
    "price" :   lerpr(   0, 20e3 ),
    "damage":   lerp(    8,  17 ),
    "energy":   4,
    "range" :   lerp(  750, 900 ),
    "falloff":  lerp(  450, 750 ),
    "speed" :   lerp(  550, 700 ),
    "heatup":   lerp(   25,  40 ),
} ).run( [N("Pincer Organ Stage I"), N("Pincer Organ Stage II"), N("Pincer Organ Stage III")] )

BioOutfit( "perleve_cerebrum.xml.template", {
    "price":        lerpr(   0, 120e3 ),
    "mass":         14,
    "desc":         desc["brain"],
    "typename":     typename["brain"],
    "cpu":          lerpr(   3,   8 ),
    "shield" :      lerp(  160, 250 ),
    "shield_regen": lerp(    5,   9 ),
    "energy":       lerp(  130, 250 ),
    "energy_regen": lerp(   12,  19 ),
} ).run( [N("Perleve Cerebrum Stage I"), N("Perleve Cerebrum Stage II")] )

BioOutfit( "perlevis_gene_drive.xml.template", {
    "price":        lerpr(   0, 140e3 ),
    "mass":         8,
    "desc":         desc["engine"],
    "typename":     typename["engine"],
    "thrust":       lerp(  145, 196 ),
    "turn":         lerp(  120, 160 ),
    "speed":        lerp(  255, 345 ),
    "fuel":         400,
    "energy_malus": lerp(    5,   5 ),
    "engine_limit": lerp(  150, 150 ),
} ).run( [N("Perlevis Gene Drive Stage I"), N("Perlevis Gene Drive Stage II")] )

BioOutfit( "perlevis_cortex.xml.template", {
    "price":        lerpr(   0, 130e3 ),
    "mass":         30,
    "desc":         desc["hull"],
    "typename":     typename["hull"],
    "cargo":        lerpr(   4, 4 ),
    "absorb":       lerpr(   0, 3 ),
    "armour":       lerp(   45, 65 )
} ).run( [N("Perlevis Cortex Stage I"), N("Perlevis Cortex Stage II")] )
