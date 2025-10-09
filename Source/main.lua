import "network.lua"

local pd = playdate
local gfx = pd.graphics
local engine = cLib

local game = 0
local settings = { render = 85, pixX = 2, pixY = 2 }


local edit = 0

local seed = pd.getCurrentTimeMilliseconds()
engine:init(seed)

engine:addEnt(0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.5, 1.8, 0.56, 0.08, 0)
engine:newSettings(settings.render, settings.pixX, settings.pixY)

function pd.update()
    if game == 0 then
        engine:newSettings(settings.render, settings.pixX, settings.pixY)

        if pd.buttonJustPressed(pd.kButtonDown) then edit += 1 elseif pd.buttonJustPressed(pd.kButtonUp) then edit -= 1 end
        if edit < 0 then edit = 0 elseif edit > 2 then edit = 2 end

        if pd.buttonJustPressed(pd.kButtonLeft) then settings[edit] -= 1 elseif pd.buttonJustPressed(pd.kButtonRight) then settings[edit] += 1 end
        if settings[edit] < 0 then settings[edit] = 0 end

        if pd.buttonJustPressed(pd.kButtonA) then game = 1 end

        pd.graphics.drawText("Render: " .. settings.render, 2, 50)
        pd.graphics.drawText("Pixel Size X: " .. settings.pixX, 2, 100)
        pd.graphics.drawText("Pixel Size Y: " .. settings.pixY, 2, 150)
    elseif game == 1 then
        engine:player()
        engine:render()
    end
    pd.drawFPS(200, 10)
end