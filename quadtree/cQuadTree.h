#define NODE_CAPACITY 4

typedef struct {
  double center[2];
  double half_dim;
} AABB;

struct cQuadTreeNode {
  AABB boundary;

  size_t indices[NODE_CAPACITY];
  unsigned int size;

  struct cQuadTreeNode* northwest;
  struct cQuadTreeNode* northeast;
  struct cQuadTreeNode* southwest;
  struct cQuadTreeNode* southeast;
};
typedef struct cQuadTreeNode cQuadTreeNode;

typedef struct {
  cQuadTreeNode *root;
  double* points;
  unsigned long* ids;
  size_t count;
  size_t capacity;
} cQuadTree;

int AABB_contains(const AABB *aabb, const double *p);
int AABB_intersects(const AABB *this, const AABB *other);

cQuadTreeNode* cQuadTreeNode_new(const AABB boundary);
void cQuadTreeNode_free(cQuadTreeNode* qt);
int cQuadTreeNode_insert(cQuadTreeNode *qt, size_t index, const double *p);

cQuadTree* cQuadTree_new(const AABB boundary);
void cQuadTree_free(cQuadTree *qt);
int cQuadTree_insert(cQuadTree *qt, const double *p, unsigned long id);
double* cQuadTree_query(const cQuadTree *qt, const AABB *range, size_t* res_size, unsigned long* ids[]);
double** cQuadTree_query_self(const cQuadTree *qt, const double half_dim, size_t *N, size_t **M);
