import "network.lua"

local pd = playdate
local gfx = pd.graphics
local engine = cLib

local seed = pd.getCurrentTimeMilliseconds()
engine:init(seed)

engine:addEnt(0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.5, 1.8, 0.56, 0.08, 0)

function pd.update()
    engine:player()
    engine:render()
    pd.drawFPS(200, 10)
end