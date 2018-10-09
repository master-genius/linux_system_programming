#ifndef MASTER_TANK_H
#define MASTER_TANK_H


#define TANK_TY_TANK    't'
#define TANK_TY_BOMB    'b'
#define TANK_TY_PRIZE   'p'
#define TANK_TY_ENEMY   'e'
#define TANK_TY_SHIEL   's'
#define TANK_TY_ROAD    'r'

#define BOMB_A      '.'
#define BOMB_B      'o'
#define BOMB_C      '*'

#define BOMB_A_POWER    16
#define BOMB_B_POWER    32
#define BOMB_C_POWER    64

#define TANK_INIT_POWER 1024



struct TRoot {
    char type;
    char name[256];
    char idstr[32];
};

struct Armour {
    struct TRoot *tr;
    
    char *body[BODY_LINE];
    int body_end;

    int min_top;
    int max_right;

    char weapon;
    int weapon_power;

    int speed;

    int power;
}

#define BODY_LINE   16

struct Tank {
    struct Armour * ar;
    

};

struct Aircraft {
    struct Armour *ar;

}




#endif

