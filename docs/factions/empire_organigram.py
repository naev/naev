# The empire organigram (military part for now). Add everything you want, it may fit.

import pygraphviz as pgv

G=pgv.AGraph(directed=True)

# Ranks
G.add_subgraph(name="0")
sub = G.get_subgraph("0")
sub.graph_attr['rank']='same'
sub.add_node("His (Her?) Very Holy Majesty The Emperor Of The Universe", shape='octagon')

G.add_subgraph(name="1")
sub = G.get_subgraph("1")
sub.graph_attr['rank']='same'
sub.add_node("Holy-class", shape='octagon')

G.add_subgraph(name="2")
sub = G.get_subgraph("2")
sub.graph_attr['rank']='same'
sub.add_node("Highest-Excellency-class", shape='octagon')

G.add_subgraph(name="3")
sub = G.get_subgraph("3")
sub.graph_attr['rank']='same'
sub.add_node("Excellency-class", shape='octagon')

G.add_subgraph(name="4")
sub = G.get_subgraph("4")
sub.graph_attr['rank']='same'
sub.add_node("Eminence-class", shape='octagon')

G.add_subgraph(name="5")
sub = G.get_subgraph("5")
sub.graph_attr['rank']='same'
sub.add_node("Aristocracy", shape='octagon')

G.add_subgraph(name="6")
sub = G.get_subgraph("6")
sub.graph_attr['rank']='same'
sub.add_node("Common-class", shape='octagon')

G.add_edge("His (Her?) Very Holy Majesty The Emperor Of The Universe", "Holy-class", style='invis')
G.add_edge("Holy-class", "Highest-Excellency-class", style='invis')
G.add_edge("Highest-Excellency-class", "Excellency-class", style='invis')
G.add_edge("Excellency-class", "Eminence-class", style='invis')
G.add_edge("Eminence-class", "Aristocracy", style='invis')
G.add_edge("Aristocracy", "Common-class", style='invis')


nomNdismiss = "Nominates\n and can dismiss"

# Functions
sub = G.get_subgraph("0")
emperor = "The Emperor\n has full powers over the entire universe"
sub.add_node(emperor)

###
sub = G.get_subgraph("1")
spouse = "First Imperial Spouse"
sub.add_node(spouse)
G.add_edge( emperor, spouse, label="Play cards with" )

gal_archbishop = "Galactic High Archbishop"
sub.add_node(gal_archbishop)
G.add_edge( emperor, gal_archbishop, label="Directly obeys" )

archking = "Archking of Timbuktu\n The only dignitary allowed to pronounce death sentence\n against any holy or (highest)excellency-class dignitaries\n He only has to cut himself a finger after each execution"
sub.add_node(archking)
G.add_edge( emperor, archking )

iguana = "Pet Iguana"
sub.add_node(iguana)
G.add_edge( emperor, iguana, label="Walks" )

optician = "Optician of the Emperor\n Only person in the universe allowed to look the Emperor in the eyes\n (but his own eyes have to be burned afterwards)"
sub.add_node(optician)
G.add_edge( emperor, optician, label="Fixes his glasses" )

###
sub = G.get_subgraph("2")
concubin = "First Imperial Concubin"
sub.add_node(concubin)
G.add_edge( emperor, concubin, label="Play chess with" )
G.add_edge( spouse, concubin, label="Can dismiss\n with agreement of the Senate" )

chancelor = "High Chancellor of the Senate"
sub.add_node(chancelor)
G.add_edge( emperor, chancelor, label="Directly obeys" )
G.add_edge( iguana, chancelor, label="Can pee on" )

achbishop4 = "Fifth Galactic Archbishop"
sub.add_node(achbishop4)
G.add_edge( gal_archbishop, achbishop4, label=nomNdismiss )

kings = "Other Kings"
sub.add_node(kings)
G.add_edge( archking, kings, label="Nominates\n and can dismiss" )

cyberking = "Cyber-king of Orion\n Can oppose veto to any military action\n (except if any imperial squire opposes a veto to this veto)"
sub.add_node(cyberking)
G.add_edge( archking, cyberking, label=nomNdismiss )

iguana_spouse = "Pet Iguana's Spouse"
sub.add_node(iguana_spouse)
G.add_edge( iguana, iguana_spouse, label="Play cards with" )

butler = "Butler of the Imperial House"
sub.add_node(butler)
G.add_edge( emperor, butler, label="Directly obeys" )
G.add_edge( spouse, butler, label=nomNdismiss )

###
sub = G.get_subgraph("3")
senate = "The Senate"
sub.add_node(senate)
G.add_edge( chancelor, senate, label=nomNdismiss )

marshall = "Great Marshall of the Empire\n coordinates all large-scale military operations"
sub.add_node(marshall)
G.add_edge( butler, marshall, label=nomNdismiss )
G.add_edge( optician, marshall, label="Can dismiss" )

squire = "High Squire of the Emperor"
sub.add_node(squire)
G.add_edge( butler, squire, label=nomNdismiss )

vizir = "Great Vizir of the Empire\n Chief of the Police"
sub.add_node(vizir)
G.add_edge( butler, vizir, label=nomNdismiss )
G.add_edge( concubin, vizir, label="Can oppose a veto\n to any decision" )

###
sub = G.get_subgraph("4")
staff_senate = "Second deputee Chief of Staff of the Senate"
sub.add_node(staff_senate)
G.add_edge( chancelor, staff_senate, label=nomNdismiss )
G.add_edge( iguana_spouse, staff_senate, label="Can dismiss" )

Lsquire = "Lower Squires of the Emperor"
sub.add_node(Lsquire)
G.add_edge( squire, Lsquire, label=nomNdismiss )

###
sub = G.get_subgraph("5")
spouse_captain = "High Captain of the First Imperial Spouse's guard\n Commands all ships based on Halir\n and all the land forces on Dune"
sub.add_node(spouse_captain)
G.add_edge( spouse, spouse_captain, label=nomNdismiss )

holy_general = "Highest General of the Holy Force\n Commands all troops based in the sol system :'("
sub.add_node(holy_general)
G.add_edge( achbishop4, holy_general, label=nomNdismiss )

legate = "Legate-in-chief of the army\n Commands most Imperial land units"
sub.add_node(legate)
G.add_edge( kings, legate, label=nomNdismiss )
G.add_edge( senate, legate, label="Can dismiss\n unless any Imperial Concubin opposes a veto" )

squire_admiral = "Great Admiral of Interception\n Commands most Imperial interception forces"
sub.add_node(squire_admiral)
G.add_edge( squire, squire_admiral, label=nomNdismiss )

iguana_admiral = "Great Admiral of Bulk Forces\n Commands most Imperial heavy fleets"
sub.add_node(iguana_admiral)
G.add_edge( iguana, iguana_admiral, label=nomNdismiss )

iguana_caretaker = "Pet Iguana's Chief Caretaker"
sub.add_node(iguana_caretaker)
G.add_edge( iguana, iguana_caretaker, label="Takes care of" )
G.add_edge( senate, iguana_caretaker, label=nomNdismiss )

###
sub = G.get_subgraph("6")
concubin_captain = "Low Captain of the First Imperial Concubin's guard\n Commands all the land forces based on Halir"
sub.add_node(concubin_captain)
G.add_edge( concubin, concubin_captain, label=nomNdismiss )

senate_admiral = "Great Admiral of the Senate's Fleet\n Commands the senate's fleet, based in Arcturus Gamma"
sub.add_node(senate_admiral)
G.add_edge( staff_senate, senate_admiral, label=nomNdismiss )
G.add_edge( senate, senate_admiral, label="Controls his actions" )

genetician = "Genetician of the Archking"
sub.add_node(genetician)
G.add_edge( archking, genetician, label="Ensures he always has\n enough fingers" )
G.add_edge( gal_archbishop, genetician, label="Can Dismiss" )

interpreter = "Pet Iguana's Spouse's Interpreter"
sub.add_node(interpreter)
G.add_edge( iguana_spouse, interpreter, label="Interprets" )

deputee_caretaker = "Second Pet Iguana's Deputee Caretaker"
sub.add_node(deputee_caretaker)
G.add_edge( iguana_caretaker, deputee_caretaker, label=nomNdismiss )

optician_o = "Optician of the Emperor's optician."
sub.add_node(optician_o)
G.add_edge( optician, optician_o, label="Ensures he can serve\n more than one time" )

bodyguard = "Imperial bodyguards\n Can serve as a special commando"
sub.add_node(bodyguard)
G.add_edge( butler, bodyguard, label="Directly obeys" )

squire_squadron = "Squadron of the Imperial squires\n Elite fighter squadron."
sub.add_node(squire_squadron)
G.add_edge( Lsquire, squire_squadron, label="Directly obeys" )

militia = "Great Master of the militias\n coordinates the 362 irregular militias\n that have been attached to the army"
sub.add_node(militia)
G.add_edge( vizir, militia, label=nomNdismiss )

###
iguana_captain = "Low Captain of the Pet Iguana's guard\n Commands all ships based on planet Oma\n and all land troops on Dolmen"
G.add_node(iguana_captain)
G.add_edge( deputee_caretaker, iguana_captain, label=nomNdismiss )

# Draw
G.layout(prog='dot')
G.draw("organigram.pdf")
