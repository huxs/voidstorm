dofile "fps.lua"

-- Pass flags for what to debug render
function debug_view()

   fps.update(dt)
   
   font.write("FPS: " .. tostring(fps.count), vec2.new(0.01, 0.04), false)
   font.write("# Entities: " .. tostring(es.getNrOfEntities()), vec2.new(0.01, 0.01), false)

   p = 0
   for i,v in pairs(sprites) do
      --font.write("Entity: " .. i .. " Sprite: " .. tostring(v), vec2.new(10, p * 20 + 50), false)
      if v ~= nil then
	 p = p + 1
      end
   end

end

-- Create a number of sprites for performance testing.
function debug_sprites()

   local sizex, sizey = 50, 50
   local count = 1000
   local circles = 8

   for i=1,count,1 do

      -- Calculate position.
      local norm = ((circles * 2 * 3.14) / count) * i
      local circlei = norm / (2 * 3.14)
      local x = math.cos(norm) * (sizex + 50) * circlei + 50 + 250
      local y = math.sin(norm) * (sizey + 50) * circlei + 50 + 250

      -- Calculate color.
      local n = i / count
      local h = n * 360
      local s = math.max(n, 0.8)
      local v = 1
      local hsv = color.new(h,s,v,1)

      local e = Enemy(wheelTexture, vec2.new(x, y))
      e:setColor(color.toRGBFromHSV(hsv))
      
   end    

end

