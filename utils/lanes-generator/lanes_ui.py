import matplotlib.pyplot as plt
from matplotlib.patches import Patch

COLORMAP = plt.cm.gist_rainbow


def directedSystemPairs( problem, activated, Lfaction, systems ):
    '''Return a dictionary: {(src_sys_index, dst_sys_index): {faction_index_1, ...}}.'''
    out = {}
    sil, sjl, lanesLoc2globs = problem.internal_lanes[3:6]

    for i, jpname in enumerate(systems.jpnames):
        for target in jpname:
            out[i, systems.sysdict[target]] = set()

    for i, (sysas, jpname, lanesLoc2glob) in enumerate(zip(systems.sysass, systems.jpnames, lanesLoc2globs)):
        for jj in lanesLoc2glob:
            if activated[jj]:
                for no in sil[jj], sjl[jj]:
                    no -= len(sysas)  # Adjust for index offset.
                    if no >= 0:
                        out[i, systems.sysdict[jpname[no]]].add(Lfaction[jj])
    return out


def printLanes( problem, activated, Lfaction, systems ):
    '''Display the active lanes'''
    nsys = len(systems.nodess)
    nfac = len(systems.facnames)
    sil, sjl, lanesLoc2globs = problem.internal_lanes[3:6]

    sys_edge_factions = directedSystemPairs(problem, activated, Lfaction, systems)
    # Symmetrize, at least for now. (A faction might have a dead-end on the far side of a jump, making it one-way in a sense.)
    for i,j in [(i, j) for (i, j) in sys_edge_factions if i>j]:
        sys_edge_factions.setdefault((j, i), set()).update(sys_edge_factions.pop((i, j)))
    
    fig, (glob_ax, loc_ax) = plt.subplots(1, 2, figsize=(18, 10))
    globmap = glob_ax.scatter(systems.xlist ,systems.ylist, color='b')
    for (i, j), factions in sys_edge_factions.items():
        xij = [systems.xlist[i], systems.xlist[j]]
        yij = [systems.ylist[i], systems.ylist[j]]
        if not factions:
            glob_ax.plot(xij, yij, color='lightgrey')
        for i, f in enumerate(factions):
            nf = len(factions)
            glob_ax.plot(xij, yij, linewidth=nf, color=COLORMAP(f/nfac), dashes=(0, 5*i, 5, 5*(nf-i-1)))

    glob_ax.legend(handles=[Patch(color=COLORMAP(f/nfac), label=name) for (f,name) in enumerate(systems.facnames)])

    annot = glob_ax.annotate('', xy=(0, 0), xytext=(10, 10), textcoords='offset points', bbox={'fc': 'w'})
    annot.set_visible(False)

    def hover(event):
        vis = annot.get_visible()
        if event.inaxes == glob_ax:
            cont, ind = globmap.contains(event)
            if cont:
                n = ind['ind'][0]
                plot_system(n)
                annot.xy = globmap.get_offsets()[n]
                annot.set_text(systems.sysnames[n])
                annot.set_visible(True)
                fig.canvas.draw_idle()
            elif vis:
                annot.set_visible(False)
                fig.canvas.draw_idle()

    fig.canvas.mpl_connect('motion_notify_event', hover)

    def plot_system(i):
        nodes = systems.nodess[i]
        names = systems.sysass[i] + [f'\u2192{jn}' for jn in systems.jpnames[i]]
        lanesLoc2glob = lanesLoc2globs[i]
        
        loc_ax.clear()
        loc_ax.title.set_text(systems.sysnames[i])
        xlist, ylist = zip(*nodes)
        loc_ax.scatter(xlist, ylist, color='b')
        for xy, name in zip(nodes, names):
            loc_ax.annotate(name, xy=xy, xytext=(10, 10), textcoords='offset points', bbox={'fc': 'w'})
        
        for jj in lanesLoc2glob:
            if activated[jj]:
                no1 = sil[jj]
                no2 = sjl[jj]
            
                x1, y1 = nodes[no1]
                x2, y2 = nodes[no2]
            
                col = COLORMAP(Lfaction[jj]/nfac)
                loc_ax.plot([x1,x2], [y1,y2], color=col)
                
                
    plt.gca().set_aspect('equal')
    plt.show()
