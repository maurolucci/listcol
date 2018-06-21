/*
 * GENCLASSICINST - Generate an instance for the classic Coloring Problem from a graph
 * Made in 2018 by Daniel Severin and Mauro Lucci
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>

ILOSTLBEGIN

using namespace std;

/* CONSTANTS */

#define PI 3.14159265358979323846
#define EDGES_PER_COLOR 3.14
#define LOG_COLORS_PER_AVERAGE 0.785
#define AVERAGES_PER_SIGMA 1.5
#define EPSILON 0.00001
#define MAXTIME 600.0

/* FUNCTIONS */

/*
 * bye - finish executing and show a message
 */
void bye(char *string)
{
	cout << string << endl;
	exit(1);
}

/*
* urnd - generate numbers with uniform random distribution
*  if flag=false, the interval is [a, b]
*  if flag=true, the interval is [a, b)
*/
float urnd(float a, float b, bool flag)
{
	return a + rand() * (b - a) / (float)(RAND_MAX + (flag ? 1 : 0));
}

/*
* grnd - generate numbers with gaussian random distribution
* (it uses Box-Muller algorithm)
*/
float grnd(float mu, float sigma)
{
	float z, u1, u2;

	/* u1 and u2 are given from an uniform distribution of (0, 1] */
	u1 = 1.0 - urnd(0, 1, true);
	u2 = 1.0 - urnd(0, 1, true);

	if (urnd(0, 1, 1) < 0.5) z = sqrt(-2.0 * log(u1)) * cos(2.0 * PI * u2);
	else z = sqrt(-2.0 * log(u1)) * sin(2.0 * PI * u2);
	return z * sigma + mu;
}

/*
 * main - Main program
 */
int main(int argc, char **argv)
{
	cout << "GENCLASSICINST - Generate an instance for the classic Coloring Problem from a graph." << endl;

	if (argc < 2) {
		cout << "Usage: genclassicinst file" << endl;
		cout << "It takes file.graph and generates file.cost and file.list." << endl;
		bye("Bye!");
	}

	char *filename = argv[1];

	/* open graph file */
	char filename_extension[210];
	strncpy(filename_extension, filename, 200);
	strcat(filename_extension, ".graph");
	FILE *stream = fopen(filename_extension, "rt");
	if (!stream) bye("Graph file cannot be opened");
	int vertices, edges;
	fscanf(stream, "%d:%d\n", &vertices, &edges);

	/* do not accept graph of less than 4 vertices or stable sets */
	if (vertices < 4) bye("Number of vertices out range!");
	if (edges < 1 || edges > vertices*(vertices - 1) / 2) bye("Number of edges out of range!");

	/* ask for memory */
	int *degrees = new int[vertices];
	int **adjacency = new int*[vertices];
	for (int u = 0; u < vertices; u++) {
		degrees[u] = 0;
		adjacency[u] = new int[vertices];
		for (int v = 0; v < vertices; v++) adjacency[u][v] = 0;
	}
	//int *edge_u = new int[edges];
	//int *edge_v = new int[edges];

	/* read edges */
	for (int e = 0; e < edges; e++) {
		int u, v;
		fscanf(stream, "%d,%d\n", &u, &v);
		if (u < 0 || u >= v || v >= vertices) {
			cout << "Error reading edge " << e + 1 << "!" << endl;
			bye("Bye!");
		}
		if (adjacency[u][v] != 0) {
			cout << "A repeated edge was found: (" << u << ", " << v << ")" << endl;
			bye("Bye!");
		}
		else {
			degrees[u]++;
			degrees[v]++;
			//edge_u[e] = u;
			//edge_v[e] = v;
			adjacency[u][v] = e + 1;
			adjacency[v][u] = e + 1;
		}
	}
	fclose(stream);

	/* the number of colors is set to Delta(G) */
	int colors = 0;
	for (int v = 0; v < vertices; v++) {
		if (degrees[v] > colors) colors = degrees[v];
	}

	cout << "Statistics:" << endl;
	int clique_size = vertices * (vertices - 1) / 2;
	float density = 100.0 * (float)edges / (float)clique_size;
	cout << "  |V| = " << vertices << ", |E| = " << edges << " (density = " << density << "%), |C| = " << colors << "." << endl;
	if (clique_size == edges) bye("The graph is a clique!");

	/* generate costs and write them to file */
	strncpy(filename_extension, filename, 200);
	strcat(filename_extension, ".cost");
	stream = fopen(filename_extension, "wt");
	if (!stream) bye("Cost file cannot be created");
	fprintf(stream, "%d\n", colors);
	for (int k = 0; k < colors; k++) fprintf(stream, "1 ");
	fprintf(stream, "\n");
	fclose(stream);

	/* compute the clique Q of maximum size with maximum number of neighbors */

	/* generate one variable per vertex */
	IloEnv Xenv;
	IloNumVarArray Xvars(Xenv);
	for (int v = 0; v < vertices; v++) Xvars.add(IloNumVar(Xenv, 0.0, 1.0, ILOBOOL));

	/* generate objective function */
	IloModel Xmodel(Xenv);
	IloExpr fobj(Xenv, 0);
	for (int v = 0; v < vertices; v++) fobj += (10000.0 + (float)degrees[v]) * Xvars[v];
	Xmodel.add(IloMaximize(Xenv, fobj));

	/* generate anti-adjacency constraints */
	for (int u = 0; u < vertices - 1; u++) {
		for (int v = u + 1; v < vertices; v++) {
			if (adjacency[u][v] == 0) Xmodel.add(Xvars[u] + Xvars[v] <= 1);
		}
	}

	IloCplex cplex(Xmodel);
	cplex.setDefaults();
	cplex.setOut(Xenv.getNullStream());
	cplex.setWarning(Xenv.getNullStream());
	cplex.setParam(IloCplex::NumParam::WorkMem, 2048);
	cplex.setParam(IloCplex::NumParam::TreLim, 2048);
	cplex.setParam(IloCplex::IntParam::NodeFileInd, 0);
	cplex.setParam(IloCplex::NumParam::TiLim, MAXTIME);
	cplex.setParam(IloCplex::NumParam::EpGap, 0.0);
	cplex.setParam(IloCplex::NumParam::EpAGap, 0.0);
	cplex.setParam(IloCplex::NumParam::EpInt, EPSILON);
	cplex.setParam(IloCplex::IntParam::Threads, 1);
	cplex.setParam(IloCplex::IntParam::RandomSeed, 1);

	cplex.extract(Xmodel);
	//cplex.exportModel("form.lp");

	/* solve it! */
	cplex.solve();
	IloCplex::CplexStatus status = cplex.getCplexStatus();
	if (status != IloCplex::Optimal) {
		switch (status) {
		case IloCplex::InfOrUnbd:
		case IloCplex::Infeasible: bye("Infeasible :(");
		case IloCplex::AbortTimeLim: bye("Time limit reached :(");
		default: bye("Unexpected error :(");
		}
	}

	/* save optimal solution: if clique[v]=-1 then v is not part of Q, if clique[v]=j>=0 then v is painted with j */
	int *clique = new int[vertices];
	clique_size = 0;
	cout << "Max Clique = {";
	for (int v = 0; v < vertices; v++) {
		if (cplex.getValue(Xvars[v]) > 0.5) {
			cout << " " << v;
			clique[v] = clique_size;
			clique_size++;
		}
		else clique[v] = -1;
	}
	cout << " }, size = " << clique_size << endl;

	/* write list files: consider that each vertex v of the clique is painted with color v */
	strncpy(filename_extension, filename, 200);
	strcat(filename_extension, ".list");
	stream = fopen(filename_extension, "wt");
	if (!stream) bye("List file cannot be created");
	fprintf(stream, "%d:%d\n", vertices, colors);
	bool *L_set = new bool[colors];
	for (int u = 0; u < vertices; u++) {
		if (clique[u] >= 0) {
			/* the list of a vertex u from Q is {clique[u]} */
			fprintf(stream, "1  %d\n", clique[u]);
		}
		else {
			/* the list from the remaining vertices is {1...Delta} / colors_used_by_neighbors_from_Q */
			int L_size = colors;
			for (int k = 0; k < colors; k++) L_set[k] = true;
			for (int v = 0; v < vertices; v++) {
				if (u != v && adjacency[u][v] > 0 && clique[v] >= 0) {
					L_set[clique[v]] = false;
					L_size--;
				}
			}
			fprintf(stream, "%d  ", L_size);
			for (int k = 0; k < colors; k++) if (L_set[k]) fprintf(stream, "%d ", k);
			fprintf(stream, "\n");
		}
	}
	fclose(stream);

	/* free memory */
	delete[] L_set;
	delete[] clique;
	//delete[] edge_v;
	//delete[] edge_u;
	for (int v = 0; v < vertices; v++) delete[] adjacency[v];
	delete[] adjacency;
	delete[] degrees;
	return 0;
}