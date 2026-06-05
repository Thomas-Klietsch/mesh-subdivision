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

#include <iostream>
#include <memory>
#include <string>

#include "./mesh/half-edge.hpp"
#include "./mesh/export.hpp"
#include "./mesh/import.hpp"
#include "./mesh/subdivision/polymorhic.hpp"
#include "./mesh/subdivision/butterfly.hpp"
#include "./mesh/subdivision/butterfly_tensor.hpp"
#include "./mesh/subdivision/doo.hpp"

int main(
	[[maybe_unused]] int argc,
	[[maybe_unused]] char* argv[]
) {
	// Load mesh into half-data, with twin edge validation
	// Triangle polygons
	// std::unique_ptr<Mesh::HalfEdge> const p_mesh
	// 	= Mesh::Import( "cube_tri.obj", Mesh::FileType::Wavefront, true );
	// Quad polygons
	std::unique_ptr<Mesh::HalfEdge> const p_mesh
		= Mesh::Import( "cube.obj", Mesh::FileType::Wavefront, true );

	if ( !p_mesh ) {
		std::cout << "Failed to import half edge data.\n";
		return EXIT_FAILURE;
	}

	// Create Doo subdivision algorithm, with optional value of 0.6
	std::unique_ptr<Mesh::Subdivision::Polymorphic> p_algorithm
		= std::make_unique<Mesh::Subdivision::Doo>( 0.6 );

	// Perform three subdivisions
	auto p_subdiv = p_algorithm->process( p_mesh );
	p_subdiv = p_algorithm->process( p_subdiv );
	p_subdiv = p_algorithm->process( p_subdiv );

	if ( !p_subdiv ) {
		std::cout << "Failed to subdivide half edge data.\n";
		return EXIT_FAILURE;
	}

	// Set all polygons to smooth shading,
	// since the Doo algorithm does not support it.
	for ( auto& p : p_subdiv->polygon )
		p->f_smooth = true;

	// Save half-data as Wavefront data, with weighted vertex normals
	if ( !Mesh::Export( "mesh-subdiv.obj", p_subdiv, Mesh::FileType::Wavefront, Mesh::VertexNormal::Weighted ) ) {
		std::cout << "Failed to export half edge data.\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
};
