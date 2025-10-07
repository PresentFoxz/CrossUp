-- network.lua

-- import necessary libraries
import "CoreLibs/utilities/where"
import "CoreLibs/object"
import "CoreLibs/graphics"
import "CoreLibs/ui"

local net <const> = playdate.network

class("NetworkManager").extends()

function NetworkManager:init()
    self.connected = false
    self.buffer = {}
    self.ServerConn = nil
end

function NetworkManager:connect(address, port)
    self.ServerConn = net.tcp.new(address, port)

    self.ServerConn:open(function (connected, err)
        if connected then
            self.connected = true
            print("✅ Connected to " .. address .. ":" .. port)
        else
            self.connected = false
            print("❌ Failed to connect: " .. tostring(err))
            connection = 1
        end
    end)
end

function NetworkManager:update()
    if self.connected and self.ServerConn and self.ServerConn:getBytesAvailable() > 0 then
        local chunk = self.ServerConn:read()
        if chunk then
            self.buffer[#self.buffer + 1] = chunk
        end
    end
end

function NetworkManager:readPacket(device)
    if self.connected and self.ServerConn then
        local bufAsString = table.concat(self.buffer, "")
        local newlineIndex = string.find(bufAsString, "\n")
        if not newlineIndex then
            return false -- No complete packet yet
        end

        local packet = string.sub(bufAsString, 1, newlineIndex - 1)
        self.buffer = {string.sub(bufAsString, newlineIndex + 1)}
        if not device then
            print("📥 Received: " .. packet)
        end
        return packet
    end
end

function NetworkManager:sendPacket(message, device)
    if self.connected and self.ServerConn then
        local success, err = pcall(function()
            self.ServerConn:write(message .. "\n")
        end)
        if not device then
            if success then
                print("📤 Sent: " .. message)
            else
                print("❌ Error sending packet: " .. tostring(err))
            end
        end
    else
        if not device then
            print("❌ Cannot send packet, not connected.")
        end
    end
end