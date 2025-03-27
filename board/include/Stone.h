#ifndef STONE_H
#define STONE_H

class Stone
{
    private:
        int x;
        int y;
        // 'B' = black, 'W' = white
        char player;
    public:
        Stone(int x, int y, char player);
        ~Stone();
        int getX();
        int getY();
        char getPlayer();
};


#endif // STONE_H