#pragma once

typedef struct edge {
	int to_vertex;
	int weight;
} Edge;

typedef struct edgeNode {
	Edge edge;
	struct edgeNode *next;
} *EdgeNodePtr;

typedef struct edgeList {
	EdgeNodePtr head;
} EdgeList;

typedef struct graph {
	int V;
	EdgeList *edges;
} Graph;

// Functions for building graphs, adding edges
Graph new_graph(int n);
EdgeNodePtr new_node(int destination, int weight);
void add_edge(Graph *self, int source, int destination, int weight);

// Functions for destroying graphs/freeing memory
void destroy_edgeNode(EdgeNodePtr node);
void destroy_graph(Graph *self);

// Shortest path algorithms
void dijkstra(Graph *self, int source, int* distance, int* previous);
void floyd_warshall(Graph *self, int** distance, int** next);

// Utility/helper functions; prints adjacency lists for each node
void print_graph(Graph *self);

// Functions used within Dijkstra algorithm
int array_contains(int *array, int size, int value);
int array_minimum(int *distance, int *contains, int size);