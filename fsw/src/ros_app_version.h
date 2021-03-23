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
*************************************************************************/

/*! @file ros_app_version.h
 * @brief Purpose:
 *
 *  The ros App header file containing version information
 *
 */

#ifndef ROS_APP_VERSION_H
#define ROS_APP_VERSION_H

/* Development Build Macro Definitions */

#define ROS_APP_BUILD_NUMBER 48 /*!< Development Build: Number of commits since baseline */
#define ROS_APP_BUILD_BASELINE \
    "v1.2.0-rc1" /*!< Development Build: git tag that is the base for the current development */

/* Version Macro Definitions */

#define ROS_APP_MAJOR_VERSION 1  /*!< @brief ONLY APPLY for OFFICIAL releases. Major version number. */
#define ROS_APP_MINOR_VERSION 1  /*!< @brief ONLY APPLY for OFFICIAL releases. Minor version number. */
#define ROS_APP_REVISION      99 /*!< @brief ONLY APPLY for OFFICIAL releases. Revision version number. */
#define ROS_APP_MISSION_REV   0  /*!< @brief ONLY USED by MISSION Implementations. Mission revision */

#define ROS_APP_STR_HELPER(x) #x /*!< @brief Helper function to concatenate strings from integer macros */
#define ROS_APP_STR(x) \
    ROS_APP_STR_HELPER(x) /*!< @brief Helper function to concatenate strings from integer macros */

/*! @brief Development Build Version Number.
 * @details Baseline git tag + Number of commits since baseline. @n
 * See @ref cfsversions for format differences between development and release versions.
 */
#define ROS_APP_VERSION ROS_APP_BUILD_BASELINE "+dev" ROS_APP_STR(ROS_APP_BUILD_NUMBER)

/*! @brief Development Build Version String.
 * @details Reports the current development build's baseline, number, and name. Also includes a note about the latest
 * official version. @n See @ref cfsversions for format differences between development and release versions.
 */
#define ROS_APP_VERSION_STRING                       \
    " ros App DEVELOPMENT BUILD " ROS_APP_VERSION \
    ", Last Official Release: v1.1.0" /* For full support please use this version */

#endif /* ROS_APP_VERSION_H */

/************************/
/*  End of File Comment */
/************************/
