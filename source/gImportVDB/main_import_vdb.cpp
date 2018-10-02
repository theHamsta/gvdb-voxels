//----------------------------------------------------------------------------------
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this
//    list of conditions and the following disclaimer in the documentation and/or
//    other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may
//    be used to endorse or promote products derived from this software without specific
//   prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Version 1.0: Rama Hoetzlein, 5/1/2017
//----------------------------------------------------------------------------------

#include "gvdb.h"
using namespace nvdb;

#include <stdlib.h>
#include <stdio.h>
// #include <conio.h>
// #include "file_png.h"		// sample utils

#include <openvdb/openvdb.h>
#include <openvdb/io/File.h>

VolumeGVDB gvdb;

int main()
{
	int w = 1024, h = 768;

	// Initialize GVDB
	printf ( "Starting GVDB.\n" );
	gvdb.SetVerbose ( true );		// enable/disable console output from gvdb
	gvdb.SetCudaDevice ( GVDB_DEV_FIRST );
	gvdb.Initialize ();
	printf ( "Starting GVDB.\n" );
	gvdb.AddPath ( "/local/xu29mapu/projects/gvdb-voxels/source/shared_assets" );

	// Load VDB
	char scnpath[1024];

	if ( !gvdb.getScene()->FindFile ( "/local/xu29mapu/projects/gvdb-voxels/source/shared_assets/" "bunny.vdb", scnpath ) ) {
		gprintf ( "Cannot find vdb file.\n" );
		gerror();
	}

	printf ( "Loading VDB. %s\n", scnpath );
	gvdb.SetChannelDefault ( 16, 16, 1 );

	if ( !gvdb.LoadVDB ( scnpath ) ) {                	// Load OpenVDB format
		gerror();
	}

	gvdb.SaveVBX ( "bunny.vbx" );						// Save VBX format

	// At this point, data was converted from vdb to vbx format.
	// The vbx file can now be used by gvdb (e.g. render using InteractiveGL)

	// Remainder of this sample demonstrates level set rendering to file.

	// Set volume params
	gvdb.getScene()->SetSteps ( 0.25, 16, 0.25 );				// Set raycasting steps
	gvdb.getScene()->SetExtinct ( -1.0f, 1.5f, 0.0f );		// Set volume extinction
	gvdb.getScene()->SetVolumeRange ( 0.0f, 1.0f, -1.0f );	// Set volume value range (for a level set)
	gvdb.getScene()->SetCutoff ( 0.005f, 0.01f, 0.0f );
	gvdb.getScene()->LinearTransferFunc ( 0.00f, 0.25f, Vector4DF( 1, 1, 0, 0.05f ), Vector4DF( 1, 1, 0, 0.03f ) );
	gvdb.getScene()->LinearTransferFunc ( 0.25f, 0.50f, Vector4DF( 1, 1, 1, 0.03f ), Vector4DF( 1, 0, 0, 0.02f ) );
	gvdb.getScene()->LinearTransferFunc ( 0.50f, 0.75f, Vector4DF( 1, 0, 0, 0.02f ), Vector4DF( 1, .5f, 0, 0.01f ) );
	gvdb.getScene()->LinearTransferFunc ( 0.75f, 1.00f, Vector4DF( 1, .5f, 0, 0.01f ), Vector4DF( 0, 0, 0, 0.005f ) );
	gvdb.getScene()->SetBackgroundClr ( 0, 0, 0, 1 );
	gvdb.CommitTransferFunc ();


	Camera3D* cam = new Camera3D;						// Create Camera
	cam->setFov ( 30.0 );
	cam->setOrbit ( Vector3DF( -10, 30, 0 ), Vector3DF( 14.2f, 15.3f, 18.0f ), 130, 1.0f );
	gvdb.getScene()->SetCamera( cam );
	gvdb.getScene()->SetRes ( w, h );

	Light* lgt = new Light;								// Create Light
	lgt->setOrbit ( Vector3DF( 30, 50, 0 ), Vector3DF( 15, 15, 15 ), 200, 1.0 );
	gvdb.getScene()->SetLight ( 0, lgt );

	printf ( "Creating screen buffer. %d x %d\n", w, h );
	gvdb.AddRenderBuf ( 0, w, h, 4 );					// Add render buffer

	gvdb.TimerStart ();
	gvdb.Render ( SHADE_LEVELSET, 0, 0 );			// Render as volume
	float rtime = gvdb.TimerStop();
	printf ( "Render volume. %6.3f ms\n", rtime );

	printf ( "Writing img_importvdb.png\n" );
	unsigned char* buf = ( unsigned char* ) malloc ( w * h * 4 );
	gvdb.ReadRenderBuf ( 0, buf );						// Read render buffer

	// save_png ( "img_importvdb.png", buf, w, h, 4 );		// Save image as png

	// auto mat = cv::Mat( h, w, CV_8UC1, buf, w * sizeof( unsigned char ) );
	// cv::imwrite( "file.png", mat );
	auto grid = openvdb::FloatGrid::create();
	auto accessor = grid->getAccessor();

	for ( int y = 0; y < h; y++ ) {
		for ( int x = 0; x < w; x++ ) {
			for ( int c = 0; c < 4; c++ ) {
				openvdb::Coord coord{x, y, c};
				accessor.setValue( coord, static_cast<float>( buf[( y * w + x ) * 4 + c] ) );
			}
		}
	}

	openvdb::io::File file ( "out.vdb" );
	openvdb::GridPtrVec grids;
	grids.push_back( grid );

	file.write( grids );
	file.close();


	free ( buf );
	delete cam;
	delete lgt;

	gprintf ( "Done.\n" );
	return 0;
}