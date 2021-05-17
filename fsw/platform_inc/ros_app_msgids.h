/************************************************************************
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
** File: ros_app_msgids.h
**
** Purpose:
**  Define ros App  Message IDs
**
** Notes:
**
**
*************************************************************************/
#ifndef _ros_app_msgids_h_
#define _ros_app_msgids_h_

#include "cfe_msgids.h"

/* Command Message IDs */
#define ROS_APP_CMD_MID        (CFE_PLATFORM_CMD_MID_BASE + 0x96)
#define ROS_APP_SEND_HK_MID    (CFE_PLATFORM_CMD_MID_BASE + 0x97)

/* Telemetry Message IDs */
#define ROS_APP_HK_TLM_MID     (CFE_PLATFORM_TLM_MID_BASE + 0x97)

#endif /* _ros_app_msgids_h_ */

/************************/
/*  End of File Comment */
/************************/
