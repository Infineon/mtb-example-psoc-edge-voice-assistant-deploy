/*****************************************************************************
 * File Name        : ifx_storage.h
 *
 * Description      : This header file contains the declarations and
 * configurations for storage functions used to read and write embeddings to
 * flash memory using LittleFS.
 *
 *******************************************************************************
 * (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
 * Technologies AG. All rights reserved.
 * This software, associated documentation and materials ("Software") is
 * owned by Infineon Technologies AG or one of its affiliates ("Infineon")
 * and is protected by and subject to worldwide patent protection, worldwide
 * copyright laws, and international treaty provisions. Therefore, you may use
 * this Software only as provided in the license agreement accompanying the
 * software package from which you obtained this Software. If no license
 * agreement applies, then any use, reproduction, modification, translation, or
 * compilation of this Software is prohibited without the express written
 * permission of Infineon.
 *
 * Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
 * IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
 * THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
 * SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
 * Infineon reserves the right to make changes to the Software without notice.
 * You are responsible for properly designing, programming, and testing the
 * functionality and safety of your intended application of the Software, as
 * well as complying with any legal requirements related to its use. Infineon
 * does not guarantee that the Software will be free from intrusion, data theft
 * or loss, or other breaches ("Security Breaches"), and Infineon shall have
 * no liability arising out of any Security Breaches. Unless otherwise
 * explicitly approved by Infineon, the Software may not be used in any
 * application where a failure of the Product or any consequences of the use
 * thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

#ifndef _IFX_STORAGE_H_
#define _IFX_STORAGE_H_

#include <stdint.h>

#include "ifx_voice_id.h"
#include "cybsp.h"

#include "lfs.h"
#include "lfs_sd_bd.h"
#include "lfs_spi_flash_bd.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define FS_STORAGE_START_ADDRESS (0xA00000UL)
#define FS_STORAGE_SIZE (0x400000UL)

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/**
 * \brief Initializes the storage system for voice embeddings
 *
 * \return Returns the initialization status
 * \retval CY_RSLT_SUCCESS Storage system initialized successfully
 * \retval CY_RSLT_TYPE_ERROR Storage system initialization failed
 *
 * \post Storage system is initialized and ready for read/write operations
 *
 * \note This function must be called before any other storage operations
 * \warning Ensure proper power supply during initialization to prevent corruption
 */
cy_rslt_t ifx_storage_init(void);

/**
 * \brief Writes voice embeddings to the storage system
 *
 * \param[in] embeddings Pointer to the voice embeddings structure to be stored
 *
 * \pre Storage system must be initialized using ifx_storage_init()
 * \post Voice embeddings are persistently stored in flash memory
 *
 * \note The function will overwrite existing embeddings with the same ID
 * \warning Do not power off the device during write operations
 */
void ifx_storage_write(ifx_voice_id_embeddings_t *embeddings);

/**
 * \brief Reads voice embeddings from the storage system
 *
 * \param[out] embeddings Pointer to the structure where embeddings will be stored
 *
 * \pre Storage system must be initialized using ifx_storage_init()
 * \post Embeddings structure is populated with data from storage
 *
 * \note Returns empty embeddings if no data is found
 * \warning Ensure sufficient memory is allocated in the embeddings structure
 */
void ifx_storage_read(ifx_voice_id_embeddings_t *embeddings);

#endif /* _IFX_STORAGE_H_ */

/* [] END OF FILE */
