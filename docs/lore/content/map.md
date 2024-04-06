---
title: Missions
---
<% content_for :javascript do %>
<script src="https://cdnjs.cloudflare.com/ajax/libs/sigma.js/2.4.0/sigma.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/graphology/0.25.4/graphology.umd.min.js"></script>
<script>
    // Create a graphology graph
    const graph = new graphology.Graph();

    var ssys = [<% @items.find_all('/ssys/*.md').each do |s| %>
    <%= "{ name:\"#{s[:name]}\", x:#{s[:x]}, y:#{s[:y]} }," %>
    <% end %>];
    var n = ssys.length;
    for (var i=0; i<n; i++) {
        var s = ssys[i];
        graph.addNode( s.name, { label: s.name, x: s.x, y: s.y, size: 5, color: "white", borderColor: "white" } );
    }
    var jumps = [<% @items.find_all('/ssys/*.md').each do |s| s[:jumps].each do |j| %>
    <%= "{ a:\"#{s[:name]}\", b:\"#{j}\" }," %>
    <% end end %>];
    var nj = jumps.length;
    for (var i=0; i<n; i++) {
        var j = jumps[i];
        graph.addEdge( j.a, j.b, { size: 1, color: 'blue' } );
    }

    //graph.addNode("1", { label: "Node 1", x: 0, y: 0, size: 10, color: "blue" });
    //graph.addNode("2", { label: "Node 2", x: 1, y: 1, size: 20, color: "red" });
    //graph.addEdge("1", "2", { size: 5, color: "purple" });

    // Instantiate sigma.js and render the graph
    const sigmaInstance = new Sigma( graph, document.getElementById("starmap"), {
        //defaultNodeType: "bordered",
        //nodeProgramClasses: {
        //    bordered: NodeBorderProgram,
        //},
    } );
</script>
<% end %>

<div id="starmap" style="width: 100%; height: 600px; background: black"></div>
