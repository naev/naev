#!/usr/bin/env python
# This file generates safe lanes

import copy
import math
import numpy as np
import scipy.sparse as sp
from sksparse.cholmod import cholesky

from lanes_perf import timed, timer
from lanes_ui import printLanes
from lanes_systems import Systems
from union_find import UnionFind

JUMP_CONDUCTIVITY = .001  # TODO : better value
DISCONNECTED_THRESHOLD = 1e-12
FACTIONS_LANES_BUILT_PER_ITERATION = 1
MAX_ITERATIONS = 20
MIN_ANGLE = 10*math.pi/180 # Path triangles can't be more acute.
ALPHA = 9  # Lane efficiency parameter.

def inSysStiff( nodess, factass, g2ass, loc2globNs ):
    '''Compute insystem paths. TODO maybe : use Delauney triangulation instead?'''
    si  = []  # Element to build the sparse default matrix
    sj  = []
    sv  = []
    sil = [] # Lanes in local numerotation
    sjl = []
    sr  = [] # Lanes reserved for a faction
    loc2globs = []
    system = [] # Tells which system the lane is in
    
    crit = math.cos(MIN_ANGLE)
    i  = 0
    ii = 0

    for k, nodes in enumerate(nodess):
        loc2glob = []
        loc2globN = loc2globNs[k]
        for n in range(len(nodes)):
            xn, yn = nodes[n]
            
            na = g2ass[loc2globN[n]]  # Find global asset numerotation of local node
            if na>=0:
                fn = factass[na]
            else: # It's not an asset
                fn = -1
            
            for m in range(n): # Only m<n, because symmetry
                xm, ym = nodes[m]
                ma = g2ass[loc2globN[m]]
                if ma>=0:
                    fm = factass[ma]
                else: # It's not an asset
                    fm = -1
                
                lmn = math.hypot( xn-xm, yn-ym )
                
                # Check if very close path exist already.
                forbidden = False
                for p in range(len(nodes)):
                    if n==p or m==p:
                        continue
                    xi, yi = nodes[p]
                    
                    # The scalar product should not be too close to 1
                    # That would mean we've got a flat triangle
                    lni = math.hypot(xn-xi, yn-yi)
                    lmi = math.hypot(xm-xi, ym-yi)
                    dpn = ((xn-xm)*(xn-xi) + (yn-ym)*(yn-yi)) / ( lmn * lni )
                    dpm = ((xm-xn)*(xm-xi) + (ym-yn)*(ym-yi)) / ( lmn * lmi )
                    if (dpn > crit and lni < lmn) or (dpm > crit and lmi < lmn):
                        forbidden = True
                        break
                    
                if forbidden:
                    continue
                
                si.append( i+n )
                sj.append( i+m )
                sv.append( 1/lmn )
                sil.append( n )
                sjl.append( m )
                sr.append((fn,fm)) # This tells who is allowed to build along the lane
                system.append(k)
                
                loc2glob.append(ii)
                ii += 1
                
        i += len(nodes)
        loc2globs.append(loc2glob)

    return ( si, sj, sv, sil, sjl, loc2globs, sr, system )


class SafeLaneProblem:
    '''Representation of the optimization problem to be solved.'''
    def __init__( self, systems ):
        si0 = [] #
        sj0 = [] #
        sv0 = [] # For the construction of the sparse weighted connectivity matrix
        biconnect = UnionFind(len(systems.sysnames))  # Partitioning of the systems by reversible-jump connectedness.

        for i, (jpname, loc2globi, jp2loci, namei) in enumerate(zip(systems.jpnames, systems.loc2globs, systems.jp2locs, systems.sysnames)):
            for j in range(len(jpname)):
                k = systems.sysdict[jpname[j]] # Get the index of target

                jpnamek = systems.jpdicts[k]
                loc2globk = systems.loc2globs[k]
                jp2lock = systems.jp2locs[k]

                if i < k or namei not in jpnamek:
                    continue # We only want to consider reversible jumps, and only once per pair.

                m = jpnamek[namei] # Index of system i in system k numerotation

                si0.append(loc2globi[jp2loci[j]]) # Assets connectivity (jp only)
                sj0.append(loc2globk[jp2lock[m]]) # 
                sv0.append(JUMP_CONDUCTIVITY)
                biconnect.union(i, k)

        # Create anchors to prevent the matrix from being singular
        # Anchors are jumpoints, there is 1 per connected set of systems
        # A Robin condition will be added to these points and we will check the rhs
        # thanks to them because in u (not utilde) the flux on these anchors should
        # be 0, otherwise, it means that the 2 non-null terms on the rhs are on
        # separate sets of systems.
        self.anchors = [systems.loc2globs[i][0] for i in biconnect.findall() if systems.loc2globs[i]]

        # Get the stiffness inside systems
        self.default_lanes = (si0,sj0,sv0)
        self.internal_lanes = inSysStiff( systems.nodess, systems.factass, systems.g2ass, systems.loc2globs )


def buildStiffness( problem, activated, systems ):
    '''Computes the conductivity matrix'''
    def_si, def_sj, def_sv = problem.default_lanes[:3]
    int_si, int_sj, int_sv0  = problem.internal_lanes[:3]

    # Take activated lanes into account
    int_sv = int_sv0.copy()
    for i in range(len(int_sv0)):
        if activated[i]:
            int_sv[i] = (ALPHA+1)*int_sv0[i]
    
    # Assemble both lists
    si = def_si + int_si
    sj = def_sj + int_sj
    sv = def_sv + int_sv
    
    # Build the sparse matrix
    sii = si + sj + si + sj
    sjj = si + sj + sj + si
    svv = sv*2 + [-x for x in sv]*2

    mu = max(sv)  # Just a parameter to make a Robin condition that does not spoil the spectrum of the matrix
    for i in problem.anchors:
        sii.append(i)
        sjj.append(i)
        svv.append(mu)

    stiff = sp.csc_matrix( ( svv, (sii, sjj) ) )
    # Rem : it may become mandatory at some point to impose nb of dofs

    return stiff

@timed
def compute_PPts_QtQ( problem, utilde, systems ):
    '''Gives the matrix that computes penibility form potentials.
      By chance, this does not depend on the presence of lane.'''
    nfact = len(systems.presences[0])
    nass = len(systems.ass2g)
    
    # First : compute the (sparse) matrix that transforms utilde into u
    pi = []
    pj = []
    pv = []
    
    di = [[] for i in range(nfact)]
    dv = [[] for i in range(nfact)]
    
    for i, ai in enumerate(systems.ass2g):
        facti = systems.factass[i]
        presi = systems.presass[i]
        for j in range(i):# Stuff is symmetric, we use that
            if utilde[ai,j] <= DISCONNECTED_THRESHOLD:
                continue
            ij = (i*(i-1))//2 + j # Multiindex
            
            # Just a substraction
            pi.append(i)
            pj.append(ij)
            pv.append(1)
            pi.append(j)
            pj.append(ij)
            pv.append(-1)
            
            factj = systems.factass[j]
            presj = systems.presass[j]
            
            # EXPERIMENT: Could weight v here by 1/distances[g2sys[ai],g2sys[aj]], to avoid strange shapes for peripheric systems.
            # Build the diagonal ponderators
            if facti >= 0:
                di[facti].append(ij)
                dv[facti].append(presi)
            if factj >= 0:
                di[factj].append(ij)
                dv[factj].append(presj)
            
    P = sp.csr_matrix( ( pv, (pi, pj) ) )
    # P*utilde^T = u^T
    
    Pp = ( P @ sp.csr_matrix( (dv[k], (di[k], di[k])), (P.shape[1], P.shape[1]) ) for k in range(nfact) )
    PPl = [ (Ppk @ Ppk.transpose()).todense() for Ppk in Pp ]
    
    si, sj = problem.internal_lanes[:2]
    
    # Then : the matrix that gives penibility on each internal lane from u
    qi = []
    qj = []
    qv = []
    for i in range(len(si)):
        # Just a substraction (again)
        qi.append(i)
        qj.append(si[i]) # TODO : check the transpose
        qv.append(1)
        qi.append(i)
        qj.append(sj[i])
        qv.append(-1)
            
    Q = sp.csr_matrix( (qv, (qi, qj)), (len(si), len(systems.g2ass)) )
    # Q*u = p
    
    return PPl, Q.transpose() @ Q


@timed
def getGradient( problem, u, lamt, PPl, pres_0, activated, systems ):
    '''Get the gradient from state and adjoint state.
       Luckily, this does not depend on the stiffness itself (by linearity wrt. stiffness).'''
    si, sj, sv = problem.internal_lanes[:3]
    sr = problem.internal_lanes[6] # Tells who has right to build on each lane
    sy = problem.internal_lanes[7] # Tells the system
    
    sz = len(si)

    nfact = len(PPl)
    gl = []
    for k in range(nfact): # .2
        
        # Build the list of all interesting dofs
        myDofs = []
        for i in range(len(systems.loc2globs)):
            
            if pres_0[i][k] <= 0: # Does this faction have presence here ?
                continue
            
            loc2glob = systems.loc2globs[i] # Idices of the assets in the global numerotation
            myDofs = myDofs + loc2glob
            

        lal = np.zeros(lamt.shape)
        lal[myDofs,:] = lamt[myDofs,:] @ PPl[k]
        #lal = lamt @ PPl[k]
        glk = np.zeros((sz,1))

        for i in range(sz):
            
            if activated[i]: # The lane has already been activated: no need to re-compute its gradient
                continue
            
            if pres_0[sy[i]][k] <= 0: # Does this faction have presence here ?
                continue
            
            faction_may_build = (k in sr[i]) or (sr[i] == (-1, -1))
            if not faction_may_build:
                continue
            
            sis = si[i]
            sjs = sj[i]
            
            LUTll = np.dot( lal[[sis,sjs],:] , u[[sis,sjs],:].T )
            glk[i] = ALPHA*sv[i] * ( LUTll[0,0] + LUTll[1,1] - LUTll[0,1] - LUTll[1,0] )
            
        gl.append(glk)
    return gl
        

def activateBestFact( problem, gl, activated, Lfaction, pres_c, pres_0 ):
    '''Activates the best lane in each system for each faction. Returns the number activated. '''
    # Find lanes to keep in each system
    sv, sil, sjl, lanesLoc2globs, sr  = problem.internal_lanes[2:7]
    nsys = len(lanesLoc2globs)
    nactivated = 0
    nfact = len(pres_c[0])
    
    g1l = [gl[k] * np.c_[sv] for k in range(nfact)]
    
    for i in range(nsys):
        lanesLoc2glob = lanesLoc2globs[i]
        
        if all(activated[k] for k in lanesLoc2glob):
            continue

        # Faction with more presence choose first
        ploc = pres_0[i]
        sind = np.argsort(ploc)
        sind = np.flip(sind)
        
        for ff in range(nfact):
            f = sind[ff]

            for lane in range(FACTIONS_LANES_BUILT_PER_ITERATION):
                if pres_c[i][f] <= 0.: # This faction has no presence
                    continue
                
                gloc = g1l[f][lanesLoc2glob]
                ind1 = np.argsort(gloc.transpose()) # For some reason, it does not work without transpose :/
                ind = [lanesLoc2glob[k] for k in ind1[0]]
        
                # Find a lane to activate
                prev_nactivated = nactivated
                for k in ind:
                    faction_may_build = (f in sr[k]) or (sr[k] == (-1, -1))
                    if (not activated[k]) and (pres_c[i][f] >= 1/sv[k] / systems.dist_per_presence[f]) and faction_may_build:
                        pres_c[i][f] -= 1/sv[k] / systems.dist_per_presence[f]
                        activated[k] = True
                        Lfaction[k] = f
                        nactivated += 1
                        break
                    
                # The faction did not activate anything:
                # There is no hope in activating anything anymore.
                if nactivated == prev_nactivated:
                    pres_c[i][f] = -1

    return nactivated


@timed
def optimizeLanes( systems, problem ):
    sz = len(problem.internal_lanes[0])
    activated = [False] * sz # Initialization : no lane is activated
    Lfaction = [-1] * sz;
    pres_c = copy.deepcopy(systems.presences)
    nfact = len(systems.presences[0])
    
    # TODO : It could be interesting to use sparse format for the RHS as well (see)
    ftilde = np.eye( len(systems.g2ass) )
    ftilde = ftilde[:,systems.ass2g] # Keep only lines corresponding to assets
    
    for i in range(MAX_ITERATIONS):
        stiff = buildStiffness( problem, activated, systems ) # ~0.008 s
        stiff_c = cholesky(stiff) #.001s

        # Compute direct and adjoint state
        utilde = stiff_c.solve_A( ftilde ) # .004 s

        # Compute QQ and PP, and use utilde to detect connected parts of the mesh
        # It's in the loop, but only happends once
        if i == 0: # .5 s
            PPl, QQ = compute_PPts_QtQ( problem, utilde, systems )

        rhs = - QQ @ utilde
        lamt = stiff_c.solve_A( rhs ) #.010 s
        
        # Compute the gradient and activate. Runtime depends on how many degrees of freedom remain.
        gl = getGradient( problem, utilde, lamt, PPl, pres_c, activated, systems )

        if not activateBestFact( problem, gl, activated, Lfaction, pres_c, systems.presences ):
            break

    print('Converged in', i+1, 'iterations.')
    return (activated, Lfaction)

if __name__ == "__main__":
    systems = Systems()
    problem = SafeLaneProblem( systems )
    
    activated, Lfaction = optimizeLanes( systems, problem )
    printLanes( problem, activated, Lfaction, systems )
