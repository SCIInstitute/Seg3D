# Implicit Model

This filter creates a segmentation by generating an implicit surface represented by a radial basis function (RBF) interpolation.

## Detailed Description

In our algorithm, an implicit surface that is represented by a radial basis function (RBF) interpolation is used to generate a segmentation {cite:p}`TO2002`.

As an input, the algorithm is provided with a set of points on the boundary of the object that needs to be segmented from the images.
Each slice should contain at least three points for our algorithm is return a reasonable segmentation.
Our algorithm constructs an approximate function whose value vanishes at the given points and whose value is positive inside the object and negative outside.

In order to construct such a function, a radial basis function interpolation technique is used.
The value of the function is set to be zero at the given points.
On each slice, the polygon with the smallest perimeter passing through all point is constructed (we use an approximate algorithm for the Euclidean traveling salesman problem described below).
This polygon is used to determine  approximate normals on the surface of the object.
We use the approximate normals to construct two sets of points inside and outside the polygon (at some constant distance from the given points).
We then set the value of the function to be some positive and negative constant inside and outside the polygon, respectively.
The thin plate RBF spline is used to interpolate the points and construct a function, f(x), whose level set for f(x)=0 approximately constructs the surface of the object.

The construction of the polygon is described in this paragraph.
The minimization of the perimeter of a polygon passing thought a set of points is the Euclidean traveling salesman problem (TSP).
On each slice, the given points are used to construct a graph in which the points are the nodes and the distance between them are the weights of the corresponding edges.
The approximate algorithm for the Euclidean TSP begins by constructing a minimal spanning tree (MST) of the graph.
The nodes on the tree with an odd number of neighbors are then matched such that the sum of their edge weights is minimized (we use a brute-force algorithm since the number of points are small).
These edges (that connect the matched nodes) are added to the tree, and the new graph (the MST + matched edges) is traversed in the depth-first search (DFS) manner to obtain the polygon.
