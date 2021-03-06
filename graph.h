#ifndef _GRAPH_H_
#define _GRAPH_H_

#ifndef VISUALC
#include "mwis_sewell/mwss.h"
#else
extern "C" {
#include "mwis_sewell/mwss.h"
}
#endif

/* Additional structure for arrays of nodepnt */
typedef struct nodepntArray {
	nodepnt* list;
	int n_list;
	nodepntArray() {};
	nodepntArray(nodepnt *list, int n_list) : list(list), n_list(n_list) {};
	~nodepntArray() {};
} nodepntArray;

#include <vector>
#include <list>

#define INTFACTOR 1000.0
#define THRESHOLD 0.1
#define MAXTIME_MWIS 300.0

//#define STABLE_POOL

#ifndef BRANCHING_STRATEGY
#define BRANCHING_STRATEGY 0  // 0: Branching on edges
                              // 1: Branching on colors
                              // 2: Branching on indistinguishable colors
#endif

enum BRANCH_STATUS {NONE, JOIN, COLLAPSE, CHOOSE, REMOVE};

class Graph {

  public:

  // Build graph
  Graph();
  Graph(char *filename_graph, char *filename_costs, char *filename_lists);

  // Destructor
  ~Graph();

  // Get number of vertices
  int get_n_vertices();

  // Get number of colors
  int get_n_colors();

  // Get size of V[i]
  int get_n_V(int i);

  // Get the j-th color of C[i] 
  int get_C(int i, int j);

  // Get size of C[i]
  int get_n_C(int i);

  // Get cost of color K[i]
  int get_color_cost(int i);

  // Get the max w_k such that vertex v belongs to V_k
  int get_m(int v);

  // Get N(v) \cap Vk
  int get_n_neighbours(int v, int k);

  // Get |L(v)| of every vertex v
  void get_W1(std::vector<int> &n_L);

  // Get |L(v) \cap K| of every vertex v
  void get_W2(std::vector<int> &n_L);

  // Are vertices u and v adjacent?
  bool is_edge(int u, int v);

  // Set the weight of a vertex
  void set_vertex_weight(int v, double y);

  // Print graph
  void print_graph();

  // Solve the MWSSP in V[i]: ¿Does it exists a maximum stable set in V[i] with a weight greater than goal?
  //  If the answer is yes, then the result is saved in best_stable and n_best_stable
  bool solve_MWSSP(int i, double goal, nodepnt **best_stable, int *n_best_stable);

  // Check stable set
  bool check_stable(int i, nodepnt *stable, int n_stable);
  
  // Check coloring TODO
  bool check_coloring(std::vector<int> &);

  // LP's stable representation (bool *) to Graph's stable representation (nodepnt *), and viceversa
  void column_to_stable (bool *column, int size, nodepnt **stable);
  void stable_to_column (nodepnt *stable, int size, bool **column);

  // Translate a stable set of the father to a stable set of the current son
  void translate_stable_set (int color, nodepnt *stable_father, int n_stable_father, nodepnt **stable_son, int *n_stable_son);

  // Find a stable set covering
  void coloring_heuristic(std::vector<std::list<nodepntArray>>& stable_sets);

  // Copy the local pool to the global pool
  void update_pool();

  // Build a new graph by joining the vertices u and v
  Graph *join_vertices(int u, int v);

  // Build a new graph by collapsing the vertices u and v
  Graph *collapse_vertices(int u, int v);

  // Build a new graph where color k is the only color in the list of v
  Graph* choose_color(int v, int k);

  // Build a new graph where color k is removed from the list of v
  Graph* remove_color(int v, int k);

  // Build a new graph where vertex v must be colored with the color class of color k
  Graph* choose_indistinguishable_color(int v, int k);

  // Build a new graph where vertex v must not be colores with the color class of color k
  Graph* remove_indistinguishable_color(int v, int k);

  // Get branch status
  BRANCH_STATUS get_branch_status();

  // Get branching's vertices
  int get_vertex_u();
  int get_vertex_v();

  // Get vertex information (used for branching on colors)
  int get_n_total_vertices();     // Get the number of vertices of the original graph
  int get_current_vertex(int i);  // Get the current vertex that stands for the original vertex i

  private:

  MWSSgraphpnt G;                     // Pointer to Sewell's graph
                                      // Recall that vertices start at 1
  std::vector<int> w;                 // Cost vector 
  std::vector<int> K;                 // Vector of indistinguishable colors (K is a subset of [0,...,w.size() - 1])
  std::vector<std::vector<int>> C;    // For each 0 <= i < K.size(), C[i] is the vector with the equivalence class of color K[i]
  std::vector<nodepntArray> V;        // For each 0 <= i < K.size(), V[i] has all the vertices having K[i] in their list

  std::vector<std::list<nodepntArray>> global_pool; // For each 0 <= i < K.size(), global_pool[i] is a list with all the maximal stable sets of V[i] globally found
  std::vector<std::list<nodepntArray>> local_pool;  // For each 0 <= i < K.size(), local_pool[i] is a list with all the maximal stable sets of V[i] locally found

  BRANCH_STATUS st;   // Branching status

  int branch_vertex_u;       // Vertex u
  int branch_vertex_v;       // Vertex v
  int branch_color_k;        // Color k
  
  std::vector<int>  vertex_mapping; // For each i (original vertex), vertex_mapping[i] is the vertex that stands for i in the current graph

  // Read graph
  void read_graph(char *filename);

  // Read costs
  void read_costs(char *filename);

  // Read lists
  void read_lists(char *filename);

  // Heuristic resolution of the MWSSP in V[i]
  //  We only explore the stable sets in stabs[i]
  //  If the answer is yes, then the result is saved in best_stable and n_best_stable
  bool solve_MWSSP_heuristic (int i, double goal, nodepnt **best_stable, int *n_best_stable);

  // Exact resolution of the MWSSP in V[i]
  //  We apply Sewell's algorithm
  //  If the answer is yes, then the result is saved in best_stable and n_best_stable
  bool solve_MWSSP_exact (int i, double goal, nodepnt **best_stable, int *n_best_stable);

  // Maximize best_stable in V[i] and update n_best_stable
  void maximize_stable_set(int i, nodepnt **best_stable, int *n_best_stable);

};

#endif
