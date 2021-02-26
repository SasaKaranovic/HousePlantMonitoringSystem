#include "main.h"
#include "PlantSystem.h"



bool PlanSystem_ParseBuffer(uint8_t *pBuffer, uint8_t nBufferLen, uint8_t *cmd, uint8_t *dataType, uint8_t *dataLength)
{
	// Check for null pointers
	if( (pBuffer==NULL) || (cmd==NULL) || (dataType==NULL) || (dataLength==NULL))
	{
		return false;
	}

	// Check buffer length
	if(nBufferLen < PLANTSYSTEM_MIN_HEADER_LENGHT)
	{
		return false;
	}


	*cmd = pBuffer[0];
	*dataType = pBuffer[1];
	*dataLength = (pBuffer[2] << 8) | pBuffer[3];

	// Check command
	if((*cmd<=CMD_FIRST) || (*cmd>=CMD_LAST))
	{
		return false;
	}

	// Check datatype
	if((*dataType<=DATATYPE_FIRST) || (*dataType>=DATATYPE_LAST))
	{
		return false;
	}

	
	return true;

}