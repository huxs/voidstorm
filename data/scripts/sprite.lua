-- Sprite = Sprite
-- Sprite, Collision = CollidableSprite
-- Sprite, Collision, Physics = MovableCollidableSprite

dofile "class.lua"

Sprite = class(nil)


function Sprite.new(self, pos)

   self.entity = es.createEntity()
   
   es.setPosition(self.entity, pos)
   es.addSprite(self.entity)

   table.insert(sprites, self.entity, self)

end

function Sprite:destroy()

   sprites[self.entity] = nil
   
   es.destroyEntity(self.entity)
   
end

function Sprite:setPosition(pos)

   es.setPosition(self.entity, pos)

end

function Sprite.getPosition(self)

   return es.getPosition(self.entity)
 
end

function Sprite:setRotation(value)

   es.setRotation(self.entity, value)

end

function Sprite:getRotation()

   return es.getRotation(self.entity)

end


function Sprite:setDepth(value)

   es.setDepth(self.entity, value)

end

function Sprite:getDepth()

   return es.getDepth(self.entity)

end

function Sprite:setScale(value)

   es.setScale(self.entity, value)

end

function Sprite:getScale()

   return es.getScale(self.entity)

end

function Sprite:setTexture(texture)

   es.setTexture(self.entity, texture)

end

function Sprite:setSize(size)

   es.setSize(self.entity, size)

end

function Sprite:setTextureAndSize(texture)

   es.setTexture(self.entity, texture)

   es.setSize(self.entity, texture:size())

end

function Sprite:setOrigin(origin)

   es.setOrigin(self.entity, origin)

end

function Sprite:setColor(color)

   es.setColor(self.entity, color)

end

function Sprite:getColor()

   return es.getColor(self.entity)

end

function Sprite:animateHue()

   local hsv = color.toHSVFromRGB(self:getColor()) + color.new(60 * dt, 0, 0, 0)

   if hsv.h > 360 then
      hsv.h = 0
   end 

   self:setColor(color.toRGBFromHSV(hsv))
   
end

CollidableSprite = class(Sprite)

function CollidableSprite.new(self, pos, type, mask)

   Sprite.new(self, pos)

   es.addCollision(self.entity, type, mask)
   
end

function CollidableSprite:setCircleShape(radius)

   es.setCircleShape(self.entity, radius)
   
end

function CollidableSprite:addResponder()

   es.addResponder(self.entity)

end

function CollidableSprite:setType(type)

   es.setType(self.entity, type)

end

function CollidableSprite:getType()

   return es.getType(self.entity)

end

function CollidableSprite:setMask(mask)

   es.setMask(self.entity, mask)

end

function CollidableSprite:setOffset(x, y)

   es.setOffset(self.entity, y)

end

function CollidableSprite:setRadius(r)

   es.setRadius(self.entity, r)

end

MovableCollidableSprite = class(CollidableSprite)

function MovableCollidableSprite.new(self, pos, type, mask, radius)

   CollidableSprite.new(self, pos, type, mask, radius)

   es.addPhysics(self.entity)

end

function MovableCollidableSprite:setMass(mass)

   es.setMass(self.entity, mass)

end

function MovableCollidableSprite:addForce(force)

   es.addForce(self.entity, force)

end

function MovableCollidableSprite:setVelocity(vel)

   es.setVelocity(self.entity, vel)

end

function MovableCollidableSprite:getVelocity()

   return es.getVelocity(self.entity)

end

function MovableCollidableSprite:getCollidedEntities()

   return es.getCollidedEntity(self.entity)

end
