import matplotlib.pyplot as plt

FACTION_COLORS = ["g","orange","brown","darkred","silver","aqua","y","b","purple","grey"]


def printLanes( internal_lanes, activated, Lfaction, systems ):
    '''Display the active lanes'''
    nsys = len(systems.nodess)
    sil = internal_lanes[3]
    sjl = internal_lanes[4]
    lanesLoc2globs = internal_lanes[5]
    
    fig, (glob_ax, loc_ax) = plt.subplots(1, 2, figsize=(18, 10))
    globmap = glob_ax.scatter(systems.xlist ,systems.ylist, color='b')
    for i, jpname in enumerate(systems.jpnames):
        for target in jpname:
            j = systems.sysdict[target]
            glob_ax.plot([systems.xlist[i], systems.xlist[j]], [systems.ylist[i], systems.ylist[j]], color='r')

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
        aloc = [activated[k] for k in lanesLoc2glob]
        #floc = [Lfaction[k] for k in lanesLoc2glob]
        
        loc_ax.clear()
        loc_ax.title.set_text(systems.sysnames[i])
        xlist, ylist = zip(*nodes)
        loc_ax.scatter(xlist, ylist, color='b')
        for xy, name in zip(nodes, names):
            loc_ax.annotate(name, xy=xy, xytext=(10, 10), textcoords='offset points', bbox={'fc': 'w'})
        
        for j, jj in enumerate(lanesLoc2glob):
            if aloc[j]:
                no1 = sil[jj]
                no2 = sjl[jj]
            
                x1, y1 = nodes[no1]
                x2, y2 = nodes[no2]
            
                col = FACTION_COLORS[ Lfaction[jj] ]
                loc_ax.plot([x1,x2], [y1,y2], color=col)
        
    plt.show()
