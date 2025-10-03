#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define CacheS 10000

typedef struct {
    int dest_x;
    int dest_y;
    int cost;
} AirRoute;

typedef struct {
    int cost;
    AirRoute air_routes[5];
    int num_air_routes;
} Hexagon;

typedef struct {
    long long int cost;
    int x;
    int y;
} Node;

typedef struct {
    Node* nodes;
    int size;
    int capacity;
} PriorityQueue;

typedef struct {
    int x1;
    int y1;
    int x2;
    int y2;
    long long int cost;
} CacheEntry;

typedef struct { int q, r, s; } Cube;

Hexagon*** map = NULL;
long long int num_rows = 0;
long long int num_cols = 0;

CacheEntry* cache = NULL;
int cacheCount = 0;

void init_map(int n_rows, int n_cols);
bool change_cost(int x, int y, int v, int radius);
bool toggle_air_route(int x1, int y1, int x2, int y2);
long long travel_cost(int xp, int yp, int xd, int yd);

bool is_valid(int r, int c);
int hex_dist(int x1, int y1, int x2, int y2);

void pq_init(PriorityQueue* pq);
void pq_push(PriorityQueue* pq, long long int cost, int x, int y);
Node pq_pop(PriorityQueue* pq);
bool pq_is_empty(PriorityQueue* pq);
void pq_free(PriorityQueue* pq); 

void clear_cache();
int get_cached_cost(int x1, int y1, int x2, int y2);
void set_cached_cost(int x1, int y1, int x2, int y2, int cost);

int main(void) {

    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        perror("Error opening output.txt");
        return 1;
    }

    char cmd[100];
    int inital = 0;

    while (scanf("%s", cmd) != EOF) {
        if (strcmp(cmd, "init") == 0) {
            int n_cols, n_rows;
            if (scanf("%d %d", &n_cols, &n_rows)==2){
                init_map(n_rows, n_cols);
                inital = 1;
                printf("OK\n");
                fprintf(output_file, "OK\n");
            }
        } else if(strcmp(cmd, "change_cost") == 0) {
            int x;
            int y;
            int v;
            int radius;
            if(scanf("%d %d %d %d", &x, &y, &v, &radius)==4){
                if (!inital) {
                    printf("KO\n");
                    fprintf(output_file, "KO\n");
                }else {
                    if (change_cost(x, y, v, radius)) {
                        printf("OK\n");
                        fprintf(output_file, "OK\n");
                    } else {
                        printf("KO\n");
                        fprintf(output_file, "KO\n");
                    }
                }
            }
        } else if (strcmp(cmd, "toggle_air_route") == 0) {
            int x1;
            int y1;
            int x2;
            int y2;
            if(scanf("%d %d %d %d", &x1, &y1, &x2, &y2)==4){
                if (!inital) {
                    printf("KO\n");
                    fprintf(output_file, "KO\n");
                } else {
                    if (toggle_air_route(x1, y1, x2, y2)) {
                        printf("OK\n");
                        fprintf(output_file, "OK\n");
                    } else {
                        printf("KO\n");
                        fprintf(output_file, "KO\n");
                    }
                }
            }
        } else if (strcmp(cmd, "travel_cost") == 0) {
            int xp;
            int yp;
            int xd;
            int yd;
            if(scanf("%d %d %d %d", &xp, &yp, &xd, &yd)==4){
                if (!inital) {
                    printf("-1\n");
                    fprintf(output_file, "-1\n");
                } else {
                    long long cost = travel_cost(xp, yp, xd, yd);
                    printf("%lld\n", cost);
                    fprintf(output_file, "%lld\n", cost);
                }
            }   
        }
    }
    fclose(output_file);
    return 0;
}

void init_map(int n_rows, int n_cols) {
    clear_cache();

    if (map != NULL) {
        for (int i = 0; i < num_rows; ++i) {
            for (int j = 0; j < num_cols; ++j) {
                if (map[i][j] != NULL) {
                    free(map[i][j]);
                }
            }
            free(map[i]);
        }
        free(map);
    }
    
    num_rows = n_rows;
    num_cols = n_cols;

    map = (Hexagon***)malloc(num_rows * sizeof(Hexagon**));
    if (map == NULL) {
        exit(1);
    }
    for (int i = 0; i < num_rows; ++i) {
        map[i] = (Hexagon**)malloc(num_cols * sizeof(Hexagon*));
        if (map[i] == NULL) {
            exit(1);
        }
        for (int j = 0; j < num_cols; ++j) {
            map[i][j] = (Hexagon*)malloc(sizeof(Hexagon));
            map[i][j]->cost = 1;
            map[i][j]->num_air_routes = 0;
        }
    }
}

bool change_cost(int x, int y, int v, int radius) {
    if (!is_valid(y, x) || radius <= 0 || v < -10 || v > 10) {
        return false;
    }
    clear_cache();

    for (int i = 0; i < num_rows; ++i) {
        for (int j = 0; j < num_cols; ++j) {
            int dist = hex_dist(x, y, j, i);
            if (dist < radius) {
                double scaling_factor = (double)(radius - dist) / (double)radius;
                int cost_change = floor(v * fmax(0.0, scaling_factor));

                map[i][j]->cost += cost_change;
                if (map[i][j]->cost < 0) map[i][j]->cost = 0;
                if (map[i][j]->cost > 100) map[i][j]->cost = 100;

                for (int k = 0; k < map[i][j]->num_air_routes; ++k) {
                    map[i][j]->air_routes[k].cost += cost_change;
                    if (map[i][j]->air_routes[k].cost < 0) map[i][j]->air_routes[k].cost = 0;
                    if (map[i][j]->air_routes[k].cost > 100) map[i][j]->air_routes[k].cost = 100;
                }
            }
        }
    }
    return true;
}

bool toggle_air_route(int x1, int y1, int x2, int y2) {
    if (!is_valid(y1, x1) || !is_valid(y2, x2)) {
        return false;
    }
    clear_cache();
    // Se presente, tolgo
    Hexagon* start_hex = map[y1][x1];
    for (int i = 0; i < start_hex->num_air_routes; ++i) {
        if (start_hex->air_routes[i].dest_x == x2 && start_hex->air_routes[i].dest_y == y2) {
            for (int j = i; j < start_hex->num_air_routes - 1; ++j) {
                start_hex->air_routes[j] = start_hex->air_routes[j + 1];
            }
            start_hex->num_air_routes--;
            return true;
        }
    }
    // Ho già n max Air Routes
    if (start_hex->num_air_routes >= 5) {
        return false;
    }

    /* 
    int total_cost = start_hex->cost;
    int count = 1;
    for(int i = 0; i < start_hex->num_air_routes; ++i) {
        total_cost += start_hex->air_routes[i].cost;
        count++;
    }
    */
    // Aggiungo AIR Route:
    int newAirCost = start_hex->cost;
    if (newAirCost < 0) newAirCost = 0;
    if (newAirCost > 100) newAirCost = 100;

    start_hex->air_routes[start_hex->num_air_routes].dest_x = x2;
    start_hex->air_routes[start_hex->num_air_routes].dest_y = y2;
    start_hex->air_routes[start_hex->num_air_routes].cost = newAirCost;
    start_hex->num_air_routes++;
    return true;
}

long long travel_cost(int xp, int yp, int xd, int yd) {
    if (!is_valid(yp, xp) || !is_valid(yd, xd)) {
        return -1;
    }
    if (xp == xd && yp == yd) {
        return 0;
    }

    long long cached_cost = get_cached_cost(xp, yp, xd, yd);
    if (cached_cost != -2LL) {
        return cached_cost;
    }

    long long int** min_costs = (long long int**)malloc(num_rows * sizeof(long long int*));
    if (min_costs == NULL) {
        return -1;
    }
    for(int i = 0; i < num_rows; ++i) {
        min_costs[i] = (long long int*)malloc(num_cols * sizeof(long long int));
        if (min_costs[i] == NULL) {
            for (int k = 0; k < i; ++k) {
                free(min_costs[k]);
            }
            free(min_costs);
            return -1;
        }
        for(int j = 0; j < num_cols; ++j) {
            min_costs[i][j] = -1;
        }
    }

    min_costs[yp][xp] = 0;
    PriorityQueue pq;
    pq_init(&pq);
    pq_push(&pq, 0, xp, yp);

    const int even_r_neighbors_row[] = { 0, -1, -1,  0, +1, +1};
    const int even_r_neighbors_col[] = {+1,  0, -1, -1, -1,  0};
    const int odd_r_neighbors_row[]  = { 0, -1, -1,  0, +1, +1};
    const int odd_r_neighbors_col[]  = {+1, +1,  0, -1,  0, +1};

    long long int final_cost = -1;

    while (!pq_is_empty(&pq)) {
        Node current = pq_pop(&pq);
        long long int cost = current.cost;
        int x = current.x;
        int y = current.y;

        if (min_costs[y][x] != -1 && cost > min_costs[y][x]) {
            continue;
        }
        if (x == xd && y == yd) {
            final_cost = cost;
            break;
        }

        Hexagon* current_hex = map[y][x];
    
        if(current_hex->cost > 0) {
            const int* dr = (y % 2 != 0) ? odd_r_neighbors_row : even_r_neighbors_row;
            const int* dc = (y % 2 != 0) ? odd_r_neighbors_col : even_r_neighbors_col;
            // Via terra
            for (int i = 0; i < 6; ++i) {
                int new_y = y + dr[i];
                int new_x = x + dc[i];
                if (is_valid(new_y, new_x)) {
                    long long int new_cost = cost + current_hex->cost;
                    if (min_costs[new_y][new_x] == -1 || new_cost < min_costs[new_y][new_x]) {
                        min_costs[new_y][new_x] = new_cost;
                        pq_push(&pq, new_cost, new_x, new_y);
                    }
                }
            }
        }
        // Rotta aerea
        for (int i = 0; i < current_hex->num_air_routes; ++i) {
            if (current_hex->air_routes[i].cost > 0){
                int new_y = current_hex->air_routes[i].dest_y;
                int new_x = current_hex->air_routes[i].dest_x;
                if(is_valid(new_y, new_x)) {
                    long long int new_cost = cost + current_hex->air_routes[i].cost;
                    if (min_costs[new_y][new_x] == -1 || new_cost < min_costs[new_y][new_x]) {
                        min_costs[new_y][new_x] = new_cost;
                        pq_push(&pq, new_cost, new_x, new_y);
                    }
                }
            }
        }
    }

    pq_free(&pq);
    for(int i = 0; i < num_rows; ++i) {
        free(min_costs[i]);
    }
    free(min_costs);
    set_cached_cost(xp, yp, xd, yd, final_cost);
    return final_cost;
}


bool is_valid(int r, int c) {
    return r >= 0 && r < num_rows && c >= 0 && c < num_cols;
}

Cube offset_to_cube(int col, int row) {
    int q = col - (row - (row & 1)) / 2;
    int r = row;
    int s = -q - r;
    return (Cube){q, r, s};
}

int cube_distance(Cube a, Cube b) {
    return (abs(a.q - b.q) + abs(a.r - b.r) + abs(a.s - b.s)) / 2;
}

int hex_dist(int x1, int y1, int x2, int y2) {
    Cube a = offset_to_cube(x1, y1);
    Cube b = offset_to_cube(x2, y2);
    return cube_distance(a, b);
}


void init_cache() {
    if (cache == NULL) {
        cache = (CacheEntry*)malloc(CacheS * sizeof(CacheEntry));
        if (cache == NULL) {
            exit(1);
        }
    }
    for (int i = 0; i < CacheS; i++) {
        cache[i].cost = -2;
        cache[i].x1 = -1;
    }
    cacheCount = 0;
}

void clear_cache() {
    if (cache != NULL) {
        init_cache();
    }
}

unsigned int hash(int x1, int y1, int x2, int y2) {
    return (unsigned int)((x1 * 73856093) ^ (y1 * 19349663) ^ (x2 * 83492791) ^ (y2 * 67867979)) % CacheS;
}

int get_cached_cost(int x1, int y1, int x2, int y2) {

    if (cache == NULL || cacheCount == 0) return -2;

    unsigned int index = hash(x1, y1, x2, y2);

    for(int i = 0; i < CacheS; ++i) {
        int currentInd = (index + i) % CacheS;
        if (cache[currentInd].cost != -2 &&
            cache[currentInd].x1 == x1 && cache[currentInd].y1 == y1 &&
            cache[currentInd].x2 == x2 && cache[currentInd].y2 == y2) {
                return cache[currentInd].cost;
        }
        if(cache[currentInd].cost == -2) {
             return -2;
        }
    }
    return -2;
}

void set_cached_cost(int x1, int y1, int x2, int y2, int cost) {
    if (cache == NULL) init_cache();

    unsigned int ind = hash(x1, y1, x2, y2);

    for(int i = 0; i < CacheS; ++i) {
        int current_index = (ind + i) % CacheS;
        if (cache[current_index].cost == -2 || (cache[current_index].x1 == x1 && cache[current_index].y1 == y1 && cache[current_index].x2 == x2 && cache[current_index].y2 == y2)) {
            if(cache[current_index].cost == -2) {
                 if(cacheCount < CacheS) cacheCount++;
            }
            cache[current_index] = (CacheEntry){x1, y1, x2, y2, cost};
            return;
        }
    }
    cache[ind] = (CacheEntry){x1, y1, x2, y2, cost};
}


void pq_init(PriorityQueue* pq) {
    pq->size = 0;
    pq->capacity = 10;
    pq->nodes = (Node*)malloc(sizeof(Node) * pq->capacity);
    if (pq->nodes == NULL) {
        exit(1);
    }
}

void pq_push(PriorityQueue* pq, long long int cost, int x, int y) {
    if (pq->size >= pq->capacity) {
        pq->capacity *= 2;
        Node* new_nodes = (Node*)realloc(pq->nodes, sizeof(Node) * pq->capacity);
        if (new_nodes == NULL) {
            exit(1);
        }
        pq->nodes = new_nodes;
    }

    Node node = {cost, x, y};
    pq->nodes[pq->size] = node;
    int i = pq->size;
    pq->size++;

    while (i > 0 && pq->nodes[i].cost < pq->nodes[(i - 1) / 2].cost) {
        Node temp = pq->nodes[i];
        pq->nodes[i] = pq->nodes[(i - 1) / 2];
        pq->nodes[(i - 1) / 2] = temp;
        i = (i - 1) / 2;
    }
}

Node pq_pop(PriorityQueue* pq) {
    if (pq->size <= 0) {
        return (Node){-1, -1, -1};
    }

    Node root = pq->nodes[0];
    pq->nodes[0] = pq->nodes[pq->size - 1];
    pq->size--;

    int i = 0;
    while (1) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;

        if (left < pq->size && pq->nodes[left].cost < pq->nodes[smallest].cost) {
            smallest = left;
        }
        if (right < pq->size && pq->nodes[right].cost < pq->nodes[smallest].cost) {
            smallest = right;
        }

        if (smallest != i) {
            Node temp = pq->nodes[i];
            pq->nodes[i] = pq->nodes[smallest];
            pq->nodes[smallest] = temp;
            i = smallest;
        } else {
            break;
        }
    }
    return root;
}

bool pq_is_empty(PriorityQueue* pq) {
    return pq->size == 0;
}

void pq_free(PriorityQueue* pq) {
    if (pq->nodes != NULL) {
        free(pq->nodes);
        pq->nodes = NULL;
    }
    pq->size = 0;
    pq->capacity = 0;
}
