pico-8 cartridge // http://www.pico-8.com
version 42
__lua__
------------------------------
-- player class (cat)
player_class = {}
player_class.__index = player_class

-- in the creation of the player, initialize gesture_total_timer
function player_class:new(x,y)
  local obj = {
    x = x,
    y = y,
    base_y = y,
    lives = 5,
    sprites = {0,2,4,6},
    frame = 1,
    timer = 0,             -- for frame change
    gesture_total_timer = 0, -- for total gesture duration
    frame_speeds = {15,5,15,5},
    animation = "idle",
    hurt = false,
    hurt_timer = 0
  }
  setmetatable(obj,player_class)
  return obj
end

function player_class:update()
  if self.hurt then
    -- (code for hurt without changes)
    self.hurt_timer = self.hurt_timer + 1
    self.sprites = {40}
    self.frame = 1
    local offset = 0
    if self.hurt_timer < 8 then
      offset = -self.hurt_timer
    elseif self.hurt_timer < 16 then
      offset = -(16 - self.hurt_timer)
    else
      self.hurt = false
      self.sprites = {0,2,4,6}
      self.animation = "idle"
      self.timer = 0
      self.frame = 1
      offset = 0
    end
    self.y = self.base_y + offset
    return
  end

  -- block for gesture animation
  if self.animation == "gesture" then
      -- increment both counters
      self.gesture_total_timer = self.gesture_total_timer + 1
      self.timer = self.timer + 1
      
      if self.frame_speeds[self.frame] and self.timer >= self.frame_speeds[self.frame] then
          self.timer = 0
          self.frame = (self.frame % #self.sprites) + 1
      end
      
      -- once total gesture time has passed, return to idle
      if self.gesture_total_timer >= 20 then  -- 20 frames for a complete animation
          self.animation = "idle"
          self.sprites = {0,2,4,6}
          self.frame_speeds = {15,5,15,5}
          self.timer = 0
          self.frame = 1
          self.gesture_total_timer = 0
      end
      return
  end

  -- normal logic (idle/attack) based on mouse input:
  local desired_animation = (mouse.button() == 1) and "attack" or "idle"
  if desired_animation ~= self.animation then
    self.animation = desired_animation
    if self.animation == "attack" then
      self.sprites = {8,10}
      self.frame_speeds = {10,10}
    else
      self.sprites = {0,2,4,6}
      self.frame_speeds = {15,5,15,5}
    end
    self.timer = 0
    self.frame = 1
  end

  self.timer = self.timer + 1
  if self.frame_speeds[self.frame] and self.timer >= self.frame_speeds[self.frame] then
    self.timer = 0
    self.frame = (self.frame % #self.sprites) + 1
  end
end

function player_class:draw()
  spr(self.sprites[self.frame], self.x, self.y, 2, 2)
end

------------------------------
-- enemy class (ghost)
enemy = {}
enemy.__index = enemy

function enemy:new()
  local side = (rnd(1) < 0.5) and "left" or "right"
  local start_x = (side == "right") and 128 or -16
  local flip = (side == "left")
  local start_y = flr(rnd(128))
  
  -- calculation of number of symbols (no changes compared to original logic)
  local t = (score - 20) / 80
  if t < 0 then t = 0 end
  if t > 1 then t = 1 end
  local p1 = (1 - t)*0.70 + t*0.40   -- probability of 1 symbol
  local p2 = (1 - t)*0.20 + t*0.30   -- probability of 2 symbols
  local p3 = (1 - t)*0.05 + t*0.20   -- probability of 3 symbols
  local p4 = (1 - t)*0.05 + t*0.10   -- probability of 4 symbols
  local r = rnd(1)
  local symbol_count = 1
  if r < p1 then
    symbol_count = 1
  elseif r < p1 + p2 then
    symbol_count = 2
  elseif r < p1 + p2 + p3 then
    symbol_count = 3
  else
    symbol_count = 4
  end
  
  local possible_symbols = {46, 47, 62, 63}
  local symbols = {}
  for i=1, symbol_count do
    add(symbols, possible_symbols[flr(rnd(#possible_symbols)) + 1])
  end
  
  -- speed is calculated with a score based increase
  local base_speed = 0.005
  local speed_increase = score * 0.00005
  
  local obj = {
    start_x = start_x,       -- fixed initial position
    start_y = start_y,
    x = start_x,
    y = start_y,
    target_x = player.x,
    target_y = player.y,
    progress = 0,
    speed = base_speed + speed_increase,
    timer = 0,
    sine_amplitude = 5,
    sine_speed = 0.025,
    sprite = 66,
    flip = flip,
    active = true,
    vanishing = false,
    vanish_timer = 0,
    vanish_frame = 1,
    vanish_frame_speed = 5,
    vanish_sprites = {68,70,72,74,76},
    symbols = symbols,
    symbol_pattern = ""
  }
  setmetatable(obj, enemy)
  return obj
end

function enemy:update()
  if not self.active then return end

  -- update speed based on score
  self.speed = 0.005 + score * 0.00005

  if #self.symbols == 0 and not self.vanishing then
    self.vanishing = true
    self.vanish_timer = 0
    self.vanish_frame = 1
    self.vanish_sprites = {72,74,76}  -- gesture vanish animation (16x16)
    return
  end

  if self.vanishing then
    self.vanish_timer = self.vanish_timer + 1
    if self.vanish_timer >= self.vanish_frame_speed then
      self.vanish_timer = 0
      self.vanish_frame = self.vanish_frame + 1
      if self.vanish_frame > #self.vanish_sprites then
        self.active = false
      end
    end
    return
  end

  self.timer = self.timer + self.sine_speed
  self.progress = min(self.progress + self.speed, 1)
  -- interpolation from initial position (start_x, start_y) to target
  local base_x = self.start_x + (self.target_x - self.start_x) * self.progress
  local base_y = self.start_y + (self.target_y - self.start_y) * self.progress
  local current_y = base_y + self.sine_amplitude * sin(self.timer)

  if abs(base_x - player.x) < 12 and abs(current_y - player.y) < 12 then
    self.vanishing = true
    self.collision = true  -- indicates that it is triggered by collision
    self.vanish_timer = 0
    self.vanish_frame = 1
    if not player.hurt then
      player.hurt = true
      player.hurt_timer = 0
      player.base_y = player.y  -- store original position
      if player.lives > 0 then
        player.lives = player.lives - 1
      end
    end
    self.x = base_x
    self.y = current_y
    return
  end
  
  self.x = base_x
  self.y = current_y
end

function enemy:draw()
  if self.active then
    if self.vanishing then
      local current_sprite = self.vanish_sprites[self.vanish_frame]
      spr(current_sprite, self.x, self.y, 2, 2, self.flip)
    else
      spr(self.sprite, self.x, self.y, 2, 2, self.flip)
    end
    
    if self.symbols and #self.symbols > 0 then
      local total_width = #self.symbols * 8
      local ghost_width = 16
      local start_x = self.x + (ghost_width - total_width) * 0.5
      local symbol_y = self.y - 8
      for i, sym in ipairs(self.symbols) do
         spr(sym, start_x + (i-1)*8, symbol_y)
      end
    end
  end
end

------------------------------
-- game global variables
------------------------------
enemies = {}           -- list of ghosts
spawn_timer = 60       -- spawn timer
vanish_count = 0
score = 0              -- global score

------------------------------
-- mouse configuration (simulated with gamepad)
------------------------------
-- virtual cursor position
virtual_mouse_x = 64
virtual_mouse_y = 64
mouse_speed = 2  -- pixels per frame

mouse = {
  init = function()
    -- initialize virtual mouse position to center
    virtual_mouse_x = 64
    virtual_mouse_y = 64
  end,
  pos = function() return virtual_mouse_x, virtual_mouse_y end,
  button = function() 
    -- use gamepad button 4 (X button) to simulate mouse click
    return (btn(4) and 1) or 0
  end,
  just_pressed = false,
  just_released = false
}
mouse_prev = false

function update_mouse_press()
  local current = (mouse.button() == 1)
  mouse.just_pressed = current and (not mouse_prev)
  mouse.just_released = (not current) and mouse_prev
  mouse_prev = current
end

function update_virtual_mouse()
  -- update virtual mouse position with gamepad D-pad
  -- btn(0) = left, btn(1) = right, btn(2) = up, btn(3) = down
  if btn(0) then virtual_mouse_x = max(0, virtual_mouse_x - mouse_speed) end
  if btn(1) then virtual_mouse_x = min(127, virtual_mouse_x + mouse_speed) end
  if btn(2) then virtual_mouse_y = max(0, virtual_mouse_y - mouse_speed) end
  if btn(3) then virtual_mouse_y = min(127, virtual_mouse_y + mouse_speed) end
end

--------------------------------------
-- pdollar recognizer functions
--------------------------------------
current_stroke_id = 1
drawing_symbol = false
symbol_lines = {}   -- points of the drawn gesture (stored as {x,y})
pdollar = nil
gesture_to_sprite = {
  hline = 46,
  vline = 47,
  uparrow = 62,
  downarrow = 63
}

------------------------------
-- _init(): initialize game, pdollar, and menu state
------------------------------
function _init()
  scene = "menu"  -- game state: "menu" or "game"
  player = player_class:new(56,56)
  -- create a ghost for the menu; position it at the side with a default symbol (hline)
  menu_ghost = enemy:new()
  menu_ghost.flip = false
  menu_ghost.x = 100
  menu_ghost.y = 50
  menu_ghost.symbols = {46}
  menu_ghost.start_x = 100
  menu_ghost.start_y = 50
  menu_ghost.target_x = 100
  menu_ghost.target_y = 50
  
  
  enemies = {}
  spawn_timer = 60
  vanish_count = 0
  score = 0
  mouse.init()  -- initialize virtual mouse position
  current_stroke_id = 1
  drawing_symbol = false
  symbol_lines = {}
  pdollar = new_pdollar_recognizer()
  music(-1)  -- stop music in menu state
end

------------------------------
-- _update(): state manager for menu and game updates
------------------------------
function _update()
  if scene == "menu" then
    update_menu()
  elseif scene == "game" then
    update_game()
  elseif scene == "gameover" then
    update_gameover()
  end
end

--------------------------------------
-- update_menu(): update logic for the menu state
--------------------------------------
function update_menu()
  -- update player and menu ghost
  player:update()
  if menu_ghost then
    menu_ghost:update()  -- update vanish animation if triggered
  end
  
  update_virtual_mouse()
  update_mouse_press()
  local mx, my = mouse.pos()
  local mbtn = mouse.button()
  
  if mbtn == 1 then
    if not drawing_symbol then
      drawing_symbol = true
      symbol_lines = {}
      add(symbol_lines, {mx + 4, my + 4})
    else
      local last_point = symbol_lines[#symbol_lines]
      if last_point and ((mx + 4) ~= last_point[1] or (my + 4) ~= last_point[2]) then
        add(symbol_lines, {mx + 4, my + 4})
      end
    end
  end
  
  if mouse.just_released then
    if #symbol_lines > 0 then
      local gesture_points = {}
      for i, pt in ipairs(symbol_lines) do
        add(gesture_points, {x = pt[1], y = pt[2], id = current_stroke_id})
      end
      local res = pdollar:recognize(gesture_points)
      -- solo permitir gestos hline o vline (o los que quieras)
      if (res.name == "hline" or res.name == "vline") and res.score > 0.9 then
        -- comprobar si el símbolo reconocido coincide con el del fantasma
        if menu_ghost.symbols and menu_ghost.symbols[1] == gesture_to_sprite[res.name] then
          -- activar animación de desaparición del fantasma
          menu_ghost.vanishing = true
          menu_ghost.vanish_timer = 0
          menu_ghost.vanish_frame = 1
        end
      end
      current_stroke_id = current_stroke_id + 1
      symbol_lines = {}
    end
    drawing_symbol = false
  end
  
  
  -- update ghost vanish animation in menu state
  if menu_ghost and menu_ghost.vanishing then
    menu_ghost.vanish_timer = menu_ghost.vanish_timer + 1
    if menu_ghost.vanish_timer >= menu_ghost.vanish_frame_speed then
      menu_ghost.vanish_timer = 0
      menu_ghost.vanish_frame = menu_ghost.vanish_frame + 1
      if menu_ghost.vanish_frame > #menu_ghost.vanish_sprites then
        -- once the ghost vanish animation completes, start the game and play music
        scene = "game"
        music(0)
      end
    end
  end
end

--------------------------------------
-- update_game(): original game update logic
--------------------------------------
function update_game()
  if player.lives <= 0 then
    scene = "gameover"
    music(-1)
    return
  end

  player:update()
  
  for e in all(enemies) do
    e:update()
  end
  
  -- accumulate count of ghosts that disappeared in this frame
  local vanish_this_frame = 0
  for i = #enemies, 1, -1 do
    if not enemies[i].active then
      -- only count for points if not due to collision
      if not enemies[i].collision then
        vanish_this_frame = vanish_this_frame + 1
      end
      del(enemies, enemies[i])
      vanish_count = vanish_count + 1
    end
  end

  -- add points only for ghosts eliminated by gesture
  if vanish_this_frame > 0 then
    if vanish_this_frame > 1 then
      score = score + vanish_this_frame * 2
    else
      score = score + vanish_this_frame
    end
  end
  
  update_virtual_mouse()
  update_mouse_press()
  local mx, my = mouse.pos()
  local mbtn = mouse.button()
  
  if mbtn == 1 then
    if not drawing_symbol then
      drawing_symbol = true
      symbol_lines = {}
      add(symbol_lines,{mx + 4, my + 4})
    else
      local last_point = symbol_lines[#symbol_lines]
      if last_point and ((mx + 4) ~= last_point[1] or (my + 4) ~= last_point[2]) then
        add(symbol_lines,{mx + 4, my + 4})
      end
    end
  end
  
  if mouse.just_released then
    if #symbol_lines > 0 then
      local gesture_points = {}
      for i, pt in ipairs(symbol_lines) do
        add(gesture_points, {x = pt[1], y = pt[2], id = current_stroke_id})
      end
      local res = pdollar:recognize(gesture_points)
      if res.name ~= "no match" and res.score > 0.9 then
        if res.name == "hline" then
          player.animation = "gesture"
          player.sprites = {12,14}
          player.frame_speeds = {10,10}
          sfx(0)
        elseif res.name == "vline" then
          player.animation = "gesture"
          player.sprites = {32,34}
          player.frame_speeds = {10,10}
          sfx(1)
        elseif res.name == "uparrow" then
          player.animation = "gesture"
          player.sprites = {36,38}
          player.frame_speeds = {10,10}
          sfx(2)
        elseif res.name == "downarrow" then
          player.animation = "gesture"
          player.sprites = {32,38}
          player.frame_speeds = {10,10}
          sfx(3)
        end
        player.timer = 0
        player.frame = 1
        player.gesture_total_timer = 0
  
        -- maintain logic to remove the corresponding symbol in enemies
        local mapped_sprite = gesture_to_sprite[res.name]
        for e in all(enemies) do
          if e.symbols and #e.symbols > 0 and e.symbols[1] == mapped_sprite then
            del(e.symbols, e.symbols[1])
          end
        end
      end
      current_stroke_id = current_stroke_id + 1
      symbol_lines = {}
    end
    drawing_symbol = false
  end
  
  spawn_timer = spawn_timer - 1
  if spawn_timer <= 0 and (#enemies < 8) then
    add(enemies, enemy:new())
    spawn_timer = max(20, 60 - vanish_count * 2)
  end
end

--------------------------------------
-- update_gameover(): update logic for the game over state
--------------------------------------
function update_gameover()
  player:update()
  update_virtual_mouse()
  update_mouse_press()
  if mouse.just_released then
    _init()
  end
end

------------------------------
-- _draw(): state manager for menu and game draws
------------------------------
function _draw()
  if scene == "menu" then
    draw_menu()
  elseif scene == "game" then
    draw_game()
  elseif scene == "gameover" then
    draw_gameover()
  end
end

--------------------------------------
-- draw_menu(): drawing the menu state (background, player, ghost, and instructions)
--------------------------------------
function draw_menu()
  cls()
  map(0,0,0,0,128,128)
  player:draw()
  menu_ghost:draw()
  
  -- draw the drawn gesture
  for i = 2,#symbol_lines do
    local p1 = symbol_lines[i - 1]
    local p2 = symbol_lines[i]
    for dx = -1,1 do
      for dy = -1,1 do
        line(p1[1] + dx, p1[2] + dy, p2[1] + dx, p2[2] + dy,7)
      end
    end
  end
  
  local mx, my = mouse.pos()
  spr(64, mx, my)
  
  -- draw menu instruction text (all in english)
  print("use d-pad to move", 10, 10, 7)
  print("draw symbol on ghost", 10, 18, 7)
  print("press x to draw", 10, 26, 7)
end

--------------------------------------
-- draw_game(): original game drawing logic
--------------------------------------
function draw_game()
  cls()
  map(0, 0, 0, 0, 128, 128)
  player:draw()
  for e in all(enemies) do
    e:draw()
  end
  
  -- draw the line of the drawn gesture
  for i = 2,#symbol_lines do
    local p1 = symbol_lines[i - 1]
    local p2 = symbol_lines[i]
    for dx = -1,1 do
      for dy = -1,1 do
        line(p1[1] + dx, p1[2] + dy, p2[1] + dx, p2[2] + dy,7)
      end
    end
  end
  
  local mx, my = mouse.pos()
  spr(64, mx, my)
  
  -- draw player lives (top left corner)
  for i = 0,4 do
    if i < player.lives then
      spr(65, i * 8, 0)
    else
      spr(80, i * 8, 0)
    end
  end
  
  -- draw score in the top right corner (orange color, 9)
  local score_text = "score: "..score
  local text_width = #score_text * 4
  print(score_text, 128 - text_width - 2, 2, 9)
end

--------------------------------------
-- draw_gameover(): drawing the game over state
--------------------------------------
function draw_gameover()
  cls()
  map(0,0,0,0,128,128)
  player:draw()

  local go_text = "game over"
  local go_x = (128 - #go_text * 4) / 2
  print(go_text, go_x, 40, 8)

  local score_text = "score: "..score
  local score_x = (128 - #score_text * 4) / 2
  print(score_text, score_x, 80, 10)

  local pa_text = "press x to play again"
  local pa_x = (128 - #pa_text * 4) / 2
  print(pa_text, pa_x, 90, 7)

  local mx, my = mouse.pos()
  spr(64, mx, my)
end

--------------------------------------
-- functions of the pdollar recognizer
--------------------------------------
function new_point(x, y, id)
  return {x = x, y = y, id = id}
end

function new_point_cloud(name, points)
  local pc = {}
  pc.name = name
  pc.points = resample(points, 8)
  pc.points = scale(pc.points)
  pc.points = translateto(pc.points, {x=0, y=0})
  return pc
end

function new_result(name, score, time)
  return {name = name, score = score, time = time}
end

function new_pdollar_recognizer()
  local recognizer = {}
  recognizer.pointclouds = {}
  
  recognizer.pointclouds[1] = new_point_cloud("hline", {new_point(23, 62, 1), new_point(25, 62, 1), new_point(35, 62, 1), new_point(60, 62, 1), new_point(78, 62, 1), new_point(89, 62, 1), new_point(96, 62, 1), new_point(100, 62, 1), new_point(106, 62, 1), new_point(111, 62, 1)})
  recognizer.pointclouds[2] = new_point_cloud("hline", {new_point(72, 10, 1), new_point(24, 48, 2), new_point(29, 49, 2), new_point(52, 53, 2), new_point(88, 59, 2), new_point(97, 60, 2)})
  recognizer.pointclouds[3] = new_point_cloud("hline", {new_point(72, 10, 1), new_point(28, 68, 2), new_point(97, 53, 2)})

  recognizer.pointclouds[4] = new_point_cloud("vline", {new_point(78, 10, 1), new_point(58, 27, 2), new_point(58, 39, 2), new_point(58, 56, 2), new_point(58, 72, 2), new_point(58, 82, 2), new_point(58, 84, 2)})
  recognizer.pointclouds[5] = new_point_cloud("vline", {new_point(82, 12, 1), new_point(47, 33, 2), new_point(53, 48, 2), new_point(59, 73, 2), new_point(69, 105, 2)})
  recognizer.pointclouds[6] = new_point_cloud("vline", {new_point(73, 9, 1), new_point(68, 30, 2), new_point(68, 32, 2), new_point(64, 57, 2), new_point(56, 89, 2)})

  recognizer.pointclouds[7] = new_point_cloud("uparrow", {new_point(33, 69, 2), new_point(35, 66, 2), new_point(40, 57, 2), new_point(48, 47, 2), new_point(51, 43, 2), new_point(59, 50, 2), new_point(65, 57, 2), new_point(66, 62, 2), new_point(70, 68, 2)})
  recognizer.pointclouds[8] = new_point_cloud("uparrow", {new_point(72, 14, 1), new_point(39, 69, 2), new_point(57, 52, 2), new_point(67, 45, 2), new_point(70, 48, 2), new_point(80, 66, 2)})
  recognizer.pointclouds[9] = new_point_cloud("uparrow", {new_point(38, 62, 2), new_point(44, 52, 2), new_point(58, 34, 2), new_point(63, 32, 2), new_point(73, 49, 2), new_point(84, 67, 2)})

  recognizer.pointclouds[10] = new_point_cloud("downarrow", {new_point(82, 12, 1), new_point(39, 62, 2), new_point(50, 81, 2), new_point(57, 92, 2), new_point(59, 93, 2), new_point(61, 92, 2), new_point(64, 81, 2), new_point(71, 61, 2), new_point(75, 52, 2)})
  recognizer.pointclouds[11] = new_point_cloud("downarrow", {new_point(39, 43, 2), new_point(48, 62, 2), new_point(56, 73, 2), new_point(65, 69, 2), new_point(87, 34, 2)})
  recognizer.pointclouds[12] = new_point_cloud("downarrow", {new_point(84, 8, 1), new_point(41, 54, 2), new_point(47, 69, 2), new_point(63, 90, 2), new_point(75, 92, 2), new_point(86, 67, 2)})
  
  function recognizer:recognize(points)
    if #points < 2 then return new_result("no match", 0.0, 0) end
    local t0 = time()
    local candidate = new_point_cloud("", points)
    local best_index = -1
    local huge = 32767
    local best_score = huge

    for i, pc in ipairs(self.pointclouds) do
      local d = greedycloudmatch(candidate.points, pc)
      if d < best_score then
        best_score = d
        best_index = i
      end
    end

    local t1 = time()
    if best_index == -1 then
      return new_result("no match", 0.0, (t1-t0)*1000)
    else
      local score_val = best_score > 1.0 and (1.0 / best_score) or 1.0
      return new_result(self.pointclouds[best_index].name, score_val, (t1-t0)*1000)
    end
  end
  
  function recognizer:addgesture(name, points)
    local pc = new_point_cloud(name, points)
    add(self.pointclouds, pc)
    local count = 0
    for i, p in ipairs(self.pointclouds) do
      if p.name == name then count += 1 end
    end
    return count
  end
  
  function recognizer:deleteusergestures()
    local numpointclouds = 16
    while (#self.pointclouds > numpointclouds) do
      del(self.pointclouds, self.pointclouds[#self.pointclouds])
    end
    return numpointclouds
  end
  
  return recognizer
end

function resample(points, n)
  if #points < 2 then return points end
  local I = pathlength(points) / (n - 1)
  local D = 0
  local newpoints = {}
  add(newpoints, points[1])
  local i = 2
  while i <= #points do
    if points[i].id == points[i-1].id then
      local d = distance(points[i-1], points[i])
      if (D + d) >= I then
        local t = (I - D) / d
        local qx = points[i-1].x + t * (points[i].x - points[i-1].x)
        local qy = points[i-1].y + t * (points[i].y - points[i-1].y)
        local q = new_point(qx, qy, points[i].id)
        add(newpoints, q)
        insert_at(points, i, q)
        D = 0
      else
        D = D + d
      end
    end
    i += 1
  end
  if (#newpoints == n - 1) then
    add(newpoints, new_point(points[#points].x, points[#points].y, points[#points].id))
  end
  return newpoints
end

function pathlength(points)
  local d = 0
  for i = 2, #points do
    if points[i].id == points[i-1].id then
      d = d + distance(points[i-1], points[i])
    end
  end
  return d
end

function scale(points)
  local huge = 32767
  local minx = huge
  local maxx = -huge
  local miny = huge
  local maxy = -huge
  for i=1, #points do
    local p = points[i]
    minx = min(minx, p.x)
    miny = min(miny, p.y)
    maxx = max(maxx, p.x)
    maxy = max(maxy, p.y)
  end
  local size = max(maxx - minx, maxy - miny)
  local newpoints = {}
  for i=1, #points do
    local p = points[i]
    local qx = (p.x - minx) / size
    local qy = (p.y - miny) / size
    add(newpoints, new_point(qx, qy, p.id))
  end
  return newpoints
end

function translateto(points, pt)
  local c = centroid(points)
  local newpoints = {}
  for i=1, #points do
    local p = points[i]
    local qx = p.x + pt.x - c.x
    local qy = p.y + pt.y - c.y
    add(newpoints, new_point(qx, qy, p.id))
  end
  return newpoints
end

function centroid(points)
  local x = 0
  local y = 0
  for i=1, #points do
    x += points[i].x
    y += points[i].y
  end
  return new_point(x/#points, y/#points, 0)
end

function distance(p1, p2)
  local dx = p2.x - p1.x
  local dy = p2.y - p1.y
  return sqrt(dx * dx + dy * dy)
end

function insert_at(tbl, index, value)
  for j = #tbl, index, -1 do
    tbl[j+1] = tbl[j]
  end
  tbl[index] = value
end

function greedycloudmatch(points, pc)
  local huge = 32767
  local e = 0.5
  local step = flr((#points)^(1 - e))
  local best = huge
  for i = 1, #points, step do
    local d1 = clouddistance(points, pc.points, i)
    local d2 = clouddistance(pc.points, points, i)
    best = min(best, min(d1, d2))
  end
  return best
end

function clouddistance(pts1, pts2, start)
  local huge = 32767
  local matched = {}
  for i=1, #pts1 do matched[i] = false end
  local sum = 0
  local i = start
  repeat
    local index = -1
    local min_d = huge
    for j=1, #pts2 do
      if not matched[j] then
        local d = distance(pts1[i], pts2[j])
        if d < min_d then
          min_d = d
          index = j
        end
      end
    end
    matched[index] = true
    local weight = 1 - (((i - start + #pts1) % #pts1) / #pts1)
    sum = sum + weight * min_d
    i = (i % #pts1) + 1
  until i == start
  return sum
end


__gfx__
00000000000000001000000000000000000000000000000010000000000000000000000000000000000000000000000000000000011000000000001000000000
11100000000000110111011111110111001100000000000001110111111101110000000000000000000000000000000000000001110000110000011000000000
11111111110111100111111111111111001111011110111001111111111111110100000000000000000000000000000000000111100011100000111000000000
01111111111111100011111111111110000111111111111100111111111111100110000000001110010000000000000000001111111111104000111111101111
01111111111111100001aa1111a1a11000011111111111110001aa1111a1a1100110111111101110011000000000111000001111111111000401111111111111
001aa1111a1a11000001aa1a1aa1a10000001a111111aa100001aa1a1aa1a1000111111111111110011011111110111000001111111111100041111111111110
001aa1a1aa1a10000001aa1a1aa1a14000001a1aa1a1aa100001aa1a1aa1a10001111111111111000111111111111110000011111a111a1400041111a1111100
001aa1a1aa1a100000101aaa1aaa104000001a1aa1a1aa4000101aaa1aaa101000111111111110000111111111111100000011a11a111a4000014111a11a1100
0101aaa1aaa101040001011111110140000101aaa1aaa1410001011111110104001111111111104000111111111110000000111aa11aa400000155511aa11100
00101111111010400000005555500400000010111111104000000055555000400011111111111040001111111111100401010111111145000000555511111010
00010555550004000001155555551400000000055555004000011555555514100101a1a1a1a10400001111111111104010001011111455000001555551150101
000155555551401000011555555511000000015555555140000115555555410100101a111a1014100101a1a1a1a1040010000005555550000000555555555001
0011505550511001000000555505501000001150555051100000005555055001000005555500410100101a111a10411110011055555000000000005505555501
00010555550500010000555555505510000001055555050000005555555055010011505550551101001155555555110110110555555500000000000055555501
00055555555550010005555555555510000005555555555000055555555555010015055555050010001505555505010010055555555500000000005555555551
00555555555551100055555555555510000055555555551000555555555555110055555555551100005555555555100001555555555550000000055555555551
00000040000000000000000000000000000000000000000000000000000100000000001000010000000000000000004000000000000000000000000000000000
010000040000001000000000000000010001111000000000000000000001100000000111001100000000000000000040000001111100000000000000000c0000
011011141110111000000000000001100000111110000000000100000001110000001111011110000000000000000400000111111111000000000000000c0000
011111114111111000001011111011100000111111100000000011100001111000001111111110000000000000000400001aa71117aa100008888880000c0000
011a11114111110000011111111111101111111111110000000011111111111000001a1111a110000000011111004000411aa7a1a7aa110000000000000c0000
001a11a114511000011111111111110011111111111110000000111111111110000111a11a1a11000001111111114000411aa7a1a7aa111000000000000c0000
0011aa1111551000111111111111110001111111a111100000000111111111100001111a1a1a110000111111555110004111aaa1aaa1110000000000000c0000
0011116165551000011111111111110000115111a11110000000111111111a10000111a11a1a1100011111155551110040011155511100000000000000000000
010111161555510000011a111111a1000011555a1111100000001a111a11a11000001a1111a110001111aa555511111041101555551010000000000000000000
001011111155500001011a111111a1000004555551110000000011aa11aa11100010111161111010011155555511110011555055505551100000000000000000
0100055505555000101011aa11aa101000401555555000000001011111111101000100555550010000055555505100001155550005555110000000000a000a00
1000555550050000100151111111010004000055555500001000101111111010001155555555511000055555055000100555005550055501000b000000a0a000
101105555555000010155555550000004100011055550000010000055550410000115555555501100000555055050001005055555550500100b0b000000a0000
10115055555550001010055550510000011001550555500001015055555514000000155555551000000500050055500100055555555500100b000b0000000000
10055555555555001005555505544400001100555555550001050555555550400101115555511000005555555555500100555555555550100000000000000000
01555555555555001555555555550444000115555555555000115555555500040011000000000000005555555555511000555555555551000000000000000000
00777700000000000000077777700000000000000000000000000000000000000000007070700000000000000000000000000000000000000000000000000000
070000700ee0ee000000777777770000000000000000000000555500777700000000070707070000000000000000000000000000000000000000000000000000
70000007eeeeeee00007777777777000000000000000000005555557775770000000707070707000000000000000000000000000000000000000000000000000
70000007eeeeeee00007557775557000000077770000000055500775777557000007050705050000000000000000000000000000000000000000000000000000
70000007eeeeeee00007575757557000007777777700000055007777777777700000505050507000000000505000000000000000000000000000000000000000
700000070eeeee000007755755577000077777777770000055007777777777700007050705070000000005070500000000000000000000000000000000000000
0700007000eee0000007777777777000777777777777000005077777777777700000707070707000000070707070000000000070700000000000000000000000
00777700000e00000007755555777000777777777777000000077777777777700007050505070000000005050500000000000505000000000000000000000000
00000000000000000007775557777000777777777777700000077777777777000000705050707000000070505070000000000070500000000000000000000000
0ee0ee00000000000007777777777000777777777777700000077777777770000007070707070000000007070700000000000007000000000000000000000000
e00e00e0000000000007777777777000757777577777700000077777777770000000707070707000000070707070000000000000000000000000000000000000
e00000e0000000000007777777777000775775777777700000007777777700000007070707070000000007070700000000000000000000000000000000000000
e00000e0000000000007777777777000077777777777700000000777777000000000707070707000000000707000000000000000000000000000000000000000
0e000e00000000000007777777777000007555777777700700000007777000000007070707070000000000000000000000000000000000000000000000000000
00e0e000000000000000777777777700000077777777777000000000077000000000707070707000000000000000000000000000000000000000000000000000
000e0000000000000000077777777770000007777777770000000000000700000000070707070700000000000000000000000000000000000000000000000000
111111111111551111111111dddd5555555555dd55555555555555555ddddd111111511111111111555555555dd5555500000000000000000000000000000000
1111111111115511155551155ddd5555555555dd55555555555555555ddddd111111111111111111555555555dd5555500000000000000000000000000000000
550555555155555555d55d5dd5dd5555551555dd55555555555555555dddd5111111111111511111551555555dd5555500000000800000000000000888000000
551555555155155511111555dddd5555555555dd55555555555555555ddddd55d555555555555555555555555dd5555500800800000800000080080000080000
111111111111115511111115dddddddddddddddddddddddddddddddddddddddd55ddd55555555555dddddddddddddddd08080000000000000888000000008000
111111111151111511111115dddddddddddddddddddddddddddddddddddddd111511111100051555dddddddddddddddd00800000000000000080000000000000
111111111115111111111115dddddddddddddddddddddddddddddddddddddd111111151055111111dddddddddddddddd00000000000000800000000000000080
111111111115111111111515dddddddddddddddddddddddddd6dd6666ddddd111111111011111111dddddddddddddddd00000000000000000000000000000000
110111111115111111555555dddd66666666666666666666666666666ddddd111111111111111111666666666666666600000000000000000000000000000000
111511111115515111111115dd55111666666666666666666666666dd6dddd155111515000111111666666666666666600000000000000000000000000000000
551111111155555555555555dd555516666666666666666666d66ddddddddd111551155500111111666666666666666600000000000000000000000000000000
555555555155555ddddddddddd55551ddd66666666666666666666dd6ddddd55d55dd5d555555555666666666666666600000000000000000000000000000000
555555555155555ddddddddddd55551ddd66666666666666666666dd6ddddd55d55dd5d555555555666666666666666600000000000000000000000000000000
511555111111155555555555dd55555d5566666666666666666666d5dd5ddddddd55d55555555555666666666666666600000000000000000000000000000000
1115551111111555dddddddddd56666ddd6666666666666666666ddd55dddd555555111111155151666666666666666600000000000000000000000000000000
1111151111111551ddddddddddddddd5556666666666666666666dd5d5dddd111111111111111115666666666666666600000000000000000000000000000000
111515111111155dddddddddddd6ddd5dd6666666666666666666dd5d5dddd111111111111011111666666666666666600000000000000000000000000000000
111111111111151dddddddd5dddddd5d666666666666666666666d5dd5dddd111111511111115111666666666666666600000000c000000000000000c0000000
11155511111155ddddddddd1ddd66665666ddd666666666666666d5655dddd111111511155011111666d6d66666666660000000000000000000000000c000000
5115551111555555511115d5dd666666666666666666666666666666666ddd111111551555155111666666666666666600000000000000000000000000000000
51d5555555d5dddddddddddddd666666666666666666666666666666666ddddddddddddddddd5d556666666666666666000000000000000000000000000c0000
d1ddddddddd66666666666666666666666666666666666666666666666666dddd6d6666d6666dddd6666666666666666000000000000c000000000000000c000
d1ddddddddd66666666666666666666666666666666666666666666666666666666666666666dddd666666666666666600000000000000000000000000000000
d1ddddddd6d6666666666666666666666666666666666666666666666666666666666666666ddddd666666666666666600000000000000000000000000000c00
dddddddddddd666666666666666666666666666666666666666666666666666666666666666ddddd66666666666666660000000000000c000000000000000000
dddddddddddd666666666666666666666666666666666666666666666666666666666666666ddddd666666666666666600000000000000000000000000000c00
dddddddddddd6666666666d666666666666666666666666666666d66666666666666666666dddddd66666666666666660000000000000c000000000000000c00
dddddddddddd66666666666666666666666666666666666666666d666666666666666666dddddddd666666666666666600000000000000000000000000000000
dd666dddddd6666666666666666666666666666666666666666666666666666666666666666ddddd666666666666666600000000000000000000000000000000
dddddd66dd66d66666666666666666666666666666666666666666666666666666666666666ddddd66666666666666660000000000000c000000000000000c00
dddd666666d6d66666666666666666666666666666666666666666666666666666666666dddddddd6666666666666666000000000000c0c0000000000000ccc0
ddd6dd66ddd6dd6666666666666666666666666666666666666666666666666d66666666dddddddd66666666666666660000000000000c000000000000000c00
ddd6ddffddd6dd66666666666666666666666666666666666666666666666666666666d6dddddddd666666666666666600000000000000000000000000000000
ddddddddddd6dd66666666666666666666666666666d666666666666666666666666666ddddddddd666666666666666600000000b000000000000000b0000000
ddd66f77ff6ddd666666666666666666666666666666666666666666666666666666666ddddddddd66666666666666660000000b000000000000000bbb000000
ddd66fff666d66666d6dd6d6666666666666666666666666666666666666666666666666dddddddd6666666666666666000000b000b00000000000b000b00000
ddd66fff666d66666d6dd6d6666666666666666666666666666666666666666666666666dddddddd666666666666666600000b00000b000000000b00000b0000
ddd66666666dddd66666666666666666666666666666666666666666666666666666666d5555555566666666666666660000000000000000000000000000b000
5556666666655656d5555556666666666666666666666666666666666666666655555555555555556666666666666666000b000000000b00000b000000000b00
555d6666666556555555555666666666666666666666666666666666666666d55155515555515555666666666666666600000000000000000000000000000000
5556666666655f55d555555666666666666666666666666666666666666666dd555555555555155566666666666666660b000000000000000b000000000000b0
5555d6666dd65555d555555d66666666666666666666666666666666666666d55551555155555551666666666666666600000000000000000000000000000bbb
555d55555566f6f6d555555ddddddddddddddddd666ddd66d6d6d666666666d55555555555555515dddddddd666ddd66000000000000000000000000000000b0
5555555555d666666555555dddddddddddddddddddddddddddddddddddddddd55555515555515555dddddddddddddddd00000000000000000000000000000000
55555555555d6666d555555dddddddddddddddddddddddddddddddddddddddd55555555511551555dddddddddddddddd00000000000000000000000000000000
55555555555d56dd555555ddddddddddddddddddddddddddddddddddddddddd55555555555555515dddddddddddddddd00000000000000000000000000000000
5555555555555555555555ddddddddddddddddddddddddddddddddddddddddd55555555155555555dddddddddddddddd00000000000000000000000000000000
5555555555555555555555ddddddddddddddddddddddddddddddddddddddddd55555555555555551dddddddddddddddd00000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000000000a000000000000a0
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a00000000000000aaa
000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000a00000a0000000000a0
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a0000000a000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a00000a000000000a00000a0000
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a00000000000a000a00000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a00000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a000000000000000a0000000
__map__
60616263646a6b6a6b6a6a676879786967686900000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
70717273747a7b7a7b7a7a77787979797778796e000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
8081828384858a8b8a8b8a67687879797d7e7d7e000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
908493949596979b9a9b9a77787979796d6e6d6e000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
908ba3a4a5a6a79596978687888888897d7e7d7e000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
90919a8a8ba3a4a5a6a78b8a8b8a979899000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
909393949596979a9b9a93949596979899000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
9184a3a4a5a6a7959697a3a4a5a6979899000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
90919394959697a5a6a79b9a9b9a979899000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
9093a3a4a5a6a7a5a6a79394959697a500000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
9084939495969797979394959697a79899000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
9091a3a4a5a6a7979394959697a7a7a8a9000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
a0a1a2a3a4a5a6a7a3a4a5a6a7a7a8a900000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
b0b1b2b3b4b5b3b4b5b6b3b4b5b7b8b900000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
__sfx__
000200002d0502e0502f0503005030050000000000000000000002910028100271002710027100281000000000000000000000023200242002520026200262000000000000000000000000000000000000000000
00020000314502a450254502145020450261003310000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0002000023350293502f35029350233501c3001c30000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00020000272501f250192501f25027250000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
311800002b5550c5050c5052e555325550c5050c5050c5050c5050c5050c50532555315550c505325550c505335550c505325550c505315550c505325550c5052e5550c5050c5050c5050c5050c5050c5050c505
011800001f0411f0421f0422204126041260422604226042260322603226032260322602226022260222602224041240422404224042240322403224022240222204122042220422204222032220322202222022
011800000e4350e435003050e4350e435003050e435003050e4350e435003050e4350e435003050e435003050c4350c435003050c4350c435003050c435003050a4350a435003050a4350a435003050a43500305
011800000707007070000000707007070000000707000000070700707000000070700707000000070700000006070060700000006070060700000006070000000707007070000000707007070000000707000000
311800002b5550c5050c5052e555325550c5050c5050c5050c5050c5050c50532555315550c505325550c505335550c505325550c505315550c505325550c505305550c5050c5050c5050c5050c5050c5050c505
011800001f0411f0421f0422204126041260422604226042260322603226032260322602226022260222602227041270422704227042270322703227022270222404124042240422404224032240322402224022
011800000e4350e435000000e4350e435000000e435000000e4350e435000000e4350e435000000e435000000f4350f435000000f4350f435000000f435000000c4350c435000000c4350c435000000c43500000
01180000070700707000000070700707000000070700000007070070700000007070070700000007070000000f0700f070000000f0700f070000000f070000000607006070000000607006070000000607000000
3118000029555000002c5550000030555000002c5550000028555000002b5550000030555000002b5550000029555000002c5550000030555000002c555000002955500000000000000000000000000000000000
011800001d5401d5401d5401d5401d5401d5401d5401d5401f5401f5401f5401f5401c5401c5401c5401c54020540205402054020540205402054020540205402054020540205402054020540205402054020540
011800000000018335183350c00018335183350c000183350c00018335183350c00018335183350c000183350c00018335183350c00018335183350c000183350c00018335183350c00018335183350c00018335
011800000507005070000000507005070000000507000000040700407000000040700407000000040700000005070050700000005070050700000005070000000507005070000000507005070000000507000000
3118000029555000002c5550000030555000002c5550000028555000002b5550000030555000002b5550000029555000002c5550000030555000002c555000002a55500000000000000000000000000000000000
011800001854018540185401854018540185401854018540195401954019540195401754017540175401754018540185401854018540185401854018540185401a5401a5401a5401a5401a5401a5401a5401a540
011800001433514335000001433514335000001433500000133351333500000133351333500000133350000014335143350000014335143350000014335000001533515335000001533515335000001533500000
0118000005050050500000005050050500000005050050500c0500c050000000c0500c050000000c0500c05005050050500000005050050500000005050050500605006050000000605006050000000605006050
__music__
00 30313233
00 34353637
00 38393a3b
02 3c3d3e3f

