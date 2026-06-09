#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

#include "./../../mesh/subdivision/polymorhic.hpp"

#include "./../../mathematics/constant.hpp"
#include "./../../mesh/half-edge.hpp"

namespace Mesh::Subdivision {

	// D. Doo, M. Sabin (1978)
	// Behavior of recursive division surfaces near extraordinary points

	class DooSabin final : public Mesh::Subdivision::Polymorphic {

	public:

		DooSabin() {};

		std::unique_ptr<Mesh::HalfEdge> process(
			std::unique_ptr<Mesh::HalfEdge> const& p_input
		) const override {
			if ( p_input == nullptr ) {
				std::cout << "Error! Can not process a null pointer.\n";
				return nullptr;
			}

			// Check that all edges have a next and twin (closed surface)
			for ( auto const& e : p_input->edge )
				if ( e->twin == nullptr || e->next == nullptr || e->previous == nullptr ) {
					std::cout << "Error! Doo-Sabin algorithm can only process a closed surface mesh.\n";
					return nullptr;
				}

			// Check that all vertices is a "corner" (3+ polygons)
			for ( auto const& v : p_input->vertex )
				// 3 polygons + itself
				if ( v.use_count() < 4 ) {
					std::cout << "Error! Doo-Sabin algorithm need vertices with at least three (3) edges.\n";
					return nullptr;
				}

			// Subdivided mesh
			std::unique_ptr<Mesh::HalfEdge> mesh = std::make_unique<Mesh::HalfEdge>();

			// Scheme uses three (3) different polygon constructions
			// From polygons
			for ( auto const& poly : p_input->polygon ) {
				std::vector<std::shared_ptr<Mesh::Data::Edge>> data;
				for ( auto const& edge : poly->edge ) {
					auto const vertex = mesh->add_vertex( CalculateVertex( edge ) );
					data.emplace_back( std::make_shared<Mesh::Data::Edge>( vertex ) );
				}
				mesh->add_polygon( data );
			}

			// From edges
			for ( auto const& edge : p_input->edge ) {
				// Only generate from twinned edges once
				if ( edge < edge->twin ) {
					auto const& v1 = mesh->add_vertex( CalculateVertex( edge ) );
					auto const& v4 = mesh->add_vertex( CalculateVertex( edge->next ) );
					auto const& v3 = mesh->add_vertex( CalculateVertex( edge->twin ) );
					auto const& v2 = mesh->add_vertex( CalculateVertex( edge->twin->next ) );
					mesh->add_polygon( {
							std::make_shared<Mesh::Data::Edge>( v1 ),
							std::make_shared<Mesh::Data::Edge>( v2 ),
							std::make_shared<Mesh::Data::Edge>( v3 ),
							std::make_shared<Mesh::Data::Edge>( v4 )
						} );
				}
			}

			// From vertices
			for ( auto const& vertex : p_input->vertex ) {
				// Use count (without itself) is number of edges,
				// and the number of edges in the new polygon
				std::size_t const n = vertex.use_count() - 1;

				std::vector<std::shared_ptr<Mesh::Data::Edge>> data;

				// Get an edge from a vertex
				std::shared_ptr<Mesh::Data::Edge> edge{ nullptr };
				for ( auto const& e : p_input->edge )
					if ( e->vertex == vertex )
						edge = e;

				for ( std::size_t i{ 0 }; i < n; ++i ) {
					// Should never happen, but check anyway
					if ( edge == nullptr ) {
						std::cout << "Error! Doo algorithm encountered a null pointer edge.\n";
						return nullptr;
					}
					auto const vertex = mesh->add_vertex( CalculateVertex( edge ) );
					// Edges are processed in reverse order, so insert at front
					data.emplace( data.begin(), std::make_shared<Mesh::Data::Edge>( vertex ) );
					// Set next edge to process
					edge = edge->twin->next;
				}
				mesh->add_polygon( data );
			}

			// Finish mesh data
			mesh->connect_shared_edges();
			return mesh;
		};

	private:

		// All new vertices in this scheme is calculated the same way
		Double3 CalculateVertex(
			std::shared_ptr<Mesh::Data::Edge> const& p_edge
		) const {
			// Polygon centre
			Double3 const centre = p_edge->polygon->centre();
			// Edge end point
			Double3 const v1 = p_edge->vertex->location;
			// Other edge end points
			Double3 const v2 = p_edge->next->vertex->location;
			Double3 const v3 = p_edge->previous->vertex->location;

			//return ( v1 + ( v1 + v2 ) / 2. + ( v1 + v3 ) / 2. + centre ) / 4.;
			return ( v1 * 2. + ( v2 + v3 ) / 2. + centre ) / 4.;
		};

	};

};
