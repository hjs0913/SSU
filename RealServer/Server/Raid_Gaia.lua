my_id = 99999;
my_element = 0;
my_lv = 100;    //파티원 평균으로 하자 
my_name = "파트너";
my_hp = 5100000;
my_physical_attack = 350;
my_magical_attck = 400;
my_physical_defence = 500;
my_magical_defence = 350;
my_basic_attack_factor = 50;
my_defence_factor = 0.0018;
my_x = 150;
my_y = 0;
my_z = 150;
my_job = 0;  //변경가능하도록 

function set_uid(id)
   my_id = id;
   return my_element,my_lv, my_name, my_hp, my_physical_attack, my_magical_attck, 
        my_physical_defence, my_magical_defence, my_basic_attack_factor, my_defence_factor;
end