my_id = 99999;
my_element = 3;
my_lv = 15;
my_name = "Ÿ���� ��";
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

-- �ʱ� ��ġ ������ ���
min_x = 2660;
max_x = 2960;
min_z = 1990;
max_z = 2290;


function set_uid(id, x, y, z)
   my_id = id;
   my_x = x;
   my_y = y;
   my_z = z;
   return my_element,my_lv, my_name, my_hp, my_physical_attack, my_magical_attck, 
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
   if (math.abs(my_x - x) > 120) or (math.abs(my_z - z) > 120) then
      return false;
   else
      -- �i�ư��� ���� C�� §��
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
   if (player_z <= (z+10)) then
         if((z-10) <= player_z) then
             if (player_x <= (x+10)) then
                if((x-10) <= player_x) then
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