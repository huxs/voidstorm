Ray = class(nil)

function Ray.new(self, pos, dir, mask)
   
   self.entity = es.createEntity()
   self.dir = dir
   
   es.addCollision(self.entity, type.ray, mask)
   es.setRayShape(self.entity, dir)
   es.setPosition(self.entity, pos)
   es.addResponder(self.entity)

end

function Ray.destroy(self)

   es.destroyEntity(self.entity)

end

function Ray.setPosition(self, pos)

   es.setPosition(self.entity, pos)

end
 
function Ray.setDirection(self, dir)

   es.setDirection(self.entity, dir)
   self.dir = dir

end

function Ray.draw(self, color)

   local p = es.getPosition(self.entity)
   local d = self.dir - p

   voidstorm.drawLine(p, p + d, color)

end
