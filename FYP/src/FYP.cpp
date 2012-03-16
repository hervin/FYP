//============================================================================
// Name        : CannyEdgeDetector.cpp
// Author      : Hannah Ervin
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
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

void nonmax(const Image<float>& dx_in, const Image<float>& dy_in, const Image<float>& magnitude_in, vector<ImageRef>& edgels, float threshold){
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
				dx*=-1;
				dy*=-1;
		}

		if(0.4142*dy > dx) {
			direction = ImageRef(1,0);
		} else if (dy > 0.4142*dx){
			direction = ImageRef(1,1);
		} else if (dy > -0.4142*dx) {
			direction = ImageRef(1,0);
		} else if (0.4142*dy > -dx) {
			direction = ImageRef(1,-1);
		} else {
			direction = ImageRef(1,0);
		}

		if(mag > magnitude_in[scan+direction] && mag > magnitude_in[scan-direction]){
			edgels.push_back(scan);
		}

	}while(scan.next(border,size-border));

}

void draw_edgels(const vector<ImageRef>& edgels, Image<byte> out){
	for(int i =0; i< edgels.size(); i++){
		out[edgels[i]] = 255;
	}
}

//get_colour_profile(vector<ImageRef>& edgels_in, vector<ImageRef>& dir_in, Image<Rgb<byte> > in, vector<Rgb[21]> profile_out)


int main()
{
	try{
		cout << "Hello World!" <<endl;
		//LOAD IMAGE
		string img_name("flower");
		Image<Rgb<byte> > im;
		im = img_load(img_name+".jpg");
		ImageRef size = im.size();
		Image<float> gray(size);
		convert_image(im,gray);

		//BLUR
		Image<float> blurred(size);
		convolveGaussian(gray, blurred, 5, 1.0);
		img_save(blurred, img_name+"gblur.jpg");

		Image<float> dx(size);
		Image<float> dy(size);
		gradient(blurred,dx,dy);

		Image<float> magnitude(size);
		gradient_magnitude(dx,dy,magnitude);
		float threshold = 0.0002;
		vector<ImageRef> edgels;
		nonmax(dx,dy,magnitude,edgels, threshold);

		Image<byte> output(size);
		draw_edgels(edgels, output);
		img_save(output, img_name+"out.jpg");

	}
	catch(Exceptions::All error){
	    std::cerr << "Error: " << error.what << std::endl;
	}

	return 0;

}
