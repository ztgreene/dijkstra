#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "graph.h"

/* Initialises a new, empty, graph with n vertices
	Takes int
	Returns a graph with vertices = n
*********************************************/
Graph new_graph(int n) {

	Graph new_graph;

	// Sets number of vertices to n
	new_graph.V = n;

	// Creates array of adjacency lists, allocating memory
	new_graph.edges = (EdgeList*)malloc(n * sizeof(EdgeList));

	// Initialise each adjacency list as empty
	for (int i = 0; i < n; ++i) {
		new_graph.edges[i].head = NULL;
	}

	return new_graph;
}

/* Function to add edge
	 Takes a graph, source vertex, destination vertex, and weight of edge
	 Calls new_node() function, and adds edge to graph.
**************************************************************************/
void add_edge(Graph *self, int source, int destination, int weight) {

	// Creates new node, inserts into graph.
	EdgeNodePtr node = new_node(destination, weight);
	node->next = self->edges[source].head;
	self->edges[source].head = node;
}

/*  Helper function to create new node
	Takes two ints: destination node, and weight 
	Returns new node with:	 to_vertex = destination,
							 weight - weight.
*********************************************************/
EdgeNodePtr new_node(int destination, int weight) {

	EdgeNodePtr new_node = malloc(sizeof(*new_node));
	new_node->next = NULL;

	// Adds edge
	new_node->edge.to_vertex = destination;
	new_node->edge.weight = weight;

	return new_node;
}



/*	 Destroys graph, freeing all memory
	 Takes a graph
******************************************/
void destroy_graph(Graph *self) {
	int vertexCount = self->V;
	for (int i = 0; i < vertexCount; i++) {
		destroy_edgeNode(self->edges[i].head);
	}
	free(self->edges);
}
/* Helper function used when destroying graph
**********************************************/
void destroy_edgeNode(EdgeNodePtr node) {
	EdgeNodePtr current = node;
	while (current != NULL) {
		EdgeNodePtr to_free = current;
		current = current->next;
		free(to_free);
	}
}


/* Utility function which prints a graph's adjacency lists for each vertex:
	 ---weight-->destination
**********************************************/
void print_graph(Graph *self) {

	for (int i = 0; i < self->V; ++i) {
		EdgeNodePtr temp = self->edges[i].head;
		Edge *current = &temp->edge;
		printf("\n Adjacency list of vertex %d\n head ", i);
		while (temp) {
			printf("---%d-->%d  ", current->weight, current->to_vertex);
			temp = temp->next;
			current = &temp->edge;
		}
	}
}



/* Dijkstra's Shortest Path
	Takes a graph, source vertex, and two arrays of length = vertex count

	Finds shortest distance from source vertex to all other vertices, alters 'distance' array
	The previous node visited for each path ending at that vertex is recorded in 'previous' array

	Entries of resultant 'distance' array are shortest distance to vertex from source
	Entries of resultant 'previous' array are vertex visited before arriving at vertex
*********************************************************************************************/
void dijkstra(Graph *self, int source, int *distance, int *previous) {

	int vertexCount = self->V;
	int *visited = malloc(vertexCount*(sizeof(int))); // This is array of vertices ('vertex set')

	// Initialising arrays
	for (int i = 0; i < vertexCount; i++) {
		visited[i] = 0;				// All vertices are unvisited
		distance[i] = INT_MAX;		// Distance to vertices is infinite
		previous[i] = -1;			// All vertices have no previous
	}
	distance[source] = 0;			// Distance from source to source = 0

	while (array_contains(visited, vertexCount, 0)) {			 // While vertices remain unvisited
		int nearNode = array_minimum(distance, visited, vertexCount);	 // Find nearest unvisited vertex
		visited[nearNode] = 1;							// Set as visited

		EdgeNodePtr temp = self->edges[nearNode].head;		// For each neighbouring vertex
		Edge *current = &temp->edge;

		while (temp) {
			if (visited[current->to_vertex] == 0) {					// Check if vertex is not visited
				int alt = current->weight + distance[nearNode];		// 
				if (alt < distance[current->to_vertex]) {			// A shorter path to vertex has been found
					distance[current->to_vertex] = alt;				// Update distance
					previous[current->to_vertex] = nearNode;		// Remember how we get there
				}
			}
			temp = temp->next;			// Continue traversing adjacency list
			current = &temp->edge;
		}
	}
	// Visited array not needed, so free memory
	free(visited);
}

/* Helper functions for Dijkstra implementation

	Takes array, length of array, and a value
	Returns true if array contains value
*********************************************************/
int array_contains(int *array, int size, int value) {
	
	for (int i = 0; i < size; i++) {
		if (array[i] == value) {
			return 1;		// returns true
		}
	}
	return 0;				// returns false if not found
}

/* This is used to find 'nearest unvisited vertex'
	Takes array of distances, and array of vertices where:
		(contains[i] = 1 for visited, 0 if not visited)

	Returns index of closest vertex that is unvisited
****************************************************************/
int array_minimum(int *distance, int *contains, int size) {
	int minimum = INT_MAX; // set as INF first
	int index;
	for (int i = 0; i < size; i++) {
		if (contains[i] == 0) {
			if (distance[i] < minimum) {
				minimum = distance[i];
				index = i;
			}
		}
	}

	return index;
}



/* Floyd-Warshall algorithm
	Takes a graph, and two 2D arrays.

	Finds the shorted distance between any pair of vertices,
	and records these distances in the 'distance' table.
	Pathway taken to get to each vertex can be rebuilt by examining 'next'
	table.
********************************************************************/
void floyd_warshall(Graph *self, int** distance, int** next) {

	int vertexCount = self->V;

	// populate distance array with weights
	for (int i = 0; i < vertexCount; i++) {

		EdgeNodePtr temp = self->edges[i].head;		// For each neighbouring vertex
		Edge *current = &temp->edge;

		while (temp) {
			distance[i][current->to_vertex] = current->weight;
			next[i][current->to_vertex] = current->to_vertex;

			temp = temp->next;			// Coninue traversing adjacency list
			current = &temp->edge;
		}
	}

	// Loop through all pairs of vertices
	for (int k = 0; k < vertexCount; k++) {
		for (int i = 0; i < vertexCount; i++) {
			for (int j = 0; j < vertexCount; j++) {

				if (distance[i][j] > distance[i][k] + distance[k][j]) { // New shortest dist(i,j) via k

					distance[i][j] = distance[i][k] + distance[k][j];
					next[i][j] = next[i][k]; // Next is 'remembered'
				}
			}
		}
	}
}

