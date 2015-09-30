function Sprite:new(type, mask, pos, radius)

   self.entity = es.createEntity()
   
   es.setPosition(self.entity, pos)

   es.addSprite(self.entity)
   es.addPhysics(self.entity)
   es.addCircleShape(self.entity, type, mask, radius)

   table.insert(objects, self.entity, self)

end

function Sprite:destroy()

   objects[self.entity] = nil
   
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

function Sprite:setType(type)

   es.setType(self.entity, type)

end

function Sprite:setMask(mask)

   es.setMask(self.entity, mask)

end

function Sprite:setOffset(x, y)

   es.setOffset(self.entity, y)

end

function Sprite:setRadius(r)

   es.setRadius(self.entity, r)

end

function Sprite:setMass(mass)

   es.setMass(self.entity, mass)

end

function Sprite:setTexture(texture)

   es.setTexture(self.entity, texture)

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

function Sprite:writeID()

   debugSprite(self)
   
end

