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

	// E. Catmull, J Clark (1978)
	// Recursively generated B-spline surfaces on arbitrary topological meshes

	class CatmullClarkQuad final : public Mesh::Subdivision::Polymorphic {

	public:

		CatmullClarkQuad() {};

		std::unique_ptr<Mesh::HalfEdge> process(
			std::unique_ptr<Mesh::HalfEdge> const& p_input
		) const override {
			if ( p_input == nullptr ) {
				std::cout << "Error! Can not process a null pointer.\n";
				return nullptr;
			}

			// Check that all edges have a next and twin (closed surface)
			for ( auto const& p : p_input->polygon )
				if ( p->edge.size() != 4 ) {
					std::cout << "Error! Catmull-Clark-Quad algorithm can only process quad (four sided) polygons.\n";
					return nullptr;
				}

			// Check that all edges have a next and twin (closed surface)
			for ( auto const& e : p_input->edge )
				if ( e->twin == nullptr || e->next == nullptr || e->previous == nullptr ) {
					std::cout << "Error! Catmull-Clark-Quad algorithm can only process a closed surface mesh.\n";
					return nullptr;
				}

			// Check that all vertices is a quad "corner" (4 polygons)
			for ( auto const& v : p_input->vertex )
				// 4 polygons + itself
				if ( v.use_count() != 5 ) {
					std::cout << "Error! Catmull-Clark-Quad algorithm need vertices with at least four (4) edges.\n";
					return nullptr;
				}

			// Subdivided mesh
			std::unique_ptr<Mesh::HalfEdge> mesh = std::make_unique<Mesh::HalfEdge>();

			// Scheme uses three (3) different polygon constructions, figure 11

			// From polygons (q11,q12,q22,q21)
			for ( auto const& poly : p_input->polygon ) {
				std::vector<std::shared_ptr<Mesh::Data::Edge>> data;
				for ( auto const& edge : poly->edge ) {
					auto const vertex = mesh->add_vertex( CalculateVertex( edge ) );
					data.emplace_back( std::make_shared<Mesh::Data::Edge>( vertex ) );
				}
				mesh->add_polygon( data );
			}

			// From edges (q12,q13,q23,q22)
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

			// From vertices (q22,q23,q33,q32)
			for ( auto const& vertex : p_input->vertex ) {
				// Temp data
				std::vector<std::shared_ptr<Mesh::Data::Edge>> data;

				// Get an edge from a vertex
				std::shared_ptr<Mesh::Data::Edge> edge{ nullptr };
				for ( auto const& e : p_input->edge )
					if ( e->vertex == vertex )
						edge = e;

				// Number of corners is fixed at 4
				for ( std::uint8_t i{ 0 }; i < 4; ++i ) {
					// Should never happen, but check anyway
					if ( edge == nullptr ) {
						std::cout << "Error! Catmull-Clark-Quad algorithm encountered a null pointer edge.\n";
						return nullptr;
					}
					auto const& vertex = mesh->add_vertex( CalculateVertex( edge ) );
					// Edges are processed in reverse order, so insert at front
					data.emplace( data.begin(), std::make_shared<Mesh::Data::Edge>( vertex ) );
					edge = edge->twin->next;
				}
				mesh->add_polygon( data );
			}

			// Finish mesh data
			mesh->connect_shared_edges();
			return mesh;
		};

	private:

		// All new vertices in this scheme is calculated the same way.
		// Figure 11x
		Double3 CalculateVertex(
			std::shared_ptr<Mesh::Data::Edge> const& p_edge
		) const {
			// This corner
			Double3 const p11 = p_edge->vertex->location;
			// Next corner
			Double3 const p12 = p_edge->next->vertex->location;
			// Opposite corner
			Double3 const p22 = p_edge->next->next->vertex->location;
			// Previous corner
			Double3 const p21 = p_edge->previous->vertex->location;
			return ( p11 * 9. + ( p12 + p21 ) * 3. + p22 ) / 16.;
		};

	};

};
