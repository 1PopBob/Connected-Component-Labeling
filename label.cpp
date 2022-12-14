#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include "bmplib.h" 
#include "queue.h"
using namespace std;

void usage() { 
    cerr << "usage: ./label <options>" << endl;
    cerr <<"Examples" << endl;
    cerr << "./label test_queue" << endl;
    cerr << "./label gray <input file> <outputfile>" << endl;
    cerr << "./label binary <inputfile> <outputfile>" << endl;
    cerr << "./label segment <inputfile> <outputfile>" << endl;
}

//Function prototypes go here
void test_queue();
void clean(unsigned char ***input,unsigned char **gray,unsigned char **binary, int **labeled_image,int height , int width);
void rgb2gray(unsigned char ***in,unsigned char **out,int height,int width);
void gray2binary(unsigned char **in,unsigned char **out,int height,int width);
int component_labeling(unsigned char **binary_image,int **label,int height,int width);
void label2RGB(int  **labeled_image, unsigned char ***rgb_image,int num_segments,int height,int width);
bool addToExplored(Location, int*, Location*);
bool checkDuplicate(Location, const int* size, Location *explored);

// The main function, you do not need to make any changes to this function 
// However, we encourage you to try to understand what's going on in the main function
int main(int argc,char **argv) {

    srand(time(0));
    if(argc < 2 )  {
        usage();
        return -1;
    }        
    unsigned char ***input=0;
    unsigned char **gray=0;
    unsigned char **binary=0;
    int **labeled_image=0;
    if( strcmp("test_queue",argv[1]) == 0 ) { 
        test_queue();
    } 
    else if(strcmp("gray",argv[1]) == 0 ) {
        if(argc <4 ) {
            cerr << "not enough argument for gray" << endl;
            return -1;
        }
        int height, width;
        input = readRGBBMP(argv[2],&height,&width);
        if(input == 0)
        {
            cerr << "unable to open " << argv[2] << " for input." << endl;
            return -1;
        }
        //dynamically allocated a 2D gray image array            
        gray = new unsigned char*[height];
        for(int i=0;i<height;i++){
          gray[i] = new unsigned char[width];
        }
        //call rgb2gray to get the gray image from rgb
        rgb2gray(input,gray,height,width);
        if(writeGSBMP(argv[3],gray,height,width) != 0) { 
            cerr << "error writing file " << argv[3] << endl;
            clean(input,gray,binary,labeled_image,height,width);
            return -1;
        }
        //call the function that handles memory deallocation
        clean(input,gray,binary,labeled_image,height,width);

    }
    else if(strcmp("binary",argv[1]) == 0 ) {
        if(argc <4 ) {
            cerr << "not enough arguemnt for binary" << endl;
            return -1;
        }            
        int height,width;
        input = readRGBBMP(argv[2],&height,&width);
        if(input == 0)
        {
            cerr << "unable to open " << argv[2] << " for input." << endl;
            clean(input,gray,binary,labeled_image,height,width);
            return -1;
        }            
        //dynamically allocated a 2D gray image array
        //dynamically allocated a 2D binary image array    
        gray = new unsigned char*[height];
        binary = new unsigned char*[height];
        for(int i=0;i<height;i++){
          gray[i] = new unsigned char[width];
          binary[i] = new unsigned char[width];
        }
        //call rgb2gray to get the gray image from rgb
        rgb2gray(input,gray,height,width);
        //call gray2binary to get the binary image from gray
        gray2binary(gray,binary,height,width);
        if(writeBinary(argv[3],binary,height,width) != 0) { 
            cerr << "error writing file " << argv[3] << endl;
            clean(input,gray,binary,labeled_image,height,width);
            return -1;
        }
        //call the function that handles memory deallocation
        clean(input,gray,binary,labeled_image,height,width);
     
    }
    else if(strcmp("segment",argv[1]) == 0 ) {
        if(argc <4 ) {
            cerr << "not enough arguemnt for segment" << endl;
            return -1;
        } 
        int height,width;
        input = readRGBBMP(argv[2],&height,&width);
        if(input == 0)
        {
            cerr << "unable to open " << argv[2] << " for input." << endl;
            clean(input,gray,binary,labeled_image,height,width);
            return -1;
        }            
        //dynamically allocated a 2D gray image array
        //dynamically allocated a 2D binary image array  
        gray = new unsigned char*[height];
        binary = new unsigned char*[height];
        for(int i=0;i<height;i++){
          gray[i] = new unsigned char[width];
          binary[i] = new unsigned char[width];
        }
        rgb2gray(input,gray,height,width);
        gray2binary(gray,binary,height,width);
        //dynamically allocated a 2D array for labels.
        labeled_image = new int*[height];
        for(int i=0;i<height;i++){
          labeled_image[i] = new int[width];
          for(int j=0;j<width;j++){
            labeled_image[i][j] = 0;
          }
        }
        int segments = component_labeling(binary, labeled_image, height, width);
        cout<<"Segments found: "<< segments << endl;
        //replace 3D input image with 0 to be used as output.
        for(int i=0;i<height;i++){
            for(int j=0;j<width;j++){
                for(int k=0;k<RGB;k++){
                  input[i][j][k] = 0;
                } 
            }
        }
        //label2rgb
        label2RGB(labeled_image, input , segments, height, width);
        if(writeRGBBMP(argv[3],input,height,width) != 0) {
            cerr << "error writing file " << argv[3] << endl;
            clean(input,gray,binary,labeled_image,height,width);
            return -1;
        }
        //call the function that handles memory deallocation
        clean(input,gray,binary,labeled_image,height,width);
    }
   return 0;
}

//Loop over the 'in' image array and calculate the single 'out' pixel value using the formula
// GS = 0.2989 * R + 0.5870 * G + 0.1140 * B 
void rgb2gray(unsigned char ***in,unsigned char **out,int height,int width) {
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            out[i][j] = (0.2989 * in[i][j][0]) + (0.5870 * in[i][j][1]) + (0.1140 * in[i][j][2]);
        }
    }
}

//Loop over the 'in' gray scale array and create a binary (0,1) valued image 'out'
//Set the 'out' pixel to 1 if 'in' is above the THRESHOLD (already defined), else 0
void gray2binary(unsigned char **in,unsigned char **out,int height,int width) {
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            if(in[i][j] > THRESHOLD)
            {
                   out[i][j] = 1;
            }
            else
            {
                out[i][j] = 0;
            }
        }
    }
}

//This is the function that does the work of looping over the binary image and doing the connected component labeling
//See the guide for more detail.
//- Should return number of segments or components found
//- Two disjoint components should not share the same label.
int component_labeling(unsigned char **binary_image,int **label,int height,int width) {
    Location pixel;
    pixel.row = 0;
    pixel.col = 0;
    Location nextPixel;
    int components = 0, current_label = 0;
    // expored array size
    int exploredArraySize = 0;
    int *size = &exploredArraySize;
    Location* explored = new Location[height*width];
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            label[i][j] = 0;
        }
    }
    /*
     * For loop to go through whole image
     * while loop once we find white pixel that continues until queue is exhausted (meaning no white left)
     * +1 component
     */
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            if(binary_image[i][j] == 1)
            {
                pixel.row = i;
                pixel.col = j;
                // will not recount segement we found and explored
                if(checkDuplicate(pixel, size, explored))
                {
                    continue;
                }
                addToExplored(pixel, size, explored);
                components++;
                current_label++;
                label[pixel.row][pixel.col] = current_label;
                Queue q(height*width);
                q.push(pixel);
                while(!q.is_empty())
                {
                    pixel = q.pop();
                    if(binary_image[pixel.row + 1][pixel.col] == 1)
                    {
                        nextPixel.row = pixel.row + 1;
                        nextPixel.col = pixel.col;
                        if(addToExplored(nextPixel, size, explored))
                        {
                            q.push(nextPixel);
                            label[nextPixel.row][nextPixel.col] = current_label;
                        }
                    }
                    if(binary_image[pixel.row][pixel.col - 1] == 1)
                    {
                        nextPixel.row = pixel.row;
                        nextPixel.col = pixel.col - 1;
                        if(addToExplored(nextPixel, size, explored))
                        {
                            q.push(nextPixel);
                            label[nextPixel.row][nextPixel.col] = current_label;
                        }
                    }
                    if(binary_image[pixel.row - 1][pixel.col] == 1)
                    {
                        nextPixel.row = pixel.row - 1;
                        nextPixel.col = pixel.col ;
                        if(addToExplored(nextPixel, size, explored))
                        {
                            q.push(nextPixel);
                            label[nextPixel.row][nextPixel.col] = current_label;
                        }
                    }
                    if(binary_image[pixel.row][pixel.col + 1] == 1)
                    {
                        nextPixel.row = pixel.row;
                        nextPixel.col = pixel.col + 1;
                        if(addToExplored(nextPixel, size, explored))
                        {
                            q.push(nextPixel);
                            label[nextPixel.row][nextPixel.col] = current_label;
                        }
                    }
                }
            }
        }
    }
    delete[] explored;
    return components;
}    

bool addToExplored(Location pixel, int* size, Location* explored)
{
    bool add = false;
    bool dup = checkDuplicate(pixel, size, explored);
    if(!dup)
    {
        explored[*size] = pixel;
        (*size)++;
        add = true;
    }
    return add;
}

bool checkDuplicate(Location pixel, const int* size, Location *explored)
{
    bool duplicate = false;
    for(int i = 0; i < *size; i++)
    {
        Location temp = explored[i];
        if(temp.row == pixel.row && temp.col == pixel.col)
        {
            duplicate = true;
            break;
        }
    }
    return duplicate;
}

//Randomly assign a color (RGB) to each segment or component
//No two segments should share the same color.
void label2RGB(int **labeled_image, unsigned char ***rgb_image,int num_segments,int height,int width)
{
    int* colors = new int[num_segments];
    int rgb = 0;

    for(int i = 0; i < num_segments; i++)
    {
        colors[i] = rand() % 255 + 1;
    }
    int temp = num_segments;
    for(int k = 0; k < num_segments; k++, temp--)
    {
        rgb = rand() % 2;
        for(int i = 0; i < height; i++)
        {
            for(int j = 0; j < width; j++)
            {
                if(labeled_image[i][j] == 0)
                {
                    rgb_image[i][j][0] = 0;
                }
                else if(labeled_image[i][j] == temp)
                {
                    rgb_image[i][j][rgb] = colors[k];
                }
            }
        }
    }
    delete[] colors;
}

void clean(unsigned char ***input,unsigned char **gray,unsigned char **binary, int **labeled_image,int height , int width) {
    if(input) {
        for(int i = 0; i < height; i++)
        {
            for(int j = 0; j < width; j++)
            {
                delete[] input[i][j];
            }
            delete[] input[i];
        }
        delete[] input;
    } 
    if(gray){
        for(int i = 0; i < height; i++)
        {
            delete[] gray[i];
        }
        delete[] gray;
    }
    if(binary){
        for(int i = 0; i < height; i++)
        {
            delete[] binary[i];
        }
        delete[] binary;
    }
    if(labeled_image){
        for(int i = 0; i < height; i++)
        {
            delete[] labeled_image[i];
        }
        delete[] labeled_image;
    }
}


//This function is used to test queue implementation.
void test_queue() { 
    // create some locations;
    Location three_one, two_two;
    three_one.row = 3; three_one.col = 1;
    two_two.row = 2; two_two.col = 2;

    //create an Queue with max capacity 5
    Queue q(5);

    cout << boolalpha;
    cout << q.is_empty() << endl;           // true
    q.push(three_one);
    cout << q.is_empty() << endl;           // false
    q.push(two_two);

    Location loc = q.pop();
    cout << loc.row << "," << loc.col << endl; // 3 1
    loc = q.pop();
    cout << loc.row << "," << loc.col << endl; // 2 2
    cout << q.is_empty() << endl;           // true
}