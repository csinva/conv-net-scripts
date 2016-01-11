# reading
## watershed
- region starts at local minima and floods up until it meets next watershed
- continue until reach maximum
"watershed" - boundary lines
"catchment basin" - the basins themselves
"plateau" - connected component with same values
- 2 general methods
      - find basins then take set complent
      - compute partition and find boundaries
- 2 methods to compute path a drop would follow
      - immersion algorithm
          - start with minima, increase water level until basins meet
      - distance function 
          - can be defined based on weights of edges
          - assume each pixel which isn't min has a neighbor with lower value
- accuracy
      - digital distances should represent Euclidean distance
      - produces oversegmentation of images
- graphs
     - forest = undirected acyclic graph
     - tree = connected undirected acyclic graph
      - valued graph = like weighted graph, but weights on vertices

## zlateski_2015 paper
- disaffinity graph - smaller weight, more likely to belong to same segment
- R_split - higher means less unnecessary splits
- R_merge - higher means less unnecessary merges
- preprocessing
     1. the edge connecting two basins is assigned the same weight as the minimal edge connecting the basins in the original disaffinity graph
     - preprocess the original disaffinity graph before watershed by setting all edgeweights below T min to a common low value
- watershed algorithm
    - basins of attraction of steepest descent dynamics, and has runtime that is linear in the number of disaffinity graph edges- basins similar to watershed cuts [3],[4]
    - regional minimum is a graph
    - start with strongest edges (lowest weights)
    - steepest descent graph - every vertex has one outgoing edge (the minimum edge)
- dealing with oversegmentation
    - saliency of two adjacent segments is defined as the value of the minimal disaffinity between the vertices of the two segments
      - if saliency < T_min connect the segments
      - therefore everywhere we replace T_min with 0
    - replace anything >T_max with infinity, so they don't connect
    - singleton vertices are considered as background
    - single linkage clustering, each step merges two clusters connected by an edge with the lowest weight in the original graph
      - modify to merge 2 clusters connected by edge with func(weight,cluster_sizes)
    - use size of true segments as prior knowledge
