#pragma once

#include <memory>

#include "./../../mesh/half-edge.hpp"

namespace Mesh::Subdivision {

	class Polymorphic {

	public:

		// Apply the algorithm to the mesh, and return a new mesh.
		// A nullptr is returned, if the input can not be processed.
		virtual std::unique_ptr<Mesh::HalfEdge> process(
			std::unique_ptr<Mesh::HalfEdge> const& p_input
		) const = 0;

	};

};
