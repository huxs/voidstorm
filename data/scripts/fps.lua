fps = {}
fps.frames = 0
fps.dtime = 0
fps.count = 0

function fps.update(dt)

    fps.frames = fps.frames + 1
    fps.dtime = fps.dtime + dt

    if fps.frames == 120 then
        fps.count = fps.frames / fps.dtime
        fps.frames = 0
        fps.dtime = 0
    end
    
end
