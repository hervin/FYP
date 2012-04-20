//============================================================================
// Name        : CannyEdgeDetector.cpp
// Author      : Hannah Ervin
// Version     :
// Copyright   : Your copyright notice
// Description : Final Year Project - edge detection and classification
//============================================================================

#include <TooN/TooN.h>
#include <cvd/image_io.h>
#include <cvd/convolution.h>
#include <cvd/image.h>
#include <cvd/videodisplay.h>
#include <cvd/gl_helpers.h>
#include <cvd/image_convert.h>
#include <cmath>
#include <iostream>
#include <vector>
#include "RgbProfile.h"
#include "kmeans.h"
#include <boost/program_options.hpp>


using namespace std;
using namespace CVD;
using namespace TooN;

void display_image(Image <byte>& im){
	VideoDisplay window(im.size());
	glDrawPixels(im);
	glFlush();
}


void gradient(const Image<float>& in, Image<float>& dx_out, Image<float>& dy_out){
	ImageRef size = in.size();
	ImageRef scan (0,0);
	ImageRef dx(1,0);
	ImageRef dy(0,1);

	do{
		dx_out[scan]=in[scan+dx]-in[scan-dx];
		dy_out[scan]=in[scan+dy]-in[scan-dy];
	}while(scan.next(size));
}

void gradient_magnitude(const Image<float>& dx_in, const Image<float>& dy_in, Image<float>& magnitude_out){
	ImageRef size = dx_in.size();
	ImageRef scan (0,0);
	do{
		float dx = dx_in[scan];
		float dy = dy_in[scan];
		magnitude_out[scan] = dx*dx+dy*dy;
	}while(scan.next(size));
}

void nonmax(const Image<float>& dx_in,
		const Image<float>& dy_in,
		const Image<float>& magnitude_in,
		vector<ImageRef>& edgels,
		vector<ImageRef>& directions,
		float threshold){

	ImageRef border(1,1);
	ImageRef scan=border;
	ImageRef size = dx_in.size();

	do{
		float mag = magnitude_in[scan];
		if(mag < threshold) continue;

		// first find the direction
		ImageRef direction;
		float dx = dx_in[scan];
		float dy = dy_in[scan];
		if(dx<0){
			// simplify direction classification by reducing the space to a half plane
			dx*=-1;
			dy*=-1;
		}
		if(0.4142*dy > dx) {
			direction = ImageRef(0,1);
		} else if (dy > 0.4142*dx){
			direction = ImageRef(1,1);
		} else if (dy > -0.4142*dx) {
			direction = ImageRef(1,0);
		} else if (0.4142*dy > -dx) {
			direction = ImageRef(1,-1);
		} else {
			direction = ImageRef(0,1);
		}

		// if we're pointing in opposite direction to dx dy, then flip
		if(direction.x*dx_in[scan] + direction.y*dy_in[scan] < 0){
		  direction.x*=-1;
		  direction.y*=-1;
		}

		if(mag > magnitude_in[scan+direction] && mag > magnitude_in[scan-direction]){
			edgels.push_back(scan);
			directions.push_back(direction);
		}
	}while(scan.next(border,size-border));

}

void draw_edgels(const vector<ImageRef>& edgels, Image<byte> out){
	for(int i = 0; i < edgels.size(); i++){
		out[edgels[i]] = 255;
	}
}
void draw_coloured_edgels(const vector<ImageRef>& edgels, const kmeans<RgbProfile>& clusters, Image<Rgb<byte> >& out){
	for(int i = 0; i < edgels.size(); i++){
		int cluster = clusters.get_cluster(i);
		switch(cluster){
		case 0:
			out[edgels[i]] = Rgb<byte>(0,0,255);
			break;
		case 1:
			out[edgels[i]] =  Rgb<byte>(0,255,0);
			break;
		case 2:
			out[edgels[i]] = Rgb<byte>(255,0,0);
			break;
		case 3:
			out[edgels[i]] =  Rgb<byte>(0,100,100);
			break;
		case 4:
			out[edgels[i]] =  Rgb<byte>(100,100,0);
			break;
		case 5:
			out[edgels[i]] = Rgb<byte>(100,0,100);
			break;
		case 6:
			out[edgels[i]] = Rgb<byte>(100,100,100);
			break;
		default:
			out[edgels[i]] = Rgb<byte>(255,255,255);
		}
	}
}

void get_colour_profile(const vector<ImageRef>& edgels_in,
				const vector<ImageRef>& dir_in,
				const Image<Rgb<byte> > in,
				vector<RgbProfile >& profile_out,
				int profile_length){
	//test image to check correct profiles are being extracted
	//Image<Rgb<byte> > test(in.size());
	cerr << edgels_in.size() << endl;
	for(unsigned int i = 0; i < edgels_in.size(); i++){
		RgbProfile output(profile_length);
		output.build(edgels_in[i],dir_in[i],in,profile_length);
		profile_out.push_back(output);
	}
	//img_save(test, "edgetest.jpg");
}


int main()
{
	try{
		//LOAD IMAGE
		string img_name("test3");
		Image<Rgb<byte> > im;
		im = img_load(img_name+".jpg");
		ImageRef size = im.size();
		Image<float> gray(size);
		convert_image(im,gray);

		//BLUR
		Image<float> blurred(size);
		convolveGaussian(gray, blurred, 5, 1.0);
		img_save(blurred, img_name+"gblur.jpg");

		//COMPUTE IMAGE GRADIENTS
		Image<float> dx(size);
		Image<float> dy(size);
		gradient(blurred,dx,dy);

		//COMPUTE GRADIENT MAGNITUDES
		Image<float> magnitude(size);
		gradient_magnitude(dx,dy,magnitude);

		//NON MAXIMA SUPPRESSION AND THRESHOLDING
		float threshold = 0.00001;
		vector<ImageRef> edgels;
		vector<ImageRef> directions;
		nonmax(dx, dy, magnitude, edgels, directions, threshold);

		cerr << "edgels and directions computed" << endl;

		//GET COLOUR PROFILES
		vector<RgbProfile > colour_profiles;
		get_colour_profile(edgels,directions,im, colour_profiles, 21);

		//DRAW IMAGE OF EDGES
		Image<byte> output(size,(byte)0);
		draw_edgels(edgels, output);
		img_save(output, img_name+"out.jpg");

		cerr << "colour profiles created" << endl;

		//CLUSTER EDGES
		int k = 6;
		kmeans<RgbProfile> edgel_clusterer(k);
		edgel_clusterer.cluster(colour_profiles,k);

		cerr << "colour profiles clustered" << endl;

		//SHOW CLUSTERS
		Rgb<byte> black(0,0,0);
		Image<Rgb<byte> > coloured_edges(size,black);
		draw_coloured_edgels(edgels,edgel_clusterer, coloured_edges);
		img_save(coloured_edges, img_name+"_colourededges.jpg");

	}
	catch(Exceptions::All error){
	    std::cerr << "Error: " << error.what << std::endl;
	}

	return 0;

}

