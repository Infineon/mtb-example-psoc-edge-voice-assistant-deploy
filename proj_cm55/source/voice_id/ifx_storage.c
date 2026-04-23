/*****************************************************************************
 * File Name        : ifx_storage.c
 *
 * Description      : This source file contains the implementation of storage
 *                    functions for reading and writing embeddings to flash
 * memory using LittleFS.
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

/*******************************************************************************
* Header Files
*******************************************************************************/

#include "ifx_storage.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ifx_voice_id.h"
#include "app_logger.h"

/*******************************************************************************
* Macros
*******************************************************************************/

#define IFX_EMBEDDINGS_FILE_NAME "embeddings"

/*******************************************************************************
* Global Variables
*******************************************************************************/

/* File Systems configurations */
static mtb_serial_memory_t serial_memory_obj;
static cy_stc_smif_mem_context_t smif_mem_context;
static cy_stc_smif_mem_info_t smif_mem_info;
static struct lfs_config lfs_cfg;
lfs_t lfs;


/*******************************************************************************
* Function Name: ifx_storage_init
********************************************************************************
* Summary:
* Initialize flash based storage 
*
* Parameters:
*  None
* 
* Return:
*  Result of storage initialization
*
*******************************************************************************/
cy_rslt_t ifx_storage_init(void) {
    cy_rslt_t result = CY_RSLT_SUCCESS;

   result = mtb_serial_memory_setup(
        &serial_memory_obj,
        MTB_SERIAL_MEMORY_CHIP_SELECT_1,
        CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.base,
        CYBSP_SMIF_CORE_0_XSPI_FLASH_hal_config.clock,
        &smif_mem_context,
        &smif_mem_info,
        &smif0BlockConfig);

    if (CY_RSLT_SUCCESS != result) {
        app_log_print("ERROR: mtb_serial_memory_setup returns error status!\r\n");
    }

    /* Customize the memory location in flash where you want to implement file
     * system block */
    lfs_spi_flash_bd_configure_memory(&lfs_cfg, FS_STORAGE_START_ADDRESS, FS_STORAGE_SIZE);

    /* Create the SPI flash block device */
    result = lfs_spi_flash_bd_create(&lfs_cfg, &serial_memory_obj);

    if (CY_RSLT_SUCCESS != result) {
        app_log_print("ERROR: Creating SPI flash block device failed!\r\n");
    }

    return result;
}


/*******************************************************************************
* Function Name: ifx_storage_read
********************************************************************************
* Summary:
* Read embeddings 
*
* Parameters:
*  *embeddings
* 
* Return:
*  None
*
*******************************************************************************/
void ifx_storage_read(ifx_voice_id_embeddings_t *embeddings) {
    lfs_file_t file;
    uint32_t magic;
    int32_t err;

    if (NULL == embeddings) {
        app_log_print("ERROR: Invalid input parameter!\r\n");
        return; /* cannot set storage_status */
    }
    embeddings->storage_status = IFX_VOICE_ID_SUCCESS;

    /* Mount the filesystem */
    err = lfs_mount(&lfs, &lfs_cfg);

    /* Reformat if we cannot mount the filesystem.
     * This should only happen when littlefs is set up on the storage device for
     * the first time.
     */
    if (err != 0) {
        app_log_print(
            "\nError in mounting. This could be the first time littlefs is "
            "used on the storage device.\n");
        app_log_print("Formatting the block device...\n");

        err = lfs_format(&lfs, &lfs_cfg);
        if (err != 0) {
            app_log_print("ERROR: Formatting failed!\r\n");
            embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_FORMAT;
            return;
        }

        err = lfs_mount(&lfs, &lfs_cfg);
        if (err != 0) {
            app_log_print("ERROR: Mount after format failed!\r\n");
            embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_MOUNT;
            return;
        }
    }

    /* Read the current float array */
    err = lfs_file_open(&lfs, &file, IFX_EMBEDDINGS_FILE_NAME, LFS_O_RDWR | LFS_O_CREAT);
    if (err != 0) {
        app_log_print("ERROR: File open failed!\r\n");
        embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_FILE_OPEN;
        (void)lfs_unmount(&lfs);
        return;
    }

    err = lfs_file_rewind(&lfs, &file);
    if (err != 0) {
        app_log_print("ERROR: File rewind failed!\r\n");
        embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_FILE_REWIND;
        (void)lfs_file_close(&lfs, &file);
        (void)lfs_unmount(&lfs);
        return;
    }

    /* First read and validate the magic number */
    err = lfs_file_read(&lfs, &file, &magic, sizeof(uint32_t));
    if (err < 0) {
        app_log_print("ERROR: Reading magic number failed!\r\n");
        embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_MAGIC;
        (void)lfs_file_close(&lfs, &file);
        (void)lfs_unmount(&lfs);
        return;
    }

    if (magic != IFX_VOICE_ID_MAGIC_NUMBER) {
        app_log_print("\tNo valid data in the flash memory. Skip the reading.\r\n");
        embeddings->user_count = 0U;
        embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_MAGIC;
        (void)lfs_file_close(&lfs, &file);
        (void)lfs_unmount(&lfs);
        return;
    }

    embeddings->magic = magic;

    /* Then read the number of users */
    err = lfs_file_read(&lfs, &file, &embeddings->user_count, sizeof(uint8_t));
    if (err < 0) {
        app_log_print("ERROR: Reading user count failed!\r\n");
        embeddings->user_count = 0U;
        embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_READ_USER_COUNT;
        (void)lfs_file_close(&lfs, &file);
        (void)lfs_unmount(&lfs);
        return;
    }
    if (embeddings->user_count > IFX_MAX_SUPPORTED_USERS) {
        app_log_print("ERROR: Stored user count (%u) exceeds max supported (%u). Treating as empty.\r\n",
               embeddings->user_count, (unsigned)IFX_MAX_SUPPORTED_USERS);
        embeddings->user_count = 0U;
        embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_USER_COUNT_RANGE;
        (void)lfs_file_close(&lfs, &file);
        (void)lfs_unmount(&lfs);
        return;
    }

    /* Then read the embeddings data */
    err = lfs_file_read(&lfs, &file, embeddings->users,
                        sizeof(float) * IFX_EMBEDDINGS_LENGTH_WORDS *
                            IFX_NUM_ENROLLMENT_EMBEDDINGS * IFX_MAX_SUPPORTED_USERS);
    if (err < 0) {
        app_log_print("ERROR: Reading embeddings data failed!\r\n");
        embeddings->user_count = 0U;
        embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_READ_EMBEDDINGS;
        (void)lfs_file_close(&lfs, &file);
        (void)lfs_unmount(&lfs);
        return;
    }

    /* The storage is not updated until the file is closed successfully */
    (void)lfs_file_close(&lfs, &file);

    /* Release any resources we were using */
    (void)lfs_unmount(&lfs);
    embeddings->storage_status = IFX_VOICE_ID_SUCCESS;
    return;
}


/*******************************************************************************
* Function Name: ifx_storage_write
********************************************************************************
* Summary:
* Write embeddings 
*
* Parameters:
*  *embeddings
* 
* Return:
*  None
*
*******************************************************************************/

void ifx_storage_write(ifx_voice_id_embeddings_t *embeddings) {
    lfs_file_t file;
    int32_t err;

    if (NULL == embeddings) {
        app_log_print("ERROR: Invalid input parameter!\r\n");
        return; /* cannot set storage_status */
    }
    embeddings->storage_status = IFX_VOICE_ID_SUCCESS;

    /* Set magic number for validation */
    embeddings->magic = IFX_VOICE_ID_MAGIC_NUMBER;

    /* Mount the filesystem */
    err = lfs_mount(&lfs, &lfs_cfg);

    /* Reformat if we cannot mount the filesystem */
    if (err != 0) {
        app_log_print(
            "\nError in mounting. This could be the first time littlefs is "
            "used on the storage device.\n");
        app_log_print("Formatting the block device...\n");

        err = lfs_format(&lfs, &lfs_cfg);
        if (err != 0) {
            app_log_print("ERROR: Formatting failed!\r\n");
            embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_FORMAT;
            return;
        }

        err = lfs_mount(&lfs, &lfs_cfg);
        if (err != 0) {
            app_log_print("ERROR: Mount after format failed!\r\n");
            embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_MOUNT;
            return;
        }
    }

    /* Open file for writing */
    err = lfs_file_open(&lfs, &file, IFX_EMBEDDINGS_FILE_NAME, LFS_O_RDWR | LFS_O_CREAT);
    if (err != 0) {
        app_log_print("ERROR: File open failed!\r\n");
        embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_FILE_OPEN;
        (void)lfs_unmount(&lfs);
        return;
    }

    err = lfs_file_rewind(&lfs, &file);
    if (err != 0) {
        app_log_print("ERROR: File rewind failed!\r\n");
        embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_FILE_REWIND;
        (void)lfs_file_close(&lfs, &file);
        (void)lfs_unmount(&lfs);
        return;
    }

    /* First write the magic number for validation */
    err = lfs_file_write(&lfs, &file, &embeddings->magic, sizeof(uint32_t));
    if (err < 0) {
        app_log_print("ERROR: Writing magic number failed!\r\n");
        embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_WRITE_MAGIC;
        (void)lfs_file_close(&lfs, &file);
        (void)lfs_unmount(&lfs);
        return;
    }

    /* Then write the number of users */
    err = lfs_file_write(&lfs, &file, &embeddings->user_count, sizeof(uint8_t));
    if (err < 0) {
        app_log_print("ERROR: Writing user count failed!\r\n");
        embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_WRITE_USER_COUNT;
        (void)lfs_file_close(&lfs, &file);
        (void)lfs_unmount(&lfs);
        return;
    }

    /* Then write the embeddings data */
    err = lfs_file_write(&lfs, &file, embeddings->users,
                         sizeof(float) * IFX_EMBEDDINGS_LENGTH_WORDS *
                             IFX_NUM_ENROLLMENT_EMBEDDINGS * IFX_MAX_SUPPORTED_USERS);
    if (err < 0) {
        app_log_print("ERROR: Writing embeddings data failed!\r\n");
        embeddings->storage_status = IFX_VOICE_ID_STORAGE_ERROR_WRITE_EMBEDDINGS;
        (void)lfs_file_close(&lfs, &file);
        (void)lfs_unmount(&lfs);
        return;
    }

    /* The storage is not updated until the file is closed successfully */
    (void)lfs_file_close(&lfs, &file);

    /* Release any resources we were using */
    (void)lfs_unmount(&lfs);
    embeddings->storage_status = IFX_VOICE_ID_SUCCESS;
}

/* [] END OF FILE */
