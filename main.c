#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "graph.h"

/* Function Prototypes
************************/
int** make_dem(int size, int roughness);
int cost_funcA(int diff);
int cost_funcB(int diff);
void print_2D(int** array2D, int size);
void print_2D_ascii(int** array2D, int size);

int** copy_dem(int** array2D, int size);

// Graph functions
void build_graph(Graph self, int** array2D, int size, int allowNegativeWeights);
void possibleMove(Graph self, int** array2D, int size, int sourceVertex, int sourceValue, int x, int y, int allowNegativeWeights);

void fw_complete(int** dem, int size);
void fw_free_memory(Graph *graph, int **fw_distance, int **fw_next, int vertexCount);
int** fw_initialise_distance(int vertexCount);
int** fw_initialise_next(int vertexCount);
void trace_path_fw(int** dem, int** next, int size);

void dijkstra_complete(int** dem, int size);
void dijkstra_free_memory(Graph *graph, int *distance, int* previous);
void trace_path_dijkstra(int** dem, int* previous, int size);
/************************************************************************/


/* Map code
******************/
int** make_dem(int size, int roughness) {
	int seed = time(NULL);
	srand(seed);
	int** dem = malloc(size * sizeof *dem);
	for (int x = 0; x < size; x++) {
		dem[x] = malloc(size * sizeof *dem[x]);
		for (int y = 0; y < size; y++) {
			dem[x][y] = -1;
		}
	}
	int r = roughness;

	dem[0][0] = 50 - r / 2 + rand() % r;
	dem[size - 1][0] = 50 - r / 2 + rand() % r;
	dem[0][size - 1] = 50 - r / 2 + rand() % r;
	dem[size - 1][size - 1] = 50 - r / 2 + rand() % r;

	for (int step = (size - 1); step > 0; step /= 2) {
		r = r > 1 ? r / 2 : r;
		if (r < 1) r = 1;
		for (int cx = 0; cx < (size - 1) / step; cx++) {
			for (int cy = 0; cy < (size - 1) / step; cy++) {
				int a = dem[cx*step][cy*step];
				int b = dem[cx*step + step][cy*step];
				int c = dem[cx*step][cy*step + step];
				int d = dem[cx*step + step][cy*step + step];

				dem[cx*step + step / 2][cy*step + step / 2] = (a + b + c + d) / 4 + rand() % r - r / 2;

				dem[cx*step + step / 2][cy*step] = (a + b) / 2 + rand() % r - r / 2;
				dem[cx*step][cy*step + step / 2] = (a + c) / 2 + rand() % r - r / 2;
				dem[cx*step + step][cy*step + step / 2] = (b + d) / 2 + rand() % r - r / 2;
				dem[cx*step + step / 2][cy*step + step] = (c + d) / 2 + rand() % r - r / 2;
			}
		}
	}
	for (int x = 0; x < size; x++) {
		for (int y = 0; y < size; y++) {
			dem[x][y] = dem[x][y]<0 ? 0 : dem[x][y];
			dem[x][y] = dem[x][y]>99 ? 99 : dem[x][y];
		}
	}
	return dem;
}

int cost_funcA(int diff) {
	int cost = 1;
	if (diff > 0) cost += diff * diff;
	return cost;
}

int cost_funcB(int diff) {
	int cost = 1;
	if (diff > 0)
		cost += diff * diff;
	else
		cost += diff;
	return cost;
}

void print_2D(int** array2D, int size) {
	for (int x = 0; x < size; x++) {
		for (int y = 0; y < size; y++) {
			if (array2D[x][y] >= 0) {
				printf("%2d ", array2D[x][y]);
			}
			else {
				printf("() ");
			}
		}
		printf("\n");
	}
}

void print_2D_ascii(int** array2D, int size) {
	char *shades = " .-:=+*#%@";
	for (int x = 0; x < size; x++) {
		for (int y = 0; y < size; y++) {
			if (array2D[x][y] >= 0) {
				char shade = shades[array2D[x][y] * 10 / 100];
				printf("%c%c", shade, shade);
			}
			else {
				printf("()");
			}
		}
		printf("\n");
	}
}



/* Main function
******************/
int main() {

	// Set size of map (2^n + 1), and roughness
	int size =33;
	int roughness = 132;

	// Makes DEM with size, roughness
	int** map = make_dem(size, roughness);

	// Prints DEM with values for each vertex
	print_2D(map, size);
	printf("\n");


	// Prints a shortest path and energy cost found using Dijkstra's algorithm
	// (no regenerative braking)
	dijkstra_complete(map, size);

	// Prints a shortest path and energy cost found using Floyd-Warshall algorithm
	// (with regenerative braking)
	fw_complete(map, size);


	// Free memory for DEM
	for (int i = 0; i < size; i++) {
		free(map[i]);
	}
	free(map);

	printf("\nPress enter to exit\n");
	getchar();
	return 0;
}


/*
Complete Dijkstra implementation

 Takes a 2D array (DEM), and the size of DEM
	- Builds a graph with no negative edge weights,
	- Calculates shortest paths from vertex 0 (top left of DEM),
	- Reconstructs path to last vertex (bottom right of DEM),
	- Creates copy of DEM with heights changed to -1 for vertices on the path
	- Prints DEM to show path
	- Frees all dynamically allocated memory

******************************************************************************/
void dijkstra_complete(int** dem, int size) {

	int vertexCount = size * size;

	// Initialise distance/previous arrays for algorithm
	int *distance = malloc(vertexCount*(sizeof(int)));
	int *previous = malloc(vertexCount*(sizeof(int)));

	// Initialise and build graph
	Graph graph = new_graph(size*size);
	build_graph(graph, dem, size, 1); // 0 => Graph contains no negative edge weights

	printf("\n\nDijkstra's Shortest Path:\n");

	dijkstra(&graph, 0, distance, previous);
	trace_path_dijkstra(dem, previous, size);

	printf("Shortest path energy cost = %d\n", distance[vertexCount - 1]);

	dijkstra_free_memory(&graph, distance, previous);
}


/*
 Complete Floyd-Warshall implementation

 Takes a 2D array (DEM), and the size of DEM
	- Builds a graph which can include negative edge weights,
	- Calculates all shortest paths using Floyd-Warshall,
	- Reconstructs path form first to last vertex (top left, to bottom right),
	- Creates copy of DEM with heights changed to -1 for vertices on the path
	- Prints DEM to show path
	- Frees all dynamically allocated memory

****************************************************************************/
void fw_complete(int** dem, int size) {

	int vertexCount = size * size;

	// intialises |V|x|V| arrays for Floyd-Warshall
	int** fw_distance = fw_initialise_distance(vertexCount);
	int** fw_next = fw_initialise_next(vertexCount);

	Graph graph = new_graph(vertexCount);
	build_graph(graph, dem, size, 1); // 1 => Graph contains negative edge weights


	printf("\n\nFloyd-Warshall's Shortest Path:\n");

	floyd_warshall(&graph, fw_distance, fw_next);
	trace_path_fw(dem, fw_next, size);

	printf("Shortest path energy cost = %d\n", fw_distance[0][vertexCount - 1]);

	fw_free_memory(&graph, fw_distance, fw_next, vertexCount);
}


/* Helper function used when tracing shortest paths
   Takes a dem, copies it, returns copy
****************************************************/
int** copy_dem(int** array2D, int size) {

	int** copy = malloc(size * sizeof *copy);
	for (int x = 0; x < size; x++) {
		copy[x] = malloc(size * sizeof *copy[x]);
		for (int y = 0; y < size; y++) {
			copy[x][y] = array2D[x][y];
		}
	}

	return copy;
}

/* Takes an empty graph, a DEM, size, and an integer either 0 or != 0.

   Function calculates all possible moves from each vertex [x][y] location of DEM,
   and adds those edges to the graph using possibleMove() helper function.

   Weights of edges are calculated by possibleMove where:

			allowNegativeWeights = 0,	uses cost_funcA() (edge weights > 0)
			allowNegativeWeights != 0,	uses cost_funcB() (allows negative edge weights)
*****************************************************************************************/
void build_graph(Graph self, int** array2D, int size, int allowNegativeWeights) {
	for (int x = 0; x < size; x++) {
		for (int y = 0; y < size; y++) {

			int sourceVertex = x * size + y;
			int sourceValue = array2D[x][y];

			if (x != 0) {
				// Can move 'North'
				possibleMove(self, array2D, size, sourceVertex, sourceValue, x - 1, y, allowNegativeWeights);
			}
			if (x != size - 1) {
				// Can move 'South'
				possibleMove(self, array2D, size, sourceVertex, sourceValue, x + 1, y, allowNegativeWeights);
			}
			if (y != 0) {
				// Can move 'West'
				possibleMove(self, array2D, size, sourceVertex, sourceValue, x, y - 1, allowNegativeWeights);
			}
			if (y != size - 1) {
				// Can move 'East'
				possibleMove(self, array2D, size, sourceVertex, sourceValue, x, y + 1, allowNegativeWeights);
			}

		}
	}
}

/* Helper function for buid_graph function above.
Takes Graph, DEM, size, source vertex, its value, x and y values, and integer to allow for negative weights
Adds edge from source vertex to destination vertex, costed according to 'allowNegativeWeights'
*****************************************************************************************************************/
void possibleMove(Graph self, int** array2D, int size, int sourceVertex, int sourceValue, int x, int y, int allowNegativeWeights) {

	int destinationVertex = x * size + y;
	int destinationVertexValue = array2D[x][y];

	int cost;	// Dependent on allowNegativeWeights

	if (allowNegativeWeights == 0) {		// Edge weights > 0
		cost = cost_funcA(destinationVertexValue - sourceValue);
	}
	else {									// Negative weights permitted
		cost = cost_funcB(destinationVertexValue - sourceValue);
	}
	
	add_edge(&self, sourceVertex, destinationVertex, cost);
}


/* Takes DEM, array of 'previous' vertices produced by Dijkstra function, size of DEM
   Traces shortest path onto copy of DEM, and prints result
***************************************************************************************/
void trace_path_dijkstra(int** dem, int* previous, int size) {

	int** map = copy_dem(dem, size);  // Copies DEM

	map[size - 1][size - 1] = -1; // Marks last vertex as visited
	int steps = previous[size*size-1];// Gets previous of final vertex

	while (steps != -1) {			  // Retraces path until at source vertex
		map[steps / size][steps%size] = -1;
		steps = previous[steps];
	}

	print_2D_ascii(map, size);

	// Free memory
	for (int i = 0; i < size; i++) {
		free(map[i]);
	}
	free(map);
}

/* Takes DEM, 2D array of 'next' vertices produced by Floyd-Warshall function, size of DEM
   Traces shortest path onto copy of DEM, and prints result
*********************************************************************************************/
void trace_path_fw(int** dem, int** next, int size) {

	int** map = copy_dem(dem, size);  // Copies DEM

	map[0][0] = -1;				  // Marks source as visited
	map[size - 1][size - 1] = -1; // Marks last vertex as visited

	int steps = next[size*size - 1][0];// Gets previous of final vertex

	while (steps != 0) {			  // build path until at source vertex
		map[steps / size][steps%size] = -1;
		steps = next[steps][0];
	}

	print_2D_ascii(map, size);

	// Free memory
	for (int i = 0; i < size; i++) {
		free(map[i]);
	}
	free(map);
}

/* Helper functions to allocate memory for, and initialise 2D arrays for Floyd-Warshall
   Takes number of vertices, returns 2D 'distance' array
******************************************************************************************/
int** fw_initialise_distance(int vertexCount) {

	int** fwDist = malloc(vertexCount * sizeof *fwDist);
	for (int x = 0; x < vertexCount; x++) {
		fwDist[x] = malloc(vertexCount * sizeof *fwDist[x]);
		for (int y = 0; y < vertexCount; y++) {
			fwDist[x][y] = INT_MAX/2; // Note: we can't use INT_MAX due to errors testing
		}							  // inequalities in algorithm:
	}								  // if (INT_MAX > INT_MAX + INT_MAX)
	return fwDist;
}
/* Takes number of vertices, returns 2D 'next' array
*****************************************************/
int** fw_initialise_next(int vertexCount) {

	int** fwNext = malloc(vertexCount * sizeof *fwNext);
	for (int x = 0; x < vertexCount; x++) {
		fwNext[x] = malloc(vertexCount * sizeof *fwNext[x]);
		for (int y = 0; y < vertexCount; y++) {
			fwNext[x][y] = -1;
		}
	}
	return fwNext;
}

/* Frees all dynamically allocated memory used in Dijkstra's algorithm, including graph
****************************************************************************************/
void dijkstra_free_memory(Graph *graph, int *distance, int* previous) {
	free(distance);
	free(previous);
	destroy_graph(graph);
}

/* Frees all dynamically allocated memory used in Floyd-Warshall algorithm, including graph
********************************************************************************************/
void fw_free_memory(Graph *graph, int **fw_distance, int **fw_next, int vertexCount) {
	for (int i = 0; i < vertexCount; i++) {
		free(fw_distance[i]);
		free(fw_next[i]);
	}
	free(fw_distance);
	free(fw_next);
	destroy_graph(graph);
}
