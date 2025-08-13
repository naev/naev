#!/usr/bin/env python
"""
Trivial script to simulate pirate ditributions.
"""

def patrol( x ):
    #return max(-50+0.5*x, 0)
    return max(-50+1.2*x, 0)

def loner_weak( x ):
    #return max(-100+x, 0)
    return 100+0.5*x

def loner_strong( x ):
    return x

def squad( x ):
    return max(-150 + 1.2*x, 0)

def capship( x ):
    return max(-400 + 1.70*x, 0)

def simulate( x ):
    wsum = patrol(x) + loner_weak(x) + loner_strong(x) + squad(x) + capship(x)
    print( f"""{x} presence
    patrol: {patrol(x) / wsum * 100:.1f}%
    loner_weak: {loner_weak(x) / wsum * 100:.1f}%
    loner_strong: {loner_strong(x) / wsum * 100:.1f}%
    squad: {squad(x) / wsum * 100:.1f}%
    capship: {capship(x) / wsum * 100:.1f}%
""")


simulate( 100 )
simulate( 300 )
simulate( 500 )
simulate( 800 )
