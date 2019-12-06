-- Override the dofile function
function dofile(filename)
	host:dofile(filename);
end

function recurse(printObj, level)
	if (level > 10) then return ""; end
	if type(printObj) == "string" then
		return printObj;
	elseif type(printObj) == "number" then
		return string.format("%.7g", printObj);
	elseif type(printObj) == "boolean" then
		if (printObj) then return "true"; else return "false"; end
	elseif type(printObj) == "table" then
		local tblStr = "{\n";
		for k,v in pairs(printObj) do
			if (v ~= printObj and not (level > 1 and k == "_G")) then
				tblStr = tblStr..string.rep("\t",level)..k.."="..recurse(v, level+1).."\n";
			end
		end
		return tblStr..string.rep("\t",level-1).."}";
	elseif type(printObj) == "userdata" then
		local mt = getmetatable(printObj);
		if (mt) then
			return recurse(mt, level);
		end
		return tostring(printObj);
	else
		return tostring(printObj);
	end
end

function o(printObj)
	local writeStr = recurse(printObj,1);
	if (#writeStr > 0) then
		luap:write(writeStr);
	end
end

function print(printObj)
	local writeStr = recurse(printObj,1);
	if (#writeStr > 0) then
		host:debugPrint(writeStr);
		-- write to log file
		local logFile = io.open("scriptlog.txt", "a");
		io.output(logFile):write(writeStr);
		io.close(logFile);
	end
end

function startSession(name, timeout)
	if (type(name) ~= "string") then name = "sessid"; end
	if (type(timeout) ~= "number") then timeout = 1200; end
	luap:startSession(name, timeout);
end

function saveSession()
	luap:saveSession("local function _s() " .. serialize(SESSION) .. " end SESSION=_s()");
end

function killSession(name)
	if (type(name) ~= "string") then name = "sessid"; end
	luap:killSession(name);
end

function location(uri)
	if (type(uri) ~= "string") then uri = "."; end
	luap:location(uri);
end

function abort()
	luap:abort();
end

-----------------------------------------

-- Retrieve settings from file
--[[changeSettingsEvent = {
	wndResX		= 1024,
	wndResY		= 768,
	fsResX		= 1024,
	fsResY		= 768,
	bpp			= 32,
	refreshRate	= 60,
	fullscreen 	= false,
	vsync		= true
};
settingsFile = io.open("engine.cfg", "r");
if (settingsFile ~= nil) then
	fileStr = io.input(settingsFile):read("*all");
	if (fileStr ~= nil) then
		for k, v in string.gmatch(fileStr, "(%a+)%s+=%s+(%w+)") do
			if (changeSettingsEvent[k] ~= nil) then
				if (type(changeSettingsEvent[k]) == "number") then
					changeSettingsEvent[k] = tonumber(v);
				elseif (type(changeSettingsEvent[k]) == "boolean") then
					changeSettingsEvent[k] = (v == "true" or v == 1);
				end
				print("setting ["..k.."]="..v.."\n");
			end
		end
	end
	io.close(settingsFile);
else
	print("engine.cfg not found\n");
end
print(changeSettingsEvent);
print("\n");
-- change engine settings based on init file
Engine:triggerEvent("SYS_CHANGE_SETTINGS", changeSettingsEvent);

-- Temp stuff follows

-- Test dofile
dofile("data/script/gui.lua");

function calledFromCode()
	print("Called from code!\n");
	return "return string";
end

function handleEventSomething()
	print("EVENT_SOMETHING handled in script\n");
end

Engine:registerHandler("EVENT_SOMETHING", handleEventSomething, 0);
--]]
--[[testEngineEventData = {
	mInt = 1,
	mFloat = 255.83145531,
	mString = "3",
	mBool = true
};

Engine:registerHandler("TEST_ENGINE_EVENT", handleTestEngineEvent, 0);
Engine:registerHandler("TEST_ENGINE_EVENT", handleTestEngineEvent2, 1);
Engine:registerHandler("TEST_ENGINE_EVENT", handleTestEngineEvent2, 0);
Engine:triggerEvent("TEST_ENGINE_EVENT", testEngineEventData);
Engine:unregisterHandler("TEST_ENGINE_EVENT", handleTestEngineEvent);
Engine:unregisterHandler("TEST_ENGINE_EVENT", handleTestEngineEvent2);
--]]