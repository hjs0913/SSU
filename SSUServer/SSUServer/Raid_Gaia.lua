my_id = 99999;
my_element = 0;
my_lv = 100;
my_name = "가이아";
--my_hp = 5100000;
my_hp = 1000000;
my_physical_attack = 350;
my_magical_attck = 400;
my_physical_defence = 500;
my_magical_defence = 350;
my_basic_attack_factor = 50;
my_defence_factor = 0.0018;
my_x = 300;
my_y = 0;
my_z = 300;

p1_first_position_x = 0;
p1_first_position_z = 0;
p1_second_position_x = 0;
p1_second_position_z = 0;
p1_third_position_x = 0;
p1_third_position_z = 0;
p1_fourth_position_x = 0;
p1_fourth_position_z = 0;

function set_uid(id)
   my_id = id;
   return my_element, my_lv, my_name, my_hp, my_physical_attack, my_magical_attck, 
        my_physical_defence, my_magical_defence, my_basic_attack_factor, my_defence_factor;
end

function pattern_one_set_position(x, z, look_x, look_z, right_x, right_z)
-- 장판의 위치정하기
    p1_first_position_x = x + look_x*50;
    p1_first_position_z = z + look_z*50;
    p1_second_position_x = x - look_x*50;
    p1_second_position_z = z - look_z*50;
    p1_third_position_x = x + right_x*50;
    p1_third_position_z = z + right_z*50;
    p1_fourth_position_x = x - right_x*50;
    p1_fourth_position_z = z - right_z*50;

    return p1_first_position_x, p1_first_position_z, p1_second_position_x, p1_second_position_z,
       p1_third_position_x, p1_third_position_z, p1_fourth_position_x, p1_fourth_position_z;
-- 5초후 터트리기
-- 장판위에 플레이어가 있는지 확인 및 데미지 처리
end

function pattern_one_set_damage(player)
    player_x = API_get_x(player);
    player_z = API_get_z(player);

    dis1 = math.sqrt(math.pow((player_x - p1_first_position_x), 2) + math.pow((player_z - p1_first_position_z), 2) );
    dis2 = math.sqrt(math.pow((player_x - p1_second_position_x), 2) + math.pow((player_z - p1_second_position_z), 2) );
    dis3 = math.sqrt(math.pow((player_x - p1_third_position_x), 2) + math.pow((player_z - p1_third_position_z), 2) );
    dis4 = math.sqrt(math.pow((player_x - p1_fourth_position_x), 2) + math.pow((player_z - p1_fourth_position_z), 2));

    p_hp = API_get_hp(player);

    if((dis1 < 20) or (dis2 < 20) or (dis3 < 20) or (dis4 < 20)) then
        return p_hp - 2000;
    else 
        return p_hp;
    end
end

