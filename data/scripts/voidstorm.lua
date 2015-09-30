dofile "debug.lua"
dofile "map.lua"

function GameUpdateAndRender()

   if initialized == false then

      -- Preload the texture to be used.
      boulderTexture = texture.new("boulder.dds")
      wheelTexture = texture.new("wheel.dds")
      diamondTexture = texture.new("diamond.dds")

      -- Initialize all the entities in the map.
      Map.start();

      --debug_sprites()

      voidstorm.setPostProcessParams(1.9, 1.1, 1.0)

      initialized = true
      
   end

   debug_view()
   
   Map.update()

end
