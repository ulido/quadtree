#define NODE_CAPACITY 4

typedef struct {
  double center[2];
  double half_dim;
} AABB;

struct QuadTreeNode {
  AABB boundary;

  double points[2*NODE_CAPACITY];
  unsigned int size;

  struct QuadTreeNode* northwest;
  struct QuadTreeNode* northeast;
  struct QuadTreeNode* southwest;
  struct QuadTreeNode* southeast;
};
typedef struct QuadTreeNode QuadTreeNode;

int AABB_contains(const AABB *aabb, const double *p);
int AABB_intersects(const AABB *this, const AABB *other);

QuadTreeNode* QuadTreeNode_new(const AABB boundary);
void QuadTreeNode_free(QuadTreeNode* qt);
int QuadTreeNode_insert(QuadTreeNode *qt, const double* p);
double *QuadTreeNode_query(const QuadTreeNode *qt, const AABB *range, unsigned int* res_size);
void QuadTreeNode_debug_traverse(const QuadTreeNode *qt, int level);

