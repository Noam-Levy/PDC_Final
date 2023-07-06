typedef struct {
  int id;
  float x1, x2, a, b;
} Point;

typedef struct {
  int N;        // number of points in the set
  int K;        // minimal number of points to satisfy the proximity criteria
  int tCount;   // time interval
  float D;      // maximum distance btween points to satisfy criteria
  Point** points;
} metadata;

metadata* readData(char *path);
