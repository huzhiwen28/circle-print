//*****************************************************************************
//* Foxit Software Inc.
//* Copyright(C) 2006-2009, all rights reserved.
//*
//* The following code is copyrighted and contains proprietary information
//* and trade secrets of Foxit Software Inc.
//*
//* ---DESCRIPTION ABOUT THIS FILE
//* Header file for FPDFTEXT module - a part of Foxit PDF SDK DLL.
//* Functions in this header file, require "FPDFTEXT" module to be enabled in your SDK license.
//* If you want to purchase Foxit PDF SDK license and use ANY of the following functions, please
//* explicitly request for enabling FPDFTEXT module.
//*
//* ---IMPORTANT: 
//*	FPDFTEXT module depends on FPDFVIEW module.
//*
//* ---REVISION HISTORY
//* May. 11, 2007. Version 1.0 (Initial version).
//*	Apr. 20, 2009. Version 2.0
//*****************************************************************************

#ifndef _FPDFTEXT_H_
#define _FPDFTEXT_H_

#ifndef _FPDFVIEW_H_
#include "fpdfview.h"
#endif
// Exported Functions
#ifdef __cplusplus
extern "C" {
#endif

// Function: FPDFText_LoadPage
//			Prepare information about all characters in a page.
// Parameters: 
//			page	-	Handle to the page. Returned by FPDF_LoadPage function (in FPDFVIEW module).	
// Return value:
//			A handle to the text page information structure.
//			NULL if something goes wrong.
// Comments:
//			Application must call FPDFText_ClosePage to release the text page information.
//			If you don't purchase Text Module , this function will return NULL.
//	
DLLEXPORT FPDF_TEXTPAGE	STDCALL FPDFText_LoadPage(FPDF_PAGE page);

// Function: FPDFText_ClosePage
//			Release all resources allocated for a text page information structure.
// Parameters: 
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
// Return Value:
//			None.
//
DLLEXPORT void STDCALL FPDFText_ClosePage(FPDF_TEXTPAGE text_page);
	
// Function: FPDFText_CountChars
//			Get number of characters in a page.
// Parameters: 
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
// Return value:
//			Number of characters in the page. Return -1 for error. 
//			Generated characters, like additional space characters, new line characters, are also counted.
// Comments:
//			Characters in a page form a "stream", inside the stream, each character has an index.
//			We will use the index parameters in many of FPDFTEXT functions. The first character in the page
//			has an index value of zero.
//
DLLEXPORT int STDCALL FPDFText_CountChars(FPDF_TEXTPAGE text_page);

// Function: FPDFText_GetUnicode
//			Get Unicode of a character in a page.
// Parameters: 
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			index		-	Zero-based index of the character.
// Return value:
//			The Unicode of the particular character.
//			If a character is not encoded in Unicode and Foxit engine can't convert to Unicode,
//			the return value will be zero.
//
DLLEXPORT unsigned int STDCALL FPDFText_GetUnicode(FPDF_TEXTPAGE text_page, int index);

// Function: FPDFText_IsGenerated
//			Indicate whether a character is a generated character.
// Parameters:
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			index		-	Zero-based index of the character.
// Return value:
//			TRUE indicates a generated character and FALSE indicates an actual character in the PDF page.
// Comments:	
//			"Generated character" is character not actually encoded in the PDF page, but generated
//			by FPDFTEXT engine to keep formatting information. This happens in two cases: 1) an
//			extra space character will be generated if two characters in the same line appears to
//			be apart quite some space, 2) a new line character will be generated if two consecutive
//			characters appears to be on different line. This characters are useful when doing the
//			search.
DLLEXPORT FPDF_BOOL	STDCALL FPDFText_IsGenerated(FPDF_TEXTPAGE text_page, int index);

// Function: FPDFText_GetFontSize
//			Get the font size of a particular character.
// Parameters: 
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			index		-	Zero-based index of the character.
// Return value:
//			The font size of the particular character, measured in points (about 1/72 inch).
//			This is the typographic size of the font (so called "em size").
//
DLLEXPORT double STDCALL FPDFText_GetFontSize(FPDF_TEXTPAGE text_page, int index);

// Function: FPDFText_GetOrigin
//			Get origin position of a particular character.
// Parameters: 
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			index		-	Zero-based index of the character.
//			x			-	Pointer to a double number receiving X position of the character origin.
//			y			-	Pointer to a double number receiving Y position of the character origin.
// Return Value:
//			None.
// Comments:
//			Origin X/Y positions are measured in PDF "user space".
//
DLLEXPORT void	STDCALL	FPDFText_GetOrigin(FPDF_TEXTPAGE text_page, int index, double* x, double* y);

// Function: FPDFText_GetCharBox
//			Get bounding box of a particular character.
// Parameters: 
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			index		-	Zero-based index of the character.
//			left		-	Pointer to a double number receiving left position of the character box.
//			right		-	Pointer to a double number receiving right position of the character box.
//			bottom		-	Pointer to a double number receiving bottom position of the character box.
//			top			-	Pointer to a double number receiving top position of the character box.
// Return Value:
//			None.
// Comments:
//			All positions are measured in PDF "user space".
//
DLLEXPORT void STDCALL FPDFText_GetCharBox(FPDF_TEXTPAGE text_page, int index, double* left,
													double* right, double* bottom, double* top);

// Function: FPDFText_GetMatrix
//			Get the matrix of a particular character.
// Parameters:
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			index		-	Zero-based index of the character.
//			a			-	Pointer to a double value receiving the coefficient "a" of the matrix.
//			b			-	Pointer to a double value receiving the coefficient "b" of the matrix.
//			c			-	Pointer to a double value receiving the coefficient "c" of the matrix.
//			d			-	Pointer to a double value receiving the coefficient "d" of the matrix.
// Return value:
//			None.
// Comments:
//			A matrix defines transformation of coordinations from one space to another.
//			In PDF, a matrix is defined by the following equations:
//			x' = a * x + c * y + e;
//			y' = b * x + d * y + f;
//			FPDFText_GetMatrix function is used to get a,b,c,d coefficients of the transformation from
//			"text space" to "user space". The e,f coefficients are actually the origin position, which
//			can be fetched by FPDFText_GetOrigin function.
//
DLLEXPORT void STDCALL FPDFText_GetMatrix(FPDF_TEXTPAGE text_page, int index, double* a, double* b, double* c, double* d);

// Function: FPDFText_GetFont
//			Get font of a particular character.
// Parameters: 
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			index		-	Zero-based index of the character.
// Return value:
//			A handle to the font used by the particular character.
//			This handle can be used in FPDFFont_xxx functions for more information about the font.
//
DLLEXPORT FPDF_FONT	STDCALL	FPDFText_GetFont(FPDF_TEXTPAGE text_page, int index);

// Function: FPDFFont_GetAscent
//			Get font ascent(in 1/1000 em).
// Parameters:
//			font		-	Handle to a font. Returned by FPDFText_GetFont function.
// Return value:
//			The ascent (typically the above-baseline height of letter "h"), measured in
//			1/1000 of em size. So if a character uses a font size (em size) of 10 points,
//			and it has an ascent value of 500 (meaning half of the em), then the ascent
//			height will be 5 points (5/72 inch).
//
DLLEXPORT int STDCALL FPDFFont_GetAscent(FPDF_FONT font);

// Function: FPDFFont_GetDescent
//			Get font descent (in 1/1000 em).
// Parameters:
//			font		-	Handle to a font. Returned by FPDFText_GetFont function.
// Return value:
//			The descent (typically the under-baseline height of letter "g"), measured in
//			1/1000 of em size. Most fonts have a negative descent value. 
//
DLLEXPORT int STDCALL FPDFFont_GetDescent(FPDF_FONT font);

// Function: FPDFFont_GetName
//			Get the Name of a font.
// Parameters:
//			font		-	Handle to a font. Returned by FPDFText_GetFont function.
// Return value:
//			A pointer to a null-terminated string that specifies the name of the font.
//			Application can't modify the returned string.
//
DLLEXPORT FPDF_BYTESTRING STDCALL FPDFFont_GetName(FPDF_FONT font);

// Function: FPDFText_GetCharIndexAtPos
//			Get the index of a character at or nearby a certain position on the page.
// Parameters:
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			x			-	X position in PDF "user space".
//			y			-	Y position in PDF "user space".
//			xTolerance	-	An x-axis tolerance value for character hit detection, in point unit.
//			yTolerance	-	A y-axis tolerance value for character hit detection, in point unit.
// Return Value:
//			The zero-based index of the character at, or nearby the point (x,y).
//			If there is no character at or nearby the point, return value will be -1.
//			If an error occurs, -3 will be returned.
//
DLLEXPORT int STDCALL FPDFText_GetCharIndexAtPos(FPDF_TEXTPAGE text_page,
												 double x, double y, double xTorelance, double yTolerance);

// Flags used by FPDFText_GetCharIndexByDirection function
#define FPDFTEXT_LEFT			-1
#define FPDFTEXT_RIGHT			1
#define FPDFTEXT_UP				-2
#define FPDFTEXT_DOWN			2

// Function: FPDFText_GetCharIndexByDirection
//			Move the character index in different directions and get new character index, from a specific character.
// Parameters:
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			index		-	Zero-based index for the current character.
//			direction	-	A number indicating the moving direction. Can be one of the followings:
//							FPDFTEXT_LEFT, FPDFTEXT_UP, FPDFTEXT_RIGHT, FPDFTEXT_DOWN.
// Return Value:
//			Zero-base character index for the new position.
//			-1 if beginning of the page reached; -2 if end of the page reached;
//			-3 for failures.
// Comments:
//			FPDFTEXT moves the character pointer according to "stream order". For example, left will 
//			move to the previous character, right will move to next character. Because in PDF, "stream
//			order" can be different from "appearance order" (the order that appears to human eyes),
//			so it's possible the moving direction doesn't match the actually position movement. For example,
//			using FPDFTEXT_LEFT may actually result in a character that's all the way down in the page.
//			
DLLEXPORT int STDCALL FPDFText_GetCharIndexByDirection(FPDF_TEXTPAGE text_page, int index, int direction);

// Function: FPDFText_GetText
//			Extract unicode text string from the page.
// Parameters:
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			start_index	-	Index for the start characters.
//			count		-	Number of characters to be extracted.
//			result		-	A buffer (allocated by application) receiving the extracted unicodes.
//							The size of the buffer must be able to hold the number of characters plus a terminator.
// Return Value:
//			Number of characters written into the result buffer, excluding the trailing terminator.
// Comments:
//			This function ignores characters without unicode information.
//			
DLLEXPORT int STDCALL FPDFText_GetText(FPDF_TEXTPAGE text_page, int start_index, int count, unsigned short* result);

// Function: FPDFText_CountRects
//			Count number of rectangular areas occupied by a segment of texts.
// Parameters:
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			start_index	-	Index for the start characters.
//			count		-	Number of characters.
// Return value:
//			Number of rectangles. Zero for error.
// Comments:
//			This function, along with FPDFText_GetRect can be used by applications to detect the position
//			on the page for a text segment, so proper areas can be highlighted or something.
//			FPDFTEXT will automatically merge small character boxes into bigger one if those characters
//			are on the same line and use same font settings.
//
DLLEXPORT int STDCALL FPDFText_CountRects(FPDF_TEXTPAGE text_page, int start_index, int count);

// Function: FPDFText_GetRect
//			Get a rectangular area from the result generated by FPDFText_CountRects.
// Parameters:
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			rect_index	-	Zero-based index for the rectangle.
//			left		-	Pointer to a double value receiving the rectangle left boundary.
//			top			-	Pointer to a double value receiving the rectangle top boundary.
//			right		-	Pointer to a double value receiving the rectangle right boundary.
//			bottom		-	Pointer to a double value receiving the rectangle bottom boundary.
// Return Value:
//			None.
//
DLLEXPORT void STDCALL FPDFText_GetRect(FPDF_TEXTPAGE text_page, int rect_index, double* left, double* top,
											double* right, double* bottom);

// Function: FPDFText_GetBoundedText
//			Extract unicode text within a rectangular boundary on the page.
// Parameters:
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			left		-	Left boundary.
//			top			-	Top boundary.
//			right		-	Right boundary.
//			bottom		-	Bottom boundary.
//			buffer		-	A unicode buffer.
//			buflen		-	Number of characters (not bytes) for the buffer, excluding an additional terminator.
// Return Value:
//			If buffer is NULL or buflen is zero, return number of characters (not bytes) needed,
//			otherwise, return number of characters copied into the buffer.
//
DLLEXPORT int STDCALL FPDFText_GetBoundedText(FPDF_TEXTPAGE text_page,double left, double top, 
											  double right, double bottom,unsigned short* buffer,int buflen);


// Function: FPDFText_CountBoundedSegments
//			Get number of text segments within a rectangular boundary on the page.
// Parameters:
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			left		-	Left boundary.
//			top			-	Top boundary.
//			right		-	Right boundary.
//			bottom		-	Bottom boundary.
// Return Value:
//			Number of segments.
//
DLLEXPORT int STDCALL FPDFText_CountBoundedSegments(FPDF_TEXTPAGE text_page,
													double left, double top, double right, double bottom);

// Function: FPDFText_GetBoundedSegment
//			Get a particular segment in the result generated by FPDFText_CountBoundedSegments function.
// Parameters:
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			seg_index	-	Zero-based index for the segment.
//			start_index	-	Pointer to an integer receiving the start character index for the segment.
//			count		-	Pointer to an integer receiving number of characters in the segment.
// Return Value:
//			None.
//
DLLEXPORT void STDCALL FPDFText_GetBoundedSegment(FPDF_TEXTPAGE text_page, int seg_index, int* start_index, int* count);

// Flags used by FPDFText_FindStart function.
#define FPDF_MATCHCASE      0x00000001		//If not set, it will not match case by default.
#define FPDF_MATCHWHOLEWORD 0x00000002		//If not set, it will not match the whole word by default.

// Function: FPDFText_FindStart
//			Start a search.
// Parameters:
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			findwhat	-	A unicode match pattern.
//			flags		-	Option flags.
//			start_index	-	Start from this character. -1 for end of the page.
// Return Value:
//			A handle for the search context. FPDFText_FindClose must be called to release this handle.
//
DLLEXPORT FPDF_SCHHANDLE STDCALL FPDFText_FindStart(FPDF_TEXTPAGE text_page, FPDF_WIDESTRING findwhat,
													unsigned long flags, int start_index);

// Function: FPDFText_FindNext
//			Search in the direction from page start to end.
// Parameters:
//			handle		-	A search context handle returned by FPDFText_FindStart.
// Return Value:
//			Whether a match is found.
//
DLLEXPORT FPDF_BOOL STDCALL FPDFText_FindNext(FPDF_SCHHANDLE handle);

// Function: FPDFText_FindPrev
//			Search in the direction from page end to start.
// Parameters:
//			handle		-	A search context handle returned by FPDFText_FindStart.
// Return Value:
//			Whether a match is found.
//
DLLEXPORT FPDF_BOOL STDCALL FPDFText_FindPrev(FPDF_SCHHANDLE handle);

// Function: FPDFText_GetSchResultIndex
//			Get the starting character index of the search result.
// Parameters:
//			handle		-	A search context handle returned by FPDFText_FindStart.
// Return Value:
//			Index for the starting character.
//
DLLEXPORT int STDCALL FPDFText_GetSchResultIndex(FPDF_SCHHANDLE handle);

// Function: FPDFText_GetSchCount
//			Get the number of matched characters in the search result.
// Parameters:
//			handle		-	A search context handle returned by FPDFText_FindStart.
// Return Value:
//			Number of matched characters.
//
DLLEXPORT int STDCALL FPDFText_GetSchCount(FPDF_SCHHANDLE handle);

// Function: FPDFText_FindClose
//			Release a search context.
// Parameters:
//			handle		-	A search context handle returned by FPDFText_FindStart.
// Return Value:
//			None.
//
DLLEXPORT void STDCALL FPDFText_FindClose(FPDF_SCHHANDLE handle);

// Function: FPDFLink_LoadWebLinks
//			Prepare information about weblinks in a page.
// Parameters:
//			text_page	-	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
// Return Value:	
//			A handle to the page's links information structure.
//			NULL if something goes wrong.
// Comments:
//			Weblinks are those links implicitly embedded in PDF pages. PDF also has a type of
//			annotation called "link", FPDFTEXT doesn't deal with that kind of link.
//			FPDFTEXT weblink feature is useful for automatically detecting links in the page
//			contents. For example, things like "http://www.foxitsoftware.com" will be detected,
//			so applications can allow user to click on those characters to activate the link,
//			even the PDF doesn't come with link annotations.
//
//			FPDFLink_CloseWebLinks must be called to release resources.
//
DLLEXPORT FPDF_PAGELINK STDCALL FPDFLink_LoadWebLinks(FPDF_TEXTPAGE text_page);

// Function: FPDFLink_CountWebLinks
//			Count number of detected web links.
// Parameters:
//			link_page	-	Handle returned by FPDFLink_LoadWebLinks.
// Return Value:
//			Number of detected web links.
//
DLLEXPORT int STDCALL FPDFLink_CountWebLinks(FPDF_PAGELINK link_page);

// Function: FPDFLink_GetURL
//			Fetch the URL information for a detected web link.
// Parameters:
//			link_page	-	Handle returned by FPDFLink_LoadWebLinks.
//			link_index	-	Zero-based index for the link.
//			buffer		-	A unicode buffer.
//			buflen		-	Number of characters (not bytes) for the buffer, excluding an additional terminator.
// Return Value:
//			If buffer is NULL or buflen is zero, return number of characters (not bytes) needed,
//			otherwise, return number of characters copied into the buffer.
//
DLLEXPORT int STDCALL FPDFLink_GetURL(FPDF_PAGELINK link_page, int link_index, unsigned short* buffer,int buflen);

// Function: FPDFLink_CountRects
//			Count number of rectangular areas for the link.
// Parameters:
//			link_page	-	Handle returned by FPDFLink_LoadWebLinks.
//			link_index	-	Zero-based index for the link.
// Return Value:
//			Number of rectangular areas for the link.
//
DLLEXPORT int STDCALL FPDFLink_CountRects(FPDF_PAGELINK link_page, int link_index);

// Function: FPDFLink_GetRect
//			Fetch the boundaries of a rectangle for a link.
// Parameters:
//			link_page	-	Handle returned by FPDFLink_LoadWebLinks.
//			link_index	-	Zero-based index for the link.
//			rect_index	-	Zero-based index for a rectangle.
//			left		-	Pointer to a double value receiving the rectangle left boundary.
//			top			-	Pointer to a double value receiving the rectangle top boundary.
//			right		-	Pointer to a double value receiving the rectangle right boundary.
//			bottom		-	Pointer to a double value receiving the rectangle bottom boundary.
// Return Value:
//			None.
//
DLLEXPORT void STDCALL FPDFLink_GetRect(FPDF_PAGELINK link_page, int link_index, int rect_index, 
										double* left, double* top,double* right, double* bottom);

// Function: FPDFLink_CloseWebLinks
//			Release resources used by weblink feature.
// Parameters:
//			link_page	-	Handle returned by FPDFLink_LoadWebLinks.
// Return Value:
//			None.
//
DLLEXPORT void STDCALL FPDFLink_CloseWebLinks(FPDF_PAGELINK link_page);

//Flags used by FPDFText_PDFToText and FPDFText_PageToText
#define FPDFTEXT_STREAM_ORDER	0
#define FPDFTEXT_DISPLAY_ORDER	1

// Function: FPDFText_PDFToText
//			Convert a PDF file to a TXT File.
// Parameters:
//			sour_file	- 	Path to the PDF file you want to Convert.
//			dest_file	- 	The path of the file you want to save.
//			flag		- 	0 for stream order, 1 for appearance order.
//			password	-	A string used as the password for PDF file. 
//							If no password needed, empty or NULL can be used.
// Return value:
//			TURE for succeed, False for failed. 

DLLEXPORT FPDF_BOOL STDCALL FPDFText_PDFToText(const char * sour_file,const char * dest_file,int flag,FPDF_BYTESTRING password);

// Function: FPDFText_PageToText
//		Convert a PDF page data to a text buffer.
// Parameters:
//			doc			- 	Handle to document. Returned by FPDF_LoadDocument function.
//			page_index	- 	Index number of the page. 0 for the first page.
//			buf			- 	An output buffer used to hold the text of the page.
//			size		- 	Size of the buffer.
//			flag		- 	0 for stream order, 1 for appearance order.
//	Return value:
//			If buf is NULL or size is zero, number of characters (not bytes) needed,
//			otherwise, number of characters copied into the buf.
//
DLLEXPORT int STDCALL FPDFText_PageToText(FPDF_DOCUMENT doc,int page_index,wchar_t* buf,int size,int flag);
#ifdef __cplusplus
};
#endif

#endif//_FPDFTEXT_H_
