import "network.lua"

local pd <const> = playdate
local gfx <const> = playdate.graphics
local engine <const> = cLib

local game = 0
local edit = 1
local settings = { 85, 2, 2 }
local settingCap = { {0, 85}, {1, 4}, {1, 4} }
local settingAdd = { 1, 1, 1 }

local modelRend = 0
local modelCap = 1

local pendingGameSwitch = nil

local seed = pd.getCurrentTimeMilliseconds()
engine:init(seed)

local menu = pd.getSystemMenu()

reset, error = menu:addMenuItem("Settings", function()
    pendingGameSwitch = 0
    edit = 1
    modelRend = 0
    modelCap = 1
end)

engine:addEnt(0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.5, 1.8, 0.56, 0.08, 0)

function pd.update()
    if pendingGameSwitch ~= nil then
        game = pendingGameSwitch
        pendingGameSwitch = nil
    end
    if game == 0 then
        if pd.buttonJustPressed(pd.kButtonDown) then edit = edit + 1 elseif pd.buttonJustPressed(pd.kButtonUp) then edit = edit - 1 end
        if edit < 1 then edit = 1 elseif edit > 3 then edit = 3 end

        if pd.buttonJustPressed(pd.kButtonLeft) then settings[edit] = settings[edit] - settingAdd[edit] elseif pd.buttonJustPressed(pd.kButtonRight) then settings[edit] = settings[edit] + settingAdd[edit] end
        if settings[edit] < settingCap[edit][1] then settings[edit] = settingCap[edit][1] elseif settings[edit] > settingCap[edit][2] then settings[edit] = settingCap[edit][2] end

        if pd.buttonJustPressed(pd.kButtonRight) or pd.buttonJustPressed(pd.kButtonLeft) then engine:newSettings(settings[1], settings[2], settings[3]) end
        if pd.buttonJustPressed(pd.kButtonA) then game = 1 end

        if pd.buttonJustPressed(pd.kButtonB) then modelRend = modelRend + 1 end

        if modelRend > modelCap then modelRend = 0 end

        engine:drawObject(modelRend)

        gfx.setColor(gfx.kColorWhite)
        gfx.fillRect(0, 72, 140, 75)

        gfx.setColor(gfx.kColorBlack)
        gfx.drawText("Render: " .. settings[1], 2, 75)
        gfx.drawText("Pixel Size X: " .. settings[2], 2, 100)
        gfx.drawText("Pixel Size Y: " .. settings[3], 2, 125)
    elseif game == 1 then
        engine:player()
        engine:render()
    end
    pd.drawFPS(200, 10)
end