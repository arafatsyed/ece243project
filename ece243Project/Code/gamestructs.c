
struct Basketball{
	int dy,dx, x, y, prevX, prevY;
};
struct Net{
	int x, y, prevX, prevY;
};

struct Player{
	int x, y, prevX,prevY, playerID;
	
};

struct PowerBar{
	int x,y, prevX, prevY;
	int powerArray[10];
};

struct AimBar{
	int x,y, prevX, prevY;
	int aimBar[10];
}

struct Game{
	struct Basketball basketball;
	struct Net net;
	struct Player player;
	struct PowerBar powerBar;
	struct AimBar aimBar;
	const uint16_t background[240][320];
};

