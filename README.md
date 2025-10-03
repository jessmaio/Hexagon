# Algorithms and Data Structures 👾
Final Project - Politecnico di Milano, 2024/2025 📚

## Project Description 📝
This project is based on the final exam of the **Algorithms and Data Structures** course (A.Y. 2024/2025).  
The goal is to develop a program for **Movhex**, a transport company that needs to calculate **optimal routes** for its vehicles on a hexagonal-tiled map.

The world is modeled as a rectangular grid of hexagonal tiles.  
Each tile is uniquely identified by its coordinates `(column, row)`, starting from `(0,0)` at the bottom-left.  
Vehicles can move:
- **by land**, paying the *exit cost* of the current tile;  
- **by air**, using *directed air routes* between tiles, each with its own traversal cost.  

Both land costs and air route costs can change dynamically during execution.

---

## Supported Commands ⚙️
The program receives commands from **standard input** and prints responses to **standard output**.

- **`init <n_columns> <n_rows>`**  
  Initializes (or reinitializes) the map with all exit costs set to 1 and no air routes.  
  Response: `OK`.

- **`change_cost <x> <y> <v> <radius>`**  
  Modifies the exit cost of all tiles within `<radius>` distance from `(x, y)` accordinly to the formula below.  
  - `v ∈ [-10, 10]` is the variation applied proportionally to the distance.  
  - If the resulting cost is `0`, the tile becomes **non-traversable**.  
  Response: `OK` or `KO` if parameters are invalid.
  
  Formula:
  
  Cost(xe, ye) = Cost(xe, ye) + floor( v * max(0, (radius - DistHex((xe,ye),(x,y))) / radius ) )

- **`toggle_air_route <x1> <y1> <x2> <y2>`**  
  Adds or removes a directed air route from `(x1, y1)` to `(x2, y2)`.  
  - A tile can have at most **5 outgoing air routes**.  
  - If added, the new route’s cost is the floor of the average between the existing air routes and the tile’s land cost.  
  Response: `OK` or `KO`.

- **`travel_cost <xs> <ys> <xd> <yd>`**  
  Returns the minimum travel cost from source `(xs, ys)` to destination `(xd, yd)`.  
  - If source = destination → cost is `0`.  
  - If no path exists or coordinates are invalid → returns `-1`.  
  - The exit cost of the destination tile is always ignored.  
  Response: an integer cost or `-1`.

---

## Example Session 💻
```
init 100 100 → OK
change_cost 10 20 -10 5 → OK
travel_cost 0 0 20 0 → 20
toggle_air_route 0 0 20 0 → OK
travel_cost 0 0 20 0 → 1
```
---

## Implementation Notes 🗒️
- The map is represented as a **hexagonal grid with adjacency rules**.  
- The command `travel_cost` is expected to be much more frequent than `change_cost` and `toggle_air_route`.  
  → Efficient shortest-path algorithms (e.g., Dijkstra, A*, or optimized data structures) are strongly recommended.  
- All integers fit into **32-bit signed values**.  

---

## Algorithm & Code Explanation :rocket:

The program is implemented in **C** and follows the specification by modeling the map as a dynamic grid of hexagons.

### Data Structures 🧱
- **`Hexagon`**: represents a tile with an exit cost and up to 5 outgoing air routes.  
- **`AirRoute`**: represents a directed air connection with destination coordinates and traversal cost.  
- **`PriorityQueue`**: a binary heap used for Dijkstra’s algorithm to efficiently extract the next minimum-cost node.  
- **`Cache`**: a fixed-size hash table that stores results of previous `travel_cost` queries to speed up repeated requests.  
- **`Cube` coordinates**: used to compute distances between hexagons (`hex_dist`) with the cube-coordinate method, which is well suited for hexagonal grids.

### Main Features 🎯
- **Map initialization (`init`)**: allocates a 2D array of hexagons with default cost = 1.  
- **Cost update (`change_cost`)**: applies the given formula to update exit costs and outgoing air route costs in a radius around a target tile. Costs are clamped between `0` (non-traversable) and `100`.  
- **Air route management (`toggle_air_route`)**: adds or removes a directed connection. A tile cannot exceed 5 outgoing air routes. If added, the cost is derived from the tile’s current exit cost.  
- **Shortest path (`travel_cost`)**: implemented with **Dijkstra’s algorithm** over both land and air connections:
  - Land moves: cost equals the exit cost of the current hexagon.  
  - Air moves: cost equals the air route cost (ignoring the tile’s exit cost).  
  - Destination tile’s exit cost is not included in the path cost.  
  - Returns `-1` if no valid path exists.  
- **Cache**: results of `travel_cost` are cached with a hash-based lookup, reducing computation time for repeated queries in common source-destination pairs.

### Hexagonal Grid Representation 🗺️

The map is stored as a rectangular grid of hexagons with **offset coordinates** `(x, y)`:
- `x` = column index  
- `y` = row index  
- `(0,0)` is the bottom-left tile  
- Odd and even rows are shifted differently to maintain the hexagonal layout.

Example with 4 rows × 5 columns:
```
    (0,3)   (1,3)   (2,3)   (3,3)   (4,3)
 (0,2)   (1,2)   (2,2)   (3,2)   (4,2)
    (0,1)   (1,1)   (2,1)   (3,1)   (4,1)
 (0,0)   (1,0)   (2,0)   (3,0)   (4,0)
```

Each hexagon connects to up to 6 neighbors (fewer on map borders).  
Neighbor coordinates differ depending on whether the row is **odd** or **even**:

- **Even rows (y % 2 == 0)**  
  Neighbors: `(x+1,y), (x-1,y), (x,y+1), (x,y-1), (x-1,y+1), (x-1,y-1)`

- **Odd rows (y % 2 == 1)**  
  Neighbors: `(x+1,y), (x-1,y), (x,y+1), (x,y-1), (x+1,y+1), (x+1,y-1)`

This offset coordinate system is internally converted to **cube coordinates** `(q,r,s)` when computing distances with the formula:

DistHex((x1,y1),(x2,y2)) = ( |q1-q2| + |r1-r2| + |s1-s2| ) / 2


### Travel Cost Query Flow ✈️

When the command `travel_cost <xs> <ys> <xd> <yd>` is executed, the program follows these steps:

1. **Input validation**  
   - Check if both source `(xs, ys)` and destination `(xd, yd)` are valid hexagons.  
   - If invalid → return `-1`.  
   - If source = destination → return `0`.

2. **Cache lookup**  
   - Compute a hash from `(xs, ys, xd, yd)`.  
   - If a cached result exists → return it immediately.  

3. **Shortest-path computation (Dijkstra)**  
   - Initialize all distances with `-1` (unvisited).  
   - Set distance of source = `0`.  
   - Use a **priority queue** to always expand the node with the smallest current cost.  
   - For each expanded hexagon:  
     - **Land neighbors**: add `exit cost` of current hexagon.  
     - **Air routes**: add cost of the air connection (ignoring land exit cost).  
   - Update neighbor distances if a smaller cost is found.  
   - Stop when destination is reached.

4. **Result caching**  
   - Store the computed cost in the cache for faster future lookups.  

5. **Output**  
   - Print the minimum cost if reachable, otherwise `-1`.  

---

## Complexity 👁️‍🗨️
- **Dijkstra’s algorithm** ensures correctness with non-negative edge weights.  
- Typical complexity: `O((V+E) log V)`, where `V` is the number of hexagons and `E` the number of land + air connections.  
- The **cache** significantly reduces average-case complexity in realistic workloads where many queries repeat the same regions.

---

## Compilation & Execution ⌨️
```
make                                     #Compilation
./main < input.txt > output.txt          #Execution ( < input file > output file )
```
---

## Authors 🙋🏻‍♀️

Jessica Maio



