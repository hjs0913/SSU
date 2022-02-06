my_id = 99999;
my_lv = 15;
my_name = "타락한 닭";
my_hp = 120000;
my_physical_attack = 180;
my_magical_attck = 0;
my_physical_defence = 300;
my_magical_defence = 130;
my_basic_attack_factor = 10;
my_defence_factor = 0.0002;
my_x = 0;
my_y = 0;
my_z = 0;


function set_uid(id, x, y, z)
   my_id = id;
   my_x = x;
   my_y = y;
   my_z = z;
   return my_lv, my_name, my_hp, my_physical_attack, my_magical_attck, 
        my_phsical_defence, my_magical_defence, my_basic_attack_factor, my_defence_factor;
end

function event_npc_move(player)
   player_x = API_get_x(player);
   player_z = API_get_z(player);
   x = API_get_x(my_id);
   z = API_get_z(my_id);
   if (math.abs(my_x - x) > 120) or (math.abs(my_z - z) > 120) then
      return false;
   else
      -- 쫒아가는 것은 C로 짠다
      return true;
   end
end

function return_my_position()
   return my_x, my_y, my_z;
end

function monster_revive()
    return my_x, my_y, my_z, my_hp;
end

function attack_range(player)
   player_x = API_get_x(player);
   player_y = API_get_y(player);
   x = API_get_x(my_id);
   y = API_get_y(my_id);
   if (x == player_x) then
      if (player_y <= (y+1)) then
         if((y-1) <= player_y) then
            return true;
         else
            return false;
         end
      else 
         return false;
      end
   elseif (y == player_y) then
      if (player_x <= (x+1)) then
         if((x-1) <= player_x) then
            return true;
         else
            return false;
         end
      else 
         return false;
      end
   end
end