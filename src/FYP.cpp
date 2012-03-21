//============================================================================
// Name        : FYP.cpp
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
#include <cmath>
#include <iostream>


using namespace std;
using namespace CVD;
using namespace TooN;

void display_image(Image <byte>& im){
	VideoDisplay window(im.size());
	glDrawPixels(im);
	glFlush();
}

void sobel_filter(Image<byte>& in, Image<byte>& sobel, float scale, float *gradorientation, int gradwidth)
{
	ImageRef size = in.size();
	ImageRef border(1,1);
	ImageRef dx(1,0);
	ImageRef dy(0,1);
	ImageRef scan=border;
	do{
		double diffx = in[scan+dx]-in[scan-dx];
		double diffy = in[scan+dy]-in[scan-dy];
		sobel[scan] = scale*sqrt(diffx*diffx + diffy*diffy);
		if(diffx!=0){
			gradorientation[scan.x * gradwidth+scan.y] = atan(diffy/diffx);
			if(gradorientation[scan.x * gradwidth+scan.y]!=0){
				gradorientation[scan.x * gradwidth+scan.y] = gradorientation[scan.x * gradwidth+scan.y]*180.0/M_PI;
			}
		}
		else{
			gradorientation[scan.x * gradwidth+scan.y] = atan(diffy);
			if(gradorientation[scan.x * gradwidth+scan.y]!=0){
				gradorientation[scan.x * gradwidth+scan.y] = gradorientation[scan.x * gradwidth+scan.y]*180.0/M_PI;
			}
		}
		if(gradorientation[scan.x * gradwidth+scan.y]<-67.5){
					gradorientation[scan.x * gradwidth+scan.y] = 90;
				}
				else if (gradorientation[scan.x * gradwidth+scan.y] < -22.5){
					gradorientation[scan.x * gradwidth+scan.y] = 135;
				}
				else if(gradorientation[scan.x * gradwidth+scan.y]< 22.5){
					gradorientation[scan.x * gradwidth+scan.y] = 0;
				}
				else if(gradorientation[scan.x * gradwidth+scan.y]< 67.5){
					gradorientation[scan.x * gradwidth+scan.y] = 45;
				}
				else{
					gradorientation[scan.x * gradwidth+scan.y] = 90;
				}
		cout<<gradorientation[scan.x * gradwidth+scan.y]<<endl;
	}
	while(scan.next(border, size-border));
}


int main()
{
	try{
		cout << "Hello World!" <<endl;
		//LOAD IMAGE
		string img_name("frog");
		Image<byte> im;
		im = img_load(img_name+".jpg");
		ImageRef size = im.size();

		//BLUR
		Image<byte> blurred(size);
		convolveGaussian(im, blurred, 5, 1.0);
		img_save(blurred, img_name+"gblur.jpg");

		//SOBEL FILTER
		Image<byte> sobel(size);
		float* gradorientation;
		int x = size.x;
		int y = size.y;
		gradorientation = new float[x*y];
		sobel_filter(blurred,sobel, 5.0, gradorientation,x);
		img_save(sobel, img_name+"sobel.jpg");
		//img_save(gradorientation, img_name+"gradorientation.jpg");


		//DISPLAY RESULTS
		display_image(im);
		display_image(blurred);
		display_image(sobel);


	}
	catch(Exceptions::All error){
	    std::cerr << "Error: " << error.what << std::endl;
	}

	return 0;

}
