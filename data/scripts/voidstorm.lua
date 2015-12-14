
function GameInitialize()  

   dofile "sprite.lua"
   dofile "particles.lua"
   dofile "debug.lua"
    
   sprites = {}

   World = {}
   dofile "world.lua"

   World.initialize()
   World.start()

   voidstorm.setPostProcessParams(3.0, 1.0, 0.0)

   collectgarbage()

end

function GameUpdateAndRender()

   debug_view()
   
   World.update()

end
