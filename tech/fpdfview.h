 //*****************************************************************************
//* Foxit Software Inc.
//* Copyright(C) 2006-2009, all rights reserved.
//*
//* The following code is copyrighted and contains proprietary information
//* and trade secrets of Foxit Software Inc.
//*
//* ---DESCRIPTION ABOUT THIS FILE
//* Header file for FPDFVIEW module - a part of Foxit PDF SDK DLL.
//* Functions in this header file, require "FPDFVIEW" module to be enabled in your SDK license.
//* If you want to purchase Foxit PDF SDK license and use ANY of the following functions, please
//* explicitly request for enabling FPDFVIEW module.
//*
//* ---REVISION HISTORY
//* Jan. 30, 2006. Version 1.0 (Initial version).
//*	Apr. 20, 2009. Version 2.0
//* May. 05, 2009. version 2.0
//* Added the Grayscale output option for rendering
//*****************************************************************************

#ifndef _FPDFVIEW_H_
#define _FPDFVIEW_H_

#if defined(_WIN32) && !defined(__WINDOWS__)
#include <windows.h>
#endif

// Data types
typedef void*	FPDF_MODULEMGR;

// PDF types
typedef void*	FPDF_DOCUMENT;		
typedef void*	FPDF_PAGE;			
typedef void*	FPDF_PAGEOBJECT;	// Page object(text, path, etc)
typedef void*	FPDF_PATH;
typedef void*	FPDF_CLIPPATH;	
typedef void*	FPDF_BITMAP;	
typedef void*	FPDF_FONT;			

typedef void*	FPDF_TEXTPAGE;
typedef void*	FPDF_SCHHANDLE;
typedef void*	FPDF_PAGELINK;
typedef void*	FPDF_HMODULE;
typedef void*	FPDF_DOCSCHHANDLE;

typedef void*	FPDF_BOOKMARK;
typedef void*	FPDF_DEST;
typedef void*	FPDF_ACTION;
typedef void*	FPDF_LINK;

// Basic data types
typedef int				FPDF_BOOL;
typedef int				FPDF_ERROR;	
typedef unsigned long	FPDF_DWORD;

// String types
typedef unsigned short			FPDF_WCHAR;
typedef unsigned char const*	FPDF_LPCBYTE;

// FPDFSDK may use three types of strings: byte string, wide string (UTF-16LE encoded), and platform dependent string
typedef const char*				FPDF_BYTESTRING;

typedef const unsigned short*	FPDF_WIDESTRING;		// Foxit PDF SDK always use UTF-16LE encoding wide string,
														// each character use 2 bytes (except surrogation), with low byte first.

// For Windows programmers: for most case it's OK to treat FPDF_WIDESTRING as Windows unicode string,
//		 however, special care needs to be taken if you expect to process Unicode larger than 0xffff.
// For Linux/Unix programmers: most compiler/library environment uses 4 bytes for a Unicode character,
//		you have to convert between FPDF_WIDESTRING and system wide string by yourself.

#ifdef _WIN32_WCE
typedef const unsigned short* FPDF_STRING;
#else
typedef const char* FPDF_STRING;
#endif

#ifdef _WIN32
// On Windows system, functions are exported in a DLL
#define DLLEXPORT __declspec( dllexport )
#define STDCALL __stdcall
#else
#define DLLEXPORT
#define STDCALL
#endif

// Exported Functions
#ifdef __cplusplus
extern "C" {
#endif

// Function: FPDF_InitLibrary
//			Initialize the FPDFSDK library 
// Parameters:
//			hInstance	-	For WIN32 system only: the instance of the executable or DLL module.
// Return value:
//			None.
// Comments:
//			You have to call this function before you can call any PDF processing functions.

DLLEXPORT void STDCALL FPDF_InitLibrary(void* hInstance);

// Function: FPDF_GetModuleMgr
//			Get the module of this DLL.
// Parameters:
//			None.
// Return value:
//			FPDF_MODULEMGR	-	The handle of this module.  
DLLEXPORT FPDF_MODULEMGR STDCALL FPDF_GetModuleMgr();

// Function: FPDF_DestroyLibary
//			Release all resources allocated by the FPDFSDK library.
// Parameters:
//			None.
// Return value:
//			None.
// Comments:
//			You can call this function to release all memory blocks allocated by the library. 
//			After this function called, you should not call any PDF processing functions.
DLLEXPORT void STDCALL FPDF_DestroyLibrary();

// Function: FPDF_UnlockDLL
//			Unlock the DLL using license key info received from Foxit.
// Parameters: 
//			license_id	-	A string received from Foxit identifying the SDK license.
//			unlock_code	-	A string received from Foxit for unlocking the DLL.
// Return value:
//			None.
// Comments:
//			For SDK evaluators, this function call is not required, then all
//			rendered pages will come with an evaluation mark.
//			For purchased SDK customers, this should be the first function
//			to call before any other functions to be called.
//
DLLEXPORT void STDCALL FPDF_UnlockDLL(FPDF_BYTESTRING license_id, FPDF_BYTESTRING unlock_code);

// Function: FPDF_LoadDocument
//			Open and load a PDF document.
// Parameters: 
//			file_path	-	Path to the PDF file (including extension).
//			password	-	A string used as the password for PDF file. 
//							If no password needed, empty or NULL can be used.
// Return value:
//			A handle to the loaded document. If failed, NULL is returned.
// Comments:
//			Loaded document can be closed by FPDF_CloseDocument.
//			If this function fails, you can use FPDF_GetLastError() to retrieve
//			the reason why it fails.
//
DLLEXPORT FPDF_DOCUMENT	STDCALL FPDF_LoadDocument(FPDF_STRING file_path, 
												  FPDF_BYTESTRING password);

// Function: FPDF_LoadMemDocument
//			Open and load a PDF document from memory.
// Parameters: 
//			data_buf	-	Pointer to a buffer containing the PDF document.
//			size		-	Number of bytes in the PDF document.
//			password	-	A string used as the password for PDF file. 
//							If no password needed, empty or NULL can be used.
// Return value:
//			A handle to the loaded document. If failed, NULL is returned.
// Comments:
//			The memory buffer must remain valid when the document is open.
//			Loaded document can be closed by FPDF_CloseDocument.
//			If this function fails, you can use FPDF_GetLastError() to retrieve
//			the reason why it fails.
//
DLLEXPORT FPDF_DOCUMENT	STDCALL FPDF_LoadMemDocument(const void* data_buf, 
											int size, FPDF_BYTESTRING password);

// Structure for custom file access.
typedef struct {
	// File length, in bytes.
	unsigned long	m_FileLen;

	// A function pointer for getting a block of data from specific position.
	// Position is specified by byte offset from beginning of the file.
	// The position and size will never go out range of file length.
	// It may be possible for FPDFSDK to call this function multiple times for same position.
	// Return value: should be non-zero if successful, zero for error.
	int				(*m_GetBlock)(void* param, unsigned long position, unsigned char* pBuf, unsigned long size);

	// A custom pointer for all implementation specific data.
	// This pointer will be used as the first parameter to m_GetBlock callback.
	void*			m_Param;
} FPDF_FILEACCESS;

// Function: FPDF_LoadCustomDocument
//			Load PDF document from a custom access descriptor.
// Parameters:
//			pFileAccess	-	A structure for access the file.
//			password	-	Optional password for decrypting the PDF file.
// Return value:
//			A handle to the loaded document. If failed, NULL is returned.
// Comments:
//			The application should maintain the file resources being valid until the PDF document close.
//			Loaded document can be closed by FPDF_CloseDocument.
DLLEXPORT FPDF_DOCUMENT STDCALL FPDF_LoadCustomDocument(FPDF_FILEACCESS* pFileAccess, 
														FPDF_BYTESTRING password);

#define FPDF_ERR_SUCCESS		0		// No error.
#define FPDF_ERR_UNKNOWN		1		// Unknown error.
#define FPDF_ERR_FILE			2		// File not found or could not be opened.
#define FPDF_ERR_FORMAT			3		// File not in PDF format or corrupted.
#define FPDF_ERR_PASSWORD		4		// Password required or incorrect password.
#define FPDF_ERR_SECURITY		5		// Unsupported security scheme.
#define FPDF_ERR_PAGE			6		// Page not found or content error.

// Function: FPDF_GetLastError
//			Get last error code when an SDK function failed.
// Parameters: 
//			None.
// Return value:
//			A 32-bit integer indicating error codes (defined above).
// Comments:
//			If the previous SDK call succeeded, the return value of this function
//			is not defined.
//
DLLEXPORT unsigned long	STDCALL FPDF_GetLastError();

// Function: FPDF_GetDocPermission
//			Get file permission flags of the document.
// Parameters: 
//			document	-	Handle to document. Returned by FPDF_LoadDocument function.
// Return value:
//			A 32-bit integer indicating permission flags. Please refer to PDF Reference for
//			detailed description. If the document is not protected, 0xffffffff will be returned.
//
DLLEXPORT unsigned long	STDCALL FPDF_GetDocPermissions(FPDF_DOCUMENT document);

// Function: FPDF_GetPageCount
//			Get total number of pages in a document.
// Parameters: 
//			document	-	Handle to document. Returned by FPDF_LoadDocument function.
// Return value:
//			Total number of pages in the document.
//
DLLEXPORT int STDCALL FPDF_GetPageCount(FPDF_DOCUMENT document);

// Function: FPDF_LoadPage
//			Load a page inside a document.
// Parameters: 
//			document	-	Handle to document. Returned by FPDF_LoadDocument function.
//			page_index	-	Index number of the page. 0 for the first page.
// Return value:
//			A handle to the loaded page. If failed, NULL is returned.
// Comments:
//			Loaded page can be rendered to devices using FPDF_RenderPage function.
//			Loaded page can be closed by FPDF_ClosePage.
//
DLLEXPORT FPDF_PAGE	STDCALL FPDF_LoadPage(FPDF_DOCUMENT document, int page_index);

// Function: FPDF_GetPageWidth
//			Get page width.
// Parameters:
//			page		-	Handle to the page. Returned by FPDF_LoadPage function.
// Return value:
//			Page width (excluding non-displayable area) measured in points.
//			One point is 1/72 inch (around 0.3528 mm).
//
DLLEXPORT double STDCALL FPDF_GetPageWidth(FPDF_PAGE page);

// Function: FPDF_GetPageHeight
//			Get page height.
// Parameters:
//			page		-	Handle to the page. Returned by FPDF_LoadPage function.
// Return value:
//			Page height (excluding non-displayable area) measured in points.
//			One point is 1/72 inch (around 0.3528 mm)
//
DLLEXPORT double STDCALL FPDF_GetPageHeight(FPDF_PAGE page);

// Function: FPDF_GetPageSizeByIndex
//			Get the size of a page by index.
// Parameters:
//			document	-	Handle to document. Returned by FPDF_LoadDocument function.
//			page_index	-	Page index, zero for the first page.
//			width		-	Pointer to a double value receiving the page width (in points).
//			height		-	Pointer to a double value receiving the page height (in points).
// Return value:
//			Non-zero for success. 0 for error (document or page not found).
//
DLLEXPORT int STDCALL FPDF_GetPageSizeByIndex(FPDF_DOCUMENT document, int page_index, double* width, double* height);

// Declares of a pointer type to the callback function for the FPDF_EnumPageSize method.
// parameters:
//			page_index	-	Page index, zero for the first page.
//			width		-	The page width.
//			height		-	The page height.
// Return value:
//			None. 			
typedef void (*FPDF_ENUMPAGESIZEPROC)(int page_index, double width, double height);

// Function: FPDF_EnumPageSize
//			Enumerating all pages within the document.
// Parameters:
//			document	-	Handle to document. Returned by FPDF_LoadDocument function.
//			callback	-	A pointer to a callback function.
// Return value:
//			None.
DLLEXPORT void STDCALL FPDF_EnumPageSize(FPDF_DOCUMENT document, FPDF_ENUMPAGESIZEPROC callback);

// Page rendering flags. They can be combined with bit OR.
#define FPDF_ANNOT			0x01		// Set if annotations are to be rendered.
#define FPDF_LCD_TEXT		0x02		// Set if using text rendering optimized for LCD display.
#define FPDF_NO_GDIPLUS		0x04		// Set if you don't want to use GDI+ (for fast rendering with poorer graphic quality).
										// Applicable to desktop Windows systems only.
#define FPDF_GRAYSCALE		0x08		// Grayscale output.
#define FPDF_DEBUG_INFO		0x80		// Set if you want to get some debug info. 
										// Please discuss with Foxit first if you need to collect debug info.
#define FPDF_NO_CATCH		0x100		// Set if you don't want to catch exception.

#ifdef _WIN32
// Function: FPDF_RenderPage
//			Render contents in a page to a device (screen, bitmap, or printer).
//			This function is only supported on Windows system.
// Parameters: 
//			dc			-	Handle to device context.
//			page		-	Handle to the page. Returned by FPDF_LoadPage function.
//			start_x		-	Left pixel position of the display area in the device coordinate.
//			start_y		-	Top pixel position of the display area in the device coordinate.
//			size_x		-	Horizontal size (in pixels) for displaying the page.
//			size_y		-	Vertical size (in pixels) for displaying the page.
//			rotate		-	Page orientation: 0 (normal), 1 (rotated 90 degrees clockwise),
//								2 (rotated 180 degrees), 3 (rotated 90 degrees counter-clockwise).
//			flags		-	0 for normal display, or combination of flags defined above.
// Return value:
//			None.
//
DLLEXPORT void STDCALL FPDF_RenderPage(HDC dc, FPDF_PAGE page, int start_x, int start_y, int size_x, int size_y,
						int rotate, int flags);
#endif

// Function: FPDF_RenderPageBitmap
//			Render contents in a page to a device independent bitmap
// Parameters: 
//			bitmap		-	Handle to the device independent bitmap (as the output buffer).
//							Bitmap handle can be created by FPDFBitmap_Create function.
//			page		-	Handle to the page. Returned by FPDF_LoadPage function.
//			start_x		-	Left pixel position of the display area in the bitmap coordinate.
//			start_y		-	Top pixel position of the display area in the bitmap coordinate.
//			size_x		-	Horizontal size (in pixels) for displaying the page.
//			size_y		-	Vertical size (in pixels) for displaying the page.
//			rotate		-	Page orientation: 0 (normal), 1 (rotated 90 degrees clockwise),
//								2 (rotated 180 degrees), 3 (rotated 90 degrees counter-clockwise).
//			flags		-	0 for normal display, or combination of flags defined above.
// Return value:
//			None.
//
DLLEXPORT void STDCALL FPDF_RenderPageBitmap(FPDF_BITMAP bitmap, FPDF_PAGE page, int start_x, int start_y, 
						int size_x, int size_y, int rotate, int flags);

// Function: FPDF_QuickDrawPage
//			Draw a thumbnail of a page into a bitmap.
// Parameters: 
//			bitmap		-	Handle to the device independent bitmap (as the output buffer).
//							Bitmap handle can be created by FPDFBitmap_Create function.
//			page		-	Handle to the page. Returned by FPDF_LoadPage function.
//			start_x		-	Left pixel position of the display area in the device coordinate.
//			start_y		-	Top pixel position of the display area in the device coordinate.
//			size_x		-	Horizontal size (in pixels) for displaying the page.
//			size_y		-	Vertical size (in pixels) for displaying the page.
//			rotate		-	Page orientation: 0 (normal), 1 (rotated 90 degrees clockwise),
//								2 (rotated 180 degrees), 3 (rotated 90 degrees counter-clockwise).
//			flags		-	Currently must be zero.
// Return value:
//			None.
// Comments:
//			This functions draws a very low-resolution thumbnail of a page, sometimes with
//			inaccurate shape or position. The result thumbnail is meant for a very rough preview
//			of the page contents, just giving user some idea about how the page looks like.
//			The thumbnail is often useful in multi-threaded or progressive environment, the
//			application can first display the low-resolution thumbnail, start to respond to
//			user input, and a higher resolution thumbnail can be generated by FPDF_RenderPageBitmap 
//			function.
//
DLLEXPORT void STDCALL FPDF_QuickDrawPage(FPDF_BITMAP bitmap, FPDF_PAGE page, int start_x, int start_y, 
										  int size_x, int size_y, int rotate, int flags);

// Function: FPDF_ClosePage
//			Close a loaded PDF page.
// Parameters: 
//			page		-	Handle to the loaded page.
// Return value:
//			None.
//
DLLEXPORT void STDCALL FPDF_ClosePage(FPDF_PAGE page);

// Function: FPDF_CloseDocument
//			Close a loaded PDF document.
// Parameters: 
//			document	-	Handle to the loaded document.
// Return value:
//			None.
//
DLLEXPORT void STDCALL FPDF_CloseDocument(FPDF_DOCUMENT document);

// Function: FPDF_DeviceToPage
//			Convert the screen coordinate of a point to page coordinate.
// Parameters:
//			page		-	Handle to the page. Returned by FPDF_LoadPage function.
//			start_x		-	Left pixel position of the display area in the device coordinate.
//			start_y		-	Top pixel position of the display area in the device coordinate.
//			size_x		-	Horizontal size (in pixels) for displaying the page.
//			size_y		-	Vertical size (in pixels) for displaying the page.
//			rotate		-	Page orientation: 0 (normal), 1 (rotated 90 degrees clockwise),
//								2 (rotated 180 degrees), 3 (rotated 90 degrees counter-clockwise).
//			device_x	-	X value in device coordinate, for the point to be converted.
//			device_y	-	Y value in device coordinate, for the point to be converted.
//			page_x		-	A Pointer to a double receiving the converted X value in page coordinate.
//			page_y		-	A Pointer to a double receiving the converted Y value in page coordinate.
// Return value:
//			None.
// Comments:
//			The page coordinate system has its origin at left-bottom corner of the page, with X axis goes along
//			the bottom side to the right, and Y axis goes along the left side upward. NOTE: this coordinate system 
//			can be altered when you zoom, scroll, or rotate a page, however, a point on the page should always have 
//			the same coordinate values in the page coordinate system. 
//
//			The device coordinate system is device dependent. For screen device, its origin is at left-top
//			corner of the window. However this origin can be altered by Windows coordinate transformation
//			utilities. You must make sure the start_x, start_y, size_x, size_y and rotate parameters have exactly
//			same values as you used in FPDF_RenderPage() function call.
//
DLLEXPORT void STDCALL FPDF_DeviceToPage(FPDF_PAGE page, int start_x, int start_y, int size_x, int size_y,
						int rotate, int device_x, int device_y, double* page_x, double* page_y);

// Function: FPDF_PageToDevice
//			Convert the page coordinate of a point to screen coordinate.
// Parameters:
//			page		-	Handle to the page. Returned by FPDF_LoadPage function.
//			start_x		-	Left pixel position of the display area in the device coordinate.
//			start_y		-	Top pixel position of the display area in the device coordinate.
//			size_x		-	Horizontal size (in pixels) for displaying the page.
//			size_y		-	Vertical size (in pixels) for displaying the page.
//			rotate		-	Page orientation: 0 (normal), 1 (rotated 90 degrees clockwise),
//								2 (rotated 180 degrees), 3 (rotated 90 degrees counter-clockwise).
//			page_x		-	X value in page coordinate, for the point to be converted.
//			page_y		-	Y value in page coordinate, for the point to be converted.
//			device_x	-	A pointer to an integer receiving the result X value in device coordinate.
//			device_y	-	A pointer to an integer receiving the result Y value in device coordinate.
// Return value:
//			None.
// Comments:
//			See comments of FPDF_DeviceToPage() function.
//
DLLEXPORT void STDCALL FPDF_PageToDevice(FPDF_PAGE page, int start_x, int start_y, int size_x, int size_y,
						int rotate, double page_x, double page_y, int* device_x, int* device_y);

// Function: FPDFBitmap_Create
//			Create a Foxit Device Independent Bitmap (FXDIB).
// Parameters:
//			width		-	Number of pixels in a horizontal line of the bitmap. Must be greater than 0.
//			height		-	Number of pixels in a vertical line of the bitmap. Must be greater than 0.
//			alpha		-	A flag indicating whether alpha channel is used. Non-zero for using alpha, zero for not using.
// Return value:
//			The created bitmap handle, or NULL if parameter error or out of memory.
// Comments:
//			An FXDIB always use 4 byte per pixel. The first byte of a pixel is always double word aligned.
//			Each pixel contains red (R), green (G), blue (B) and optionally alpha (A) values.
//			The byte order is BGRx (the last byte unused if no alpha channel) or BGRA.
//			
//			The pixels in a horizontal line (also called scan line) are stored side by side, with left most
//			pixel stored first (with lower memory address). Each scan line uses width*4 bytes.
//
//			Scan lines are stored one after another, with top most scan line stored first. There is no gap
//			between adjacent scan lines.
//
//			This function allocates enough memory for holding all pixels in the bitmap, but it doesn't 
//			initialize the buffer. Applications can use FPDFBitmap_FillRect to fill the bitmap using any color.
DLLEXPORT FPDF_BITMAP STDCALL FPDFBitmap_Create(int width, int height, int alpha);

// More DIB formats
#define FPDFBitmap_Gray		1		// Gray scale bitmap, one byte per pixel.
#define FPDFBitmap_BGR		2		// 3 bytes per pixel, byte order: blue, green, red.
#define FPDFBitmap_BGRx		3		// 4 bytes per pixel, byte order: blue, green, red, unused.
#define FPDFBitmap_BGRA		4		// 4 bytes per pixel, byte order: blue, green, red, alpha.

// Function: FPDFBitmap_CreateEx
//			Create a Foxit Device Independent Bitmap (FXDIB)
// Parameters:
//			width		-	Number of pixels in a horizontal line of the bitmap. Must be greater than 0.
//			height		-	Number of pixels in a vertical line of the bitmap. Must be greater than 0.
//			format		-	A number indicating for bitmap format, as defined above.
//			first_scan	-	A pointer to the first byte of first scan line, for external buffer
//							only. If this parameter is NULL, then the SDK will create its own buffer.
//			stride		-	Number of bytes for each scan line, for external buffer only..
// Return value:
//			The created bitmap handle, or NULL if parameter error or out of memory.
// Comments:
//			Similar to FPDFBitmap_Create function, with more formats and external buffer supported. 
//			Bitmap created by this function can be used in any place that a FPDF_BITMAP handle is 
//			required. 
//
//			If external scanline buffer is used, then the application should destroy the buffer
//			by itself. FPDFBitmap_Destroy function will not destroy the buffer.
//
DLLEXPORT FPDF_BITMAP STDCALL FPDFBitmap_CreateEx(int width, int height, int format, void* first_scan, int stride);

// Function: FPDFBitmap_FillRect
//			Fill a rectangle area in an FXDIB.
// Parameters:
//			bitmap		-	The handle to the bitmap. Returned by FPDFBitmap_Create function.
//			left		-	The left side position. Starting from 0 at the left-most pixel.
//			top			-	The top side position. Starting from 0 at the top-most scan line.
//			width		-	Number of pixels to be filled in each scan line.
//			height		-	Number of scan lines to be filled.
//			red			-	A number from 0 to 255, identifying the red intensity.
//			green		-	A number from 0 to 255, identifying the green intensity.
//			blue		-	A number from 0 to 255, identifying the blue intensity.
//			alpha		-	(Only if the alpha channeled is used when bitmap created) A number from 0 to 255,
//							identifying the alpha value.
// Return value:
//			None.
// Comments:
//			This function set the color and (optionally) alpha value in specified region of the bitmap.
//			NOTE: If alpha channel is used, this function does NOT composite the background with the source color,
//			instead the background will be replaced by the source color and alpha.
//			If alpha channel is not used, the "alpha" parameter is ignored.
//
DLLEXPORT void STDCALL FPDFBitmap_FillRect(FPDF_BITMAP bitmap, int left, int top, int width, int height, 
									int red, int green, int blue, int alpha);

// Function: FPDFBitmap_GetBuffer
//			Get data buffer of an FXDIB
// Parameters:
//			bitmap		-	Handle to the bitmap. Returned by FPDFBitmap_Create function.
// Return value:
//			The pointer to the first byte of the bitmap buffer.
// Comments:
//			Applications can use this function to get the bitmap buffer pointer, then manipulate any color
//			and/or alpha values for any pixels in the bitmap.
DLLEXPORT void* STDCALL FPDFBitmap_GetBuffer(FPDF_BITMAP bitmap);

// Function: FPDFBitmap_GetWidth
//			Get width of an FXDIB.
// Parameters:
//			bitmap		-	Handle to the bitmap. Returned by FPDFBitmap_Create function.
// Return value:
//			The number of pixels in a horizontal line of the bitmap.
DLLEXPORT int STDCALL FPDFBitmap_GetWidth(FPDF_BITMAP bitmap);

// Function: FPDFBitmap_GetHeight
//			Get height of an FXDIB.
// Parameters:
//			bitmap		-	Handle to the bitmap. Returned by FPDFBitmap_Create function.
// Return value:
//			The number of pixels in a vertical line of the bitmap.
DLLEXPORT int STDCALL FPDFBitmap_GetHeight(FPDF_BITMAP bitmap);

// Function: FPDFBitmap_GetStride
//			Get number of bytes for each scan line in the bitmap buffer.
// Parameters:
//			bitmap		-	Handle to the bitmap. Returned by FPDFBitmap_Create function.
// Return value:
//			The number of bytes for each scan line in the bitmap buffer.
DLLEXPORT int STDCALL FPDFBitmap_GetStride(FPDF_BITMAP bitmap);

// Function: FPDFBitmap_Destroy
//			Destroy an FXDIB and release all related buffers. 
// Parameters:
//			bitmap		-	Handle to the bitmap. Returned by FPDFBitmap_Create function.
// Return value:
//			None.
// Comments:
//			This function will not destroy any external buffer.
//
DLLEXPORT void STDCALL FPDFBitmap_Destroy(FPDF_BITMAP bitmap);

// Function: FPDF_AllocMemory
//			Allocate memory block in FPDFSDK. This memory can be freed by FPDF_FreeMemory function.
// Parameters:
//			size		-	Byte size of requested memory block. Can not be zero.
// Return value:
//			The allocated pointer. NULL if memory not available.
// Comments:
//			Some FPDFSDK interface may require application to allocate memory for internal use of
//			FPDFSDK. In this case application must call this function to allocate memory, don't
//			use malloc() or other memory allocator.
//			If an error handler installed and exception/long jump is used in the out of memory handling,
//			this function might never return if no memory available.
//
DLLEXPORT void* STDCALL FPDF_AllocMemory(unsigned long size);

// Function: FPDF_FreeMemory
//			Free a memory area allocated by Foxit SDK.
// Parameters:
//			p		-	The pointer. Should not be NULL.
// Return value:
//			None.
// Comments:
//			In case FPDFSDK allocated some memory for user application, the user application
//			must free it to avoid memory leakage. And the application must call FPDF_FreeMemory
//			function to do that. Do NOT use c/c++ memory free() function or other similar functions.
DLLEXPORT void STDCALL FPDF_FreeMemory(void* p);

#define FPDFERR_OUT_OF_MEMORY		1		// Out of memory. The error handler should quit the application,
											// or use long jump to get out of current rendering.
#define FPDFERR_MISSING_FEATURE		2		// Missing PDF feature. The error handler can safely continue
											// with other rendering.

typedef void (*FPDF_ErrorHandler)(int code, FPDF_BYTESTRING msg);

// Function: FPDF_SetErrorHandler
//			Set a call back function when FPDFSDK has some error to report.
// Parameters:
//			func	-	Pointer to the error handler function.
// Return value:
//			None.
// Comments:
//			Currently only two error codes are defined (see above).
//
DLLEXPORT void STDCALL FPDF_SetErrorHandler(FPDF_ErrorHandler func);

// Function: FPDF_SetModulePath
//			Set the folder path for module files (like the FPDFCJK.BIN).
// Parameters;
//			module_name	-	Name of the module. Currently please use NULL (0) only.
//			folder_name	-	Name of the folder. For example: "C:\\program files\\FPDFSDK".
// Return value:
//			None.
DLLEXPORT void STDCALL FPDF_SetModulePath(FPDF_STRING module_name, FPDF_STRING folder_name);

// Structure: FPDF_GLYPHPROVIDER
//			Define an interface for generating glyph bitmaps.
//			This interface does not exist on desktop Windows system. But on alternative systems,
//			including mobile system, this interface must be implemented in order to display
//			non-embedded non-western fonts, like Chinese/Japanese/Korean characters.
//			To make use of a glyph provider, call FPDF_SetGlyphProvider function.
typedef struct {
	// Interface: MapFont
	//		Map a font with particular name. The implementation can return any pointer to some
	//		internal font structure. Or it can return NULL if font mapping not supported (like if 
	//		there is only one font available in the system).
	//		The result of this interface function will be passed back to other interface functions
	//		as "font handle".
	// Parameters:
	//		name		-	The single byte encoded name of the font. It might be a MBCS encoded name.
	//		codepage	-	The Windows code page identifier for the font, indicating the primary character set
	//						of the font. Currently it can be one of the followings:
	//						0	-	unknown character set
	//						932	-	Japanese character set
	//						936	-	Simplified Chinese character set
	//						949	-	Korean character set
	//						950	-	Traditional character set
	//						1200-	Unicode character set
	void*	(*MapFont)(FPDF_BYTESTRING name, int codepage);

	// Interface: GetGlyphBBox
	//		Get bounding box of a glyph. The boundaries are measured in PDF font units, which is
	//		1/1000 of the em size. For example, if a character's top boundary is at half of the em square,
	//		then the top value should be 500.
	//		It's OK to return a box larger than the actual bounding box of the character. So some 
	//		implementation may just return a fixed bounding box for all glyphs. But returning a 
	//		box smaller than the actually bounding box will cause some problem like during scrolling.
	// Parameters:
	//		font		-	The font handle returned by MapFont interface.
	//		unicode		-	The unicode of the character
	//		cid			-	The CID code (see Adobe CID specifications) of the character. 0 if not available.
	//						For most characters, implementation can ignore this parameter. However, if precise
	//						display of CJK characters is required, some special CID (like half width/full width,
	//						rotated, etc) needs special handling.
	//		left		-	[OUT] Pointer to returned left boundary
	//		top			-	[OUT] Pointer to returned top boundary
	//		right		-	[OUT] Pointer to returned right boundary
	//		bottom		-	[OUT] Pointer to returned bottom boundary
	void	(*GetGlyphBBox)(void* font, int unicode, int cid, int* left, int* top, int* right, int* bottom);

	// Interface: GetGlyphBitmap
	//		This interface is the main interface for getting glyph bitmaps. The input include font handle,
	//		unicode and CID code, font size, then the implementation should allocate a sufficient buffer
	//		to hold the grayscale bitmap for the character, and return its position (relative to origin
	//		of the character), size, buffer, and stride to FPDFSDK.
	//		FPDFSDK then will use the returned bitmap and position information to display the glyph.
	// Parameters:
	//		font		-	The font handle returned by MapFont interface.
	//		unicode		-	The unicode of the character
	//		cid			-	The CID code of the character. See comments in GetCharBBox interface.
	//		size		-	The font size (size for em square).
	//		left		-	[OUT] Pointer to left offset of the bitmap, from the character origin.
	//						Negative means the bitmap should move to left of the origin,
	//						positive means the bitmap should move to right side of the origin.
	//		top			-	[OUT] Pointer to top offset of the bitmap, from the character origin.
	//						Negative means the bitmap should move downward from the origin,
	//						positive means the bitmap should move to upward from the origin.
	//		width		-	[OUT] Pointer to output width (number of pixels per scanline)
	//		height		-	[OUT] Pointer to output height (number of scanlines)
	//		buffer		-	[OUT] Pointer to another pointer which points to a buffer containing the
	//						glyph bitmap. This buffer must be allocated by FPDF_AllocMemory function. 
	//						This buffer must contain scanlines of the bitmap from top to bottom.
	//						Each byte in the buffer represent a pixel of the glyph, 0 means not inside
	//						the glyph, 255 means fully inside the glyph. Implementation can also use
	//						gray scale (between 0-255) to identify pixels on the border of the glyph.
	//						Implementation doesn't need to free the buffer, FPDFSDK will do that.
	//		stride		-	[OUT] Pointer to output stride (number of bytes per scanline in the buffer).
	// Return value:
	//		Implementation should return non-zero if success, 0 if error happens (like character not found).
	int		(*GetGlyphBitmap)(void* font, int unicode, int cid, double size, int* left, int* top, 
							int* width, int* height, void** buffer, int* stride);
} FPDF_GLYPHPROVIDER;

// Function: FPDF_SetGlyphProvider
//			Make use of a custom glyph bitmap provider.
//			Not available on Desktop Windows system.
// Parameters:
//			pProvider	-	Pointer to a provider structure. This structure must be available all the time
//							(better put it in static data). And all member interfaces of this structure
//							should be properly set and implemented.
// Return value:
//			None.
DLLEXPORT void STDCALL FPDF_SetGlyphProvider(FPDF_GLYPHPROVIDER* pProvider);

// Function: FPDF_SetSystemFontFile
//			Make use of a system font. The font file must be in TrueType or Type1 format and must be
//			encoded in a standard encoding system.
//			Available for embedded Linux system only.
// Parameters:
//			file_path	-	The full path of the font file.
// Return value:
//			Non-zero for success. Zero for error.
//
DLLEXPORT int STDCALL FPDF_SetSystemFontFile(FPDF_BYTESTRING file_path);

#ifdef _WIN32_WCE

// Structure: FPDF_WCEFONTMAPPER
//			For Windows Mobile Only:
//			An interface for mapping PDF fonts to system fonts on Windows mobile.
//			Used for Chinese/Japanese/Korean fonts only.
//			By default, FPDFSDK picks the system default font for particular character sets. If your 
//			system has more than one fonts installed for a particular character set, you can build a 
//			font mapper structure and call FPDF_SetWCEFontMapper function to set to FPDFSDK engine. 
//			This will allow FPDFSDK to pick different fonts for different styles.
//			Any of the following interfaces can be NULL, then default implementation will be used.
//
typedef struct {
	// Interface: MapFontId
	//		Get a font identifier number for a particular font, from its name and character set.
	//		If the system supports more than one font for a particular character set, the implementation
	//		should assign different identifier number for each of the fonts supported. For example,
	//		assign 0 to the default font, 1 for an alternative font, etc.
	// Parameters:
	//		name		-	Zero-terminated byte string name for the font. This is the name used in
	//						PDF document. The implementation can analyze the name and try to figure out
	//						font style. For example, if the name contains "Gothic", it indicates a bold 
	//						style in Japanese font.
	//		charset		-	Windows charset identifier, like GB2312_CHARSET, SHIFTJIS_CHARSET. See
	//						MSDN document for CreateFont function.
	// Return value:
	//		An identifier number between 0 and 255 (within one byte range).
	//		FPDFSDK will use this return value in GetFontById interface.
	//
	int (*MapFontId)(FPDF_BYTESTRING name, int charset);

	// Interface: GetFontById
	//		Get face name of the font by its identifier (returned by MapFontId interface).
	// Parameters:
	//		buffer		-	An output buffer used to hold unicode name of the font face. It must be
	//						terminated by NULL character (unicode 0).
	//		size		-	Size of the buffer (number of characters, including the terminator)
	//		charset		-	Windows charset identifier, like GB2312_CHARSET, SHIFTJIS_CHARSET. See
	//						MSDN document for CreateFont function.
	//		font_id		-	The font id returned by MapFontId interface.
	// Return value:
	//		None.
	//
	void (*GetFontById)(unsigned short* buffer, int size, int charset, int font_id);

	// Interface: SubstUnicode
	//		Substitute a unicode to be displayed with another one within the font,
	//		if the unicode to be displayed doesn't exist in the font.
	// Parameters:
	//		font_id		-	The font id returned by MapFontId interface.
	//		unicode		-	Unicode to be displayed.
	// Return Value:
	//		If the unicode to be displayed doesn't exist in the font, implementation should
	//		return a unicode that exists in the font, for same or similar character.
	//		Otherwise, the original unicode should be returned.
	//
	unsigned short (*SubstUnicode)(int font_id, unsigned short unicode);
} FPDF_WCEFONTMAPPER;

// Function: FPDF_SetWCEFontMapper
//			For Windows Mobile Only: make use of a font mapper for CJK charsets.
//			This function should be called before page rendering.
// Parameters:
//			mapper		-	Pointer to the mapper structure.
// Return value:
//			None.
//
DLLEXPORT void STDCALL FPDF_SetWCEFontMapper(FPDF_WCEFONTMAPPER* mapper);

#endif	// _WIN32_WCE

#ifdef __cplusplus
};
#endif

#endif // _FPDFVIEW_H_
