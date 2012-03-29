<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
    <meta http-equiv="Content-Type" content="text/html;charset=utf-8" />
    <title>Naev Lua API</title>
    <link rel="stylesheet" href="<%=luadoc.doclet.html.link("luadoc.css")%>" type="text/css" />
</head>

<body>
<%=luadoc.doclet.html.include("header.lp")%>
<div id="container">

<div id="main">

<div id="navigation">
<%=luadoc.doclet.html.include("menu.lp", { doc=doc })%>

</div> <!-- id="navigation" -->

<div id="content">

<div id="product">
	<div id="product_logo"></div>
	<div id="product_name"><big><b>
   Naev Lua API
   </b></big></div>
   <div id="product_description">
      <p style="text-align: center;">This is the API used in Naev to interact with the game from Lua.</p>
      <p><a href="http://www.lua.org">Lua</a> is a relatively simple scripting language used for a large portion of Naev's content. Missions, events, GUIs and AIs are all written using Lua and Naev's API, as are many minor features such as system backgrounds, reputation handling and others.</p>
      <p>The Wiki has an in-depth <a href="http://wiki.naev.org/wiki/Mission_Tutorial">mission tutorial</a> covering both missions and events, and the API documentation provides common usage examples for each function.</p>
      <p>Most API modules can also be used from the in-game Lua console (accessible by pressing <strong>F2</strong> by default) for <span style="text-decoration: line-through;">cheating</span> <em>experimenting</em> to your heart's content.</p>
   </div>
</div> <!-- id="product" -->

<%if not options.nomodules and #doc.modules > 0 then%>
<h2>Modules</h2>
<table class="module_list">
<!--<tr><td colspan="2">Modules</td></tr>-->
<%for _, modulename in ipairs(doc.modules) do%>
	<tr>
		<td class="name"><a href="<%=luadoc.doclet.html.module_link(modulename, doc)%>"><%=modulename%></a></td>
		<td class="summary"><%=doc.modules[modulename].summary%></td>
	</tr>
<%end%>
</table>
<%end%>



<%if not options.nofiles and #doc.files > 0 then%>
<h2>Files</h2>
<table class="file_list">
<!--<tr><td colspan="2">Files</td></tr>-->
<%for _, filepath in ipairs(doc.files) do%>
	<tr>
		<td class="name"><a href="<%=luadoc.doclet.html.file_link(filepath)%>"><%=filepath%></a></td>
		<td class="summary"></td>
	</tr>
<%end%>
</table>
<%end%>

</div> <!-- id="content" -->

</div> <!-- id="main" -->

<div id="about">
	<p><a href="http://validator.w3.org/check?uri=referer"><img src="http://www.w3.org/Icons/valid-xhtml10" alt="Valid XHTML 1.0!" height="31" width="88" /></a></p>
</div> <!-- id="about" -->

</div> <!-- id="container" -->	
</body>
</html>
