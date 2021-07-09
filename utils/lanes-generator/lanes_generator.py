#!/usr/bin/env python
# This file generates safe lanes

import numpy as np
from operator import neg
import scipy.sparse as sp
import scipy.sparse.linalg as lgs
import scipy.linalg as slg
import math

from lanes_perf import timed, timer
from lanes_ui import printLanes
from lanes_systems import Systems

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
    
    minangle = 10 # Minimal angle between 2 lanes (deg)
    crit = math.cos(math.pi*minangle/180)
    
    i  = 0
    ii = 0

    for k in range(len(nodess)):
        nodes = nodess[k]
        loc2glob = []
        loc2globN = loc2globNs[k]
        for n in range(len(nodes)):
            xn = nodes[n][0]
            yn = nodes[n][1]
            
            na = g2ass[loc2globN[n]]  # Find global asset numerotation of local node
            if na>=0:
                fn = factass[na]
            else: # It's not an asset
                fn = -1
            
            for m in range(n): # Only m<n, because symmetry
                xm = nodes[m][0]
                ym = nodes[m][1]
                
                ma = g2ass[loc2globN[m]]
                if ma>=0:
                    fm = factass[ma]
                else: # It's not an asset
                    fm = -1
                
                lmn = math.hypot( xn-xm, yn-ym )
                
                # Check if very close path exist already.
                forbitten = False
                for p in range(len(nodes)):
                    if n==p or m==p:
                        continue
                    xi = nodes[p][0]
                    yi = nodes[p][1]
                    
                    # The scalar product should not be too close to 1
                    # That would mean we've got a flat triangle
                    lni = math.hypot(xn-xi, yn-yi)
                    lmi = math.hypot(xm-xi, ym-yi)
                    dpn = ((xn-xm)*(xn-xi) + (yn-ym)*(yn-yi)) / ( lmn * lni )
                    dpm = ((xm-xn)*(xm-xi) + (ym-yn)*(ym-yi)) / ( lmn * lmi )
                    if (dpn > crit and lni < lmn) or (dpm > crit and lmi < lmn):
                        forbitten = True
                        break
                    
                if forbitten:
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
    '''Representation of the systems, with more calculated: nodes associated to jumps, connectivity matrix (for ranged presence).'''
    def __init__( self, systems ):
        si0 = [] #
        sj0 = [] #
        sv0 = [] # For the construction of the sparse weighted connectivity matrix
        nsys = len(systems.sysnames)

        for i, (jpname, loc2globi, jp2loci, namei) in enumerate(zip(systems.jpnames, systems.loc2globs, systems.jp2locs, systems.sysnames)):
            for j in range(len(jpname)):
                k = systems.sysdict[jpname[j]] # Get the index of target

                jpnamek = systems.jpdicts[k]
                loc2globk = systems.loc2globs[k]
                jp2lock = systems.jp2locs[k]

                if namei not in jpnamek:
                    continue # It's an exit-only : we don't count this one as a link

                m = jpnamek[namei] # Index of system i in system k numerotation

                si0.append(loc2globi[jp2loci[j]]) # Assets connectivity (jp only)
                sj0.append(loc2globk[jp2lock[m]]) # 
                sv0.append( 1/1000 ) # TODO : better value

        # Remove the redundant info because right now, we have i->j and j->i
        while k<len(si0):
            if (si0[k] in sj0):
                si0.pop(k)
                sj0.pop(k)
                sv0.pop(k)
                k -= 1
            k += 1

        # Get the stiffness inside systems
        self.default_lanes = (si0,sj0,sv0)
        self.internal_lanes = inSysStiff( systems.nodess, systems.factass, systems.g2ass, systems.loc2globs )
        self.ndof = sum(map(len, systems.nodess))  # number of degrees of freedom


@timed
def buildStiffness( default_lanes, internal_lanes, activated, alpha, anchors ):
    '''Computes the conductivity matrix'''
    def_si, def_sj, def_sv = default_lanes[:3]
    int_si, int_sj, int_sv0  = internal_lanes[:3]

    # Take activated lanes into account
    int_sv = int_sv0.copy()
    for i in range(len(int_sv0)):
        if activated[i]:
            int_sv[i] = (alpha+1)*int_sv0[i]
    
    # Assemble both lists
    si = def_si + int_si
    sj = def_sj + int_sj
    sv = def_sv + int_sv
    sz = len(si)
    
    # Build the sparse matrix
    sii = [0]*(4*sz)
    sjj = [0]*(4*sz)
    svv = [0.]*(4*sz)

    sii[0::4]   = si
    sjj[0::4]   = si
    svv[0::4]   = sv

    sii[1::4] = sj
    sjj[1::4] = sj
    svv[1::4] = sv

    sii[2::4] = si
    sjj[2::4] = sj
    svv[2::4] = map(neg, sv)

    sii[3::4] = sj
    sjj[3::4] = si
    svv[3::4] = map(neg, sv)

    # impose Robin condition at anchors (because of singularity)
    mu = max(sv)  # Just a parameter to make a Robin condition that does not spoil the spectrum of the matrix
    for i in anchors:
        sii.append(i)
        sjj.append(i)
        svv.append(mu)
        #stiff[i,i] = stiff[i,i] + mu

    stiff = sp.csr_matrix( ( svv, (sii, sjj) ) )
    # Rem : it may become mandatory at some point to impose nb of dofs

    return stiff

@timed
def PenMat( nass, ndof, internal_lanes, utilde, systems ):
    '''Gives the matrix that computes penibility form potentials.
      By chance, this does not depend on the presence of lane.'''
    nfact = max(systems.factass)+1
    
    # First : compute the (sparse) matrix that transforms utilde into u
    pi = []
    pj = []
    pv = []
    
    di = [[] for i in range(nfact)]
    dj = [[] for i in range(nfact)]
    dv = [[] for i in range(nfact)]
    
    for i in range(nass):
        ai = systems.ass2g[i] # Find corresponding dof
        facti = systems.factass[i]
        presi = systems.presass[i]
        for j in range(i):# Stuff is symmetric, we use that
            if utilde[ai,j] <= 1e-12: # i and j are disconnected. Dont consider this couple
                continue
            ij = i*nass + j # Multiindex
            
            # Just a substraction
            pi.append(i)
            pj.append(ij)
            pv.append(1)
            pi.append(j)
            pj.append(ij)
            pv.append(-1)
            
            factj = systems.factass[j]
            presj = systems.presass[j]
            
            # More the assets are far from each other less interesting it is
            # This avoids strange shapes for peripheric systems
#            aj = systems.ass2g[j]
#            dis = systems.distances[systems.g2sys[ai],systems.g2sys[aj]]
#            if dis==0:
#                dis = 1
            dis = 1
                    
            # Build the diagonal ponderators
            if (facti == factj):
                di[facti].append(ij)
                dj[facti].append(ij)
                dv[facti].append( (presi+presj) / dis )
                
            else: # Foreign assets are not so interesting
                di[facti].append(ij)
                dj[facti].append(ij)
                dv[facti].append( presi / dis )
                
                di[factj].append(ij)
                dj[factj].append(ij)
                dv[factj].append( presj / dis ) 
            
    P = sp.csr_matrix( ( pv, (pi, pj) ) )
    # P*utilde^T = u^T
    
    D = [None]*nfact
    for k in range(nfact):
        D[k] = sp.csr_matrix( ( dv[k], (di[k], dj[k]) ), (P.shape[1], P.shape[1]) )
    #print(lgs.norm(D[0]-D[1], 'fro'))
    
    si, sj = internal_lanes[:2]
    
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
            
    Q = sp.csr_matrix( ( qv, (qi, qj) ), (len(si),ndof) )
    # Q*u = p
    
    return (P,Q,D)


@timed
def getGradient( internal_lanes, u, lamt, alpha, PP, PPl, pres_0 ):
    '''Get the gradient from state and adjoint state.
       Luckily, this does not depend on the stiffness itself (by linearity wrt. stiffness).'''
    si, sj, sv = internal_lanes[:3]
    sr = internal_lanes[6] # Tells who has right to build on each lane
    sy = internal_lanes[7] # Tells the system
    
    sz = len(si)
    
    #lam = lamt.dot(PP) # 0.01 s
    #LUT = lam.dot(u.transpose())
    g = np.zeros((sz,1)) # Just a vector

    nfact = len(PPl)
    gl = []
    for k in range(nfact): # .2
        lal = lamt.dot(PPl[k])
        #LUTl = lal.dot(ut)
        glk = np.zeros((sz,1))

        for i in range(sz):
            if pres_0[sy[i]][k] <= 0: # Does this faction have presence here ?
                continue
            
            # Does this faction have the right to build here ?
            cond = (sr[i][0] == k) or (sr[i][1] == k) or ((sr[i][0] == -1) and (sr[i][1] == -1))
            if not cond: # No need to compute stuff if we've dont have the right to build here !
                continue
            
            sis = si[i]
            sjs = sj[i]
            
            LUTll = np.dot( lal[[sis,sjs],:] , u[[sis,sjs],:].T )
            glk[i] = alpha*sv[i] * ( LUTll[0,0] + LUTll[1,1] - LUTll[0,1] - LUTll[1,0] )
            
            #glk[i] = alpha*sv[i] * ( LUTl[sis, sis] + LUTl[sjs, sjs] - LUTl[sis, sjs] - LUTl[sjs, sis] )
            
        gl.append(glk)
    return (g,gl)
        

def activateBestFact( internal_lanes, g, gl, activated, Lfaction, nodess, pres_c, pres_0 ):
    '''Activates the best lane in each system for each faction'''
    # Find lanes to keep in each system
    sv, sil, sjl, lanesLoc2globs, sr  = internal_lanes[2:7]
    nsys = len(lanesLoc2globs)
    
    nfact = len(pres_c[0])
    price = [.005, .007, .100, .005, .010, .007, .010, .005, .005, .010]
    
    g1 = g * np.c_[sv] # Short lanes are more interesting
    
    g1l = [None]*nfact
    for k in range(nfact):
        g1l[k] = gl[k] * np.c_[sv]
        #print(np.linalg.norm(gl[k]))
    
    #print(sv)
    
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

            ntimes = 1 # Nb of lanes per iteration for this faction
            
            for lane in range(ntimes):
                if pres_c[i][f] <= 0.: # This faction has no presence
                    continue
                
                gloc = g1l[f][lanesLoc2glob]
                ind1 = np.argsort(gloc.transpose()) # For some reason, it does not work without transpose :/
                ind = [lanesLoc2glob[k] for k in ind1[0]]
        
                # Find a lane to activate
                for k in ind:
                    cond = (sr[k][0] == f) or (sr[k][1] == f) or (sr[k] == (-1, -1))
                    if (not activated[k]) and (pres_c[i][f] >= 1/sv[k] * price[f]) and cond:
                        pres_c[i][f] -= 1/sv[k] * price[f]
                        activated[k] = True
                        Lfaction[k] = f
                        break
        
    return 1


@timed
def optimizeLanes( systems, problem, alpha=9 ):
    '''Optimize the lanes. alpha is the efficiency parameter for lanes.'''
    sz = len(problem.internal_lanes[0])
    activated = [False] * sz # Initialization : no lane is activated
    Lfaction = [-1] * sz;
    pres_c = systems.presences.copy()
    nfact = len(systems.presences[0])
    
    nass = len(systems.ass2g)
    
    # TODO : It could be interesting to use sparse format for the RHS as well (see)
    ftilde = np.eye( problem.ndof )
    ftilde = ftilde[:,systems.ass2g] # Keep only lines corresponding to assets
    
    niter = 20
    for i in range(niter):
        stiff = buildStiffness( problem.default_lanes, problem.internal_lanes, activated, alpha, systems.anchors ) # 0.02 s

        # Compute direct and adjoint state
        if i >= 1:
            utildp = utilde
        with timer('spsolve: utilde'):
            utilde = lgs.spsolve( stiff, ftilde ) # 0.11 s

        # Check stopping condition
        nu = np.linalg.norm(utilde,'fro')
        dr = nu # Just for i==0
        if i >=1:
            dr = np.linalg.norm(utildp-utilde,'fro')
            
        #print(dr/nu)
        if dr/nu <= 1e-12:
            break

        # Compute QQ and PP, and use utilde to detect connected parts of the mesh
        # It's in the loop, but only happends once
        if i == 0: # .5 s
            P, Q, D = PenMat( nass, problem.ndof, problem.internal_lanes, utilde, systems )
            
            QQ = (Q.transpose()).dot(Q)
            PP0 = P.dot(P.transpose())
            PP = PP0.todense() # PP is actually not sparse
            PPl = [None]*nfact
            for k in range(nfact): # Assemble per-faction ponderators
                Pp = P.dot(D[k])
                PP0 = Pp.dot(Pp.transpose())
                #print(PP0.count_nonzero())
                PPl[k] = PP0.todense() # Actually, for some factions, sparse is better

        rhs = - QQ.dot(utilde)  
        lamt = lgs.spsolve( stiff, rhs ) #.11 s # TODO if possible : reuse cholesky factorization
        
        # Compute the gradient.
        gNgl = getGradient( problem.internal_lanes, utilde, lamt, alpha, PP, PPl, pres_c ) # 0.2 s
        g = gNgl[0]
        gl = gNgl[1]

        activateBestFact( problem.internal_lanes, g, gl, activated, Lfaction, systems.nodess, pres_c, systems.presences ) # 0.01 s

    #print(np.max(np.c_[problem.internal_lanes[2]]))

    # And print the lanes
    print('Converged in', i+1, 'iterations, ‖Ũ‖=', np.linalg.norm(utilde,'fro'))
    
    return (activated, Lfaction)

if __name__ == "__main__":
    systems = Systems()
    problem = SafeLaneProblem( systems )
    
    activated, Lfaction = optimizeLanes( systems, problem )
    printLanes( problem.internal_lanes, activated, Lfaction, systems )
