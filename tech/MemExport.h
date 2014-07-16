#ifndef _MEM_EXPORT_
#define _MEM_EXPORT_

// -------------------------------------------------
// -------------------------------------------------


typedef enum Rot_Errors
{
	DprOK=0,				//DPR Rotary buffer status has no problem
	DprBufBsy=1,			//DPR Rotary buffer is busy
	DprEOF=2,				//DPR Rotary buffer is at  end of the  file
	DprFltErr=-1,			//unable to pack,for floating point number is too large
	DprStrToDErr=-2,		//unable to convert string to double float number
	DprMtrErr=-3,			//unable to convert motor numbers specified
	DprPlcErr=-4,			//unable to convert PLC numbers specified
	DprAxisErr=-5,			//unabled to convert axis in ABS,INC or FRAX
	DprCmdErr=-6,			//illegal command in string
	DprCmdMaxErr=-7,		//exceeded the max number of PMAC commands in a line(see DPRCMDMAX)
	DprIntErr=-8,			//integer number out of range(see integer MAX values CIRMAX etc)
	DprBufErr=-9,			//DPR rotary or internal rotary buffer size is zero
	DprOutFileErr=-10,		//DPR output file error
	DprInpFileErr=-11,		//DPR input file error
	DprNothingFill=-12,		//has nothing to fill into KSMC: the both PVT & block are NULL and the last SI_MV_STRUCT has alread used 
	DprParaInvlid=-13,
    DprSync = -14,
    DprResendBlk = -15,      //some status must sync with SI, and blk must send again.
    DprM22 = -16
}PBUF_STATUS;

#ifdef WIN32
/////////// for VC dll
#define EXPORT_API      __declspec(dllexport)
#else
#define EXPORT_API
#endif



extern "C"  {

EXPORT_API bool	pciCardOpen();
EXPORT_API bool	pciCardClose();
//bool   pciCardRestore();
EXPORT_API bool	getMacroVar(int no,double *data);
EXPORT_API bool	getCmdResponse(char * cmd, char * res);
EXPORT_API int	updateFirmWare(char *pFile);
EXPORT_API bool	getGatherResult(int nItem, int no, double *data);
EXPORT_API int	downLoadPLC(char *pFilePath, char *err=0);
EXPORT_API unsigned int	upLoadPLC(int no, char cType);
EXPORT_API int	downLoadCompensation();
EXPORT_API int	upLoadCompensation();

EXPORT_API int	defineRotBuf( int size, int *crdNum);	//16 coord
EXPORT_API int	deleteRotBuf();
EXPORT_API PBUF_STATUS feedRotBuf(int Crd, char *pString);
EXPORT_API int	clearRotBuf(int Crd);
EXPORT_API int	listRotBuf();
EXPORT_API bool GetCmdID();
EXPORT_API bool FreeCmdID();

}


#endif
