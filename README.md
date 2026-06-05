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

##### Algorithms

| Algorithm | N-gon | Surface | M/T |
| --- | --- | --- | --- |
|Butterfly|3|Any|Yes|
|Butterfly, tensor|3|Any|Yes|
|Doo|3+|Closed|No|

*N-gon*:\
Number of edges needed.\
Either a fixed value, or a minimal.\
Special case e3, process will turn mesh polygons into triangles.

*Surface*:\
Types of surfaces.\
Closed, all edges are connected to two (2) polygons.\
Open, one or two polygon(s) per edge.\
Any, same as open, supports hard/creased edges.

*M/T*:\
Support of optional surface data.\
If not supported; all material, texture, smooth shading and hard/creased edge data is lost.

##### Algorithms

*Butterfly scheme*:\
Nira Dyn, David Levine, John A. Gregory (1990)\
A butterfly subdivision scheme for surface interpolation with tension control

*Doo*:\
D. W. H. Doo (1978)\
A subdivision algorithm for smoothing down irregularly shaped polyhedrons

##### Dependencies

- C++20

- [half-edge](https://github.com/Thomas-Klietsch/half-edge)

##### To do

More algorithms.

Butterfly w matrix.

Improved butterfly.

*Catmull-Clark*:\
E. Catmull, J Clark (1978)\
Recursively generated B-spline surfaces on arbitrary topological meshes

*DDS*:\
???

*Doo-Sabin*:\
D. Doo, M. Sabin (1978)\
Behavior of recursive division surfaces near extraordinary points

*Habib-Warren (AKA midedge)*:\
A. Habib, J. Warren (1999)\
Edge and vertex insertion for a class of C1 subdivision surfaces

*Kobbelt*:\
???

*Loop*:\
Loop, Charles Teorell (1987)\
Smooth Subdivision Surfaces Based on Triangles

*Peters-Reif (AKA midedge)*:\
J. Peters, U. Reif (1997)\
The simplest subdivision scheme for smoothing polyhedra

*Root3*:\
Kobbelt, Leif (2000)\
√3-subdivision
