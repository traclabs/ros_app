/*******************************************************************************
**
**      GSC-18128-1, "Core Flight Executive Version 6.7"
**
**      Copyright (c) 2006-2019 United States Government as represented by
**      the Administrator of the National Aeronautics and Space Administration.
**      All Rights Reserved.
**
**      Licensed under the Apache License, Version 2.0 (the "License");
**      you may not use this file except in compliance with the License.
**      You may obtain a copy of the License at
**
**        http://www.apache.org/licenses/LICENSE-2.0
**
**      Unless required by applicable law or agreed to in writing, software
**      distributed under the License is distributed on an "AS IS" BASIS,
**      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**      See the License for the specific language governing permissions and
**      limitations under the License.
**
** File: ros_app.c
**
** Purpose:
**   This file contains the source code for the ros App.
**
*******************************************************************************/

/*
** Include Files:
*/
#include "ros_app_events.h"
#include "ros_app_version.h"
#include "ros_app.h"
#include "ros_app_table.h"

#include <string.h>

/*
** global data
*/
ROS_APP_Data_t ROS_APP_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/* ROS_APP_Main() -- Application entry point and main process loop         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void ROS_APP_Main(void)
{
    int32            status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Register the app with Executive services
    */
    // CFE_ES_RegisterApp();

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(ROS_APP_PERF_ID);

    /*
    ** Perform application specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = ROS_APP_Init();
    if (status != CFE_SUCCESS)
    {
        ROS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** ros Runloop
    */
    while (CFE_ES_RunLoop(&ROS_APP_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(ROS_APP_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, ROS_APP_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(ROS_APP_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            ROS_APP_ProcessCommandPacket(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(ROS_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ROS APP: SB Pipe Read Error, App Will Exit");

            ROS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(ROS_APP_PERF_ID);

    CFE_ES_ExitApp(ROS_APP_Data.RunStatus);

} /* End of ROS_APP_Main() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* ROS_APP_Init() --  initialization                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 ROS_APP_Init(void)
{
    int32 status;

    ROS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app command execution counters
    */
    ROS_APP_Data.CmdCounter = 0;
    ROS_APP_Data.ErrCounter = 0;

    /*
    ** Initialize app configuration data
    */
    ROS_APP_Data.PipeDepth = ROS_APP_PIPE_DEPTH;

    strncpy(ROS_APP_Data.PipeName, "ROS_APP_CMD_PIPE", sizeof(ROS_APP_Data.PipeName));
    ROS_APP_Data.PipeName[sizeof(ROS_APP_Data.PipeName) - 1] = 0;

    /*
    ** Initialize event filter table...
    */
    ROS_APP_Data.EventFilters[0].EventID = ROS_APP_STARTUP_INF_EID;
    ROS_APP_Data.EventFilters[0].Mask    = 0x0000;
    ROS_APP_Data.EventFilters[1].EventID = ROS_APP_COMMAND_ERR_EID;
    ROS_APP_Data.EventFilters[1].Mask    = 0x0000;
    ROS_APP_Data.EventFilters[2].EventID = ROS_APP_COMMANDNOP_INF_EID;
    ROS_APP_Data.EventFilters[2].Mask    = 0x0000;
    ROS_APP_Data.EventFilters[3].EventID = ROS_APP_COMMANDRST_INF_EID;
    ROS_APP_Data.EventFilters[3].Mask    = 0x0000;
    ROS_APP_Data.EventFilters[4].EventID = ROS_APP_INVALID_MSGID_ERR_EID;
    ROS_APP_Data.EventFilters[4].Mask    = 0x0000;
    ROS_APP_Data.EventFilters[5].EventID = ROS_APP_LEN_ERR_EID;
    ROS_APP_Data.EventFilters[5].Mask    = 0x0000;
    ROS_APP_Data.EventFilters[6].EventID = ROS_APP_PIPE_ERR_EID;
    ROS_APP_Data.EventFilters[6].Mask    = 0x0000;
    ROS_APP_Data.EventFilters[7].EventID = ROS_APP_HELLO_WORLD_INF_EID;
    ROS_APP_Data.EventFilters[7].Mask    = 0x0000;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(ROS_APP_Data.EventFilters, ROS_APP_EVENT_COUNTS, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ros App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        return (status);
    }

    /*
    ** Initialize housekeeping packet (clear user data area).
    */
    CFE_MSG_Init(&ROS_APP_Data.HkTlm.TlmHeader.Msg, CFE_SB_ValueToMsgId(ROS_APP_HK_TLM_MID), sizeof(ROS_APP_Data.HkTlm));

    /*
    ** Create Software Bus message pipe.
    */
    status = CFE_SB_CreatePipe(&ROS_APP_Data.CommandPipe, ROS_APP_Data.PipeDepth, ROS_APP_Data.PipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ros App: Error creating pipe, RC = 0x%08lX\n", (unsigned long)status);
        return (status);
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ROS_APP_SEND_HK_MID), ROS_APP_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ros App: Error Subscribing to HK request, RC = 0x%08lX\n", (unsigned long)status);
        return (status);
    }

    /*
    ** Subscribe to ground command packets
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ROS_APP_CMD_MID), ROS_APP_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ros App: Error Subscribing to Command, RC = 0x%08lX\n", (unsigned long)status);

        return (status);
    }

    /*
    ** Subscribe to the /rosout telemetry packets.
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ROS_APP_ROSOUT_INFO_MID), ROS_APP_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ros App: Error Subscribing to /rosout INFO topic, RC = 0x%08lX\n", (unsigned long)status);

        return (status);
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ROS_APP_ROSOUT_ERROR_MID), ROS_APP_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ros App: Error Subscribing to /rosout ERROR topic, RC = 0x%08lX\n", (unsigned long)status);

        return (status);
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ROS_APP_ROSOUT_FATAL_MID), ROS_APP_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ros App: Error Subscribing to /rosout FATAL topic, RC = 0x%08lX\n", (unsigned long)status);

        return (status);
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ROS_APP_ROSOUT_WARN_MID), ROS_APP_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ros App: Error Subscribing to /rosout WARN topic, RC = 0x%08lX\n", (unsigned long)status);

        return (status);
    }


    /*
    ** Register Table(s)
    */
    status = CFE_TBL_Register(&ROS_APP_Data.TblHandles[0], "RosAppTable", sizeof(ROS_APP_Table_t),
                              CFE_TBL_OPT_DEFAULT, ROS_APP_TblValidationFunc);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Ros App: Error Registering Table, RC = 0x%08lX\n", (unsigned long)status);

        return (status);
    }
    else
    {
        status = CFE_TBL_Load(ROS_APP_Data.TblHandles[0], CFE_TBL_SRC_FILE, ROS_APP_TABLE_FILE);
    }

    CFE_EVS_SendEvent(ROS_APP_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION, "ros App Initialized.%s",
                      ROS_APP_VERSION_STRING);

    return (CFE_SUCCESS);

} /* End of ROS_APP_Init() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  ROS_APP_ProcessCommandPacket                                    */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the ros    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void ROS_APP_ProcessCommandPacket(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case ROS_APP_CMD_MID:
            ROS_APP_ProcessGroundCommand(SBBufPtr);
            break;

        case ROS_APP_SEND_HK_MID:
            ROS_APP_ReportHousekeeping((CFE_MSG_CommandHeader_t *)SBBufPtr);
            break;

        case ROS_APP_ROSOUT_INFO_MID:
            ROS_APP_ReportRosoutMsg((ROS_APP_RosoutTlm_t *) SBBufPtr);
            break;
        case ROS_APP_ROSOUT_WARN_MID:
            ROS_APP_ReportRosoutMsg((ROS_APP_RosoutTlm_t *) SBBufPtr);
            break;
        case ROS_APP_ROSOUT_ERROR_MID:
            ROS_APP_ReportRosoutMsg((ROS_APP_RosoutTlm_t *) SBBufPtr);
            break;
        case ROS_APP_ROSOUT_FATAL_MID:
            ROS_APP_ReportRosoutMsg((ROS_APP_RosoutTlm_t *) SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(ROS_APP_INVALID_MSGID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ros: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }

    return;

} /* End ROS_APP_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* ROS_APP_ProcessGroundCommand() -- ros ground commands                */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void ROS_APP_ProcessGroundCommand(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process "known" ros app ground commands
    */
    switch (CommandCode)
    {
        case ROS_APP_NOOP_CC:
            if (ROS_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ROS_APP_NoopCmd_t)))
            {
                ROS_APP_Noop((ROS_APP_NoopCmd_t *)SBBufPtr);
            }

            break;

        case ROS_APP_RESET_COUNTERS_CC:
            if (ROS_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ROS_APP_ResetCountersCmd_t)))
            {
                ROS_APP_ResetCounters((ROS_APP_ResetCountersCmd_t *)SBBufPtr);
            }

            break;

        case ROS_APP_PROCESS_CC:
            if (ROS_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ROS_APP_ProcessCmd_t)))
            {
                ROS_APP_Process((ROS_APP_ProcessCmd_t *)SBBufPtr);
            }

            break;

        case ROS_APP_HELLO_WORLD_CC:
            if (ROS_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ROS_APP_ResetCountersCmd_t)))
            {
                ROS_APP_HelloCmd((ROS_APP_NoopCmd_t *)SBBufPtr);
            }

            break;

        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(ROS_APP_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid ground command code: CC = %d", CommandCode);
            break;
    }

    return;

} /* End of ROS_APP_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  ROS_APP_ReportHousekeeping                                          */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 ROS_APP_ReportHousekeeping(const CFE_MSG_CommandHeader_t *Msg)
{
  int i;
  
    /*
    ** Get command execution counters...
    */
    ROS_APP_Data.HkTlm.Payload.CommandErrorCounter = ROS_APP_Data.ErrCounter;
    ROS_APP_Data.HkTlm.Payload.CommandCounter      = ROS_APP_Data.CmdCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(&ROS_APP_Data.HkTlm.TlmHeader.Msg);
    CFE_SB_TransmitMsg(&ROS_APP_Data.HkTlm.TlmHeader.Msg, true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < ROS_APP_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(ROS_APP_Data.TblHandles[i]);
    }

    return CFE_SUCCESS;

} /* End of ROS_APP_ReportHousekeeping() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  ROS_APP_ReportRosoutMsg                                            */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a /rosout telemetry msg  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 ROS_APP_ReportRosoutMsg(const ROS_APP_RosoutTlm_t *Msg)
{
#if 0  /* 0 for no output, 1 if you want to see the /rosout message */
   CFE_TIME_SysTime_t msg_time;
   char time_text_buffer[2048];


   CFE_MSG_GetMsgTime((const CFE_MSG_Message_t *) Msg, &msg_time);
   CFE_TIME_Print(time_text_buffer, msg_time);

    printf("\n/rosout contains:"
                      "\n\tsecondary header time = %s, "
                      "\n\tsecondary header sec = %lu "
                      "\n\tsecondary header subsec = %lu "
                      "\n\tsec=%lu, "
                      "\n\tnsec=%lu, "
                      "\n\tlevel=%d, "
                      "\n\tname=%s, " 
                      "\n\tmsg=%s, "
                      "\n\tfile=%s, "
                      "\n\tfunction=%s, "
                      "\n\tline=%lu" 
                      "\n\n",
                      time_text_buffer,
                      (unsigned long) msg_time.Seconds,
                      (unsigned long) msg_time.Subseconds,
                      (unsigned long) Msg->Payload.sec, 
                      (unsigned long) Msg->Payload.nsec, 
                      Msg->Payload.level, 
                      Msg->Payload.name, 
                      Msg->Payload.msg, 
                      Msg->Payload.file, 
                      Msg->Payload.function, 
                      (unsigned long) Msg->Payload.line);
#endif

    return CFE_SUCCESS;

} /* End of ROS_APP_ReportRosoutMsg() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* ROS_APP_Noop -- ROS NOOP commands                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 ROS_APP_Noop(const ROS_APP_NoopCmd_t *Msg)
{

    ROS_APP_Data.CmdCounter++;

    CFE_EVS_SendEvent(ROS_APP_COMMANDNOP_INF_EID, CFE_EVS_EventType_INFORMATION, "ros: NOOP command %s",
                      ROS_APP_VERSION);

    return CFE_SUCCESS;

} /* End of ROS_APP_Noop */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* ROS_APP_HelloCmd -- ros NOOP commands                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void ROS_APP_HelloCmd(const ROS_APP_NoopCmd_t *Msg)
{
    CFE_TIME_SysTime_t current_met;

    ROS_APP_Data.CmdCounter++;

    current_met = CFE_TIME_GetMET();

    CFE_EVS_SendEvent(ROS_APP_HELLO_WORLD_INF_EID, CFE_EVS_EventType_INFORMATION, "ros: Hello, ros! %s.  Time is %lu sec and %lu subsec",
                      ROS_APP_VERSION,
                      (unsigned long) current_met.Seconds, 
                      (unsigned long) current_met.Subseconds);

} /* End of ROS_APP_HelloCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  ROS_APP_ResetCounters                                               */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 ROS_APP_ResetCounters(const ROS_APP_ResetCountersCmd_t *Msg)
{

    ROS_APP_Data.CmdCounter = 0;
    ROS_APP_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(ROS_APP_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION, "ros: RESET command");

    return CFE_SUCCESS;

} /* End of ROS_APP_ResetCounters() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* ROS_APP_VerifyCmdLength() -- Verify command packet length                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool ROS_APP_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(ROS_APP_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        ROS_APP_Data.ErrCounter++;
    }

    return (result);

} /* End of ROS_APP_VerifyCmdLength() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* ROS_APP_GetCrc -- Output CRC                                     */
/*                                                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ROS_APP_GetCrc(const char *TableName)
{
    int32          status;
    uint32         Crc;
    CFE_TBL_Info_t TblInfoPtr;

    status = CFE_TBL_GetInfo(&TblInfoPtr, TableName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Ros App: Error Getting Table Info");
    }
    else
    {
        Crc = TblInfoPtr.Crc;
        CFE_ES_WriteToSysLog("Ros App: CRC: 0x%08lX\n\n", (unsigned long)Crc);
    }

    return;

} /* End of ROS_APP_GetCrc */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* ROS_APP_TblValidationFunc -- Verify contents of First Table      */
/* buffer contents                                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 ROS_APP_TblValidationFunc(void *TblData)
{
    int32               ReturnCode = CFE_SUCCESS;
    ROS_APP_Table_t *TblDataPtr = (ROS_APP_Table_t *)TblData;

    /*
    ** Ros Table Validation
    */
    if (TblDataPtr->Int1 > ROS_APP_TBL_ELEMENT_1_MAX)
    {
        /* First element is out of range, return an appropriate error code */
        ReturnCode = ROS_APP_TABLE_OUT_OF_RANGE_ERR_CODE;
    }

    return ReturnCode;

} /* End of ROS_APP_TBLValidationFunc() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  ROS_APP_Process                                                     */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function Process Ground Station Command                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 ROS_APP_Process(const ROS_APP_ProcessCmd_t *Msg)
{
    int32               status;
    ROS_APP_Table_t *TblPtr;
    const char *        TableName = "ROS_APP.RosAppTable";

    /* Ros Use of Table */

    status = CFE_TBL_GetAddress((void *)&TblPtr, ROS_APP_Data.TblHandles[0]);

    if (status < CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Ros App: Fail to get table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    CFE_ES_WriteToSysLog("Ros App: Table Value 1: %d  Value 2: %d", TblPtr->Int1, TblPtr->Int2);

    ROS_APP_GetCrc(TableName);

    status = CFE_TBL_ReleaseAddress(ROS_APP_Data.TblHandles[0]);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Ros App: Fail to release table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;

} /* End of ROS_APP_ProcessCC */