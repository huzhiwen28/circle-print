//*****************************************************************************
//* Foxit Software Inc.
//* Copyright(C) 2006-2009, all rights reserved.
//*
//* The following code is copyrighted and contains proprietary information
//* and trade secrets of Foxit Software Inc.
//*
//* ---DESCRIPTION ABOUT THIS FILE
//* Header file for FPDFEDIT module - a part of Foxit PDF SDK DLL.
//* Functions in this header file, require "FPDFEDIT" module to be enabled in your SDK license.
//* If you want to purchase Foxit PDF SDK license and use ANY of the following functions, please
//* explicitly request for enabling FPDFEDIT module.
//*
//* ---REVISION HISTORY
//* Apr. 03, 2008. Version 1.0 (Initial version).
//*	Apr. 20, 2009. Version 2.0
//*****************************************************************************

#ifndef	_FPDFEDIT_H_
#define	_FPDFEDIT_H_

#ifndef _FPDFVIEW_H_
#include "fpdfview.h"
#endif
// Define all types used in the SDK. Note they can be simply regarded as opaque pointers
// or long integer numbers.

#define FPDF_ARGB(a,r,g,b)		((((COLORREF)(((BYTE)(b)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(r))<<16)))) | (((DWORD)(BYTE)(a))<<24))
#define FPDF_GetBValue(argb)    ((BYTE)(argb))
#define FPDF_GetGValue(argb)    ((BYTE)(((WORD)(argb)) >> 8))
#define FPDF_GetRValue(argb)    ((BYTE)((argb)>>16))
#define FPDF_GetAValue(argb)    ((BYTE)((argb)>>24))

#ifdef _WIN32
// On Windows system, functions are exported in a DLL
#define DLLEXPORT __declspec( dllexport )
#define STDCALL __stdcall
#else
#define DLLEXPORT
#define STDCALL
#endif

#ifdef _cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////
//
// Document functions
//
//////////////////////////////////////////////////////////////////////

// Function: FPDF_CreateNewDocument
//			Create a new PDF document.
// Parameters:	
//			None.
// Return value:
//			A handle to a document. If failed, NULL is returned.
DLLEXPORT FPDF_DOCUMENT STDCALL FPDF_CreateNewDocument();

// PDF document permissions, according to PDF Reference, Table 3.20
#define FPDF_PERM_PRINT				0x0004			// bit 3. Print the document 
#define FPDF_PERM_MODIFY			0x0008			// bit 4. Modify the contents of the document by operations other 
													// than those controlled by bits 6, 9, and 11. 
#define FPDF_PERM_EXTRACT			0x0010			// bit 5. Copy or otherwise extract text and graphics from the document
													// by operations other than that controlled by bit 10.
#define FPDF_PERM_ANNOT_FORM		0x0020			// bit 6. Add or modify text annotations, fill in interactive form fields.
													// If bit 4 is also set, create or modify interactive form fields 
#define FPDF_PERM_FILL_FORM			0x0100			// bit 9. Fill in existing interactive form fields (including 
													//	signature fields), even if bit 6 is clear. 
#define FPDF_PERM_EXTRACT_ACCESS	0x0200			// bit 10. Extract text and graphics 
#define FPDF_PERM_ASSEMBLE			0x0400			// bit 11. Assemble the document, even if bit 4 is clear.  
#define FPDF_PERM_PRINT_HIGH		0x0800			// bit 12. Print the document to a representation from which a
													// faithful digital copy of the PDF content could be generated. 

// Function: FPDF_SaveAsFile
//			Save the specified document with a new name or format.
// Parameters:	
//			document	-	Handle to document. Returned by FPDF_LoadDocument and FPDF_CreateNewDocument.
//			file_name	-	The path of PDF file different to the current opened file path(including extension).
//			permissions	-	The PDF document permissions defined above.
//			UserPwd		-	A 32-byte string, the user password to entered. Could be NULL.
//			nUserPwdLen	-	The length of the UserPwd.
//			OwnerPwd	-	A 32-byte string, the owner password to entered. Could be NULL.
//			nOwnerPwdLen-	The length of the OwnerPwd.
// Return value:
//			TRUE for succeed, FALSE for Failed.
// Comment:
//			The new file_name should be different with the old one. 
DLLEXPORT FPDF_BOOL STDCALL FPDF_SaveAsFile(FPDF_DOCUMENT document, char* file_name,
							FPDF_DWORD permissions,
							FPDF_LPCBYTE UserPwd, long nUserPwdLen,
							FPDF_LPCBYTE OwnerPwd, long nOwnerPwdLen);

//////////////////////////////////////////////////////////////////////
//
// Page functions
//
//////////////////////////////////////////////////////////////////////

// Function: FPDFPage_New
//			Construct an empty page.
// Parameters:	
//			document	-	Handle to document. Returned by FPDF_LoadDocument and FPDF_CreateNewDocument.
//			page_index	-	The index of a page.
//			width		-	The page width.
//			height		-	The page height.
// Return value:
//			The handle to the page.
// Comments:
//			Loaded page can be deleted by FPDFPage_Delete.
DLLEXPORT FPDF_PAGE STDCALL FPDFPage_New(FPDF_DOCUMENT document, int page_index, double width, double height);

// Function: FPDFPage_Delete
//			Delete a PDF page.
// Parameters:	
//			document	-	Handle to document. Returned by FPDF_LoadDocument and FPDF_CreateNewDocument.
//			page_index	-	The index of a page.
// Return value:
//			None.
DLLEXPORT void STDCALL FPDFPage_Delete(FPDF_DOCUMENT document, int page_index);

// Index values for getting boxes, used in xxx_GetRectangle and xxx_GetBBox functions.
#define FPDF_RECT_PAGE			0	// The visible rectangle.
#define FPDF_RECT_BOUNDING		1	// The bounding box of the all object in this page.
#define FPDF_RECT_MEDIABOX		2	// The boundaries of the physical medium on which the 
									// page is to be printed.
#define FPDF_RECT_CROPBOX		3	// The region to which the contents of the page are to be 
									// clipped (cropped) when displayed or printed. 
#define FPDF_RECT_TRIMBOX		4	// The region to which the contents of the page
									// should be clipped when output in a production environment. 
#define FPDF_RECT_ARTBOX		5	// The intended dimensions of the finished page after trimming. 
#define FPDF_RECT_BLEEDBOX		6	// The extent of the page's meaningful content (including potential
									// white space) as intended by the page's creator.

// Function: FPDFPage_GetRectangle
//			Get page rectangle, in points(1 point equals 1/72 inch).
// Parameters:	
//			page		-	Handle to a page. Returned by FPDFPage_New.
//			iRect		-	The index values for getting boxes defined above.
//			left		-	Pointer to a double number receiving the left of a rectangle (in points).
//			right		-	Pointer to a double number receiving the right of a rectangle (in points).
//			bottom		-	Pointer to a double number receiving the bottom of a rectangle (in points).
//			top			-	Pointer to a double number receiving the top of a rectangle (in points).
// Return value:
//			TRUE if successful, FALSE otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFPage_GetRectangle(FPDF_PAGE page, int iRect, 
								 double* left, double* right, double* bottom, double* top);

// Function: FPDFPage_SetRectangle
//			Set page rectangle, in points (1 point equals 1/72 inch).
// Parameters:	
//			page		-	Handle to a page. Returned by FPDFPage_New.
//			iRect		-	The index values for getting boxes defined above.
//			left		-	The left coordinate of a rectangle (in points).
//			right		-	The right coordinate of a rectangle (in points).
//			bottom		-	The bottom coordinate of a rectangle (in points).
//			top			-	The top coordinate of a rectangle (in points).
// Return value:
//			TRUE if successful, FALSE otherwise.
// Comment:
//			After this method, the content in this page has been changed.
//			You must call the FPDF_LoadPage to reload the page.		
DLLEXPORT FPDF_BOOL STDCALL FPDFPage_SetRectangle(FPDF_PAGE page, int iRect, 
								double left, double right, double bottom, double top);

// Function: FPDFPage_GetRotation
//			Get the page rotation. One of following values will be returned: 0(0), 1(90), 2(180), 3(270).
// Parameters:	
//			page		-	Handle to a page. Returned by FPDFPage_New.
// Return value:
//			The PDF page rotation.
// Comment:
//			The PDF page rotation is rotated clockwise.
DLLEXPORT int STDCALL FPDFPage_GetRotation(FPDF_PAGE page);

// Function: FPDFPage_SetRotation
//			Set page rotation. One of following values will be set: 0(0), 1(90), 2(180), 3(270).
// Parameters:	
//			page		-	Handle to a page. Returned by FPDFPage_New.
//			rotate		-	The value of the PDF page rotation.
// Return value:
//			None.
// Comment:
//			The PDF page rotation is rotated clockwise.
// 
//			After this method, the content in this page has been changed.
//			You must call the FPDF_LoadPage to reload the page.	
DLLEXPORT void STDCALL FPDFPage_SetRotation(FPDF_PAGE page, int rotate);

// Function: FPDFPage_InsertObject
//			Insert an object to the page. The page object is automatically freed.
// Parameters:	
//			page		-	Handle to a page. Returned by FPDFPage_New.
//			page_obj	-	Handle to a page object. Returned by FPDFPageObj_NewTextObj,FPDFPageObj_NewTextObjEx and
//							FPDFPageObj_NewPathObj.
// Return value:
//			None.
DLLEXPORT void STDCALL FPDFPage_InsertObject(FPDF_PAGE page, FPDF_PAGEOBJECT page_obj);

// Function: FPDFPage_DeleteObject
//			Delete an object from the page. The page object is automatically freed.
// Parameters:	
//			page		-	Handle to a page. Returned by FPDFPage_New.
//			index		-	The index of a page object.
// Return value:
//			None.
DLLEXPORT void STDCALL FPDFPage_DeleteObject(FPDF_PAGE page, int index);

// Function: FPDFPage_CountObject
//			Get number of page objects inside the page.
// Parameters:	
//			page		-	Handle to a page. Returned by FPDFPage_New.
// Return value:
//			The number of the page object.
DLLEXPORT int STDCALL FPDFPage_CountObject(FPDF_PAGE page);

// Function: FPDFPage_GetObject
//			Get page object by index.
// Parameters:	
//			page		-	Handle to a page. Returned by FPDFPage_New.
//			index		-	The index of a page object.
// Return value:
//			The handle of the page object. Null for failed.
DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFPage_GetObject(FPDF_PAGE page, int index);

// Function: FPDFPage_GetPageObjectIndex
//			Get the index of the page object in the specify page.
// Parameters:	
//			page		-	Handle to a page. Returned by FPDFPage_New.
//			page_obj	-	Handle to a page object. Returned by FPDFPageObj_NewTextObj,FPDFPageObj_NewTextObjEx,
//							FPDFPageObj_NewPathObj, FPDFPageObj_NewImageObj, etc.
// Return value:
//			If successful, returns the index of the page object.
//			Otherwise, returns -1.
DLLEXPORT int STDCALL FPDFPage_GetPageObjectIndex(FPDF_PAGE page, FPDF_PAGEOBJECT page_obj);


// Function: FPDFPage_GenerateContent
//			Generate PDF Page content.
// Parameters:	
//			page		-	Handle to a page. Returned by FPDFPage_New.
// Return value:
//			True if successful, false otherwise.
// Comment:
//			Before you save the page to a file, or reload the page, you must call the FPDFPage_GenerateContent function.
//			Or the changed information will be lost.
DLLEXPORT FPDF_BOOL STDCALL FPDFPage_GenerateContent(FPDF_PAGE page);

//////////////////////////////////////////////////////////////////////
//
// Page Object functions
//
//////////////////////////////////////////////////////////////////////

// Function: FPDFPageObj_Transform
//			Transform (scale, rotate, shear, move) page object.
// Parameters:	
//			page_object	-	Handle to a page object. Returned by FPDFPageObj_NewTextObj,FPDFPageObj_NewTextObjEx,
//							FPDFPageObj_NewPathObj, FPDFPageObj_NewImageObj, etc.
//			a			-	The coefficient "a" of the matrix.
//			b			-	The	coefficient "b" of the matrix.
//			c			-	The coefficient "c" of the matrix.
//			d			-	The coefficient "d" of the matrix.
//			e			-	The coefficient "e" of the matrix.
//			f			-	The coefficient "f" of the matrix.
// Return value:
//			None.
DLLEXPORT void STDCALL FPDFPageObj_Transform(FPDF_PAGEOBJECT page_object,
							double a, double b, double c, double d, double e, double f);

// Function: FPDFPageObj_Clone
//			Create a new page object based on this page object.
// Parameters:	
//			page_object	-	Handle to a page object. Returned by FPDFPageObj_NewTextObj,FPDFPageObj_NewTextObjEx,
//							FPDFPageObj_NewPathObj, FPDFPageObj_NewImageObj, etc.
// Return value:
//			Handle to a page object.
DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFPageObj_Clone(FPDF_PAGEOBJECT page_object);

// Function: FPDFPageObj_Free
//			Free a page object.
// Parameters:	
//			page_object	-	Handle to a page object. Returned by FPDFPageObj_NewTextObj,FPDFPageObj_NewTextObjEx,
//							FPDFPageObj_NewPathObj, FPDFPageObj_NewImageObj, etc.
// Return value:
//			None.
DLLEXPORT void STDCALL FPDFPageObj_Free(FPDF_PAGEOBJECT page_object);

// The page object constants.
#define FPDF_PAGEOBJ_TEXT		1
#define FPDF_PAGEOBJ_PATH		2
#define FPDF_PAGEOBJ_IMAGE		3
#define FPDF_PAGEOBJ_SHADING	4
#define FPDF_PAGEOBJ_FORM		5

// Function: FPDFPageObj_GetType
//			Get page object type.
// Parameters:	
//			page_object	-	Handle to a page object. Returned by FPDFPageObj_NewTextObj,FPDFPageObj_NewTextObjEx,
//							FPDFPageObj_NewPathObj, FPDFPageObj_NewImageObj, etc.
// Return value:
//			One of the FPDF_PAGEOBJ_xxxx constants defined above.
DLLEXPORT int STDCALL FPDFPageObj_GetType(FPDF_PAGEOBJECT page_object);

// Function: FPDFPageObj_GetBBox
//			Get page object bounding box.
// Parameters:	
//			page_object	-	Handle to a page object. Returned by FPDFPageObj_NewTextObj,FPDFPageObj_NewTextObjEx,
//							FPDFPageObj_NewPathObj, FPDFPageObj_NewImageObj, etc.
//			left		-	Pointer to a double value receiving the left of a rectangle (in points).
//			bottom		-	Pointer to a double value receiving the bottom of a rectangle (in points).
//			right		-	Pointer to a double value receiving the right of a rectangle (in points).
//			top			-	Pointer to a double value receiving the top of a rectangle (in points).
// Return value:
//			None.
DLLEXPORT void STDCALL FPDFPageObj_GetBBox(FPDF_PAGEOBJECT page_object, 
						 double* left, double* bottom, double* right, double* top );

// Function: FPDFPageObj_GetClipCount
//			Get the clip number of a page objcet.
// Parameters:	
//			page_object	-	Handle to a page object. Returned by FPDFPageObj_NewTextObj,FPDFPageObj_NewTextObjEx,
//							FPDFPageObj_NewPathObj, FPDFPageObj_NewImageObj, etc.
// Return value:
//			The clip count.
DLLEXPORT int STDCALL FPDFPageObj_GetClipCount(FPDF_PAGEOBJECT page_object);

// Function: FPDFPageObj_GetClip
//			Get the clip path of a page object.
// Parameters:	
//			page_object	-	Handle to a page object. Returned by FPDFPageObj_NewTextObj,FPDFPageObj_NewTextObjEx,
//							FPDFPageObj_NewPathObj, FPDFPageObj_NewImageObj, etc.
//			index		-	The index of a path.
// Return value:
//			The handle of a path.
DLLEXPORT FPDF_PATH STDCALL FPDFPageObj_GetClip(FPDF_PAGEOBJECT page_object, int index);

// Fill mode types
#define FPDF_FILL_NULL			0	// No fill color.
#define FPDF_FILL_ALTERNATE		1	// Represents a fill mode in which the system fills the area between odd-numbered 
									// and even-numbered polygon sides on each scan line.
#define FPDF_FILL_WINDING		2	// Represents a fill mode in which the system uses the direction in which a figure 
									// was drawn to determine whether to fill the area.

// Function: FPDFPageObj_AddClip
//			Add a clip to a page object.
// Parameters:	
//			page_object	-	Handle to a page object. Returned by FPDFPageObj_NewTextObj,FPDFPageObj_NewTextObjEx,
//							FPDFPageObj_NewPathObj, FPDFPageObj_NewImageObj, etc.
//			path		-	Handle to a path. Returned by FPDFPathObj_GetPath.
//			type		-	The fill mode types defined above.
// Return value:
//			True if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFPageObj_AddClip(FPDF_PAGEOBJECT page_object, FPDF_PATH path, int type);

// Function: FPDFPageObj_AppendPathToClip
//			Append a path to the current clip.
// Parameters:	
//			page_object	-	Handle to a page object. Returned by FPDFPageObj_NewTextObj,FPDFPageObj_NewTextObjEx,
//							FPDFPageObj_NewPathObj, FPDFPageObj_NewImageObj, etc.
//			path		-	Handle to a path. Returned by FPDFPathObj_GetPath.
//			type		-	The fill mode types defined above.
// Return value:
//			True if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFPageObj_AppendPathToClip(FPDF_PAGEOBJECT page_object, FPDF_PATH path, int type);

// Function: FPDFPageObj_RemoveClip
//			Remove a clip from the page object.
// Parameters:	
//			page_object	-	Handle to a page object. Returned by FPDFPageObj_NewTextObj,FPDFPageObj_NewTextObjEx,
//							FPDFPageObj_NewPathObj,FPDFPageObj_NewImageObj etc.
//			index		-	The index of the clip.
// Return value:
//			None.
DLLEXPORT void STDCALL FPDFPageObj_RemoveClip(FPDF_PAGEOBJECT page_object, int index);

// Function: FPDFPageObj_GetFillColor
//			Get the fill-color of a page object.
// Parameters:	
//			page_object	-	Handle of page object, could be text_object, path_object and uncolored images.
// Return value:
//			The fill-color of a page object. Constructed by 0xaarrggbb.
DLLEXPORT FPDF_DWORD STDCALL FPDFPageObj_GetFillColor(FPDF_PAGEOBJECT page_object);

// Function: FPDFPageObj_SetFillColor
//			Set the fill color of a page object.
// Parameters:	
//			page_object	-	Handle of page object, could be text_object, path_object and uncolored images.
//			argb		-	The fill color of a page object. Constructed by 0xaarrggbb.
// Return value:
//			None.
DLLEXPORT void STDCALL FPDFPageObj_SetFillColor(FPDF_PAGEOBJECT page_object, FPDF_DWORD argb);

// Function: FPDFPageObj_GetStrokeColor
//			Get the stroke color of a page object.
// Parameters:
//			page_object	-	Handle of page object, could be text_object, path_object and uncolored images.
// Return value:
//			The stroke color of a page object. Constructed by 0xaarrggbb.
DLLEXPORT FPDF_DWORD STDCALL FPDFPageObj_GetStrokeColor(FPDF_PAGEOBJECT page_object);

// Function: FPDFPageObj_SetStrokeColor
//			Set the stroke color of a page object.
// Parameters:
//			page_object	-	Handle of page object, could be text_object, path_object and uncolored images.
//			argb		-	The stroke color of a page object. Constructed by 0xaarrggbb.
// Return value:
//			None.
DLLEXPORT void STDCALL FPDFPageObj_SetStrokeColor(FPDF_PAGEOBJECT page_object, FPDF_DWORD argb);

// Function: FPDFPageObj_GetLineWidth
//			Get the line width of a path object.
// Parameters:
//			page_object		-	Handle of page object. It could be path_object or stroke text_object.
// Return Value:
//			Return the line width of the page object.
DLLEXPORT double STDCALL FPDFPageObj_GetLineWidth(FPDF_PAGEOBJECT page_object);

// Function: FPDFPageObj_SetLineWidth
//			Set the line width of a path object.
// Parameters:
//			page_object		-	Handle of page object. It could be path_object or stroke text_object.
//			width			-	The line width of the page object.
// Return Value:
//			None.
DLLEXPORT void STDCALL FPDFPageObj_SetLineWidth(FPDF_PAGEOBJECT page_object, double width);

// Line cap styles
#define FPDF_LINECAP_BUTT		0	// The stroke is squared off at the endpoint of the path. 
									// There is no projection beyond the end of the path. 
#define FPDF_LINECAP_ROUND		1	// A semicircular arc with a diameter equal to the line width is 
									// drawn around the endpoint and filled in. 
#define FPDF_LINECAP_PROJECT	2	// The stroke continues beyond the endpoint of the path 
									// for a distance equal to half the line width and is squared off. 

// Function: FPDFPageObj_GetLineCapStyle
//			Get the line cap style of a page object.
// Parameters:
//			page_object		-	Handle of page object. It could be path_object or stroke text_object.
// Return Value:
//			The line cap styles defined above.
DLLEXPORT int STDCALL FPDFPageObj_GetLineCapStyle(FPDF_PAGEOBJECT page_object);

// Function: FPDFPageObj_SetLineCapStyle
//			Set the line cap style of a page object.
// Parameters:
//			page_object		-	Handle of page object. It could be path_object or stroke text_object.
//			style			-	The line cap styles defined above.
// Return Value:
//			None.
DLLEXPORT void STDCALL FPDFPageObj_SetLineCapStyle(FPDF_PAGEOBJECT page_object, int style);

// Line join styles
#define FPDF_LINEJOIN_MITER		0	// The outer edges of the strokes for the two segments are extended 
									// until they meet at an angle
#define FPDF_LINEJOIN_ROUND		1	// An arc of a circle with a diameter equal to the line width is drawn 
									// around the point where the two segments meet, connecting the outer edges of 
									// the strokes for the two segments. 
#define FPDF_LINEJOIN_BEVEL		2	// The two segments are finished with butt caps and the resulting notch beyond
									// the ends of the segments is filled with a triangle. 

// Function: FPDFPageObj_GetLineJoinStyle
//			Get the line-join style of a page object.
// Parameters:
//			page_object		-	Handle of page object. It could be path_object or stroke text_object.
// Return Value:
//			Return the line-join styles defined above.
DLLEXPORT int STDCALL FPDFPageObj_GetLineJoinStyle(FPDF_PAGEOBJECT page_object);

// Function: FPDFPageObj_SetLineJoinStyle
//			Set the line-join style of a page object.
// Parameters:
//			page_object		-	Handle of page object. It could be path_object or stroke text_object.
//			style			-	The line-join styles defined above.
// Return Value:
//			None.
DLLEXPORT void STDCALL FPDFPageObj_SetLineJoinStyle(FPDF_PAGEOBJECT page_object, int style);

// Function: FPDFPageObj_GetMiterLimit
//			Get the miter limit of a page object.
// Parameters:
//			page_object		-	Handle of page object. It could be path_object or stroke text_object.
// Return Value:
//			The miter limit of a page_object.
// Comments:
//			The miter limit imposes a maximum on the ratio of the miter length to the line width. 
//			When the limit is exceeded, the join is converted from a miter to a bevel. 
DLLEXPORT double STDCALL FPDFPageObj_GetMiterLimit(FPDF_PAGEOBJECT page_object);

// Function: FPDFPageObj_SetMiterLimit
//			Set the miter limit of a page object.
// Parameters:
//			page_object		-	Handle of page object. It could be path_object or stroke text_object.
//			miter_limit		-	The miter limit of a page_object.
// Return Value:
//			None.
// Comments:
//			The miter limit imposes a maximum on the ratio of the miter length to the line width. 
//			When the limit is exceeded, the join is converted from a miter to a bevel. 
DLLEXPORT void STDCALL FPDFPageObj_SetMiterLimit(FPDF_PAGEOBJECT page_object, double miter_limit);

// Function: FPDFPageObj_GetDashCount
//			Get dash count of a page object.
// Parameters:
//			page_object		-	Handle of page object. It could be path_object or stroke text_object.
// Return Value:
//			The dash count of a page object.
DLLEXPORT int STDCALL FPDFPageObj_GetDashCount(FPDF_PAGEOBJECT page_object);

// Function: FPDFPageObj_SetDashCount
//			Set dash count of a page object.
// Parameters:
//			page_object		-	Handle of page object. It could be path_object or stroke text_object.
//			count			-	The dash count of a page object.
// Return Value:
//			None.
// Comments:
//			Setting the count will release the old dash array of the page object and allocate a new dash array.
DLLEXPORT void STDCALL FPDFPageObj_SetDashCount(FPDF_PAGEOBJECT page_object, int count);

// Function: FPDFPageObj_GetDashArray
//			Get dash array elements of a page object.
// Parameters:
//			page_object		-	Handle of page object. It could be path_object or stroke text_object.
//			index			-	The index of a dash array element.
// Return Value:
//			Return the dash value of the specific index of the dash array.
// Comments:
//			The dash array's elements are numbers that specify the lengths of alternating dashes and gaps; 
//			the numbers must be nonnegative and not all zero.
DLLEXPORT double STDCALL FPDFPageObj_GetDashArray(FPDF_PAGEOBJECT page_object, int index);

// Function: FPDFPageObj_SetDashArray
//			Set dash array elements of a page object.
// Parameters:
//			page_object		-	Handle of page object. It could be path_object or stroke text_object.
//			index			-	The index of a dash array element.
//			dash_value		-	The dash value of specific index of the dash array.
// Return Value:
//			None.
// Comments:
//			The dash array's elements are numbers that specify the lengths of alternating dashes and gaps; 
//			the numbers must be nonnegative and not all zero.
DLLEXPORT void STDCALL FPDFPageObj_SetDashArray(FPDF_PAGEOBJECT page_object, int index, double dash_value);

// Function: FPDFPageObj_GetDashPhase
//			Get dash phase of a page object.
// Parameters:
//			page_object		-	Handle of page object. It could be path_object or stroke text_object.
// Return Value:
//			The dash phase value of a page object.
// Comments:
//			The dash phase specifies the distance into the dash pattern at which to start the dash. 
DLLEXPORT double STDCALL FPDFPageObj_GetDashPhase(FPDF_PAGEOBJECT page_object);

// Function: FPDFPageObj_SetDashPhase
//			Set dash phase of a page object.
// Parameters:
//			page_object		-	Handle of page object. It could be path_object or stroke text_object.
//			phase_value		-	The dash phase value.
// Return Value:
//			None.
// Comments:
//			The dash phase specifies the distance into the dash pattern at which to start the dash. 
DLLEXPORT void STDCALL FPDFPageObj_SetDashPhase(FPDF_PAGEOBJECT page_object, double phase_value);


//////////////////////////////////////////////////////////////////////
//
// Text functions
//
//////////////////////////////////////////////////////////////////////

// Function: FPDFPageObj_NewTextObj
//			Create a new Text Object.
// Parameters:
//			None.
// Return Value:
//			Handle of text object.
DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFPageObj_NewTextObj();

// Function: FPDFPageObj_NewTextObjEx
//			Create a new Text Object.
// Parameters:
//			text		-	Pointer to a string. The text you want to add in a text object.
//			nwSize		-	The length of the string.
//			font		-	Handle of FPDF_FONT. It could be returned by FPDFTextObj_GetFont, 
//							FPDFFont_AddTrueType and FPDFFont_AddStandardFont.
// Return Value:
//			Handle of text object.
DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFPageObj_NewTextObjEx(FPDF_WIDESTRING text, int nwSize, FPDF_FONT font);

// Function: FPDFTextObj_GetFont
//			Get the font handle of a text object.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.
// Return Value:
//			Handle of the font.
DLLEXPORT FPDF_FONT STDCALL FPDFTextObj_GetFont(FPDF_PAGEOBJECT text_object);

// Function: FPDFTextObj_SetFont
//			Set the font into a text object.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.
//			font		-	Handle of FPDF_FONT. It could be returned by FPDFTextObj_GetFont, FPDFFont_AddTrueType 
//							and FPDFFont_AddStandardFont.  
//			font_size	-	The size of the font.  
// Return Value:
//			None.
DLLEXPORT void STDCALL FPDFTextObj_SetFont(FPDF_PAGEOBJECT text_object, FPDF_FONT font, double font_size);

// Function: FPDFTextObj_GetFontSize
//			Get the font size of a text object.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.		
// Return Value:
//			The value of the font size.
DLLEXPORT double STDCALL FPDFTextObj_GetFontSize(FPDF_PAGEOBJECT text_object);

// Function: FPDFTextObj_GetMatrix
//			Get the text object matrix.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
//			a			-	Pointer to a double value receiving the coefficient "a" of the matrix.
//			b			-	Pointer to a double value receiving the coefficient "b" of the matrix.
//			c			-	Pointer to a double value receiving the coefficient "c" of the matrix.
//			d			-	Pointer to a double value receiving the coefficient "d" of the matrix.
//			e			-	Pointer to a double value receiving the coefficient "e" of the matrix.
//			f			-	Pointer to a double value receiving the coefficient "f" of the matrix.
// Return Value:
//			None.
DLLEXPORT void STDCALL FPDFTextObj_GetMatrix(FPDF_PAGEOBJECT text_object,
							double* a, double* b, double* c, double* d, double* e, double* f);

// Function: FPDFTextObj_SetMatrix
//			Set the text object matrix.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
//			a			-	The coefficient "a" of the matrix.
//			b			-	The coefficient "b" of the matrix.
//			c			-	The coefficient "c" of the matrix.
//			d			-	The coefficient "d" of the matrix.
//			e			-	The coefficient "e" of the matrix.
//			f			-	The coefficient "f" of the matrix.
// Return Value:
//			None.
DLLEXPORT void STDCALL FPDFTextObj_SetMatrix(FPDF_PAGEOBJECT text_object,
							double a, double b, double c, double d, double e, double f);

// Function: FPDFTextObj_CountChars
//			Get the number of characters in a text object.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
// Return Value:
//			A character count in the text object.
DLLEXPORT int STDCALL FPDFTextObj_CountChars(FPDF_PAGEOBJECT text_object);

// Function: FPDFTextObj_GetUincode
//			Get the Unicode of a special character in a text object.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
//			index		-	The index of the character to get the Unicode. 
// Return Value:
//			The Unicode value.
DLLEXPORT unsigned int STDCALL FPDFTextObj_GetUincode(FPDF_PAGEOBJECT text_object, int index);

// Function: FPDFTextObj_SetUnicode
//			Set the Unicode of a special character in a text object.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
//			index		-	The index of the character to set unicode.
//			nUnicode	-	The Unicode value.
// Return Value:
//			TRUE if successful, FALSE otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFTextObj_SetUnicode(FPDF_PAGEOBJECT text_object,int index, int nUnicode);

// Function: FPDFTextObj_GetOffset
//			Calculate the physical offset from the start to the specified special character. 
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
//			index		-	The index of the character to calculate the offset to.
// Return Value:
//			Return the offset value.
DLLEXPORT double STDCALL FPDFTextObj_GetOffset(FPDF_PAGEOBJECT text_object, int index);

// Text rendering modes
#define  FPDF_TEXTMODE_FILL				0	// Fill text. 
#define  FPDF_TEXTMODE_STROKE			1	// Stroke text. 
#define  FPDF_TEXTMODE_FILLANDSTROKE	2	// Fill, then stroke text. 
#define  FPDF_TEXTMODE_INVISIBLE		3	// Neither fill nor stroke text (invisible). 

// Function: FPDFTextObj_GetTextMode
//			Get the text-rendering mode.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
// Return Value:
//			Return one of the enumeration values defined in text-rendering modes.
DLLEXPORT int STDCALL FPDFTextObj_GetTextMode(FPDF_PAGEOBJECT text_object);

// Function: FPDFTextObj_SetTextMode
//			Set the text-rendering mode.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
//			text_mode	-	One of the enumeration values defined in text-rendering modes.
// Return Value:
//			TRUE if successful, FALSE otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFTextObj_SetTextMode(FPDF_PAGEOBJECT text_object, int text_mode);

// Function: FPDFTextObj_GetCharSpace
//			Get the character-spacing value of a text object.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
// Return Value:
//			The character-spacing value.
// Comment:
//			The character-spacing value is a number specified in unscaled text space units.
DLLEXPORT double STDCALL FPDFTextObj_GetCharSpace(FPDF_PAGEOBJECT text_object);

// Function: FPDFTextObj_SetCharSpace
//			Set the character-spacing value of a text object.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
//			charspace	-	The character-spacing value. 	
// Return Value:
//			TRUE if successful, FALSE otherwise.
// Comment:
//			The character-spacing value is a number specified in unscaled text space units. 
DLLEXPORT FPDF_BOOL STDCALL FPDFTextObj_SetCharSpace(FPDF_PAGEOBJECT text_object, double charspace);

// Function: FPDFTextObj_GetWordSpace
//			Get the word-spacing value of a text object.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
// Return Value:
//			The word-spacing value.
// Comment:
//			The word-spacing value  is a number expressed in unscaled text space units.
DLLEXPORT double STDCALL FPDFTextObj_GetWordSpace(FPDF_PAGEOBJECT text_object);

// Function: FPDFTextObj_SetWordSpace
//			Set the word-spacing value of a text object.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
//			wordspace	-	The word-spacing value.
// Return Value:
//			TRUE if successful, FALSE otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFTextObj_SetWordSpace(FPDF_PAGEOBJECT text_object, double wordspace);

// Function: FPDFTextObj_Insert
//			Insert a substring at the given index within the text object.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
//			text		-	A pointer to the substring to be inserted.
//			nwSize		-	The length of the substring.
//			index		-	The index of the character before which the insertion will take place.
// Return Value:
//			Return the length of the changed string. 
//Comment:
//		The index can be between 0 and the number of characters in a text object.
//		(returned by FPDFTextObj_CountChars).
DLLEXPORT int STDCALL FPDFTextObj_Insert(FPDF_PAGEOBJECT text_object, FPDF_WIDESTRING text, int nwSize, int index);

// Function: FPDFTextObj_Delete
//			Delete a character or characters in the text object.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
//			index		-	The index of the first character to delete.
//			nCount		-	The number of characters to be removed.
// Return Value:
//			Return the character number of the changed text object.
//Comment:
//      Call this member function to delete a character or characters from a string starting with the character at index. 
//		If nCount is longer than the string, the remainder of the string will be removed.
DLLEXPORT int STDCALL FPDFTextObj_Delete(FPDF_PAGEOBJECT text_object, int index, int nCount = 1);

// Function: FPDFTextObj_Find
//			Find a substring inside a text object.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
//			text		-	A pointer to a string to search for.
//			nwSize		-	The length of the string.
//			nStart		-	The index of the character in the string to begin the search with, 
//							or 0 to start from the beginning.
// Return Value:
//			Return the zero-based index of the first character in this text object that matches the requested 
//			substring or characters; -1 if the substring or character is not found.
DLLEXPORT int STDCALL FPDFTextObj_Find(FPDF_PAGEOBJECT text_object, FPDF_WIDESTRING text, int nwSize, int nStart = 0);

// Function: FPDFTextObj_Replace
//			Replace a character with another.
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
//			OldText		-	A pointer to a string containing the character to be replaced by NewText.
//			nwOldSize	-	The length of the old text.
//			NewText		-	A pointer to a string containing the character replacing OldText.
//			nwNewSize	-	The length of the new text.
// Return Value:
//			Return the number of characters of the changed text object. -1 if the text object isn't changed.
DLLEXPORT int STDCALL FPDFTextObj_Replace(FPDF_PAGEOBJECT text_object, FPDF_WIDESTRING OldText, int nwOldSize, 
											 FPDF_WIDESTRING NewText, int nwNewSize);

// Function: FPDFTextObj_Compare
//			Compare current text object characters with another string. 
// Parameters:
//			text_object	-	Handle of text object returned by FPDFPageObj_NewTextObj or FPDFPageObj_NewTextObjEx.	
//			text		-	The string used for comparison.
//			nwSize		-	The length of the string.
// Return Value:
//			TRUE if the texts are identical, and FALSE if they are different.
DLLEXPORT FPDF_BOOL STDCALL FPDFTextObj_Compare(FPDF_PAGEOBJECT text_object, FPDF_WIDESTRING text, int nwSize);

//////////////////////////////////////////////////////////////////////
//
// Font functions
//
//////////////////////////////////////////////////////////////////////
// Get font information: name, flags, char width, bounding box, encoding.
// All metrics values are measured in 1/1000 of font size.

// Function: FPDFFont_GetFontName
//			Retrieve the face name of the font and return the name in a string.
// Parameters:
//			font		-	Handle of the font object.
// Return Value:
//			Pointer to the string.
DLLEXPORT FPDF_BYTESTRING STDCALL FPDFFont_GetFontName(FPDF_FONT font);

// The Font flags
#define FPDF_FLAG_FIXEDPITCH		1				// All glyphs have the same width.
#define FPDF_FLAG_SERIF				2				// Glyphs have serifs, which are short strokes drawn at an 
													// angle on the top and bottom of glyph stems. 
#define FPDF_FLAG_SYMBOLIC			4				// Font contains glyphs outside the Adobe standard Latin 
													// character set. This flag and the Nonsymbolic flag cannot
													// both be set or both be clear. 
#define FPDF_FLAG_SCRIPT			8				// Glyphs resemble cursive handwriting.
#define FPDF_FLAG_NONSYMBOLIC		32				// Font uses the Adobe standard Latin character set or a subset of it. 
#define FPDF_FLAG_ITALIC			64				// Glyphs have dominant vertical strokes that are slanted.
#define FPDF_FLAG_ALLCAP			0x10000			// Font contains no lowercase letters; typically used for 
													// display purposes, such as for titles or headlines. 
#define FPDF_FLAG_SMALLCAP			0x20000			// Font contains both uppercase and lowercase letters.
#define FPDF_FLAG_FORCEBOLD			0x40000			// Whether bold glyphs are painted with extra pixels even 
													// at very small text sizes.

// Function: FPDFFont_GetFlags
//			Get the value of the Flags entry in a font descriptor.
// Parameters:
//			font		-	Handle of the font object.	
// Return Value:
//			The value of the Flags entry defined above.
// Comment:
//			The value of the Flags entry in a font descriptor is an unsigned 32-bit integer containing flags 
//			specifying various characteristics of the font.
DLLEXPORT int STDCALL FPDFFont_GetFlags(FPDF_FONT font);

// Function: FPDFFont_GetBBox
//			Get the font bounding box.
// Parameters:
//			font		-	Handle of the font object.	
//			left		-	Pointer to a double value receiving the left of a rectangle (in points).
//			right		-	Pointer to a double value receiving the right of a rectangle (in points).
//			bottom		-	Pointer to a double value receiving the bottom of a rectangle (in points).
//			top			-	Pointer to a double value receiving the top of a rectangle (in points).
// Return Value:
//			None.
DLLEXPORT void STDCALL FPDFFont_GetBBox(FPDF_FONT font, double* left, double* right, double* bottom, double* top);

// Function: FPDFFont_GetCharBBox
//			Get the special character bounding box of a font object.
// Parameters:
//			font		-	Handle of the font object.	
//			char_code	-	The character code.
//			left		-	Pointer to a double value receiving the left of a rectangle (in points).
//			right		-	Pointer to a double value receiving the right of a rectangle (in points).
//			bottom		-	Pointer to a double value receiving the bottom of a rectangle (in points).
//			top			-	Pointer to a double value receiving the top of a rectangle (in points).
// Return Value:
//			None.
DLLEXPORT void STDCALL FPDFFont_GetCharBBox(FPDF_FONT font, FPDF_DWORD char_code, double* left, double* right, double* bottom, double* top);

// Function: FPDFFont_GetCharWidth
//			Get the special character width of a font object.
// Parameters:
//			font		-	Handle of the font object.		
//			char_code	-	The character code.	
// Return Value:
//			The value of the width.
DLLEXPORT int STDCALL FPDFFont_GetCharWidth(FPDF_FONT font, FPDF_DWORD char_code);

// Function: FPDFFont_GetUnicode
//			 Get the Unicode value from a character code.
// Parameters:
//			font		-	Handle of the font object.		
//			char_code	-	The character code.	
// Return Value:
//			The Unicode value.
DLLEXPORT unsigned short STDCALL FPDFFont_GetUnicode(FPDF_FONT font, FPDF_DWORD char_code);

// Function: FPDFFont_GetCharSize
//			Get the character size in the font.
// Parameters:
//			font		-	Handle of the font object.	
//			char_code	-	The character code.	
// Return Value:
//			The value of the character size.
DLLEXPORT int STDCALL FPDFFont_GetCharSize(FPDF_FONT font, FPDF_DWORD char_code);	

// Function: FPDFFont_GetCharCode
//			Get the character code from a Unicode value.
// Parameters:
//			font		-	Handle of the font object.	
//			unicode		-	The unicode of a special character.	
// Return Value:
//			The character code value.
DLLEXPORT FPDF_DWORD STDCALL FPDFFont_GetCharCode(FPDF_FONT font, unsigned short unicode);

// Function: FPDFFont_GetFontDescent
//			Get the descent value in the font.
// Parameters:
//			font		-	Handle of the font object.	
// Return Value:
//			The descent value.
// Comment:
//			The descent value describes the maximum depth below the baseline reached by glyphs in this font. 
//			The value is a negative number. 
DLLEXPORT int STDCALL FPDFFont_GetFontDescent	(FPDF_FONT font);

// Function: FPDFFont_GetFontAscent
//			Get the ascent value in the font.
// Parameters:
//			font		-	Handle of the font object.		
// Return Value:
//			The ascent value.
// Comment:
//			The ascent value describes the maximum height above the baseline reached by glyphs in this font, 
//			excluding the height of glyphs for accented characters.
DLLEXPORT int STDCALL FPDFFont_GetFontAscent(FPDF_FONT font);

// The tagFPDFLOGFONT structure defines the attributes of a font.
typedef struct tagFPDFLOGFONT {
	
	long lfWeight;					// Specifies the weight of the font in the range 0 through 1000.For example,
									// 400 is normal and 700 is bold. If this value is zero, a default weight is
									// used. Not support yet, reserved.
	unsigned char lfItalic;			// Specifies an italic font if set to TRUE. Not support yet, reserved.
	unsigned char lfCharSet;		// Specifies the character set.
	unsigned char lfPitchAndFamily; // Specifies the pitch and family of the font.
	char lfFaceName[32];			// A null-terminated string that specifies the typeface name of the font. 
} FPDFLOGFONT, *PFPDFLOGFONT; 

// Function: FPDFFont_AddTrueType
//			Create a true type font and add the font into the document.
// Parameters:
//			document	-	Handle to document. Returned by FPDF_LoadDocument and FPDF_CreateNewDocument.
//			pLogFont	-	The tagFPDFLOGFONT structure defined above.
// Return Value:
//			Handle of the new font object.
DLLEXPORT FPDF_FONT STDCALL FPDFFont_AddTrueType(FPDF_DOCUMENT document, PFPDFLOGFONT pLogFont);

// Font encoding
#define FPDF_ENCODING_DEFAULT		0 	// Built-in encoding.
#define FPDF_ENCODING_WINANSI		1	// WinAnsiEncoding.
#define FPDF_ENCODING_MACROMAN		2	// MacRomanEncoding.
#define FPDF_ENCODING_MACEXPERT		3	// MacExpertEncoding.
#define FPDF_ENCODING_STANDARD		4	// StandardEncoding. Adobe standard encoding. 
#define FPDF_ENCODING_IDENTITY		10	// Not support yet. Reserved.
#define FPDF_ENCODING_GB			11	// Not support yet. Reserved.
#define FPDF_ENCODING_BIG5			12	// Not support yet. Reserved.
#define FPDF_ENCODING_SHIFT_JIS		13	// Not support yet. Reserved.
#define FPDF_ENCODING_KOREAN		14	// Not support yet. Reserved.

// Function: FPDFFont_AddStandardFont
//			Create a standard type font and add the font into the document.
// Parameters:
//			document	-	Handle to document. Returned by FPDF_LoadDocument and FPDF_CreateNewDocument.
//			font_name	-	A null-terminated string that specifies the typeface name of the font. 
//			encoding	-	The font encoding defined above.
// Return Value:
//			Handle of the new font object.
DLLEXPORT FPDF_FONT STDCALL FPDFFont_AddStandardFont(FPDF_DOCUMENT document, char* font_name, int encoding);


//////////////////////////////////////////////////////////////////////
//
// Path functions
//
//////////////////////////////////////////////////////////////////////

// Function: FPDFPageObj_NewPathObj
//			Create a new path Object.
// Parameters:
//			None.
// Return Value:
//			Handle of path object.
DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFPageObj_NewPathObj();

// Path point types
#define FPDF_PATH_CLOSEFIGURE	1	// Specifies that the figure is automatically closed after the corresponding line 
									// or curve is drawn. The figure is closed by drawing a line from the line or 
									// curve endpoint to the point corresponding to the last PT_MOVETO.
#define FPDF_PATH_LINETO		2	// Specifies that the previous point and the corresponding point in lpPoints are
									// the endpoints of a line.
#define FPDF_PATH_BEZIERTO		4	// Specifies that the corresponding point in lpPoints is a control point or 
									// ending point for a B¨¦zier curve.
#define FPDF_PATH_MOVETO		6	// Specifies that the corresponding point in lpPoints starts a disjoint figure.

// Function: FPDFPathObj_SetPoints
//			Set the points in a path object.
// Parameters:
//			path_object		-	Handle of path object returned by FPDFPageObj_NewPathObj.
//			point_count		-	The total number of points.
//			point_x			-	The pointer to point at an array of X coordinate of points.
//			point_y			-	The pointer to point at an array of Y coordinate of points.
//			flags			-	The pointer to point at an array of the path point types defined above.
// Return Value:
//			None.
DLLEXPORT void STDCALL FPDFPathObj_SetPoints(FPDF_PAGEOBJECT path_object, int point_count, double* point_x, double* point_y, int* flags);

// Function: FPDFPathObj_InsertPoint
//			Insert a point to a path object.
// Parameters:
//			path_object		-	Handle of path object returned by FPDFPageObj_NewPathObj.
//			x				-	The X coordinate of a point.
//			y				-	The Y coordinate of a point.
//			flag			-	The path point types defined above.
//			point_index		-	The index of the point.
// Return Value:
//			None.
DLLEXPORT void STDCALL FPDFPathObj_InsertPoint(FPDF_PAGEOBJECT path_object, double x, double y, int flag, int point_index = -1);

// Function: FPDFPathObj_RemovePoint
//			Remove a point in a path object.
// Parameters:
//			path_object		-	Handle of path object returned by FPDFPageObj_NewPathObj.
//			point_index		-	The index of the point.
// Return Value:
//			None.
DLLEXPORT void STDCALL FPDFPathObj_RemovePoint(FPDF_PAGEOBJECT path_object, int point_index);///Insert clip

// Function: FPDFPathObj_ClearPoints
//			Clear the points in a path object.
// Parameters:
//			path_object		-	Handle of path object returned by FPDFPageObj_NewPathObj.
// Return Value:
//			None.
DLLEXPORT void STDCALL FPDFPathObj_ClearPoints(FPDF_PAGEOBJECT path_object);

// Function: FPDFPathObj_CountPoints
//			Get the number of points in a path object.
// Parameters:
//			path_object		-	Handle of path object returned by FPDFPageObj_NewPathObj.
// Return Value:
//			The point count in a path object.
DLLEXPORT int STDCALL FPDFPathObj_CountPoints(FPDF_PAGEOBJECT path_object);

// Function: FPDFPathObj_GetPointType
//			Get the type of a special point.
// Parameters:
//			path_object		-	Handle of path object returned by FPDFPageObj_NewPathObj.
//			point_index		-	The index of the point.
// Return Value:
//			The path point types defined above.
DLLEXPORT int STDCALL FPDFPathObj_GetPointType(FPDF_PAGEOBJECT path_object, int point_index);

// Function: FPDFPathObj_GetPointX
//			Get the X coordinate of a specific point.
// Parameters:
//			path_object		-	Handle of path object returned by FPDFPageObj_NewPathObj.
//			point_index		-	The index of the point.
// Return Value:
//			The value of the X coordinate of the point.
DLLEXPORT double STDCALL FPDFPathObj_GetPointX(FPDF_PAGEOBJECT path_object, int point_index);

// Function: FPDFPathObj_GetPointY
//			Get the Y coordinate of a specific point.
// Parameters:
//			path_object		-	Handle of path object returned by FPDFPageObj_NewPathObj.
//			point_index		-	The index of the point.
// Return Value:
//			The value of the Y coordinate of the point.
DLLEXPORT double STDCALL FPDFPathObj_GetPointY(FPDF_PAGEOBJECT path_object, int point_index);

// Function: FPDFPathObj_GetPath
//			Get the path in a path object.
// Parameters:
//			path_object		-	Handle of path object returned by FPDFPageObj_NewPathObj.
// Return Value:
//			The path in the path object.
DLLEXPORT FPDF_PATH STDCALL FPDFPathObj_GetPath(FPDF_PAGEOBJECT path_object);

// Function: FPDFPathObj_GetStroke
//			Get the stroke mode of the path.
// Parameters:
//			path_object		-	Handle of path object returned by FPDFPageObj_NewPathObj.
// Return Value:
//			Return the stroke mode, TRUE for stroking the line, FALSE for no stroking.
DLLEXPORT FPDF_BOOL STDCALL FPDFPathObj_GetStroke(FPDF_PAGEOBJECT path_object);

// Function: FPDFPathObj_GetFillMode
//			Get the fill mode of the path.
// Parameters:
//			path_object		-	Handle of path object returned by FPDFPageObj_NewPathObj.
// Return Value:
//			Return fill mode types defined above.
DLLEXPORT int STDCALL FPDFPathObj_GetFillMode(FPDF_PAGEOBJECT path_object);

// Function: FPDFPathObj_SetFillStroke
//			Set the fill and stroke mode of the path.
// Parameters:
//			path_object		-	Handle of path object returned by FPDFPageObj_NewPathObj.
//			fill_type		-	The fill mode enumeration defined above.
//			stroke			-	The stroke mode. TRUE for stroking the line, FALSE for no stroking.
// Return Value:
//			None.
DLLEXPORT void STDCALL FPDFPathObj_SetFillStroke(FPDF_PAGEOBJECT path_object, int fill_type, int stroke);	

// Function: FPDFPathObj_GetMatrix
//			Get the matrix of the path.
// Parameters:
//			path_object		-	Handle of path object returned by FPDFPageObj_NewPathObj.
//			a				-	Pointer to a double value receiving the coefficient "a" of the matrix.
//			b				-	Pointer to a double value receiving the coefficient "b" of the matrix.
//			c				-	Pointer to a double value receiving the coefficient "c" of the matrix.
//			d				-	Pointer to a double value receiving the coefficient "d" of the matrix.
//			e				-	Pointer to a double value receiving the coefficient "e" of the matrix.
//			f				-	Pointer to a double value receiving the coefficient "f" of the matrix.
// Return Value:
//			TRUE for successful, FALSE for failed.
DLLEXPORT FPDF_BOOL STDCALL FPDFPathObj_GetMatrix(FPDF_PAGEOBJECT path_object, 
									double* a, double* b, double* c, double* d,double* e, double* f);

// Function: FPDFPathObj_SetMatrix
//			Set the matrix of the path.
// Parameters:
//			image_object	-	Handle of path object returned by FPDFPageObj_NewPathObj.
//			a				-	The coefficient "a" of the matrix.
//			b				-	The coefficient "b" of the matrix.
//			c				-	The coefficient "c" of the matrix.
//			d				-	The coefficient "d" of the matrix.
//			e				-	The coefficient "e" of the matrix.
//			f				-	The coefficient "f" of the matrix.
// Return value:
//			TRUE for successful, FALSE for failed.
DLLEXPORT FPDF_BOOL STDCALL FPDFPathObj_SetMatrix(FPDF_PAGEOBJECT path_object, 
									double a, double b, double c, double d, double e, double f);

//////////////////////////////////////////////////////////////////////
//
// Form functions
//
//////////////////////////////////////////////////////////////////////

// Function: FPDFFormObj_InsertSubObject
//			Insert a page object to the form object. The page object is automatically freed.
// Parameters:	
//			form_object	-	Handle to a form object. Returned by FPDFPage_GetObject.
//			page_obj	-	Handle to a page object. Returned by FPDFPageObj_NewTextObj,FPDFPageObj_NewTextObjEx,
//							FPDFPageObj_NewPathObj, FPDFPageObj_ImageObj, etc.
// Return value:
//			None.
DLLEXPORT void STDCALL FPDFFormObj_InsertSubObject(FPDF_PAGEOBJECT form_object, FPDF_PAGEOBJECT page_obj);

// Function: FPDFFormObj_Display
//			Render contents in a form_object to a device (screen, bitmap, or printer).
// Parameters: 
//			form_object	-	Handle to the form_object. Returned by FPDFPage_GetObject function.
//			device		-	Handle to device context.
//			xPos		-	Left pixel position of the display area in the device coordinate.
//			yPos		-	Top pixel position of the display area in the device coordinate.
//			xSize		-	Horizontal size (in pixels) for displaying the page.
//			ySize		-	Vertical size (in pixels) for displaying the page.
//			iRotate		-	Page orientation: 0 (normal), 1 (rotated 90 degrees clockwise),
//								2 (rotated 180 degrees), 3 (rotated 90 degrees counter-clockwise).
// Return value:
//			None.
DLLEXPORT void STDCALL FPDFFormObj_Display(FPDF_PAGEOBJECT form_object,void* device,int xPos,int yPos,int xSize, int ySize,int iRotate);

// Function: FPDFFormObj_DeleteSubObject
//			Delete a page object from the form object. The page object is automatically freed.
// Parameters:	
//			form_object	-	Handle to a form object. Returned by FPDFPage_GetObject.
//			index		-	The index of a page object.
// Return value:
//			None.
DLLEXPORT void STDCALL FPDFFormObj_DeleteSubObject(FPDF_PAGEOBJECT form_object, int index);

// Function: FPDFFormObj_CountSubObjects
//			Get the number of page objects inside the form object.
// Parameters:	
//			form_object	-	Handle to a form object. Returned by FPDFPage_GetObject.
// Return value:
//			The number of the page objects.
DLLEXPORT int STDCALL FPDFFormObj_CountSubObjects(FPDF_PAGEOBJECT form_object);

// Function: FPDFFormObj_GetSubObject
//			Get the page object in form object.
// Parameters:	
//			form_object	-	Handle to a form object. Returned by FPDFPage_GetObject.
//			index		-	The index of a page object.
// Return value:
//			The handle of the page object. Null if failed.
DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFFormObj_GetSubObject(FPDF_PAGEOBJECT form_object, int index);

// Function: FPDFFormObj_GetSubObjectIndex
//			Get the index of the page object in the specified form object.
// Parameters:	
//			form_object	-	Handle to a form object. Returned by FPDFPage_GetObject.
//			page_obj	-	Handle to a page object. Returned by FPDFPageObj_NewTextObj,FPDFPageObj_NewTextObjEx,
//							FPDFPageObj_NewPathObj, FPDFPageObj_NewImageObj, etc.
// Return value:
//			If successful, returns the index of the page object.
//			Otherwise, returns -1.
DLLEXPORT int STDCALL FPDFFormObj_GetSubObjectIndex(FPDF_PAGEOBJECT form_object, FPDF_PAGEOBJECT page_obj);

// Function: FPDFFormObj_GenerateContent
//			Generate content of the form object.
// Parameters:	
//			form_object	-	Handle to a form object. Returned by FPDFPage_GetObject.
// Return value:
//			True for successful, and false for failed.
// Comment:
//			When the form object's information changed, you must call the FPDFFormObj_GenerateContent function.
//			Or the changed information will be lost.
DLLEXPORT FPDF_BOOL STDCALL FPDFFormObj_GenerateContent(FPDF_PAGEOBJECT form_object);

// Function: FPDFFormObj_GetMatrix
//			Get the form object matrix.
// Parameters:
//			form_object	-	Handle to a form object. Returned by FPDFPage_GetObject.
//			a			-	Pointer to a double value receiving the coefficient "a" of the matrix.
//			b			-	Pointer to a double value receiving the coefficient "b" of the matrix.
//			c			-	Pointer to a double value receiving the coefficient "c" of the matrix.
//			d			-	Pointer to a double value receiving the coefficient "d" of the matrix.
//			e			-	Pointer to a double value receiving the coefficient "e" of the matrix.
//			f			-	Pointer to a double value receiving the coefficient "f" of the matrix.
// Return Value:
//			TRUE for successful, FALSE for failed.
DLLEXPORT FPDF_BOOL STDCALL FPDFFormObj_GetMatrix(FPDF_PAGEOBJECT form_object, 
						double* a, double* b, double* c, double* d,double* e, double* f);

//////////////////////////////////////////////////////////////////////
//
// Image functions
//
//////////////////////////////////////////////////////////////////////

// Function: FPDFPageObj_NewImgeObj
//			Create a new Image Object.
// Parameters:
//			document		-	Handle to document. Returned by FPDF_LoadDocument or FPDF_CreateNewDocument function.
// Return Value:
//			Handle of image object.
DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFPageObj_NewImgeObj(FPDF_DOCUMENT document);

// Function: FPDFImageObj_LoadFromFile
//			Load Image from a image file(such as *.bmp,*.jpeg) and then set it to an image object.
// Parameters:
//			pages			-	Pointers to the start of all loaded pages.
//			nCount			-	Number of pages.
//			image_object	-	Handle of image object returned by FPDFPageObj_NewImgeObj.
//			filename		-	The full path of the image file.
//	Return Value:
//			TRUE if successful, FALSE otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFImageObj_LoadFromFile(FPDF_PAGE* pages, int nCount,FPDF_PAGEOBJECT image_object, FPDF_BYTESTRING filename);

// Function: FPDFImageObj_GetMatrix
//			Get the matrix of an image object.
// Parameters:
//			image_object	-	Handle of image object returned by FPDFPageObj_NewImgeObj.
//			a				-	Pointer to a double value receiving the coefficient "a" of the matrix.
//			b				-	Pointer to a double value receiving the coefficient "b" of the matrix.
//			c				-	Pointer to a double value receiving the coefficient "c" of the matrix.
//			d				-	Pointer to a double value receiving the coefficient "d" of the matrix.
//			e				-	Pointer to a double value receiving the coefficient "e" of the matrix.
//			f				-	Pointer to a double value receiving the coefficient "f" of the matrix.
// Return value:
//			TRUE if successful, FALSE otherwise. 
// Comments:
//			A matrix defines transformation of coordinate from one space to another.
//			In PDF, a matrix is defined by the following equations:
//			x' = a * x + c * y + e;
//			y' = b * x + d * y + f;
//			FPDFImageObj_GetMatrix function is used to get a,b,c,d,e,f coefficients of the transformation from
//			"image space" to "user space". 
DLLEXPORT FPDF_BOOL STDCALL FPDFImageObj_GetMatrix(FPDF_PAGEOBJECT image_object, 
											 double* a, double* b, double* c, double* d,double* e, double* f);

// Function: FPDFImageObj_SetMatrix
//			Set the matrix of an image object.
// Parameters:
//			image_object	-	Handle of image object returned by FPDFPageObj_NewImgeObj.
//			a				-	The coefficient "a" of the matrix.
//			b				-	The coefficient "b" of the matrix.
//			c				-	The coefficient "c" of the matrix.
//			d				-	The coefficient "d" of the matrix.
//			e				-	The coefficient "e" of the matrix.
//			f				-	The coefficient "f" of the matrix.
// Return value:
//			TRUE if successful, FALSE otherwise. 
DLLEXPORT FPDF_BOOL STDCALL FPDFImageObj_SetMatrix(FPDF_PAGEOBJECT image_object,
											 double a, double b, double c, double d, double e, double f);

// Function: FPDFImageObj_GetBitmap
//			Get the bitmap from an image object.
// Parameters:
//			image_object	-	Handle of image object returned by FPDFPageObj_NewImgeObj.
// Return value:
//			The handle of the bitmap.
DLLEXPORT FPDF_BITMAP STDCALL FPDFImageObj_GetBitmap(FPDF_PAGEOBJECT image_object);

// Function: FPDFImageObj_SetBitmap
//			Set the bitmap to an image object.
// Parameters:
//			pages			-	Pointer's to the start of all loaded pages.
//			nCount			-	Number of pages.
//			image_object	-	Handle of image object returned by FPDFPageObj_NewImgeObj.
//			bitmap			-	The handle of the bitmap which you want to set it to the image object.
// Return value:
//			TRUE if successful, FALSE otherwise. 
DLLEXPORT FPDF_BOOL STDCALL FPDFImageObj_SetBitmap(FPDF_PAGE* pages,int nCount,FPDF_PAGEOBJECT image_object, FPDF_BITMAP bitmap);

#ifdef _cplusplus
}
#endif

#endif // _FPDFEDIT_H_

