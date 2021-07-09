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

def createFactions():
    '''Creates the dico of lane-making factions'''
    factions = [
                "Empire",
                "Soromid",
                "Dvaered",
                "Za'lek",
                "Collective", # TODO : see if this one is right
                "Sirius",
                "Frontier",
                "Goddard",
                "Proteron",
                "Thurion",
               ]

    return {name: i for (i, name) in enumerate(factions)}

# Compute insystem paths. TODO maybe : use Delauney triangulation instead ?
def inSysStiff( nodess, factass, g2ass, loc2globNs ):
    #stiff = sp.csr_matrix() # Basic stiffness matrix (without lanes)
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
        #print(nodes)
        for n in range(len(nodes)):
            xn = nodes[n][0]
            yn = nodes[n][1]
            
            na = g2ass[loc2globN[n]]  # Find global asset numerotation of local node
            #print(na)
            if na>=0:
                fn = factass[na]
            else: # It's not an asset
                fn = -1
            
            for m in range(n): # Because symmetry
                #if n >= m: 
                #    continue
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
    def __init__( self, systems, factions ):
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
        self.internal_lanes = inSysStiff( systems.nodess, systems.assts[1], systems.g2ass, systems.loc2globs )

        # Merge both and generate matrix
        si = si0 + self.internal_lanes[0]
        sj = sj0 + self.internal_lanes[1]
        sv = sv0 + self.internal_lanes[2]
        sz = len(si)

        # Build the sparse matrix
        sii = [0]*(4*sz)
        sjj = [0]*(4*sz)
        svv = [0.]*(4*sz)

        sii[0::4] = si
        sjj[0::4] = si
        svv[0::4] = sv

        sii[1::4] = sj
        sjj[1::4] = sj
        svv[1::4] = sv

        sii[2::4] = si
        sjj[2::4] = sj
        svv[2::4] = map(neg, sv)

        sii[3::4] = sj
        sjj[3::4] = si
        svv[3::4] = map(neg, sv)

        self.stiff = sp.csr_matrix( ( svv, (sii, sjj) ) )

        self.default_lanes = (si0,sj0,sv0)
        self.sysdist = (systems.distances, systems.g2sys)


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

# Gives the matrix that computes penibility form potentials
# By chance, this does not depend on the presence of lane
@timed
def PenMat( nass, ndof, internal_lanes, utilde, ass2g, assts, sysdist ):
    presass = assts[0]
    factass = assts[1]
    nfact = max(factass)+1
    
    # First : compute the (sparse) matrix that transforms utilde into u
    pi = []
    pj = []
    pv = []
    
    di = [[] for i in range(nfact)]
    dj = [[] for i in range(nfact)]
    dv = [[] for i in range(nfact)]
    
    distances = sysdist[0]
    g2sys = sysdist[1]
    
    for i in range(nass):
        ai = ass2g[i] # Find corresponding dof
        facti = factass[i]
        #print(facti)
        presi = presass[i]        
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
            
            factj = factass[j]
            presj = presass[j]
            
            # More the assets are far from each other less interesting it is
            # This avoids strange shapes for peripheric systems
#            aj = ass2g[j]
#            dis = distances[g2sys[ai],g2sys[aj]]
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
    
    #print(di[0][100])
    #print(di[1][100])
    
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


# Get the gradient from state and adjoint state
# Luckly, this does not depend on the stiffness itself (by linearity wrt. stiffness)
@timed
def getGradient( internal_lanes, u, lamt, alpha, PP, PPl, pres_0 ):
    si, sj, sv = internal_lanes[:3]
    sr = internal_lanes[6] # Tells who has right to build on each lane
    sy = internal_lanes[7] # Tells the system
    
    sz = len(si)
#    print(u.shape, sz)
    sh = u.shape
    sh = sh[0]
    
    #lam = lamt.dot(PP) # 0.01 s
    #LUT = lam.dot(u.transpose())
    g = np.zeros((sz,1)) # Just a vector

    nfact = len(PPl)
    gl = []
    #ut = u.transpose()
    for k in range(nfact): # .2
        lal = lamt.dot(PPl[k])
        #LUTl = lal.dot(ut)

        glk = np.zeros((sz,1))

    #for k in range(nfact): 
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
        

# Activates the best lane in each system
def activateBest( internal_lanes, g, activated, Lfaction, nodess ):
    # Find lanes to keep in each system
    sv, sil, sjl, lanesLoc2globs = internal_lanes[2:6]
    nsys = len(lanesLoc2globs)
    
    g1 = g * np.c_[sv] # Short lanes are more interesting
    
    for i in range(nsys):
        lanesLoc2glob = lanesLoc2globs[i]
        #nodes = nodess[i]
        #aloc = activated[lanesLoc2glob]
        aloc = [activated[k] for k in lanesLoc2glob]
        
        if not (False in aloc): # There should be something to actually activate
            continue
        
        gloc = g1[lanesLoc2glob]
        ind1 = np.argsort(gloc.transpose()) # For some reason, it does not work without transpose :/
        ind = [lanesLoc2glob[k] for k in ind1[0]]

        # Find a lane to activate
        for k in ind:#range(len(ind)):
            if activated[k] == False: # One connot activate something that is already active
                break
                    
        #if admissible: # Because it's possible noone was admissible
        activated[k] = True
        Lfaction[k] = 0
        
    return 1


# Activates the best lane in each system for each faction
def activateBestFact( internal_lanes, g, gl, activated, Lfaction, nodess, pres_c, pres_0 ):
    # Find lanes to keep in each system
    sv, sil, sjl, lanesLoc2globs, sr  = internal_lanes[2:7]
    nsys = len(lanesLoc2globs)
    
    nfact = len(pres_c[0])
    price = .007  # TODO : tune metric price
    
    g1 = g * np.c_[sv] # Short lanes are more interesting
    
    g1l = [None]*nfact
    for k in range(nfact):
        g1l[k] = gl[k] * np.c_[sv]
        #print(np.linalg.norm(gl[k]))
    
    #print(sv)
    
    for i in range(nsys):
        lanesLoc2glob = lanesLoc2globs[i]
        #nodes = nodess[i]
        #aloc = activated[lanesLoc2glob]
        aloc = [activated[k] for k in lanesLoc2glob]
        
        if not (False in aloc): # There should be something to actually activate
            continue

        # Faction with more presence choose first
        ploc = pres_0[i]
        sind = np.argsort(ploc)
        sind = np.flip(sind)
        
#        if i==320: #116=Alteris 320=Raelid
#            print(pres_c[i])
#            print(sind)
        
        for ff in range(nfact):
            f = sind[ff]

            ntimes = 1 # Nb of lanes per iteration for this faction
            
            for lane in range(ntimes):
                if pres_c[i][f] <= 0.: # This faction has no presence
                    continue
                
                gloc = g1l[f][lanesLoc2glob]
                #gloc = g1[lanesLoc2glob]
                ind1 = np.argsort(gloc.transpose()) # For some reason, it does not work without transpose :/
                ind = [lanesLoc2glob[k] for k in ind1[0]]
        
                # Find a lane to activate
                for k in ind:#range(len(ind)):
                    cond = (sr[k][0] == f) or (sr[k][1] == f) or ((sr[k][0] == -1) and (sr[k][1] == -1))
                    #if (activated[k] == False) and (pres_c[i][f] >= 1/sv[k] * price):     # One connot activate something that is already active           
                    if (activated[k] == False) and (pres_c[i][f] >= 1/sv[k] * price) \
                      and cond:                   
                        pres_c[i][f] -= 1/sv[k] * price
                        activated[k] = True
                        Lfaction[k] = f
                        break
        
    return 1


@timed
def optimizeLanes( systems, problem, alpha=9 ):
    '''Optimize the lanes. alpha is the efficiency parameter for lanes.'''
    sz = len(problem.internal_lanes[0])
    ndof = problem.stiff.shape[0]
    activated = [False] * sz # Initialization : no lane is activated
    Lfaction = [-1] * sz;
    pres_c = systems.presences.copy()
    nfact = len(systems.presences[0])
    
    nass = len(systems.ass2g)
    
    # TODO : It could be interesting to use sparse format for the RHS as well (see)
    ftilde = np.eye( ndof )
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
            Pi = PenMat( nass, ndof, problem.internal_lanes, utilde, systems.ass2g, systems.assts, problem.sysdist )
            P = Pi[0]
            Q = Pi[1] 
            D = Pi[2]
            
            QQ = (Q.transpose()).dot(Q)
            #QQ = QQ.todense()
            with timer('Calculate dense P P*'):
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

        # Activate one lane per system
        #activateBest( problem.internal_lanes, g, activated, Lfaction, nodess ) # 0.01 s
        activateBestFact( problem.internal_lanes, g, gl, activated, Lfaction, systems.nodess, pres_c, systems.presences ) # 0.01 s

    #print(np.max(np.c_[problem.internal_lanes[2]]))

    # And print the lanes
    print(np.linalg.norm(utilde,'fro'))
    print(i+1)
    #print(np.linalg.norm(utilde-utilde.transpose(),'fro'))
    
    return (activated, Lfaction)

if __name__ == "__main__":
    factions = createFactions()
    systems = Systems( factions )
    problem = SafeLaneProblem( systems, factions )
    
    activated, Lfaction = optimizeLanes( systems, problem )
    printLanes( problem.internal_lanes, activated, Lfaction, systems )
