<%
if module_doc then
	from = "modules/"..module_doc.name
elseif file_doc then
	from = "files/.."..file_doc.name
else
	from = ""
end
%>

<dt><%=func.private and "local " or ""%><a name="<%=func.name%>"></a><strong><%=func.name%></strong>&nbsp;(<%=table.concat(func.param, ", ")%>)</dt>
<dd>
<%=func.description or ""%>

<%if type(func.param) == "table" and #func.param > 0 then%>
<br />
<h3>Parameters</h3>
<ul>
	<%for p = 1, #func.param do%>
	<li>
	  <%=func.param[p]%>: <%=func.param[func.param[p]] or ""%>
	</li>
	<%end%>
</ul>
<%end%>


<%if type(func.usage) == "string" then%>
<br />
<h3>Usage:</h3>
<code><%=func.usage%></code>
<%elseif type(func.usage) == "table" then%>
<h3>Usage</h3>
<ul>
	<%for _, usage in ipairs(func.usage) do%>
	<li><code><%= usage %></code></li>
	<%end%>
</ul>
<%end%>

<%if type(func.ret) == "string" then%>
<br />
<h3>Return value:</h3>
<%=func.ret%>
<%elseif type(func.ret) == "table" then%>
<h3>Return values:</h3>
<ol>
	<%for _, ret in ipairs(func.ret) do%>
	<li><%= ret %></li>
	<%end%>
</ol>
<%end%>

<%if type(func.see) == "string" then %>
<br />
<h3>See also:</h3>
	<a href="<%=func.see%>"><%=func.see%></a>
<%elseif type(func.see) == "table" and #func.see > 0 then %>
<h3>See also:</h3>
<ul>
	<%for i = 1, #func.see do%>
	<li><a href="#<%=module_doc.name.."."..func.see[i]%>">
		<%=module_doc.name.."."..func.see[i]%></a>
   </li>
	<%end%>
</ul>
<%end%>
</dd>
