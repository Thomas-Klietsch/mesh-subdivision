#pragma once

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

	class CatmullClark final : public Mesh::Subdivision::Polymorphic {

	public:

		CatmullClark() {};

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

			// Face point: centre of polygon
			// Edge point: average( edge mid point + face points )
			// Vertex point: m1*vertex + m2*average_poly + m3*avrg_middle_edges
			// m1=(n-3)/n m2=1/n m3=2/n
			// n: valence, number of edges

			for ( auto const& p : p_input->polygon ) {
				// (A) New face point, common point
				Double3 const centre = p->centre();
				auto const v1 = mesh->add_vertex( centre );

				for ( auto const& edge : p->edge ) {
					// (B) New edge point
					Double3 const edge1 = CalculateEdgePoint( edge->previous );
					auto const v2 = mesh->add_vertex( edge1 );

					// (B) New edge point
					Double3 const edge2 = CalculateEdgePoint( edge );
					auto const v4 = mesh->add_vertex( edge2 );

					// Current edge boundary states
					bool const f_edge1 = edge->previous->is_boundary(); // v2->v3
					bool const f_edge2 = edge->is_boundary(); // v3->v4

					// (C) New vertex point
					Double3 const vertex = CalculateVertex( edge->vertex, p_input );
					auto const v3 = mesh->add_vertex( vertex );

					mesh->add_polygon( {
						std::make_shared<Mesh::Data::Edge>( v1, f_edge1 ),
						std::make_shared<Mesh::Data::Edge>( v2, f_edge1 ),
						std::make_shared<Mesh::Data::Edge>( v3, f_edge2 ),
						std::make_shared<Mesh::Data::Edge>( v4, f_edge2 )
						},
						p->material_index,
						p->f_smooth
					);
				}
			}

			// Finish mesh data
			mesh->connect_shared_edges();
			return mesh;
		}

	private:

		Double3 CalculateEdgePoint(
			std::shared_ptr<Mesh::Data::Edge> const& edge
		) const {
			Double3 const v1 = edge->vertex->location;
			Double3 const v2 = edge->next->vertex->location;
			if ( edge->is_boundary() )
				return ( v1 + v2 ) / 2.;

			return ( v1 + v2 + edge->polygon->centre() + edge->twin->polygon->centre() ) / 4.;
		};

		Double3 CalculateVertex(
			std::shared_ptr<Mesh::Data::Vertex> const& p_vertex,
			std::unique_ptr<Mesh::HalfEdge> const& p_input
		) const {
			// Valence
			std::uint32_t n{ 0 };

			// Old vertex location
			Double3 const p = p_vertex->location;

			Double3 sum_edge{ Double3::Zero };
			Double3 sum_centre{ Double3::Zero };

			// FIXME better search algorithm
			// Find all edges with p_vertex
			for ( auto const& edge : p_input->edge ) {
				if ( p_vertex == edge->vertex ) {
					// Check for hard edge
					if ( edge->is_boundary() )
						return p;

					++n;
					sum_edge += p + ( edge->next->vertex->location - p ) / 2.;
					sum_centre += edge->polygon->centre();
				}
			}

			sum_edge /= static_cast<std::double_t>( n );
			sum_centre /= static_cast<std::double_t>( n );
			// n<=3, tested at start of process()
			return ( p * ( n - 3 ) + sum_edge * 2. + sum_centre ) / static_cast<std::double_t>( n );
		};

	};

};
