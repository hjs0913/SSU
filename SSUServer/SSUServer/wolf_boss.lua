my_id = 99999;
my_element = 7;
my_lv = 26;
my_name = "타락한 늑대";
my_hp = 165000;
my_physical_attack = 250;
my_magical_attck = 0;
my_physical_defence = 430;
my_magical_defence = 270;
my_basic_attack_factor = 10;
my_defence_factor = 0.0002;
my_x = 0;
my_y = 0;
my_z = 0;

-- 초기 위치 잡을때 사용
min_x = 3125;
max_x = 3425;
min_z = 3210;
max_z = 3510;


function set_uid(id, x, y, z)
   my_id = id;
   my_x = x;
   my_y = y;
   my_z = z;
   return  my_element,my_lv, my_name, my_hp, my_physical_attack, my_magical_attck, 
        my_physical_defence, my_magical_defence, my_basic_attack_factor, my_defence_factor;
end

function Initailize_position()
	return min_x, max_x, min_z, max_z;
end

function event_npc_move(player)
   player_x = API_get_x(player);
   player_z = API_get_z(player);
   x = API_get_x(my_id);
   z = API_get_z(my_id);
   if (math.abs(my_x - x) > 200) or (math.abs(my_z - z) > 200) then
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
   player_z = API_get_z(player);
   x = API_get_x(my_id);
   z = API_get_z(my_id);
   if (player_z <= (z+15)) then
         if((z-15) <= player_z) then
             if (player_x <= (x+15)) then
                if((x-15) <= player_x) then
                    return true;
                else
                    return false;
                end
             else
                return false;
             end
         else 
             return false;
         end
   else
         return false;
   end
end