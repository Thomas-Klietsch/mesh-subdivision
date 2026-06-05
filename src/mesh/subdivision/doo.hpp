// Copyright (c) 2026 Thomas Klietsch, all rights reserved.
//
// Licensed under the GNU Lesser General Public License, version 3.0 or later
//
// This program is free software: you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or ( at your option ) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General
// Public License along with this program.If not, see < https://www.gnu.org/licenses/>. 

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

	// Primary source, can be somewhat difficult to acquire, found on Wayback Machine
	// D. W. H. Doo (1978)
	// A subdivision algorithm for smoothing down irregularly shaped polyhedrons

	class Doo final : public Mesh::Subdivision::Polymorphic {

	private:

		std::double_t const scalar{ 0.5 };

	public:

		Doo(
			// Optional value, controls location of new vertices,
			// 0 = original vertex, 1 = polygon centre
			std::double_t const a_scalar = 0.5
		)
			: scalar( std::clamp( a_scalar, 0.01, 0.99 ) )
		{};

		std::unique_ptr<Mesh::HalfEdge> process(
			std::unique_ptr<Mesh::HalfEdge> const& p_input
		) const override {
			if ( p_input == nullptr ) {
				std::cout << "Error! Can not process a null pointer.\n";
				return nullptr;
			}

			// Check that all edges have a next and twin (closed surface)
			for ( auto const& e : p_input->edge )
				if ( e->twin == nullptr || e->next == nullptr ) {
					std::cout << "Error! Doo algorithm can only process a closed surface mesh.\n";
					return nullptr;
				}

			// Check that all vertices is a "corner" (3+ polygons)
			for ( auto const& v : p_input->vertex )
				// 3 polygons + itself
				if ( v.use_count() < 4 ) {
					std::cout << "Error! Doo algorithm need vertices with at least three (3) edges.\n";
					return nullptr;
				}

			// Subdivided mesh
			std::unique_ptr<Mesh::HalfEdge> mesh = std::make_unique<Mesh::HalfEdge>();

			// Scheme uses three (3) different polygon constructions

			// From polygons (Type F)
			for ( auto const& poly : p_input->polygon ) {
				std::vector<std::shared_ptr<Mesh::Data::Edge>> data;
				for ( auto const& edge : poly->edge ) {
					auto const vertex = mesh->add_vertex( CalculateVertex( edge ) );
					data.emplace_back( std::make_shared<Mesh::Data::Edge>( vertex ) );
				}
				mesh->add_polygon( data );
			}

			// From edges (Type E)
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

			// From vertices (Type V)
			for ( auto const& vertex : p_input->vertex ) {
				// Use count (without itself) is the number of edges,
				// and the number of edges in the new polygon
				std::size_t const n = vertex.use_count() - 1;

				// Temp data
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
					auto const& vertex = mesh->add_vertex( CalculateVertex( edge ), false );
					// Edges are processed in reverse order, so insert at front
					data.emplace( data.begin(), std::make_shared<Mesh::Data::Edge>( vertex ) );
					edge = edge->twin->next;
				}
				// Alternative to ...emplace(data.begin..., if using emplace_back,
				// is to reverse the vector data (#include <algorithm>),
				// std::reverse(data.begin(), data.end());
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
			// Edge vertex
			Double3 const v1 = p_edge->vertex->location;
			// New vertex
			return v1 + ( centre - v1 ) * scalar;
		};

	};

};
