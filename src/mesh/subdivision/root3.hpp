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

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

#include "./../../mesh/subdivision/polymorhic.hpp"

#include "./../../mathematics/constant.hpp"
#include "./../../mesh/half-edge.hpp"

namespace Mesh::Subdivision {

	// Kobbelt, Leif (2000)
    // √3-subdivision

	class Root3 final : public Mesh::Subdivision::Polymorphic {

	private:

		std::double_t alpha[32];

	public:

		Root3() {
			alpha[0] = 0.;
			for ( std::uint8_t n{ 1 }; n < 32; ++n )
				alpha[n] = ( 4. - 2. * std::cos( two_pi / n ) ) / 9.; // (6)
		}

		std::unique_ptr<Mesh::HalfEdge> process(
			std::unique_ptr<Mesh::HalfEdge> const& p_input
		) const override {
			if ( p_input == nullptr ) {
				std::cout << "Error! Root3 can not process a null pointer.\n";
				return nullptr;
			}

			// Check if data can be processed
			for ( auto const& polygon : p_input->polygon ) {
				for ( auto const& edge : polygon->edge ) {
					if ( edge->next == nullptr || edge->previous == nullptr ) {
						std::cout << "Error! Root3 algorithm can only process closed loop polygons.\n";
						return nullptr;
					}
					// Check that all vertices orders are covered.
					// Highly unlikely to fail.
					if ( edge->vertex.use_count() > 32 ) {
						std::cout << "Error! Root3 algorithm vertex order is limited to 32.\n";
						return nullptr;
					}
				}
			}

			// Subdivided mesh
			std::unique_ptr<Mesh::HalfEdge> mesh = std::make_unique<Mesh::HalfEdge>();

			// All edges generate a new triangle
			for ( auto const& edge : p_input->edge ) {
				Double3 const vertex = edge->vertex->location;
				auto const v1 = mesh->add_vertex( CalculateVertex( edge->vertex, p_input ) );
				auto const v3 = mesh->add_vertex( edge->polygon->centre() );

				if ( edge->is_boundary() ) {
					// Since opposing polygon might not exist,
					// this would lead to missing triangles creation.
					// Instead two triangles are created,
					// using edge point in place of missing polygon centre.
					Double3 const next_vertex = edge->next->vertex->location;
					auto const v4 = mesh->add_vertex( CalculateVertex( edge->next->vertex, p_input ) );
					auto const v2 = mesh->add_vertex( vertex + ( next_vertex - vertex ) / 2. );
					mesh->add_polygon( {
						std::make_shared<Mesh::Data::Edge>( v1 ),
						std::make_shared<Mesh::Data::Edge>( v2 ),
						std::make_shared<Mesh::Data::Edge>( v3) } );
					mesh->add_polygon( {
						std::make_shared<Mesh::Data::Edge>( v3 ),
						std::make_shared<Mesh::Data::Edge>( v2 ),
						std::make_shared<Mesh::Data::Edge>( v4) } );
				}
				else {
					auto const v2 = mesh->add_vertex( edge->twin->polygon->centre() );
					mesh->add_polygon( {
						std::make_shared<Mesh::Data::Edge>( v1 ),
						std::make_shared<Mesh::Data::Edge>( v2 ),
						std::make_shared<Mesh::Data::Edge>( v3) } );
				}
			}

			// Finish mesh data
			mesh->connect_shared_edges();
			return mesh;
		};

	private:

		// Calculate location of the new vertex
		Double3 CalculateVertex(
			std::shared_ptr<Mesh::Data::Vertex> const& vertex,
			std::unique_ptr<Mesh::HalfEdge> const& p_input
		) const {
			// Number of edges/adjacent vertices
			std::uint32_t n{ 0 };
			// Sum of adjacent vertices
			Double3 q{ Double3::Zero };
			
			// Find all edges starting from vertex
			for ( auto const& edge : p_input->edge )
				if ( vertex == edge->vertex ) {
					// If any edge is a boundary, the vertex can not be moved
					if ( edge->is_boundary() )
						return vertex->location;
					
					// Add adjacent vertex to sum
					q += edge->next->vertex->location;
					++n;
				}

			return vertex->location * ( 1. - alpha[n] ) + q * alpha[n] / n; // (2)
		};

	};

};
