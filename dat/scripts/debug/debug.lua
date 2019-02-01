-- =======================================================================================
--
-- Debug facility
-- 
-- Include this script to output debug messages
--
-- 
-- =======================================================================================


-- =======================================================================================
--
-- Script-global variables
-- 
-- =======================================================================================
--
dbg = {}
-- 
-- =======================================================================================


-- =======================================================================================
--
-- Print function
--
-- Usage :
--    dbg.stdOutput( strPrefixParam, numIndentParam, strMessageParam, boolDebugParam )
-- 
-- =======================================================================================
--
function dbg.stdOutput(strPrefixParam, numIndentParam, strMessageParam, boolDebugParam)
	-- Local variables with default values
	local strPrefix  = ""
	local numIndent  = 0
	local strMessage = ""
	local boolDebug = true

	-- Check parameters
	if type(strPrefixParam)=="string" then
		strPrefix  = strPrefixParam
	end
	if type(numIndentParam)=="number" then
		numIndent  = numIndentParam
	end
	if type(strMessageParam)=="string" then
		strMessage = strMessageParam
	end
	if type(boolDebugParam)=="boolean" then
		boolDebug = boolDebugParam
	end

	-- Built indentation and line feed strings
	local strIndent = ""
	local strLF = ""
	if numIndent>=0 then
		strIndent = string.rep("    ", numIndent)
	end
	if numIndent<0 then
		strLF = "\n"
	end

	-- Print message
	if boolDebug then
		local tabDateTime = os.date("*t")
		local strDate = string.format( "%02i", tabDateTime.day ) .. "/" .. string.format( "%02i", tabDateTime.month ) .. "/" .. string.format( "%04i", tabDateTime.year )
		local strTime = string.format( "%02i", tabDateTime.hour ) .. "." .. string.format( "%02i", tabDateTime.min ) .. "." .. string.format( "%02i", tabDateTime.sec )
		print ( string.format( "%s(%s:%s) (%s) : %s : %s%s", strLF, strDate, strTime, time.str(), strPrefix, strIndent, strMessage ) )
	end
end
-- 
-- =======================================================================================
