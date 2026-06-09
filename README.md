##### Introduction

Subdivision of mesh (half edge data), using various algorithms.

Extension to [half-edge](https://github.com/Thomas-Klietsch/half-edge) (code not included, note extension requires C++20).

##### Usage

Load mesh into half-data:

    std::unique_ptr<Mesh::HalfEdge> p_mesh = Mesh::Import( "cube_tri.obj", Mesh::Type::Wavefront );

Select an algorithm (for example Doo with optional value):

   	std::unique_ptr<Mesh::Subdivision::Polymorphic> p_algorithm = std::make_unique<Mesh::Subdivision::Doo>( 0.6 );

Perform one subdivision (note that process() will return a nullptr on failure):

	auto p_subdiv = p_algorithm->process( p_mesh );

Save subdivided mesh:

	Mesh::Export( "mesh-subdiv.obj", p_subdiv, Mesh::FileType::Wavefront, Mesh::VertexNormal::Weighted );

##### Overview

| Algorithm | N-gon | Surface | M/T | Optional |
| --- | --- | --- | --- | --- |
|Butterfly|3|Any|All|Shading|
|ButterflyTensor|3|Any|All|Shading|
|CatmullClark|3+|Any|Material|Shading|
|CatmullClarkQuad|4*|Closed|None|None|
|Doo|3+|Closed|None|None|
|DooSabin|3+|Closed|None|None|
|Loop|3|Any|All|Shading|

*N-gon*:\
Number of edges needed.\
Either a fixed value, or a minimal.\
Special cases:\
3*, process will turn mesh polygons into triangles.\
4*, vertices must be order 4 (e.g. have four edges).

*Surface*:\
Types of surfaces.\
Closed, all edges are connected to two (2) polygons.\
Open, one or two polygon(s) per edge.\
Any, same as open, supports sharp/hard/creased edges.

*M/T*:\
Support for material and/or texture coordinates.\
If not supported, the data is is lost.

*Optional*:\
Shading, if polygon is smooth or flat shaded. Lost if not supported\

##### Algorithms

*Butterfly*:\
*Butterfly-Tensor*:\
Nira Dyn, David Levine, John A. Gregory (1990)\
A butterfly subdivision scheme for surface interpolation with tension control

*Catmull-Clark*:\
*Catmull-Clark-Quad*:\
E. Catmull, J Clark (1978)\
Recursively generated B-spline surfaces on arbitrary topological meshes

*Doo*:\
D. W. H. Doo (1978)\
A subdivision algorithm for smoothing down irregularly shaped polyhedrons

*Doo-Sabin*:\
D. Doo, M. Sabin (1978)\
Behavior of recursive division surfaces near extraordinary points

*Loop*:\
Loop, Charles Teorell (1987)\
Smooth Subdivision Surfaces Based on Triangles

##### Dependencies

- C++20

- [half-edge](https://github.com/Thomas-Klietsch/half-edge)

##### To do

More algorithms...

*Improved butterfly*:\
???

*DDS*:\
???

*Habib-Warren (AKA midedge)*:\
A. Habib, J. Warren (1999)\
Edge and vertex insertion for a class of C1 subdivision surfaces

*Kobbelt*:\
???

*Peters-Reif (AKA midedge)*:\
J. Peters, U. Reif (1997)\
The simplest subdivision scheme for smoothing polyhedra

*Root3*:\
Kobbelt, Leif (2000)\
√3-subdivision

Chaikin (1974), An algorithm for high speed generation

order 4