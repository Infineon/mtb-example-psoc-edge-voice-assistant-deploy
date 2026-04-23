/*****************************************************************************
 * File Name        : ifx_voice_id.h
 *
 * Description      : This header file provides the declarations and definitions
 *                    required for implementing voice identification
 * functionality.
 *
 ********************************************************************************
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

#ifndef _IFX_VOICE_ID_H_
#define _IFX_VOICE_ID_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*******************************************************************************
* Macros
*******************************************************************************/

/* If not defined in Makefile, set default values */
#ifndef IFX_MAX_SUPPORTED_USERS
#define IFX_MAX_SUPPORTED_USERS (6U)
#endif

/* Number of embeddings saved for one user during enrollment.
* The more embeddings saved, the better the model can learn and identify voices.
*/
#ifndef IFX_NUM_ENROLLMENT_EMBEDDINGS
#define IFX_NUM_ENROLLMENT_EMBEDDINGS (5U)
#endif

/* Number of embeddings used for user verification.
 * This value determines how many embeddings are compared with the enrollment
 * embeddings during the verification process. The more embeddings used, the
 * more accurate the verification, but it also increases the processing time.
 */
#ifndef IFX_NUM_VERIFICATION_EMBEDDINGS
#define IFX_NUM_VERIFICATION_EMBEDDINGS (1U)
#endif


#define IFX_VOICE_ID_SIMILARITY_THRESHOLD 0.45f

/*******************************************************************************
 * Status Codes Defines
 ******************************************************************************/

/** Used to return the statuses. See ./core-lib/includecy_result.h */
#ifndef IFX_VOICE_ID
#define IFX_VOICE_ID (0x00870000U)
#endif /* IFX_VOICE_ID */

#ifndef IFX_VOICE_ID_STATUS_ERROR
#define IFX_VOICE_ID_STATUS_ERROR (0x200U)
#endif /* IFX_VOICE_ID_STATUS_ERROR */

/* Status code descriptions */
#define IFX_VOICE_ID_SUCCESS_STR                            "Voice-ID: Success"
#define IFX_VOICE_ID_ERROR_BAD_PARAM_STR                    "Voice-ID: ERROR: Invalid parameter"
#define IFX_VOICE_ID_ERROR_MEMORY_STR                       "Voice-ID: ERROR: Out of memory"
#define IFX_VOICE_ID_ERROR_ML_INIT_STR                      "Voice-ID: ERROR: ML model initialization failed"
#define IFX_VOICE_ID_ERROR_FE_INIT_STR                      "Voice-ID: ERROR: Feature extraction initialization failed"
#define IFX_VOICE_ID_ERROR_UNKNOWN_STR                      "Voice-ID: ERROR: Unknown error"
#define IFX_VOICE_ID_ERROR_USER_IDX_OUT_OF_RANGE_STR        "Voice-ID: ERROR: User index exceeds maximum supported users"
#define IFX_VOICE_ID_ERROR_EMBEDDING_IDX_OUT_OF_RANGE_STR   "Voice-ID: ERROR: Embedding index exceeds maximum enrollment embeddings"
#define IFX_VOICE_ID_ERROR_EMBEDDING_COPY_FAILED_STR        "Voice-ID: ERROR: Failed to copy embedding data"
#define IFX_VOICE_ID_ERROR_ML_MODEL_NULL_STR                "Voice-ID: ERROR: ML model pointer is NULL"
#define IFX_VOICE_ID_ERROR_ML_CONFIG_STR                    "Voice-ID: ERROR: ML model configuration failed"
/* Storage related status descriptions */
#define IFX_VOICE_ID_STORAGE_ERROR_BAD_PARAM_STR            "Voice-ID: ERROR: Storage invalid parameter"
#define IFX_VOICE_ID_STORAGE_ERROR_MOUNT_STR                "Voice-ID: ERROR: Storage mount failed"
#define IFX_VOICE_ID_STORAGE_ERROR_FORMAT_STR               "Voice-ID: ERROR: Storage format failed"
#define IFX_VOICE_ID_STORAGE_ERROR_FILE_OPEN_STR            "Voice-ID: ERROR: Storage file open failed"
#define IFX_VOICE_ID_STORAGE_ERROR_FILE_REWIND_STR          "Voice-ID: ERROR: Storage file rewind failed"
#define IFX_VOICE_ID_STORAGE_ERROR_MAGIC_STR                "Voice-ID: ERROR: Storage magic mismatch"
#define IFX_VOICE_ID_STORAGE_ERROR_READ_USER_COUNT_STR      "Voice-ID: ERROR: Storage read user count failed"
#define IFX_VOICE_ID_STORAGE_ERROR_USER_COUNT_RANGE_STR     "Voice-ID: ERROR: Storage user count out of range"
#define IFX_VOICE_ID_STORAGE_ERROR_READ_EMBEDDINGS_STR      "Voice-ID: ERROR: Storage read embeddings failed"
#define IFX_VOICE_ID_STORAGE_ERROR_WRITE_MAGIC_STR          "Voice-ID: ERROR: Storage write magic failed"
#define IFX_VOICE_ID_STORAGE_ERROR_WRITE_USER_COUNT_STR     "Voice-ID: ERROR: Storage write user count failed"
#define IFX_VOICE_ID_STORAGE_ERROR_WRITE_EMBEDDINGS_STR     "Voice-ID: ERROR: Storage write embeddings failed"

typedef enum
{
    IFX_VOICE_ID_SUCCESS = 0x00U,
    IFX_VOICE_ID_ERROR_BAD_PARAM = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x01U,
    IFX_VOICE_ID_ERROR_MEMORY =  IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x02U,
    IFX_VOICE_ID_ERROR_ML_INIT = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x03U,
    IFX_VOICE_ID_ERROR_FE_INIT = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x04U,
    IFX_VOICE_ID_ERROR_UNKNOWN = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x05U,
    IFX_VOICE_ID_ERROR_USER_IDX_OUT_OF_RANGE = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x06U,
    IFX_VOICE_ID_ERROR_EMBEDDING_IDX_OUT_OF_RANGE = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x07U,
    IFX_VOICE_ID_ERROR_EMBEDDING_COPY_FAILED = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x08U,
    IFX_VOICE_ID_ERROR_ML_MODEL_NULL = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x09U,
    IFX_VOICE_ID_ERROR_ML_CONFIG = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x0AU,
    /* Storage (read/write) related status codes */
    IFX_VOICE_ID_STORAGE_ERROR_BAD_PARAM = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x0BU,
    IFX_VOICE_ID_STORAGE_ERROR_MOUNT = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x0CU,
    IFX_VOICE_ID_STORAGE_ERROR_FORMAT = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x0DU,
    IFX_VOICE_ID_STORAGE_ERROR_FILE_OPEN = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x0EU,
    IFX_VOICE_ID_STORAGE_ERROR_FILE_REWIND = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x0FU,
    IFX_VOICE_ID_STORAGE_ERROR_MAGIC = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x10U,
    IFX_VOICE_ID_STORAGE_ERROR_READ_USER_COUNT = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x11U,
    IFX_VOICE_ID_STORAGE_ERROR_USER_COUNT_RANGE = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x12U,
    IFX_VOICE_ID_STORAGE_ERROR_READ_EMBEDDINGS = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x13U,
    IFX_VOICE_ID_STORAGE_ERROR_WRITE_MAGIC = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x14U,
    IFX_VOICE_ID_STORAGE_ERROR_WRITE_USER_COUNT = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x15U,
    IFX_VOICE_ID_STORAGE_ERROR_WRITE_EMBEDDINGS = IFX_VOICE_ID | IFX_VOICE_ID_STATUS_ERROR | 0x16U,
    IFX_VOICE_ID_LIMIT = 0x17,
    IFX_VOICE_ID_INFERENCE_IN_PROGRESS=0x18,
    IFX_VOICE_ID_INFERENCE_COMPLETE=0x19
} ifx_en_voice_id_status_t;


typedef enum
{
    IFX_VOICE_ID_ENROLL,
    IFX_VOICE_ID_WAIT,
    IFX_VOICE_ID_VERIFY

} ifx_voice_id_mode_t;


/* Size of the embedding in words */
#define IFX_EMBEDDINGS_LENGTH_WORDS (192U)

/* Magic number to validate embeddings data structure */
#define IFX_VOICE_ID_MAGIC_NUMBER (0xE52D9A7BUL)

/* Audio length for one embedding calculation in words */
#define FE_AUDIO_LEN (48240U)


/**
 * \brief Structure to store voice ID embeddings.
 *
 * This structure contains the necessary data for managing voice ID embeddings,
 * including a validation magic number, the count of enrolled users, and the
 * embeddings for each enrolled user.
 */
typedef struct {
    /* Magic number for validation */
    uint32_t magic;
    /* Number of enrolled users */
    uint8_t user_count;
    /* Last storage (read/write) status */
    ifx_en_voice_id_status_t storage_status;
    /* Enrolled users embeddings */
    float users[IFX_MAX_SUPPORTED_USERS][IFX_NUM_ENROLLMENT_EMBEDDINGS]
               [IFX_EMBEDDINGS_LENGTH_WORDS];
} ifx_voice_id_embeddings_t;


/**
 * \brief Initializes the Voice ID system by setting up neural network model and feature extraction.
 *
 * \return ifx_en_voice_id_status_t Result of initialization
 * \retval IFX_VOICE_ID_SUCCESS Successfully initialized voice ID system
 * \retval IFX_VOICE_ID_ERROR_MEMORY Failed to allocate required memory
 * \retval IFX_VOICE_ID_ERROR_ML_INIT Failed to initialize ML model
 * \retval IFX_VOICE_ID_ERROR_FE_INIT Failed to initialize feature extraction
 *
 * \post Memory is allocated for embeddings calculation
 *
 * \note This function must be called before using any other voice ID functions
 */
ifx_en_voice_id_status_t ifx_voice_id_init(void);

/**
 * \brief Calculates similarity scores between an input embedding and all enrolled users.
 *
 * This function calculates the similarity scores between the input embedding and each
 * enrolled user's embeddings, returning the scores for all users.
 *
 * \param[in] embeddings_data Pointer to database containing enrolled users' embeddings
 * \param[in] embedding Pointer to input embedding to match against enrolled users
 * \param[out] scores Array to store similarity scores for each user (must be able to hold IFX_MAX_SUPPORTED_USERS values)
 *
 * \return ifx_en_voice_id_status_t Status of the similarity calculation
 * \retval IFX_VOICE_ID_SUCCESS Successfully calculated all similarity scores
 * \retval IFX_VOICE_ID_ERROR_BAD_PARAM Invalid input parameters
 *
 * \pre embeddings_data must be initialized and contain at least one enrolled user
 * \pre embedding must contain a valid voice embedding
 * \pre scores must be large enough to hold IFX_MAX_SUPPORTED_USERS values
 * \post scores array contains similarity values for each enrolled user
 *
 * \note A higher score indicates a better match
 * \warning All input pointers must be non-NULL
 */
ifx_en_voice_id_status_t ifx_voice_id_get_similarity_scores(ifx_voice_id_embeddings_t* embeddings_data,
                                                         float* embedding,
                                                         float* scores);

/**
 * \brief Generates formatted output of the embeddings database for debugging purposes.
 *
 * \param[in] embeddings Pointer to database containing enrolled users' embeddings
 *
 * \return ifx_en_voice_id_status_t Status of the print operation
 * \retval IFX_VOICE_ID_SUCCESS Successfully printed embeddings
 * \retval IFX_VOICE_ID_ERROR_BAD_PARAM embeddings pointer is NULL
 *
 * \pre embeddings must be a valid pointer to an initialized embeddings database
 */
ifx_en_voice_id_status_t ifx_voice_id_print_embeddings(ifx_voice_id_embeddings_t* embeddings);

/**
 * \brief Resets the embeddings database by removing all enrolled users.
 *
 * \param[in,out] embeddings_data Pointer to embeddings database to be cleared
 *
 * \return ifx_en_voice_id_status_t Status of the clear operation
 * \retval IFX_VOICE_ID_SUCCESS Successfully cleared all enrolled users
 * \retval IFX_VOICE_ID_ERROR_BAD_PARAM embeddings_data pointer is NULL
 *
 * \pre embeddings_data must be a valid pointer to an initialized embeddings database
 * \post user_count is set to 0 and all embeddings are zeroed
 *
 * \warning This operation cannot be undone
 */
ifx_en_voice_id_status_t ifx_voice_id_clear_all_enrolled_users(ifx_voice_id_embeddings_t* embeddings_data);

/**
 * \brief Generates a voice embedding from raw audio input.
 *
 * \param[in] audio Pointer to buffer containing audio samples (16-bit PCM)
 * \param[out] embedding Pointer to buffer where the generated embedding will be stored
 *
 * \return ifx_en_voice_id_status_t Status of the inference operation
 * \retval IFX_VOICE_ID_SUCCESS Successfully generated embedding
 * \retval IFX_VOICE_ID_ERROR_BAD_PARAM Invalid input parameters
 *
 * \pre Audio buffer must contain FE_AUDIO_LEN samples
 * \pre Voice ID system must be initialized via ifx_voice_id_init()
 * \post embedding buffer contains IFX_EMBEDDINGS_LENGTH_WORDS normalized values
 *
 * \warning Both input pointers must be non-NULL
 */
ifx_en_voice_id_status_t ifx_voice_id_run_inference(int16_t* audio, float* embedding);

/**
 * \brief Adds a new voice embedding to the enrolled users database.
 *
 * \param[in,out] embeddings_data Pointer to embeddings database
 * \param[in] new_embedding Pointer to new embedding to be added
 * \param[in] person_idx Index of the person to add the embedding for (0 to IFX_MAX_SUPPORTED_USERS-1)
 * \param[in] embedding_idx Index of the embedding slot to use (0 to IFX_NUM_ENROLLMENT_EMBEDDINGS-1)
 *
 * \return ifx_en_voice_id_status_t Status of the add operation
 * \retval IFX_VOICE_ID_SUCCESS Successfully added embedding
 * \retval IFX_VOICE_ID_ERROR_BAD_PARAM Invalid input parameters
 * \retval IFX_VOICE_ID_ERROR_USER_IDX_OUT_OF_RANGE Person index exceeds maximum
 * \retval IFX_VOICE_ID_ERROR_EMBEDDING_IDX_OUT_OF_RANGE Embedding index exceeds maximum
 * \retval IFX_VOICE_ID_ERROR_EMBEDDING_COPY_FAILED Failed to copy embedding data
 *
 * \pre embeddings_data must be initialized
 * \pre new_embedding must contain a valid voice embedding
 * \post The new embedding is stored in the specified slot
 *
 * \warning Existing embedding at the specified indices will be overwritten
 */
ifx_en_voice_id_status_t ifx_voice_id_add_embedding(ifx_voice_id_embeddings_t* embeddings_data, float* new_embedding,
                                                   uint8_t person_idx, uint8_t embedding_idx);

/**
 * \brief Finds the user with the highest similarity score above a given threshold.
 *
 * This utility function analyzes an array of similarity scores and returns
 * the index of the user with the highest score that exceeds the given threshold.
 *
 * \param[in] scores Array of similarity scores for each user
 * \param[in] user_count Number of valid users in the scores array
 * \param[in] threshold Minimum similarity score required for identification
 *
 * \return int32_t Index of the user with highest score above threshold, or -1 if no user is identified
 *
 * \pre Scores must contain valid similarity scores for all enrolled users
 * \post Returns valid user index (0 to user_count-1) if a user is identified, or -1 if not
 *
 * \note The function handles NaN values in the scores array
 */
int32_t ifx_voice_id_find_max_score(float* scores, uint8_t user_count, float threshold);

/**
 * \brief Gets a human-readable description string for a Voice ID status code.
 *
 * \param[in] status Status code to get the description for
 *
 * \return const char* Pointer to a constant string containing the status description
 * \retval Non-NULL A human-readable description of the status code
 */
const char* ifx_voice_id_get_status_description(ifx_en_voice_id_status_t status);

/**
 * \brief Buffer 10ms of audio packets so that voice id inference can happen when sufficient packets are buffered.
 *
 * \param[in] audio - 10ms audio packets.
 *
 * \return Success/Failure status
 */

ifx_en_voice_id_status_t ifx_voice_id_infer(int16_t * audio, float *embedding);

/**
 * \brief Verify voice id
 *
 * \param[in] embedding - embedding of current inference
 * 
 * \param[in] embeddings_data - stored embeddings data 
 *
 * \return Success/Failure status
 */

int32_t ifx_voice_id_verify(float *embedding, ifx_voice_id_embeddings_t *embeddings_data);

#endif /* _IFX_VOICE_ID_H_ */

/* [] END OF FILE */
