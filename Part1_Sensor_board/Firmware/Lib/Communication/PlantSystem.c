#include "main.h"
#include "PlantSystem.h"

#define MIN_LOG_LEVEL_ERROR
#define LOGGER_TAG "PS"
#include "logger.h"


bool PlanSystem_ParseBuffer(uint8_t *pBuffer, uint8_t nBufferLen, uint8_t *cmd, uint8_t *dataType, uint8_t *dataLength)
{
	// Check for null pointers
	if( (pBuffer==NULL) || (cmd==NULL) || (dataType==NULL) || (dataLength==NULL))
	{
		Logger_ERROR("Invalid call argument!");
		return false;
	}

	// Check buffer length
	if(nBufferLen < PLANTSYSTEM_MIN_HEADER_LENGHT)
	{
		Logger_DEBUG("Buffer length too short");
		return false;
	}


	*cmd = pBuffer[0];
	*dataType = pBuffer[1];
	*dataLength = (pBuffer[2] << 8) | pBuffer[3];

	// Check command
	if((*cmd<=CMD_FIRST) || (*cmd>=CMD_LAST))
	{
		Logger_DEBUG("Unsupported command");
		return false;
	}

	// Check datatype
	if((*dataType<=DATATYPE_FIRST) || (*dataType>=DATATYPE_LAST))
	{
		Logger_DEBUG("Unsupported data type");
		return false;
	}

	
	return true;

}