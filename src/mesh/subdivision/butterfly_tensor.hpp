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

	// Nira Dyn, David Levine, John A. Gregory (1990)
	// A butterfly subdivision scheme for surface interpolation with tension control

	class ButterflyTensor  final : public Mesh::Subdivision::Polymorphic {

	private:

		//     | w11 w12 w13 |   | w1 |
		// w = | w21 w22 w23 | = | w2 |
		//     | w31 w32 w33 |   | w3 |

		// Default Butterfly tensor
		Double3 const w1{ 1. / 16., 0., 0. };
		Double3 const w2{ 0., 1. / 16., 0. };
		Double3 const w3{ 0., 0., 1. / 16. };

	public:

		ButterflyTensor() {};

		ButterflyTensor(
			Double3 const& w1,
			Double3 const& w2,
			Double3 const& w3
		)
			: w1( w1 )
			, w2( w2 )
			, w3( w3 )
		{};

		std::unique_ptr<Mesh::HalfEdge> process(
			std::unique_ptr<Mesh::HalfEdge> const& p_input
		) const override {
			if ( p_input == nullptr ) {
				std::cout << "Error! Can not process a null pointer.\n";
				return nullptr;
			}

			// Check if data can be processed
			for ( auto const& p : p_input->polygon ) {
				if ( p->edge.size() != 3 ) {
					std::cout << "Error! Butterfly algorithm can only process a triangle mesh.\n";
					return nullptr;
				}
				for ( auto const& e : p->edge )
					if ( e->next == nullptr || e->previous == nullptr) {
						std::cout << "Error! Butterfly algorithm can only process closed loop polygons.\n";
						return nullptr;
					}
			}

			// Subdivided mesh, with copy of material data
			std::unique_ptr<Mesh::HalfEdge> mesh = std::make_unique<Mesh::HalfEdge>( p_input );

			// Process polygons
			for ( auto const& poly : p_input->polygon ) {
				// Current polygon vertices
				auto const v1 = mesh->add_vertex( poly->edge[0]->vertex->location );
				auto const v2 = mesh->add_vertex( poly->edge[1]->vertex->location );
				auto const v3 = mesh->add_vertex( poly->edge[2]->vertex->location );
				// Edge boundary state
				bool const f_edge1 = poly->edge[0]->is_boundary(); // v1->v2
				bool const f_edge2 = poly->edge[1]->is_boundary();
				bool const f_edge3 = poly->edge[2]->is_boundary();
				// Generate new edge vertices
				auto const edge_v1 = mesh->add_vertex( CalculateEdgePoint( poly->edge[0] ) );
				auto const edge_v2 = mesh->add_vertex( CalculateEdgePoint( poly->edge[1] ) );
				auto const edge_v3 = mesh->add_vertex( CalculateEdgePoint( poly->edge[2] ) );

				// Texture is initialised to nothing
				std::shared_ptr<Mesh::Data::Texture> uv1{ nullptr };
				std::shared_ptr<Mesh::Data::Texture> uv2{ nullptr };
				std::shared_ptr<Mesh::Data::Texture> uv3{ nullptr };
				std::shared_ptr<Mesh::Data::Texture> edge_uv1{ nullptr };
				std::shared_ptr<Mesh::Data::Texture> edge_uv2{ nullptr };
				std::shared_ptr<Mesh::Data::Texture> edge_uv3{ nullptr };

				// If texture is define for original polygon
				if ( poly->is_textured() ) {
					// Get original texture data
					uv1 = mesh->add_texture( poly->edge[0]->texture->location );
					uv2 = mesh->add_texture( poly->edge[1]->texture->location );
					uv3 = mesh->add_texture( poly->edge[2]->texture->location );
					// Calculate edge texture data
					edge_uv1 = mesh->add_texture( uv1->location + ( uv2->location - uv1->location ) * .5 );
					edge_uv2 = mesh->add_texture( uv2->location + ( uv3->location - uv2->location ) * .5 );
					edge_uv3 = mesh->add_texture( uv3->location + ( uv1->location - uv3->location ) * .5 );
				}

				// Add new polygons.
				// Boundary state of new (midpoint) edges between two old edges,
				// depend on both old edge states.
				mesh->add_polygon( {
					std::make_shared<Mesh::Data::Edge>( v1, f_edge1, uv1 ),
					std::make_shared<Mesh::Data::Edge>( edge_v1, f_edge1 & f_edge3, edge_uv1 ),
					std::make_shared<Mesh::Data::Edge>( edge_v3, f_edge3, edge_uv3 ) },
					poly->material_index,
					poly->f_smooth );
				mesh->add_polygon( {
					std::make_shared<Mesh::Data::Edge>( v2, f_edge2, uv2 ),
					std::make_shared<Mesh::Data::Edge>( edge_v2, f_edge2 & f_edge1, edge_uv2 ),
					std::make_shared<Mesh::Data::Edge>( edge_v1, f_edge1, edge_uv1 ) },
					poly->material_index,
					poly->f_smooth );
				mesh->add_polygon( {
					std::make_shared<Mesh::Data::Edge>( v3, f_edge3, uv3 ),
					std::make_shared<Mesh::Data::Edge>( edge_v3, f_edge3 & f_edge2, edge_uv3 ),
					std::make_shared<Mesh::Data::Edge>( edge_v2, f_edge2, edge_uv2 ) },
					poly->material_index,
					poly->f_smooth );
				mesh->add_polygon( {
					std::make_shared<Mesh::Data::Edge>( edge_v1, f_edge1 & f_edge2, edge_uv1 ),
					std::make_shared<Mesh::Data::Edge>( edge_v2, f_edge2 & f_edge3, edge_uv2 ),
					std::make_shared<Mesh::Data::Edge>( edge_v3, f_edge3 & f_edge1, edge_uv3 ) },
					poly->material_index,
					poly->f_smooth );
			}

			// Finish mesh data
			mesh->connect_shared_edges();
			return mesh;
		};

	private:

		// Return location for vertex opposite to edge.
		// Only valid for triangles.
		Double3 Corner(
			std::shared_ptr<Mesh::Data::Edge> const& p_edge
		) const {
			return p_edge->previous->vertex->location;
			// Alternative:
			// return p_edge->next->next->vertex->location;
		};

		// Calculate middle point, and offset if non-boundary
		Double3 CalculateEdgePoint(
			std::shared_ptr<Mesh::Data::Edge> const& edge // E1
		) const {
			// Edges for two connected triangles (E1<->E4):
			//     ^
			// E3 / \ E2
			//    --- E1
			//    --- E4
			// E5 \ / E6
			//     v

			// If boundary edge: q = .5(p1+p2)
			// Else: q = .5(p1+p2) + 2w(p3+p4) - w(p5+p6+p7+p8), w=1/16
			//
			// p1,p2 edge vertices (shared by both main triangles, inner edge)
			// p3,p4 remaining vertices from main triangles
			// p5-p8 remaining vertices from neighbouring triangles
			//       (4 outer edges of main triangles)

			// 0.5 ( p1 + p2 )
			Double3 const q = ( edge->vertex->location + edge->next->vertex->location ) * .5;

			// If edge is a boundary, return q as middle point
			if ( edge->is_boundary() )
				return q;

			// Calculate s (13)
			Double3 s{ Double3::Zero };

			// Factor of 2
			s += Corner( edge ) * 2.; // p3 from E1
			s += Corner( edge->twin ) *2.; // p4 from E4

			// Factor of 1
			if ( edge->next->twin ) // E2->twin
				s -= Corner( edge->next->twin ); // p7
			if ( edge->next->next->twin ) // E3->twin
				s -= Corner( edge->next->next->twin ); // p5
			if ( edge->twin->next->twin ) // E5->twin
				s -= Corner( edge->twin->next->twin ); // p6
			if ( edge->twin->next->next->twin ) // E6->twin
				s -= Corner( edge->twin->next->next->twin ); // p8

			// Calculate ws
			Double3 const ws{
				w1.dot( s ),
				w2.dot( s ),
				w3.dot( s )
			};
			
			return q + ws;
		};

	};

};
