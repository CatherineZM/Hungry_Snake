#include "main.c"

const int ROWUNIT = 1;
const int COLUMNUNIT = 1;
const int HEIGHT = 24;
const int WIDTH = 24;
const int INF = 10000;
const int SNAKEBODY = 20000;
const int WALL = 30000;

typedef struct node {
	int x;
	int y;
}SnakeNode;

SnakeNode NextStep;
SnakeNode theTail;

int NextDirection;

snake* VirtualSnake = NULL;


int Direction[4][2] = { { 0, ROWUNIT },{ 0, -ROWUNIT },
						{ COLUMNUNIT, 0 },{ -COLUMNUNIT, 0 } };

int bfsDistance[WIDTH + 2 * COLUMNUNIT][HEIGHT + 2 * ROWUNIT];
int bfsJudgeVirtualTail[WIDTH + 2 * COLUMNUNIT][HEIGHT + 2 * ROWUNIT];
int bfsJudgeRealTail[WIDTH + 2 * COLUMNUNIT][HEIGHT + 2 * ROWUNIT];
int dfsDistance[WIDTH + 2 * COLUMNUNIT][HEIGHT + 2 * ROWUNIT];
int Mark[WIDTH + 2 * COLUMNUNIT][HEIGHT + 2 * ROWUNIT];
int currentDepth, deepest;


bool isHeadequalTail();
bool isOpposite(int a, int b);
void getFoodDistance();
bool isReachable();
bool isVirtualTailReachable_Regular();
bool isVirtualTailReachable_Random();
int eatFoodMove();
bool eatFood(int * direc);
bool isSafe(SnakeNode NewHead);
int followTailMove();
int FoodDistance(SnakeNode adj);
bool followTail(int * direc);
void DFS(int x, int y);
int getDepth(SnakeNode temp);
void snakeWander(int * direc);
//determine the next move
int snakeMove();



bool isHeadequalTail() {
    snake headn = snakeFront(head);
    snake tailn = snakeBack(head);
    if (headn.x == tailn.x && headn.y == tailn.y) {
        return true;
	}
    return false;
}

bool isOpposite(int a, int b) {
    if (0 == a && 1 == b) {
		return true;
	}
	else if (1 == a && 0 == b) {
		return true;
	}
	else if (2 == a && 3 == b) {
		return true;
	}
	else if (3 == a && 2 == b) {
		return true;
	}
	else {
		return false;
	}
}

void getFoodDistance() {
    for (int x = 1; x <= WIDTH; x += COLUMNUNIT) {
		for (int y = 1; y <= HEIGHT; y += ROWUNIT) {
			bfsDistance[x][y] = INF;														
		}
	}
    snake *p = head;
    while(1) {
        if(p == NULL) break;
        bfsDistance[p->x + 1][p->y + 1] = SNAKEBODY;
        p = p->next;
    }
    for (int x = 0; x <= WIDTH + COLUMNUNIT; x += COLUMNUNIT) {
		bfsDistance[x][0] = bfsDistance[x][HEIGHT + ROWUNIT] = WALL;
	}
    for (int y = 0; y <= HEIGHT + ROWUNIT; y += ROWUNIT) {
		bfsDistance[0][y] = bfsDistance[WIDTH + COLUMNUNIT][y] = WALL;
	}
    bfsDistance[food.x + 1][food.y + 1] = 0;

    snake * point = NULL;
    point = snakePush_Back(point, food.x + 1, food.y + 1);
    SnakeNode temp, adj;
    while(snakeSize(point) != 0) {
        temp.x = snakeFront(point).x;
        temp.y = snakeFront(point).y;
        point = snakePop_Front(point);
        for (int i = 0; i < 4; i++) {
			adj.x = temp.x + Direction[i][0];
			adj.y = temp.y + Direction[i][1];
			if (INF == bfsDistance[adj.x][adj.y]) {											
				bfsDistance[adj.x][adj.y] = bfsDistance[temp.x][temp.y] + 1;
				point = snakePush_Back(point, adj.x, adj.y);
			}
		}
    }
    point = snakeClear(point);
}

bool isReachable() {
    SnakeNode temp;
    SnakeNode Headn;
    Headn.x = snakeFront(head).x + 1;
    Headn.y = snakeFront(head).y + 1;
    for (int i = 0; i < 4; i++) {
		if (isOpposite(i, SnakeDirection)) {									
			continue;
		}
		temp.x = Headn.x + Direction[i][0];
		temp.y = Headn.y + Direction[i][1];
		if (bfsDistance[temp.x][temp.y] < INF) {								
			return true;
		}
	}
	return false;
}

bool isVirtualTailReachable_Regular() {
    int tempDirection = -1, virtualDirection = SnakeDirection;
    VirtualSnake = snakeClear(VirtualSnake);
    VirtualSnake = snakeCopy(VirtualSnake, head);
    SnakeNode MoveHead;
    MoveHead.x = snakeFront(head).x + 1;
    MoveHead.y = snakeFront(head).y + 1;
    SnakeNode adj;																	
	SnakeNode Result;
    while (MoveHead.x != food.x + 1 || MoveHead.y != food.y + 1) {
        int MinDistance = INF;
        for (int i = 0; i < 4; i++) {
			if (isOpposite(i, virtualDirection)) {
				continue;
			}
			adj.x = MoveHead.x + Direction[i][0];
			adj.y = MoveHead.y + Direction[i][1];
			if (bfsDistance[adj.x][adj.y] < MinDistance) {
				MinDistance = bfsDistance[adj.x][adj.y];
				Result.x = adj.x;
				Result.y = adj.y;
				tempDirection = i;
			}
		}
        VirtualSnake = snakePush_Front(VirtualSnake, Result.x - 1, Result.y - 1);
        if (Result.x != food.x + 1|| Result.y != food.y + 1) {							
			VirtualSnake = snakePop_Back(VirtualSnake);											
		}
        virtualDirection = tempDirection;
        if (MoveHead.x == snakeFront(head).x + 1 && MoveHead.y == snakeFront(head).y + 1) {	
			NextStep.x = Result.x;
			NextStep.y = Result.y;
			NextDirection = virtualDirection;
		}
        MoveHead.x = snakeFront(VirtualSnake).x + 1;
        MoveHead.y = snakeFront(VirtualSnake).y + 1;	
    }

    for (int x = 1; x <= WIDTH; x += COLUMNUNIT) {
		for (int y = 1; y <= HEIGHT; y += ROWUNIT) {
			bfsJudgeVirtualTail[x][y] = INF;
		}
	}
    snake *p = VirtualSnake;
    while(1) {
        if(p == NULL) break;
        bfsJudgeVirtualTail[p->x + 1][p->y + 1] = SNAKEBODY;
        p = p->next;
    }
    for (int x = 0; x <= WIDTH + COLUMNUNIT; x += COLUMNUNIT) {
		bfsJudgeVirtualTail[x][0] = bfsJudgeVirtualTail[x][HEIGHT + ROWUNIT] = WALL;
	}
    for (int y = 0; y <= HEIGHT + ROWUNIT; y += ROWUNIT) {
		bfsJudgeVirtualTail[0][y] = bfsJudgeVirtualTail[WIDTH + COLUMNUNIT][y] = WALL;
	}
    SnakeNode VirtualSnakeTail;
    VirtualSnakeTail.x = snakeBack(VirtualSnake).x + 1;
    VirtualSnakeTail.y = snakeBack(VirtualSnake).y + 1;
    bfsJudgeVirtualTail[VirtualSnakeTail.x][VirtualSnakeTail.y] = 0;

    snake * point = NULL;
    point = snakePush_Back(point, VirtualSnakeTail.x, VirtualSnakeTail.y);
    SnakeNode temp;
    while(snakeSize(point) != 0) {
        temp.x = snakeFront(point).x;
        temp.y = snakeFront(point).y;
        point = snakePop_Front(point);
        for (int i = 0; i < 4; i++) {
			adj.x = temp.x + Direction[i][0];
			adj.y = temp.y + Direction[i][1];
			if (INF == bfsJudgeVirtualTail[adj.x][adj.y]) {											
				bfsJudgeVirtualTail[adj.x][adj.y] = bfsJudgeVirtualTail[temp.x][temp.y] + 1;
				point = snakePush_Back(point, adj.x, adj.y);
			}
		}
    }
    point = snakeClear(point);
    bfsJudgeVirtualTail[VirtualSnakeTail.x][VirtualSnakeTail.y] = SNAKEBODY;
    int TailDistance = INF;
    SnakeNode VirtualSnakeHead;
    VirtualSnakeHead.x = snakeFront(VirtualSnake).x + 1;
    VirtualSnakeHead.y = snakeFront(VirtualSnake).y + 1;
    for (int i = 0; i < 4; i++) {
		adj.x = VirtualSnakeHead.x + Direction[i][0];
		adj.y = VirtualSnakeHead.y + Direction[i][1];
		if (bfsJudgeVirtualTail[adj.x][adj.y] < INF) {
			return true;
		}
	}
	return false;
}
bool isVirtualTailReachable_Random() {
    int tempDirection = -1, virtualDirection = SnakeDirection;
    VirtualSnake = snakeClear(VirtualSnake);
    VirtualSnake = snakeCopy(VirtualSnake, head);
    SnakeNode MoveHead;
    MoveHead.x = snakeFront(head).x + 1;
    MoveHead.y = snakeFront(head).y + 1;
    SnakeNode adj;																	
	SnakeNode Result;
    while (MoveHead.x != food.x + 1 || MoveHead.y != food.y + 1) {
        int MinDistance = INF;
        int random_i = rand() % 4;
        for (int t = 0; t < 4; random_i = (random_i + 1) % 4,t++) {
			if (isOpposite(random_i, virtualDirection)) {
				continue;
			}
			adj.x = MoveHead.x + Direction[random_i][0];
			adj.y = MoveHead.y + Direction[random_i][1];
			if (bfsDistance[adj.x][adj.y] < MinDistance) {
				MinDistance = bfsDistance[adj.x][adj.y];
				Result.x = adj.x;
				Result.y = adj.y;
				tempDirection = random_i;
			}
		}
        VirtualSnake = snakePush_Front(VirtualSnake, Result.x - 1, Result.y - 1);
        if (Result.x != food.x + 1|| Result.y != food.y + 1) {							
			VirtualSnake = snakePop_Back(VirtualSnake);											
		}
        virtualDirection = tempDirection;
        if (MoveHead.x == snakeFront(head).x + 1 && MoveHead.y == snakeFront(head).y + 1) {	
			NextStep.x = Result.x;
			NextStep.y = Result.y;
			NextDirection = virtualDirection;
		}
        MoveHead.x = snakeFront(VirtualSnake).x + 1;
        MoveHead.y = snakeFront(VirtualSnake).y + 1;	
    }

    for (int x = 1; x <= WIDTH; x += COLUMNUNIT) {
		for (int y = 1; y <= HEIGHT; y += ROWUNIT) {
			bfsJudgeVirtualTail[x][y] = INF;
		}
	}
    snake *p = VirtualSnake;
    while(1) {
        if(p == NULL) break;
        bfsJudgeVirtualTail[p->x + 1][p->y + 1] = SNAKEBODY;
        p = p->next;
    }
    for (int x = 0; x <= WIDTH + COLUMNUNIT; x += COLUMNUNIT) {
		bfsJudgeVirtualTail[x][0] = bfsJudgeVirtualTail[x][HEIGHT + ROWUNIT] = WALL;
	}
    for (int y = 0; y <= HEIGHT + ROWUNIT; y += ROWUNIT) {
		bfsJudgeVirtualTail[0][y] = bfsJudgeVirtualTail[WIDTH + COLUMNUNIT][y] = WALL;
	}
    SnakeNode VirtualSnakeTail;
    VirtualSnakeTail.x = snakeBack(VirtualSnake).x + 1;
    VirtualSnakeTail.y = snakeBack(VirtualSnake).y + 1;
    bfsJudgeVirtualTail[VirtualSnakeTail.x][VirtualSnakeTail.y] = 0;

    snake * point = NULL;
    point = snakePush_Back(point, VirtualSnakeTail.x, VirtualSnakeTail.y);
    SnakeNode temp;
    while(snakeSize(point) != 0) {
        temp.x = snakeFront(point).x;
        temp.y = snakeFront(point).y;
        point = snakePop_Front(point);
        for (int i = 0; i < 4; i++) {
			adj.x = temp.x + Direction[i][0];
			adj.y = temp.y + Direction[i][1];
			if (INF == bfsJudgeVirtualTail[adj.x][adj.y]) {											
				bfsJudgeVirtualTail[adj.x][adj.y] = bfsJudgeVirtualTail[temp.x][temp.y] + 1;
				point = snakePush_Back(point, adj.x, adj.y);
			}
		}
    }
    point = snakeClear(point);
    bfsJudgeVirtualTail[VirtualSnakeTail.x][VirtualSnakeTail.y] = SNAKEBODY;
    int TailDistance = INF;
    SnakeNode VirtualSnakeHead;
    VirtualSnakeHead.x = snakeFront(VirtualSnake).x + 1;
    VirtualSnakeHead.y = snakeFront(VirtualSnake).y + 1;
    for (int i = 0; i < 4; i++) {
		adj.x = VirtualSnakeHead.x + Direction[i][0];
		adj.y = VirtualSnakeHead.y + Direction[i][1];
		if (bfsJudgeVirtualTail[adj.x][adj.y] < INF) {
			return true;
		}
	}
	return false;
}
int eatFoodMove() {
    return NextDirection;
}
bool eatFood(int * direc) {
    getFoodDistance();
    if (isReachable()) {
		if (isVirtualTailReachable_Regular() || isVirtualTailReachable_Random()) {
			*direc = eatFoodMove();
			return true;
		} else {
			return false;
		}
	}
	return false;
}
bool isSafe(SnakeNode NewHead) {

    VirtualSnake = snakeClear(VirtualSnake);
    VirtualSnake = snakeCopy(VirtualSnake, head);

    VirtualSnake = snakePush_Front(VirtualSnake, NewHead.x - 1, NewHead.y - 1);
    VirtualSnake = snakePop_Back(VirtualSnake);

    for (int x = 1; x <= WIDTH; x += COLUMNUNIT) {
		for (int y = 1; y <= HEIGHT; y += ROWUNIT) {
			bfsJudgeVirtualTail[x][y] = INF;
		}
	}
    snake *p = VirtualSnake;
    while(1) {
        if(p == NULL) break;
        bfsJudgeVirtualTail[p->x + 1][p->y + 1] = SNAKEBODY;
        p = p->next;
    }
    for (int x = 0; x <= WIDTH + COLUMNUNIT; x += COLUMNUNIT) {
		bfsJudgeVirtualTail[x][0] = bfsJudgeVirtualTail[x][HEIGHT + ROWUNIT] = WALL;
	}
    for (int y = 0; y <= HEIGHT + ROWUNIT; y += ROWUNIT) {
		bfsJudgeVirtualTail[0][y] = bfsJudgeVirtualTail[WIDTH + COLUMNUNIT][y] = WALL;
	}
    SnakeNode VirtualSnakeTail;
    VirtualSnakeTail.x = snakeBack(VirtualSnake).x + 1;
    VirtualSnakeTail.y = snakeBack(VirtualSnake).y + 1;
    bfsJudgeVirtualTail[VirtualSnakeTail.x][VirtualSnakeTail.y] = 0;

    snake * point = NULL;
    point = snakePush_Back(point, VirtualSnakeTail.x, VirtualSnakeTail.y);
    SnakeNode temp, adj;
    while(snakeSize(point) != 0) {
        temp.x = snakeFront(point).x;
        temp.y = snakeFront(point).y;
        point = snakePop_Front(point);
        for (int i = 0; i < 4; i++) {
			adj.x = temp.x + Direction[i][0];
			adj.y = temp.y + Direction[i][1];
			if (INF == bfsJudgeVirtualTail[adj.x][adj.y]) {											
				bfsJudgeVirtualTail[adj.x][adj.y] = bfsJudgeVirtualTail[temp.x][temp.y] + 1;
				point = snakePush_Back(point, adj.x, adj.y);
			}
		}
    }
    point = snakeClear(point);
    bfsJudgeVirtualTail[VirtualSnakeTail.x][VirtualSnakeTail.y] = SNAKEBODY;
    int TailDistance = INF;
    SnakeNode VirtualSnakeHead;
    VirtualSnakeHead.x = snakeFront(VirtualSnake).x + 1;
    VirtualSnakeHead.y = snakeFront(VirtualSnake).y + 1;
    for (int i = 0; i < 4; i++) {
		adj.x = VirtualSnakeHead.x + Direction[i][0];
		adj.y = VirtualSnakeHead.y + Direction[i][1];
		if (bfsJudgeVirtualTail[adj.x][adj.y] < INF) {
			return true;
		}
	}
	return false;
}

int followTailMove() {
    return NextDirection;
}

int FoodDistance(SnakeNode adj) {
    if (bfsDistance[adj.x][adj.y] < INF) {										
		return bfsDistance[adj.x][adj.y];								
	} else {
		return abs(adj.x - (food.x + 1)) + abs(adj.y - (food.y + 1));						
	}
}

bool followTail(int * direc) {
    for (int x = 1; x <= WIDTH; x += COLUMNUNIT) {
		for (int y = 1; y <= HEIGHT; y += ROWUNIT) {
			bfsJudgeRealTail[x][y] = INF;
		}
	}
    snake *p = head;
    while(1) {
        if(p == NULL) break;
        bfsJudgeRealTail[p->x + 1][p->y + 1] = SNAKEBODY;
        p = p->next;
    }
    for (int x = 0; x <= WIDTH + COLUMNUNIT; x += COLUMNUNIT) {
		bfsJudgeRealTail[x][0] = bfsJudgeRealTail[x][HEIGHT + ROWUNIT] = WALL;
	}
	for (int y = 0; y <= HEIGHT + ROWUNIT; y += ROWUNIT) {
		bfsJudgeRealTail[0][y] = bfsJudgeRealTail[WIDTH + COLUMNUNIT][y] = WALL;
	}
    SnakeNode RealSnakeTail;
    RealSnakeTail.x = snakeBack(head).x + 1;
    RealSnakeTail.y = snakeBack(head).y + 1;
    bfsJudgeRealTail[RealSnakeTail.x][RealSnakeTail.y] = 0;
    snake * point = NULL;
    point = snakePush_Back(point, RealSnakeTail.x, RealSnakeTail.y);
    SnakeNode temp, adj;
    while(snakeSize(point) != 0) {
        temp.x = snakeFront(point).x;
        temp.y = snakeFront(point).y;
        point = snakePop_Front(point);
        for (int i = 0; i < 4; i++) {
			adj.x = temp.x + Direction[i][0];
			adj.y = temp.y + Direction[i][1];
			if (INF == bfsJudgeRealTail[adj.x][adj.y]) {											
				bfsJudgeRealTail[adj.x][adj.y] = bfsJudgeRealTail[temp.x][temp.y] + 1;
				point = snakePush_Back(point, adj.x, adj.y);
			}
		}
    }
    point = snakeClear(point);
    bfsJudgeRealTail[RealSnakeTail.x][RealSnakeTail.y] = SNAKEBODY;

    SnakeNode tempAdj;
    SnakeNode tempHead;
    tempHead.x = snakeFront(head).x + 1;
    tempHead.y = snakeFront(head).y + 1;
    bool tempReachable = false;
    for (int i = 0; i < 4; i++) {
		tempAdj.x = tempHead.x + Direction[i][0];
		tempAdj.y = tempHead.y + Direction[i][1];
		if (bfsJudgeRealTail[tempAdj.x][tempAdj.y] < INF) {
			tempReachable = true;
			break;
		}
	}

    int SafeDirection[4];
	int count = 0;
    SnakeNode RealHead;
    RealHead.x = snakeFront(head).x + 1;
    RealHead.y = snakeFront(head).y + 1;

    for (int i = 0; i < 4; i++) {											
		if (isOpposite(i, SnakeDirection)) {
			continue;
		}
		adj.x = RealHead.x + Direction[i][0];
		adj.y = RealHead.y + Direction[i][1];
		if (bfsJudgeRealTail[adj.x][adj.y] < INF) {
			if (isSafe(adj)) {
				SafeDirection[count] = i;
				count++;
			}
		}
	}

    int MaxDistance = -1;
    SnakeNode Result;
    if (0 == count) {														
		return false;
	}
    else {
		for (int k = 0; k < count; k++) {									
			adj.x = RealHead.x + Direction[SafeDirection[k]][0];
			adj.y = RealHead.y + Direction[SafeDirection[k]][1];
			int tempDistance = FoodDistance(adj);
			if (tempDistance > MaxDistance) {
				MaxDistance = tempDistance;
				Result.x = adj.x;
				Result.y = adj.y;
				NextDirection = SafeDirection[k];
			}
		}
		NextStep.x = Result.x;
		NextStep.y = Result.y;
		*direc = followTailMove();													
		return true;
	}
	return false;
}

void DFS(int x, int y) {
    if (WALL == dfsDistance[x][y]) {
		return;
	}
	dfsDistance[x][y] = WALL;											
	if (currentDepth > deepest) {
		deepest = currentDepth;
	}
	currentDepth++;
	SnakeNode Temp;
	for (int i = 0; i < 4; i++) {
		Temp.x = x + Direction[i][0];
		Temp.y = y + Direction[i][1];
		if (Temp.x < 1 || Temp.x > WIDTH || Temp.y < 1 || Temp.y > HEIGHT) {
			continue;
		}
		if (WALL == dfsDistance[Temp.x][Temp.y]) {
			continue;
		}
		DFS(Temp.x, Temp.y);
	}
	currentDepth--;
}
int getDepth(SnakeNode temp) {
    for (int x = 1; x <= WIDTH; x += COLUMNUNIT) {						
		for (int y = 1; y <= HEIGHT; y += ROWUNIT) {
			dfsDistance[x][y] = 0;
		}
	}
    for (int x = 0; x <= WIDTH + COLUMNUNIT; x += COLUMNUNIT) {
		dfsDistance[x][0] = dfsDistance[x][HEIGHT + ROWUNIT] = WALL;
	}
	for (int y = 0; y <= HEIGHT + ROWUNIT; y += ROWUNIT) {
		dfsDistance[0][y] = dfsDistance[WIDTH + COLUMNUNIT][y] = WALL;
	}

    snake *p = head;
    while(1) {
        if(p == NULL) break;
        dfsDistance[p->x + 1][p->y + 1] = WALL;
        p = p->next;
    }

    currentDepth = 0, deepest = -1;
    DFS(temp.x, temp.y);
    return deepest;

}
void snakeWander(int * direc) {
    SnakeNode NextHead;
    SnakeNode temp;
    SnakeNode CurrentHead;
    CurrentHead.x = snakeFront(head).x + 1;
    CurrentHead.y = snakeFront(head).y + 1;

    for (int x = 2; x <= WIDTH - COLUMNUNIT; x += COLUMNUNIT) {				
		for (int y = 2; y <= HEIGHT - ROWUNIT; y += ROWUNIT) {
			Mark[x][y] = INF;
		}
	}

    for (int x = 0; x <= WIDTH + COLUMNUNIT; x += COLUMNUNIT) {
		Mark[x][0] = Mark[x][HEIGHT + ROWUNIT] = WALL;
		Mark[x][1] = Mark[x][HEIGHT] = WALL;
	}
	for (int y = 0; y <= HEIGHT + ROWUNIT; y += ROWUNIT) {
		Mark[0][y] = Mark[WIDTH + COLUMNUNIT][y] = WALL;
		Mark[1][y] = Mark[WIDTH][y] = WALL;
	}

    snake *p = VirtualSnake;
    while(1) {
        if(p == NULL) break;
        Mark[p->x + 1][p->y + 1] = SNAKEBODY;
        p = p->next;
    }

    int MaxDepth = -100;
	int tempDirection = -1;

    for (int i = 0; i < 4; i++) {
		if (isOpposite(i, SnakeDirection)){
			continue;
		}
		temp.x = CurrentHead.x + Direction[i][0];
		temp.y = CurrentHead.y + Direction[i][1];
		if (INF == Mark[temp.x][temp.y]) {									
			int depth = getDepth(temp);
			if (depth > MaxDepth) {
				MaxDepth = depth;
				tempDirection = i;
				NextHead.x = temp.x;
				NextHead.y = temp.y;
			}
		}
	}

    *direc = tempDirection;
}
int snakeMove() {
    int nextD;
    if (!eatFood(&nextD) ){														
		if (!followTail(&nextD) ) {												
			snakeWander(&nextD);													
		}
	}
    return nextD;
}
