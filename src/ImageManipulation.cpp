#include "ImageManipulation.hpp"

Point NeighborPixel(Point pos, int direction, Mat img)
{
	switch(direction)
	{
		case LEFT: 
				if(pos.y==0) 				//left border, return the own pixel
					return pos;
				else{ 						//any other pixel in the image
					pos.y--;
					return pos;
				}
		case RIGHT:
				if(pos.y==(img.cols-1)) 	//right border, return the own pixel
					return pos;
				else{						//any other pixel in the image
					pos.y++;
					return pos;
				}
		case UP:
				if(pos.x==0)				//first row, return the own pixel
					return pos;
				else{						//any other pixel in the image
					pos.x--;
					return pos;
				}
		case DOWN:
				if(pos.x==img.rows-1)		//last row, return the own pixel
					return pos;
				else{						//any other pixel in the image
					pos.x++;
					return pos;
				}
	}
	return pos;
}
