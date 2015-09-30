

-- Rotation speed[min,max]
-- Color[min,max]

ParticleEmitter = class(nil)

function ParticleEmitter:new(id)

   print("Creating emitter " .. id)
   self.particlesPerEmit = 1
   self.spawnTime = 0.5
   self.lifetime = 0;

   self.lifetimeMin = 1.0
   self.lifetimeMax = 2.0

   self.rotationMin = 0.0
   self.rotationMax = 360.0

   -- Not sure if this is needed.
   self.colorMin = color.new(1,1,1,1)
   self.colorMax = color.new(1,1,1,1)
   
   self.sizeMin = 0.5
   self.sizeMax = 1.0
   
   self.startColor = color.new(1,1,1,1)
   self.endColor = color.new(1,1,1,0)
   
   self.force = vec2.new(1,0)
   self.spread = 0 -- angle
   self.position = vec2.new(0,0)
   self.relative = true
   self.texture = nil
   
end

ParticleEffect = class(nil)

function ParticleEffect:new(name)

   print("Creating effect " .. name)

   self.name = name   
   self.emitters = {}

end

function ParticleEffect:addEmitter()

   local emitter = ParticleEmitter(table.getn(self.emitters))
   
   table.insert(self.emitters, emitter)

   return emitter

end

function ParticleEffect:store()

   particle.store(self)

end
