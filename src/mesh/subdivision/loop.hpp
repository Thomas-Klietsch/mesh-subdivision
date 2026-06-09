#pragma once

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

#include "./../../mesh/subdivision/polymorhic.hpp"

#include "./../../mathematics/constant.hpp"
#include "./../../mesh/half-edge.hpp"

namespace Mesh::Subdivision {

	// Loop, Charles Teorell (1987)
	// Smooth Subdivision Surfaces Based on Triangles

	class Loop final : public Mesh::Subdivision::Polymorphic {

	private:

		// Pre calculated alpha values, 10 is probably enough.
		std::double_t alpha[32];

	public:

		Loop() {
			// M={3/8,1/8,0,...,0,1/8} (4.7)
			// A={1/n,1/n,...,1/n}, (4.9)

			// alpha_6 = 5/8, beta_6 = 1/2
			alpha[0] = 0.;
			for ( std::uint8_t n{ 1 }; n < 32; ++n ) {
				// (4.31)
				std::double_t const r = 3. / 8. + std::cos( two_pi / n ) / 4.;
				alpha[n] = r * r + 3. / 8.;
				// (4.16)
				// beta[n]=3./(11.-8.*alpha[n]);
			}
		};

		std::unique_ptr<Mesh::HalfEdge> process(
			std::unique_ptr<Mesh::HalfEdge> const& p_input
		) const override {
			if ( p_input == nullptr ) {
				std::cout << "Error! Can not process a null pointer.\n";
				return nullptr;
			}

			// Check that all edges have a next and previous
			for ( auto const& e : p_input->edge )
				if ( e->next == nullptr || e->previous == nullptr ) {
					std::cout << "Error! Loop algorithm can only process a closed surface mesh.\n";
					return nullptr;
				}

			// Check that all polygons are triangles
			for ( auto const& p : p_input->polygon )
				if ( p->edge.size() != 3 ) {
					std::cout << "Error! Loop algorithm only support polygons with three (3) edges.\n";
					return nullptr;
				}

			// Check that all vertices orders are covered.
			// Highly unlikely to fail.
			for ( auto const& v : p_input->vertex )
				// 32 polygons/edges + itself
				if ( v.use_count() > 32 ) {
					std::cout << "Error! Loop algorithm vertex order is limited to 32.\n";
					return nullptr;
				}

			// Subdivided mesh
			std::unique_ptr<Mesh::HalfEdge> mesh = std::make_unique<Mesh::HalfEdge>();

			for ( auto const& p : p_input->polygon ) {
				// Current polygon vertices
				auto const v1 = mesh->add_vertex( CalculateVertex( p->edge[0]->vertex, p_input ) );
				auto const v2 = mesh->add_vertex( CalculateVertex( p->edge[1]->vertex, p_input ) );
				auto const v3 = mesh->add_vertex( CalculateVertex( p->edge[2]->vertex, p_input ) );
				// Edge boundary state
				bool const f_edge1 = p->edge[0]->is_boundary(); // v1->v2
				bool const f_edge2 = p->edge[1]->is_boundary();
				bool const f_edge3 = p->edge[2]->is_boundary();
				// Generate new edge vertices
				auto const edge_v1 = mesh->add_vertex( CalculateEdgePoint( p->edge[0] ) );
				auto const edge_v2 = mesh->add_vertex( CalculateEdgePoint( p->edge[1] ) );
				auto const edge_v3 = mesh->add_vertex( CalculateEdgePoint( p->edge[2] ) );

				// Texture is initialised to nothing
				std::shared_ptr<Mesh::Data::Texture> uv1{ nullptr };
				std::shared_ptr<Mesh::Data::Texture> uv2{ nullptr };
				std::shared_ptr<Mesh::Data::Texture> uv3{ nullptr };
				std::shared_ptr<Mesh::Data::Texture> edge_uv1{ nullptr };
				std::shared_ptr<Mesh::Data::Texture> edge_uv2{ nullptr };
				std::shared_ptr<Mesh::Data::Texture> edge_uv3{ nullptr };

				// If texture is define for original polygon
				if ( p->is_textured() ) {
					// Get original texture data
					uv1 = mesh->add_texture( p->edge[0]->texture->location );
					uv2 = mesh->add_texture( p->edge[1]->texture->location );
					uv3 = mesh->add_texture( p->edge[2]->texture->location );
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
					p->material_index,
					p->f_smooth );
				mesh->add_polygon( {
					std::make_shared<Mesh::Data::Edge>( v2, f_edge2, uv2 ),
					std::make_shared<Mesh::Data::Edge>( edge_v2, f_edge2 & f_edge1, edge_uv2 ),
					std::make_shared<Mesh::Data::Edge>( edge_v1, f_edge1, edge_uv1 ) },
					p->material_index,
					p->f_smooth );
				mesh->add_polygon( {
					std::make_shared<Mesh::Data::Edge>( v3, f_edge3, uv3 ),
					std::make_shared<Mesh::Data::Edge>( edge_v3, f_edge3 & f_edge2, edge_uv3 ),
					std::make_shared<Mesh::Data::Edge>( edge_v2, f_edge2, edge_uv2 ) },
					p->material_index,
					p->f_smooth );
				mesh->add_polygon( {
					std::make_shared<Mesh::Data::Edge>( edge_v1, f_edge1 & f_edge2, edge_uv1 ),
					std::make_shared<Mesh::Data::Edge>( edge_v2, f_edge2 & f_edge3, edge_uv2 ),
					std::make_shared<Mesh::Data::Edge>( edge_v3, f_edge3 & f_edge1, edge_uv3 ) },
					p->material_index,
					p->f_smooth );
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

		// Calculate location of the new vertex
		Double3 CalculateVertex(
			std::shared_ptr<Mesh::Data::Vertex> const& vertex,
			std::unique_ptr<Mesh::HalfEdge> const& p_input
		) const {
			// Number of edges/adjacent vertices, i.e. order
			std::uint32_t n{ 0 };
			// Sum of adjacent vertices
			Double3 q{ Double3::Zero };

			// TODO better algorithm
			// Find all edges starting from vertex
			for ( auto edge : p_input->edge )
				if ( vertex == edge->vertex ) {
					// If any edge is a boundary, the vertex can not be moved
					if ( edge->is_boundary() )
						return vertex->location;

					// Add adjacent vertex to sum
					q += edge->next->vertex->location;
					++n;
				}

			// New vertex V = (Old Vertex) * alpha[n] + (Sum of all adjacent vertices/vertex order) * (1-alpha[n])
			return vertex->location * alpha[n] + q * ( 1. - alpha[n] ) / n;
		};

		// Calculate edge middle point
		Double3 CalculateEdgePoint(
			std::shared_ptr<Mesh::Data::Edge> const& edge // E1
		) const {
			// p1+p2
			Double3 q = ( edge->vertex->location + edge->next->vertex->location );

			if ( edge->is_boundary() )
				return q * 0.5;

			// Non-boundary (p1+p2)*3/8 + (p3+p4)*1/8
			return ( q * 3. + Corner( edge ) + Corner( edge->twin ) ) / 8.;
		};

	};

};
