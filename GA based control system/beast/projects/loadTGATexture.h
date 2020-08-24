//
// loadTGATexture.h
//
// Contains a useful structure for texture holding, and a method that will open and process TGA files
// in order to create a TextureImage structure for texture mapping.
//
// The LoadTGA method handles possible exceptions and prints errors to the terminal.
//
// Adapted from lesson 24 of NeHe, http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=24 [accessed 05-11-2008], copyright 1997 - 2006.
// Joseph Shaw, 14 Nov 2008 (original)
//

#ifndef LOADTGATEXTURE_H
#define LOADTGATEXTURE_H

#include <GL/glut.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>			// Header File For Variable Argument Routines
#include <string.h>     // Header for sting comparison - in particular, the memcmp function     



using namespace std;


//
// TextureImage
// Structure to hold a texture.
//
typedef	struct										
{
	GLubyte	*imageData;		// Image Data (Up To 32 Bits)
	GLuint	bpp;	// Image Color Depth In Bits Per Pixel
	GLuint	width;		// Image Width
	GLuint	height;		// Image Height
	GLuint	texID;		// Texture ID Used To Select A Texture
} TextureImage;										




bool LoadTGA(TextureImage* texture, char* filename)
//
// Loads a TGA file into memory and registers the file with OpenGL as a texure.
// Any errors are printed out to the terminal.
//
// IN:
//	texture   Pointer to an empty TextureImage structure for memory storage of the structure.
//	filename    Array of characters representing the TGA filename to be loaded.
// OUT:
//	texture   TextureImage structure which contains the in memory TGA image for texture mapping.
// Returns:
//	true if the loading of the texture was successful. false otherwise.
//

{    
	GLubyte		TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};			// Uncompressed TGA Header
	GLubyte		TGAcompare[12];							// Used To Compare TGA Header
	GLubyte		header[6];							// First 6 Useful Bytes From The Header
	GLuint		bytesPerPixel;							// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint		imageSize;							// Used To Store The Image Size When Setting Aside Ram
	GLuint		temp;								// Temporary Variable
	GLuint		type=GL_RGBA;							// Set The Default GL Mode To RBGA (32 BPP)
  
  FILE *file = fopen(filename, "rb");						// Open The TGA File

  if (file == NULL)							// Did The File Even Exist? *Added Jim Strong*
	{
		cout << "File \"" << filename << "\" did not exist!" << endl;
		return false;							// Return False
	}
	else if (fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare))
	{
	  cout << "Cannot read in header data - file too small." << endl;
		fclose(file);							// If Anything Failed, Close The File
		return false;							// Return False
	}
	else if (memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0)
	{
		cout << "Header is not TGA format." << endl;
		fclose(file);							// If Anything Failed, Close The File
		return false;							// Return False
  }
	else if (fread(header,1,sizeof(header),file)!=sizeof(header))
	{
		cout << "Header is not TGA format." << endl;
		fclose(file);							// If Anything Failed, Close The File
		return false;							// Return False
	}

  texture->width  = header[1] * 256 + header[0];					// Determine The TGA Width	(highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2];					// Determine The TGA Height	(highbyte*256+lowbyte)
    
 	if(	texture->width	<=0	||						// Is The Width Less Than Or Equal To Zero
		texture->height	<=0	)					// Is The Height Less Than Or Equal To Zero
	{
	  cout << "Width or height of TGA was less or equal to zero" << endl;
		fclose(file);								// If Anything Failed, Close The File
		return false;								// Return False
	}
	if (header[4]!=24 && header[4]!=32)					// Is The TGA 24 or 32 Bit?
	{
	  cout << "TGA file was not 24 or 32 bit." << endl;
	  fclose(file);								// If Anything Failed, Close The File
		return false;								// Return False
  }

  texture->bpp	= header[4];							// Grab The TGA's Bits Per Pixel (24 or 32)
	bytesPerPixel	= texture->bpp/8;						// Divide By 8 To Get The Bytes Per Pixel
	imageSize	= texture->width*texture->height*bytesPerPixel;			// Calculate The Memory Required For The TGA Data

  texture->imageData=(GLubyte *)malloc(imageSize);				// Reserve Memory To Hold The TGA Data

	if(	texture->imageData==NULL ||						// Does The Storage Memory Exist?
		fread(texture->imageData, 1, imageSize, file)!=imageSize)		// Does The Image Size Match The Memory Reserved?
	{
		if(texture->imageData!=NULL)						// Was Image Data Loaded
			free(texture->imageData);					// If So, Release The Image Data

    cout << "Texture memory problem - storage memory is none-existant or the image size does not map the memory reserved." << endl;
		fclose(file);								// Close The File
		return false;								// Return False
	}

  for(GLuint i=0; i<int(imageSize); i+=bytesPerPixel)				// Loop Through The Image Data
	{										// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp=texture->imageData[i];						// Temporarily Store The Value At Image Data 'i'
		texture->imageData[i] = texture->imageData[i + 2];			// Set The 1st Byte To The Value Of The 3rd Byte
		texture->imageData[i + 2] = temp;					// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	fclose (file);									// Close The File

  // Build A Texture From The Data
	glGenTextures(1, &texture[0].texID);						// Generate OpenGL texture IDs

	glBindTexture(GL_TEXTURE_2D, texture[0].texID);					// Bind Our Texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);		// Linear Filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);		// Linear Filtered

  if (texture[0].bpp==24)								// Was The TGA 24 Bits
	{
		type=GL_RGB;								// If So Set The 'type' To GL_RGB
	}

  glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, GL_UNSIGNED_BYTE, texture[0].imageData);

	return true;									// Texture Building Went Ok, Return True
}

#endif
